
#include "rpi.h"
#include "fat32.h"
#include "fat32-helpers.h"
#include "pi-sd.h"
#include "../../libpi/rpi.h"
#include "../constants.h"
#include "../world-state/world.h"
#include "../boot/world-gen.h"
#include "../world-state/pending.h"
#include "../world-state/hashtable.h"
#include "../world-state/player.h"
#include "boot_server.h"
#include "../pi-side/uart-helpers.h"
#include "../heap/allocator.h"


char * filename0 = "SERVER0.BIN";
char * filename1 = "SERVER1.BIN";

file_t * save_game(world_t * world, player_t * player, uint32_t * out_size) {
    trace("saving game\n");
    file_t * file_info = mymalloc(sizeof(file_t));
    memset(file_info, 0, sizeof(file_t));
    trace("allocated file_info\n");

    file_info->player = *player;
    file_info->info = *world->info;
    /* Only save edits - pending_add is stubbed and pending is always empty */
    file_info->pending_size = 0;

    uint32_t count = 0;
    trace("reached here\n");
    for (uint32_t i = 0; i < world->edits.cap; i++) {
        world_entry_t *curr = world->edits.entries[i];
        
        while (curr != NULL) {
            trace("setting entry %d\n", count);
            if (count >= MAX_EDITS) {
                trace("Warning: Too many edits to save, truncating!");
                break; 
            }

            file_info->edit_blocks[count].block = curr->block;
            file_info->edit_blocks[count].pos   = curr->pos;
            // file_info->edit_blocks[count].full  = curr->full;
            
            count++;
            curr = curr->next;
        }
        
        if (count >= MAX_EDITS) break;
    }
    trace("reached past edits\n");
    file_info->edits_size = count;

    *out_size = sizeof(file_t);
    return file_info;
}

void load_game(file_t *file_info, struct world *world, player_t *player) {

    *player = file_info->player;
    
    if (world->info) {
        memcpy((void*)world->info, &file_info->info, sizeof(world_info_t));
    }

    world->pending.size = 0;
    world->pending.cap = MAX_PENDING;
    // trace("currently here\n");
    for (uint32_t i = 0; i < file_info->pending_size; i++) {
        flat_entry_t *f = &file_info->pending_blocks[i];
        
        world_entry_t entry = {
            .block = f->block,
            .pos = f->pos,
            // .full = f->full,
            .next = NULL
        };

        if (!pending_add(&world->pending, entry)) {
            trace("Failed to restore entry to pending table during load");
        }
    }

    // trace("currently here now\n");
    for (uint32_t i = 0; i < world->edits.cap; i++) {
        world->edits.entries[i] = NULL;
    }
    world->edits.size = 0;
    // world->edits.cap = MAX_EDITS;

    // trace("currently hereee\n");
    for (uint32_t i = 0; i < file_info->edits_size; i++) {
        pos_t pos = file_info->edit_blocks[i].pos;
        block_t block = file_info->edit_blocks[i].block;
        /* Skip invalid entries (corruption or padding) */
        if (!world_pos_is_valid(pos) || block > BLOCK_WATER) {
            trace("skipping invalid edit %d: pos=(%d,%d,%d) block=%d\n",
                  i, (int)pos.x, (int)pos.y, (int)pos.z, (int)block);
            continue;
        }
        trace("setting entry %d\n", i);
        table_set_entry(&world->edits, block, pos);
        uart_put8('\r');
        uart_put_str("BLOCK");
        uart_put8(' ');
        uart_put_int((int)pos.x);
        uart_put8(' ');
        uart_put_int((int)pos.y);
        uart_put8(' ');
        uart_put_int((int)pos.z);
        uart_put8(' ');
        uart_put_int(block);
        uart_put8('\n');
        trace("sent BLOCK %d %d %d %d\n", (int)pos.x, (int)pos.y, (int)pos.z, (int)block);
    }
    uart_put8('\r');
    uart_put_str("PLAYER ");
    uart_put_int(player->position.x);
    uart_put_str(" ");
    uart_put_int(player->position.y);
    uart_put_str(" ");
    uart_put_int(player->position.z);
    uart_put_str("\n");
    trace("sent PLAYER %d %d %d\n", (int)player->position.x, (int)player->position.y, (int)player->position.z);
}

