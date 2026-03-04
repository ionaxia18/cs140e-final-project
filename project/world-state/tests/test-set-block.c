#include "rpi.h"
#include "world.h"
#include "hashtable.h"

world_entry_t expected_entries[1024];
uint32_t count = 0;

void fill_table(world_t* w) {
    for (int x = w->info->min.x; x < w->info->max.x; x += 2) {
        for (int y = w->info->min.y; y < w->info->max.y; y += 2) {
            for (int z = w->info->min.z; z < w->info->max.z; z += 2) {
                world_pos_t p = {x, y, z};
                uint32_t index = table_empty_index(expected_entries, w->edits.cap, p);
                expected_entries[index] = (world_entry_t){BLOCK_GRASS, p, true};
                count++;
            }
        }
    }
    trace("Count: %d\n", count);
}


void compare_world(world_t* w) {
    for (int i = 0; i < w->edits.cap; i++) {
        if (expected_entries[i].full != w->edits.entries[i].full) {
            trace("Fullness differs at index %d\n", i);
            assert(false);
        }
        if (expected_entries[i].full) {
            if (expected_entries[i].block != w->edits.entries[i].block || !block_pos_equal(expected_entries[i].pos, w->edits.entries[i].pos)) {
                trace("Differs at index %d\n", i);
                assert(false);
            }
        }
    }
    
    trace("Comparison test passed\n");
}
void notmain(void) {
    // create sample config for world
    world_info_t info = {
        .seed = 0,
        .min = (world_pos_t){-20, -20, -20},
        .max = (world_pos_t){0, 0, 0},
        .edits_cap = 8192,
        .pending_cap = 8192,
    };
    trace("Size of world_entry_t: %d\n", sizeof(world_entry_t));

    world_t* w = world_create(&info);


    if (!w) {
        panic("Failed to create world");
    }

    fill_table(w);

    for (int x = w->info->min.x; x < w->info->max.x; x += 2) {
        for (int y = w->info->min.y; y < w->info->max.y; y += 2) {
            for (int z = w->info->min.z; z < w->info->max.z; z += 2) {
                world_pos_t p = {x, y, z};
                world_set_block(w, p, BLOCK_GRASS);
            }
        }
    }
  
    trace("Count: %d, Size: %d\n", count, w->edits.size);
    assert(count == w->edits.size);

    compare_world(w);
    //assert(memcmp(w->edits.entries, expected_entries, w->edits.cap * sizeof(world_entry_t)) == 0);
    
    
}