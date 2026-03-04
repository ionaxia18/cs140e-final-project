#include "rpi.h"
#include "world.h"
#include "hashtable.h"

uint32_t count = 0;

void compare_world(world_t* w, world_entry_t* expected_entries) {
    for (int i = 0; i < w->edits.cap; i++) {
        if (expected_entries[i].full != w->edits.entries[i].full) {
            trace("Fullness differs at index %d\n", i);
            assert(false);
        }
        if (expected_entries[i].full) {
            if (expected_entries[i].block != w->edits.entries[i].block || !block_pos_equal(expected_entries[i].pos, w->edits.entries[i].pos)) {
                trace("Differs at index %d\n", i);
                trace("Expected: block %d, pos %d, %d, %d, Got: block %d, pos %d, %d, %d\n", expected_entries[i].block, expected_entries[i].pos.x, expected_entries[i].pos.y, expected_entries[i].pos.z, w->edits.entries[i].block, w->edits.entries[i].pos.x, w->edits.entries[i].pos.y, w->edits.entries[i].pos.z);
        
                assert(false);
            }
        }
    }
    
    trace("Comparison test passed\n");
}

void fill_table(world_t* w, world_entry_t* expected_entries) {

   
    for (int x = w->info->min.x; x < w->info->max.x; x += 2) {
        for (int y = w->info->min.y; y < w->info->max.y; y += 2) {
            for (int z = w->info->min.z; z < w->info->max.z; z += 2) {
                world_pos_t p = {x, y, z};
                uint32_t index = table_empty_index(expected_entries, w->edits.cap, p);
                bool is_step4 = (x - w->info->min.x) % 4 == 0
                                && (y - w->info->min.y) % 4 == 0
                                && (z - w->info->min.z) % 4 == 0;
                block_t block = is_step4 ? BLOCK_DIRT : BLOCK_GRASS;
                expected_entries[index] = (world_entry_t){block, p, true};
                count++;
            }
        }
    }
    trace("Count: %d\n", count);
}



void edit_world(world_t* w) {
    for (int x = w->info->min.x; x < w->info->max.x; x += 2) {
        for (int y = w->info->min.y; y < w->info->max.y; y += 2) {
            for (int z = w->info->min.z; z < w->info->max.z; z += 2) {
                world_pos_t p = {x, y, z};
                world_set_block(w, p, BLOCK_GRASS);
            }
        }
    }

    for (int x = w->info->min.x; x < w->info->max.x; x += 4) {
        for (int y = w->info->min.y; y < w->info->max.y; y += 4) {
            for (int z = w->info->min.z; z < w->info->max.z; z += 4) {
                world_pos_t p = {x, y, z};
                world_set_block(w, p, BLOCK_DIRT);
            }
        }
    }
}

void create_world(uint32_t seed, world_pos_t min, world_pos_t max, uint32_t edits_cap, uint32_t pending_cap) {
    world_info_t info = {
        .seed = seed,
        .min = min,
        .max = max,
        .edits_cap = edits_cap,
        .pending_cap = pending_cap,
    };

    world_t* w = world_create(&info);

    if (!w) {
        panic("Failed to create world");
    }
    world_entry_t* copy = kmalloc(edits_cap * sizeof(world_entry_t));

    fill_table(w, copy);
    edit_world(w);
    trace("Count: %d, Size: %d\n", count, w->edits.size);
    assert(count == w->edits.size);

    compare_world(w, copy);
   
}

void notmain(void) {
    //test different size worlds
    create_world(0, (world_pos_t){0, 0, 0}, (world_pos_t){5, 5, 5}, 128, 128);
    count = 0;
    create_world(0, (world_pos_t){-5, -5, -5}, (world_pos_t){5, 5, 5}, 1024, 1024);
    count = 0;
    create_world(0, (world_pos_t){-10, -10, -10}, (world_pos_t){10, 10, 10}, 8192, 8192);
    count = 0;
    




    
}