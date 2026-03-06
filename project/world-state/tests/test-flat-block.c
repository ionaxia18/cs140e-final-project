#include "rpi.h"
#include "world.h"
#include "world-gen.h"
#include "hashtable.h"
#include "player.h"

void get_block_info(world_t* w, pos_t p) {
    trace("Block info at %d, %d, %d: %d\n", p.x, p.y, p.z, world_base_block(w, p));
}

void notmain(void) {
    // create sample config for world
    world_info_t info = {
        .seed = 0,
        .min = (pos_t){-60, -70, -60},
        .max = (pos_t){-50, -50, -50},
        .edits_cap = 2000,
        .pending_cap = 2000,
    };

    player_t player = {.player_id = 0,
        .position = (pos_t) {0, 0, 0},
        .rotation = (p_rot_t) {0, 0}
    };

    world_t* w = world_create(&info);


    if (!w) {
        panic("Failed to create world");
    }

    //SUPER FLAT TESTS
    // should be air
    for (int x = w->info->min.x; x < w->info->max.x; x++) {
        for (int y = w->info->min.y; y < w->info->max.y; y++) {
            for (int z = w->info->min.z; z < w->info->max.z; z++) {
                pos_t p = {x, y, z};
                //trace("Block info at %d, %d, %d: %d\n", p.x, p.y, p.z, world_get_block(w, p));
                if (y == -60) {
                    assert(world_get_block(w, p) == BLOCK_GRASS);
                } else if (y == -61 || y == -62) {
                    assert(world_get_block(w, p) == BLOCK_DIRT);
                } else if (y == -63 || y == -64) {
                    assert(world_get_block(w, p) == BLOCK_STONE);
                } else {
                    assert(world_get_block(w, p) == BLOCK_AIR);
                }
            }
        }
    }
    world_destroy(w);
    trace("Flat block test passed\n");

}
