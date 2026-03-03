#include "rpi.h"
#include "world.h"
#include "world-gen.h"

// Deterministic hash: same inputs produce same output
static uint32_t hash2d(uint32_t seed, int16_t x, int16_t z) {
    uint32_t h = seed;
    h ^= (uint32_t)(uint16_t)x * 374761393u;
    h ^= (uint32_t)(uint16_t)z * 668265263u;
    h = (h ^ (h >> 13)) * 1274126177u;
    return h ^ (h >> 16);
}

// Query block in flat world with mountains
block_t make_flat_with_mtns(uint32_t seed, world_pos_t p) {
    int base_height = -60;
    // hash gives 0-15, make a mountain if hash < 2 
    uint32_t h = hash2d(seed, p.x, p.z);
    int mtn_height = (h & 0xF) < 2 ? (int)((h >> 4) & 0x7) : 0;

    int top = base_height + mtn_height;

    if (p.y > top)       return BLOCK_AIR;
    if (p.y == top)      return mtn_height > 0 ? BLOCK_STONE : BLOCK_GRASS;
    if (p.y > top - 3)   return BLOCK_DIRT;
    if (p.y > top - 5)   return BLOCK_BEDROCK;

    return BLOCK_AIR;
}

// Query block in flat world
// Seed = 0
block_t flat_world(world_pos_t p) {
    int16_t y = p.y;
    if (y == -60) {
        return BLOCK_GRASS;
    } else if (y < -60 && y > -63) {
        return BLOCK_DIRT;
    } else if (y <= -63 && y > -65) {
        return BLOCK_BEDROCK;
    } else {
        return BLOCK_AIR;
    }
}

block_t world_base_block(const world_t* w, world_pos_t p) {
    const world_info_t* info = w->info;
    if (info->seed == 0) {
        return flat_world(p);
    } else {
        return make_flat_with_mtns(info ->seed, p);
    }
}