fat32_fs_t initialize_fs(pi_dirent_t * directory) {
    kmalloc_init_mb(FAT32_HEAP_MB);
    pi_sd_init();
    /* Give SD card time to be ready - critical when run immediately after delete.bin */
    delay_ms(200);

#ifndef TRACE_OFF
    printk("Reading the MBR.\n");
#endif
    mbr_t *mbr = mbr_read();

#ifndef TRACE_OFF
    printk("Loading the first partition.\n");
#endif
    mbr_partition_ent_t partition;
    memcpy(&partition, mbr->part_tab1, sizeof(mbr_partition_ent_t));
    assert(mbr_part_is_fat32(partition.part_type));

#ifndef TRACE_OFF
    printk("Loading the FAT.\n");
#endif
    fat32_fs_t fs = fat32_mk(&partition);

#ifndef TRACE_OFF
    printk("Loading the root directory.\n");
#endif
    pi_dirent_t root = fat32_get_root(&fs);
    *directory = root;
    return fs;
}

int create_boot_file(uint32_t seed, pi_dirent_t * root, fat32_fs_t * fs) {
    char * filename = "";
    if (seed == 0) {
        filename = filename0;
    }
    if (seed == 1) {
        filename = filename1;
    }

    pi_dirent_t * boot_file = fat32_stat(fs, root, filename);
    if (boot_file != NULL) {
#ifndef TRACE_OFF
        printk("File already exists\n");
#endif
        return 0;
    }

    world_info_t info = {
        .seed = seed,
        .min = (pos_t){0, -60, 0},
        .max = (pos_t){16, -44, 16},
        .edits_cap = 2048,
        .pending_cap = 1024,
    };

    world_t* w = world_create(&info);
    if (!w) {
        panic("Failed to create world");
        return 0;
    }

    if (seed != 0) {
        write_flat_mtn_world(seed, w);
    }

    player_t player = {.player_id = 0,
        .position = (pos_t) {0, -60, 0},
        .rotation = (p_rot_t) {0, 0}
    };

    assert(fat32_create(fs, root, filename, 0));

    /* SD card needs time to settle after create; avoids hang on delete-then-remake */
    delay_ms(300);

    uint32_t out_size = 0;
    file_t * result = save_game(w, &player, &out_size);

    pi_file_t file_save = {
        .data = (char *)result,
        .n_data = out_size,
        .n_alloc = out_size
    };
    trace("about to save\n");
    delay_ms(100);  /* extra settle before write - first run after delete is flaky */
    assert(fat32_write(fs, root, filename, &file_save));
    trace("saved game\n");
    myfree(result);
    return 1;
}

int get_current_state(uint32_t seed,  pi_dirent_t * root, fat32_fs_t * fs, world_t * world, player_t * player) {
  char * filename = "";
    if (seed == 0) {
        filename = filename0;
    }
    if (seed == 1) {
        filename = filename1;
    }
    create_boot_file(seed, root, fs);
    pi_file_t *new_file = fat32_read(fs, root, filename);
    trace("finished reading file");
    load_game((file_t *)new_file->data, world, player);
    return 1;
}

int save_current_state(world_t * world, player_t * player, uint32_t seed, pi_dirent_t * root, fat32_fs_t * fs) {
    char * filename = "";
    if (seed == 0) {
        filename = filename0;
    }
    if (seed == 1) {
        filename = filename1;
    }

    // mymalloc(FAT32_HEAP_MB);
    pi_dirent_t * boot_file = fat32_stat(fs, root, filename);
    if (boot_file == NULL) {
#ifndef TRACE_OFF
        printk("File doesn't exist\n");
#endif
        return 0;
    }
#ifndef TRACE_OFF
    printk("Creating BOOT_SERVER.bin\n");
#endif
    uint32_t out_size = 0;
    file_t * result = save_game(world, player, &out_size);
    pi_file_t file_save = {
        .data = (char *)result,
        .n_data = out_size,
        .n_alloc = out_size
    };
    trace("about to save");
    fat32_write(fs, root, filename, &file_save);
    myfree(result);
    return 1;
}

int delete_boot_file(uint32_t seed, pi_dirent_t * root, fat32_fs_t * fs) {
    char * filename = "";
    if (seed == 0) {
        filename = filename0;
    }
    if (seed == 1) {
        filename = filename1;
    }

    // kmalloc_init_mb(FAT32_HEAP_MB);
    pi_dirent_t * boot_file = fat32_stat(fs, root, filename);
    if (boot_file == NULL) {
        return 0;
    }
    fat32_delete(fs, root, filename);
    return 1;
}