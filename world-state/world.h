#ifndef WORLD_H
#define WORLD_H

#include <stdint.h>
#include <stdbool.h>

typedef uint8_t block_t;

enum {
    BLOCK_AIR   = 0,
    BLOCK_BEDROCK = 1,
    BLOCK_DIRT  = 2,
    BLOCK_GRASS = 3,
};

typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} world_pos_t;

typedef uint32_t world_key_t;

/* Create integer key of 10 bit chunks: x bits | y bits | z bits
Theoretically limits world max size to 1024x1024x1024
Converts position into key for hash table */
world_key_t world_make_key(world_pos_t p);

// Convert key back into position 
world_pos_t world_read_key(world_key_t k);

typedef struct {
    uint32_t seed;

    // Testing purposes: restrict size of world
    world_pos_t min;
    world_pos_t max;

    // Hash table capacities (for kmalloc)
    uint32_t edits_cap;
    uint32_t pending_cap;
} world_config_t;

typedef struct world world_t;

/* Allocate and init world state hash tables
Returns null if fail */
world_t* world_create(const world_config_t* cfg);

// Reset world to original state
void world_reset(world_t* w);

// Get base block info at position p from seed
block_t world_base_block(const world_t* w, world_pos_t p);

// Get current block info at position p 
block_t world_get_block(const world_t* w, world_pos_t p);

/* Modify block info at position p: 
If new_block == base_block, no edit is recorded
Else, insert change into edits and pending tables
Returns F if out of bounds or if tables are full */
bool world_set_block(world_t* w, world_pos_t p, block_t new_block);

// Break block by setting state to air
static inline bool world_break_block(world_t* w, world_pos_t p) {
    return world_set_block(w, p, BLOCK_AIR);
}

#endif // WORLD_H







