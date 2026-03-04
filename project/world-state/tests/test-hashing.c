#include "rpi.h"
#include "world.h"
#include "hashtable.h"
#include "world-gen.h"  

bool test_indices[8][8][8];
bool expected_indices[8][8][8];

// fill in table with all grass positions 
void fill_table(world_t* w) {
    world_entry_t entry;
    entry.block = BLOCK_GRASS;
    entry.full = true;

    for (int x = w->info->min.x; x < w->info->max.x; x++) {
        for (int z = w->info->min.z; z < w->info->max.z; z++) {
            pos_t p = {x, -60, z};
            uint32_t index = table_hash_index(p, w->edits.cap);
            trace("Index for pos %d, %d, %d is %d\n", x, -60, z, index);
            w->edits.entries[index] = entry;
            w->edits.entries[index].pos = p;
       
            expected_indices[x + 65][-60 + 65][z + 65] = true;
            w->edits.size++;
        }
    }
}   

void notmain(void) {
    // create sample config for world
    world_info_t info = {
        .seed = 0,
        .min = (pos_t){-65, -65, -65},
        .max = (pos_t){-58, -58, -58},
        .edits_cap = 256,
        .pending_cap = 256,
    };
    
    player_t player = {.player_id = 0,
        .position = (pos_t) {0, 0, 0},
        .rotation = (p_rot_t) {0, 0}
    };

    world_t* w = world_create(&info, &player);



    if (!w) {
        panic("Failed to create world");
    }

    fill_table(w);

    world_entry_t entry;

    for (int x = w->info->min.x; x < w->info->max.x; x++) {
        for (int y = w->info->min.y; y < w->info->max.y; y++) {
            for (int z = w->info->min.z; z < w->info->max.z; z++) {
                pos_t p = {x, y, z};
                uint32_t index = table_hash_index(p, w->edits.cap);
                if (y != -60) {
                    assert(table_get_entry(&w->edits, p) == NULL);
                } else {
                    assert(table_get_entry(&w->edits, p) != NULL && table_get_entry(&w->edits, p)->block == BLOCK_GRASS);
                    test_indices[x + 65][y + 65][z + 65] = true;
                }
            }
           
        }
    }
    assert(memcmp(test_indices, expected_indices, sizeof(test_indices)) == 0);
    trace("Comparison test passed\n");
}