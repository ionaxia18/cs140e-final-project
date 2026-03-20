#ifndef WORLD_H
#define WORLD_H

#include <stdint.h>
#include <stdbool.h>
#include "rpi.h"

typedef uint8_t block_t;

enum {
    BLOCK_AIR   = 0,
    BLOCK_STONE = 1,
    BLOCK_GRASS  = 2,
    BLOCK_DIRT = 3,
    BLOCK_COBBLESTONE = 4,
    BLOCK_WOOD = 5,
    BLOCK_GLOWSTONE = 6,
    BLOCK_CAKE = 7,
    BLOCK_PUMPKIN = 8
};

// might want to move out of world.h
typedef struct {
    float x;
    float y;
    float z;
} pos_t;

enum {
    BLOCK_TOP = 0,
    BLOCK_LEFT = 1,
    BLOCK_FRONT = 2,
    BLOCK_RIGHT = 3,
    BLOCK_BOTTOM = 4
};

typedef struct world_entry {
    block_t block;
    pos_t pos;
    bool full;
    struct world_entry* next;
} world_entry_t;

typedef struct {
    world_entry_t** entries;   // hash buckets for edits (separate chaining)
    uint32_t cap;
    uint32_t size;
} world_table_t;

typedef struct {
    world_entry_t* entries;    // flat array used by pending buffer
    uint32_t cap;
    uint32_t size;
    uint32_t* indices;
} pending_table_t;

typedef struct {
    uint32_t seed;

    // Testing purposes: restrict size of world
    pos_t min;
    pos_t max;

    // Hash table capacities (for kmalloc), must be powers of 2
    uint32_t edits_cap; // 64000
    uint32_t pending_cap; // 1024
} world_info_t;

struct world {
    const world_info_t* info;
    world_table_t edits;
    pending_table_t pending;
};

typedef uint32_t world_key_t;

typedef struct world world_t;

void print_pos(pos_t p);

void world_print(world_t* w);
/* Create integer key of 10 bit chunks: x bits | y bits | z bits
Theoretically limits world max size to 1024x1024x1024 ie -512 to 511 for each
Converts position into key for hash table */
world_key_t world_make_key(pos_t p);

// Convert key back into position 
pos_t world_read_key(world_key_t k);

// Check if pos is within support range
bool world_pos_is_valid(pos_t p);

/* Allocate and init world state hash tables
Returns null if fail */
world_t* world_create(const world_info_t* info);

// Reset world to original state
void world_reset(world_t* w);

// Check if two positions are equal
bool block_pos_equal(pos_t p1, pos_t p2);


// Get current block info at position p 
block_t world_get_block(const world_t* w, pos_t p);

char * block_to_str(block_t block);

/* Modify block info at position p: 
If new_block == base_block, no edit is recorded
Else, insert change into edits and pending tables
Returns F if out of bounds or if tables are full */
bool world_set_block(world_t* w, pos_t p, block_t new_block);

// Break block by setting state to air
static inline bool world_break_block(world_t* w, pos_t p) {
    return world_set_block(w, p, BLOCK_AIR);
}

// Free world
void world_destroy(world_t* w);


#endif // WORLD_H







