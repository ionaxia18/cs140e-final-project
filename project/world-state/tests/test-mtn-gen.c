#include "rpi.h"
#include "world.h"
#include "world-gen.h"
#include "hashtable.h"
pos_t first[150];
pos_t second[150];


void print_mtns(world_t* w, pos_t* arr) {
    trace("Printing mountains...\n");
    int i = 0;
    for (int x = w->info->min.x; x < w->info->max.x; x++) {
        for (int y = w->info->min.y; y < w->info->max.y; y++) {
            for (int z = w->info->min.z; z < w->info->max.z; z++) {
                pos_t p = {x, y, z};
                block_t block = world_get_block(w, p);
                if (block == BLOCK_STONE) {
                    trace("Block STONE at %d, %d, %d: %d\n", p.x, p.y, p.z, block);
                    arr[i++] = p;
                }
            }
        }
    }
}

void notmain(void) {
    // create sample config for world
    world_info_t info = {
        .seed = 1,
        .min = (pos_t){0, -60, 0},
        .max = (pos_t){16, -44, 16},
        .edits_cap = 4096,
        .pending_cap = 4096,
    };

    player_t player = {.player_id = 0,
        .position = (pos_t) {0, 0, 0},
        .rotation = (p_rot_t) {0, 0}
    };

    world_t* w = world_create(&info, &player);

    if (!w) {
        panic("Failed to create world");
    }

    print_mtns(w, first);
    print_mtns(w, second);

    assert(memcmp(first, second, sizeof(first)) == 0);
    world_destroy(w);
    trace("Comparison test passed\n");

}
