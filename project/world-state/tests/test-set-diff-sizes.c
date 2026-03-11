#include "rpi.h"
#include "world.h"
#include "hashtable.h"
#include "../heap/allocator.h"
#include "player.h"

static uint32_t count = 0;

void fill_table(world_t* w, world_entry_t* expected_entries) {
    for (int x = w->info->min.x; x < w->info->max.x; x += 2) {
        for (int y = w->info->min.y; y < w->info->max.y; y += 2) {
            for (int z = w->info->min.z; z < w->info->max.z; z += 2) {
                pos_t p = (pos_t){x, y, z};
                bool is_step4 = (int)(x - w->info->min.x) % 4 == 0
                                 && (int)(y - w->info->min.y) % 4 == 0
                                 && (int)(z - w->info->min.z) % 4 == 0;
                block_t block = is_step4 ? BLOCK_DIRT : BLOCK_GRASS;
                expected_entries[count++] = (world_entry_t){block, p, true, NULL};
            }
        }
    }
    trace("Count: %d\n", count);
}



void edit_world(world_t* w) {
    for (int x = w->info->min.x; x < w->info->max.x; x += 2) {
        for (int y = w->info->min.y; y < w->info->max.y; y += 2) {
            for (int z = w->info->min.z; z < w->info->max.z; z += 2) {
                pos_t p = (pos_t){x, y, z};
                world_set_block(w, p, BLOCK_GRASS);
            }
        }
    }

    for (int x = w->info->min.x; x < w->info->max.x; x += 4) {
        for (int y = w->info->min.y; y < w->info->max.y; y += 4) {
            for (int z = w->info->min.z; z < w->info->max.z; z += 4) {
                pos_t p = (pos_t){x, y, z};
                world_set_block(w, p, BLOCK_DIRT);
            }
        }
    }
}

void create_world(uint32_t seed, pos_t min, pos_t max, uint32_t edits_cap, uint32_t pending_cap) {
    world_info_t info = {
        .seed = seed,
        .min = min,
        .max = max,
        .edits_cap = edits_cap,
        .pending_cap = pending_cap,
    };

    player_t player = {.player_id = 0,
        .position = (pos_t) {0, 0, 0},
        .rotation = (p_rot_t) {0, 0}
    };

    world_t* w = world_create(&info);

    if (!w) {
        panic("Failed to create world");
    }
    world_entry_t* expected = mymalloc(edits_cap * sizeof(world_entry_t));
    memset(expected, 0, edits_cap * sizeof(world_entry_t));

    fill_table(w, expected);
    edit_world(w);
    trace("Count: %d, Size: %d\n", count, w->edits.size);
    assert(count == w->edits.size);

    for (uint32_t i = 0; i < count; i++) {

    
        world_entry_t e = expected[i];
        world_entry_t* actual = table_get_entry(&w->edits, e.pos);
        if (!e.full && !actual->full) {
            continue;
        }
        // if (!actual->full) {
        //     panic("Actual is not full for index %d", i);
        // }
        // if (!e.full) {
        //     panic("Expected is not full for index %d", i);
        // }
        if (actual->block != e.block) {
            panic("Actual block does not match expected block for index %d", i);
        }
        if (!block_pos_equal(actual->pos, e.pos)) {
            panic("Actual position does not match expected position for index %d", i);
        }
    }
    world_destroy(w);
    myfree(expected);
}

void notmain(void) {
    // test different size worlds
    create_world(0, (pos_t){0, 0, 0},      (pos_t){5, 5, 5},      128,  128);
    count = 0;
    create_world(0, (pos_t){-5, -5, -5},   (pos_t){5, 5, 5},      1024, 256);
    count = 0;
    create_world(0, (pos_t){-10, -10, -10}, (pos_t){10, 10, 10},  2048, 512);
    count = 0;
}