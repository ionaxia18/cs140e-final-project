#include "rpi.h"
#include "flatworld.h"



// fill in table with all grass positions 
void fill_table(world_t* w) {
    world_entry_t entry;
    entry.block = BLOCK_GRASS;
    entry.empty = false;

    for (int x = w->info->min.x; x < w->info->max.x; x++) {
        for (int z = w->info->min.z; z < w->info->max.z; z++) {
            world_pos_t p = {x, -60, z};
            uint32_t index = block_hash_index(w, p);
            trace("Index for pos %d, %d, %d is %d\n", x, -60, z, index);
            w->edits.entries[index] = entry;
            w->edits.entries[index].pos = p;
            w->edits.size++;
        }
    }
}   

void notmain(void) {
    // create sample config for world
    world_info_t info = {
        .seed = 0,
        .min = (world_pos_t){-65, -65, -65},
        .max = (world_pos_t){-59, -59, -59},
        .edits_cap = 256,
        .pending_cap = 256,
    };

    world_t* w = world_create(&info);


    if (!w) {
        panic("Failed to create world");
    }

    fill_table(w);

    world_entry_t entry;

    for (int x = w->info->min.x; x < w->info->max.x; x++) {
        for (int y = w->info->min.y; y < w->info->max.y; y++) {
            for (int z = w->info->min.z; z < w->info->max.z; z++) {
                world_pos_t p = {x, y, z};
                uint32_t index = block_hash_index(w, p);
                if (y != -60) {
                    assert(world_get_entry(w, &entry, p) == false);
                } else {
                    assert(world_get_entry(w, &entry, p) == true && entry.block == BLOCK_GRASS);
                }
            }
           
        }
    }
}