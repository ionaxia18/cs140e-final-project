#include "rpi.h"
#include "world.h"
#include "world-gen.h"


void get_block_info(world_t* w, world_pos_t p) {
    trace("Block info at %d, %d, %d: %d\n", p.x, p.y, p.z, world_base_block(w, p));
}

void notmain(void) {
    // create sample config for world
    world_info_t info = {
        .seed = 0,
        .min = (world_pos_t){-255, -255, -255},
        .max = (world_pos_t){0, 0, 0},
        .edits_cap = 256,
        .pending_cap = 256,
    };

    world_t* w = world_create(&info);


    if (!w) {
        panic("Failed to create world");
    }


    // should be air
    get_block_info(w, (world_pos_t){0, 0, 0});
    assert(world_base_block(w, (world_pos_t){0, 0, 0}) == BLOCK_AIR);
    get_block_info(w, (world_pos_t){0, 10, 0});
    assert(world_base_block(w, (world_pos_t){0, 10, 0}) == BLOCK_AIR);

    get_block_info(w, (world_pos_t){0, -59, 0});
    assert(world_base_block(w, (world_pos_t){0, -59, 0}) == BLOCK_AIR);

    // oob tests
    get_block_info(w, (world_pos_t){0, -1000, 0});
    assert(world_base_block(w, (world_pos_t){0, -1000, 0}) == BLOCK_AIR);
    get_block_info(w, (world_pos_t){0, 1000, 0});
    assert(world_base_block(w, (world_pos_t){0, 1000, 0}) == BLOCK_AIR);
    get_block_info(w, (world_pos_t){0, 0, -1000});
    assert(world_base_block(w, (world_pos_t){0, 0, -1000}) == BLOCK_AIR);
    get_block_info(w, (world_pos_t){0, 0, 1000});
    assert(world_base_block(w, (world_pos_t){0, 0, 1000}) == BLOCK_AIR);

    // should be stuff
    get_block_info(w, (world_pos_t){0, -60, 0});
    assert(world_base_block(w, (world_pos_t){0, -60, 0}) == BLOCK_GRASS);

    get_block_info(w, (world_pos_t){0, -61, 0});
    assert(world_base_block(w, (world_pos_t){0, -61, 0}) == BLOCK_DIRT);
    get_block_info(w, (world_pos_t){0, -62, 0});
    assert(world_base_block(w, (world_pos_t){0, -62, 0}) == BLOCK_DIRT);
    
    get_block_info(w, (world_pos_t){10, -63, 0});
    assert(world_base_block(w, (world_pos_t){10, -63, 0}) == BLOCK_BEDROCK);
    get_block_info(w, (world_pos_t){100, -64, 100});
    assert(world_base_block(w, (world_pos_t){100, -64, 100}) == BLOCK_BEDROCK);
    get_block_info(w, (world_pos_t){-100, -65, -100});
    assert(world_base_block(w, (world_pos_t){-100, -65, -100}) == BLOCK_AIR);

}
