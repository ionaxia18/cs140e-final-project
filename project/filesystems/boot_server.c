
#include "rpi.h"
#include "fat32.h"
#include "fat32-helpers.h"
#include "pi-sd.h"
#include "../../libpi/rpi.h"
#include "../constants.h"
#include "../world-state/world.h"
#include "../world-state/world-gen.h"
#include "../world-state/pending.h"
#include "../world-state/hashtable.h"
#include "../world-state/player.h"
#include "boot_server.h"

char * filename = "SERVER.BIN";

int create_boot_file(void) {
    kmalloc_init_mb(FAT32_HEAP_MB);
    pi_sd_init();

    printk("Reading the MBR.\n");
    mbr_t *mbr = mbr_read();

    printk("Loading the first partition.\n");
    mbr_partition_ent_t partition;
    memcpy(&partition, mbr->part_tab1, sizeof(mbr_partition_ent_t));
    assert(mbr_part_is_fat32(partition.part_type));

    printk("Loading the FAT.\n");
    fat32_fs_t fs = fat32_mk(&partition);

    printk("Loading the root directory.\n");
    pi_dirent_t root = fat32_get_root(&fs);

    pi_dirent_t * boot_file = fat32_stat(&fs, &root, filename);
    if (boot_file != NULL) {
        printk("File already exists\n");
        return 0;
    }

    world_info_t info = {
        .seed = 0,
        .min = (pos_t){0, -60, 0},
        .max = (pos_t){16, -44, 16},
        .edits_cap = 4096,
        .pending_cap = 1024,
    };

    world_t* w = world_create(&info);
    if (!w) {
        panic("Failed to create world");
        return 0;
    }

    player_t player = {.player_id = 0,
        .position = (pos_t) {0, -60, 0},
        .rotation = (p_rot_t) {0, 0}
    };

    printk("Creating BOOT_SERVER.bin\n");
    assert(fat32_create(&fs, &root, filename, 0));
    file_t * file_data = kmalloc(sizeof(file_t));
    file_data->world = *w;     
    file_data->player = player;

    pi_file_t file_save = {
        .data = (char *)file_data,
        .n_data = sizeof(file_t),
        .n_alloc = sizeof(file_t)

    };
    assert(fat32_write(&fs, &root, filename, &file_save));
    return 1;
}

file_t *get_current_state(void) {
     kmalloc_init_mb(FAT32_HEAP_MB);
  pi_sd_init();

  printk("Reading the MBR.\n");
  mbr_t *mbr = mbr_read();

  printk("Loading the first partition.\n");
  mbr_partition_ent_t partition;
  memcpy(&partition, mbr->part_tab1, sizeof(mbr_partition_ent_t));
  assert(mbr_part_is_fat32(partition.part_type));

  printk("Loading the FAT.\n");
  fat32_fs_t fs = fat32_mk(&partition);

  printk("Loading the root directory.\n");
  pi_dirent_t root = fat32_get_root(&fs);
  pi_dirent_t *boot_file = fat32_stat(&fs, &root, filename);
  if (boot_file == NULL) {
    return NULL;
  }
  pi_file_t *new_file = fat32_read(&fs, &root, filename);
  file_t *state = kmalloc(sizeof(file_t));
  state = (file_t *)new_file->data;
  return state;
}

int save_current_state(file_t *file_info) {
    kmalloc_init_mb(FAT32_HEAP_MB);
    pi_sd_init();

    printk("Reading the MBR.\n");
    mbr_t *mbr = mbr_read();

    printk("Loading the first partition.\n");
    mbr_partition_ent_t partition;
    memcpy(&partition, mbr->part_tab1, sizeof(mbr_partition_ent_t));
    assert(mbr_part_is_fat32(partition.part_type));

    printk("Loading the FAT.\n");
    fat32_fs_t fs = fat32_mk(&partition);

    printk("Loading the root directory.\n");
    pi_dirent_t root = fat32_get_root(&fs);

    pi_dirent_t * boot_file = fat32_stat(&fs, &root, filename);
    if (boot_file == NULL) {
        printk("File doesn't exist\n");
        return 0;
    }
    printk("Creating BOOT_SERVER.bin\n");
    pi_file_t file_save = {
        .data = (char *)file_info,
        .n_data = sizeof(file_t),
        .n_alloc = sizeof(file_t)
    };
    fat32_write(&fs, &root, filename, &file_save);
    return 1;
}

int delete_boot_file(void) {
    kmalloc_init_mb(FAT32_HEAP_MB);
    pi_sd_init();

    printk("Reading the MBR.\n");
    mbr_t *mbr = mbr_read();

    printk("Loading the first partition.\n");
    mbr_partition_ent_t partition;
    memcpy(&partition, mbr->part_tab1, sizeof(mbr_partition_ent_t));
    assert(mbr_part_is_fat32(partition.part_type));

    printk("Loading the FAT.\n");
    fat32_fs_t fs = fat32_mk(&partition);

    printk("Loading the root directory.\n");
    pi_dirent_t root = fat32_get_root(&fs);

    pi_dirent_t * boot_file = fat32_stat(&fs, &root, filename);
    if (boot_file == NULL) {
        return 0;
    }
    fat32_delete(&fs, &root, filename);
    return 1;
}