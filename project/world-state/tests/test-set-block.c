#include "rpi.h"
#include "world.h"
#include "hashtable.h"

static uint32_t count = 0;
void notmain(void) {
    // create sample config for world
    world_info_t info = {
        .seed = 0,
        .min = (pos_t){-20, -20, -20},
        .max = (pos_t){0, 0, 0},
        .edits_cap = 2048,
        .pending_cap = 1024,
    };
    trace("Size of world_entry_t: %d\n", sizeof(world_entry_t));

    player_t player = {.player_id = 0,
        .position = (pos_t) {0, 0, 0},
        .rotation = (p_rot_t) {0, 0}
    };

    world_t* w = world_create(&info, &player);
    if (!w) {
        panic("Failed to create world");
    }

    for (int x = w->info->min.x; x < w->info->max.x; x += 2) {
        for (int y = w->info->min.y; y < w->info->max.y; y += 2) {
            for (int z = w->info->min.z; z < w->info->max.z; z += 2) {
                pos_t p = (pos_t){x, y, z};
                world_set_block(w, p, BLOCK_GRASS);
                count++;
            }
        }
    }

    trace("Count: %d, Size: %d\n", count, w->edits.size);
    assert(count == w->edits.size);

    for (int x = w->info->min.x; x < w->info->max.x; x++) {
        for (int y = w->info->min.y; y < w->info->max.y; y++) {
            for (int z = w->info->min.z; z < w->info->max.z; z++) {
                pos_t p = (pos_t){x, y, z};
                bool is_step2 = ((x - w->info->min.x) % 2 == 0) &&
                                ((y - w->info->min.y) % 2 == 0) &&
                                ((z - w->info->min.z) % 2 == 0);
                world_entry_t* e = table_get_entry(&w->edits, p);
                if (is_step2) {
                    assert(e != NULL && e->block == BLOCK_GRASS);
                } else {
                    assert(e == NULL);
                }
            }
        }
    }

    trace("Comparison test passed\n");
    world_destroy(w);
}