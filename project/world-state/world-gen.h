#include "rpi.h"
#include "world.h"
#include "hashtable.h"


block_t world_base_block(const world_t* w, world_pos_t p);

// Query block in flat world with mountains
block_t make_flat_with_mtns(uint32_t seed, world_pos_t p);

// Query block in flat world
// Seed = 0
block_t flat_world(world_pos_t p);
