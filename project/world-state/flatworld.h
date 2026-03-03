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

typedef struct {
    uint32_t seed;

    // Testing purposes: restrict size of world
    world_pos_t min;
    world_pos_t max;

    // Hash table capacities (for kmalloc), must be powers of 2
    uint32_t edits_cap;
    uint32_t pending_cap;
} world_info_t;

typedef struct {
    block_t block;
    world_pos_t pos;
    bool full;
} world_entry_t;

typedef struct {
    world_entry_t* entries;
    uint32_t cap;
    uint32_t size;
} world_table_t;

struct world {
    const world_info_t* info;
    world_table_t edits;
    world_table_t pending;
};

typedef uint32_t world_key_t;

typedef struct world world_t;


/* Create integer key of 10 bit chunks: x bits | y bits | z bits
Theoretically limits world max size to 1024x1024x1024 ie -512 to 511 for each
Converts position into key for hash table */
world_key_t world_make_key(world_pos_t p);

// Convert key back into position 
world_pos_t world_read_key(world_key_t k);

// Check if pos is within support range
bool world_pos_is_valid(world_pos_t p);

/* Allocate and init world state hash tables
Returns null if fail */
world_t* world_create(const world_info_t* info);

// Reset world to original state
void world_reset(world_t* w);

// Check if two positions are equal
bool block_pos_equal(world_pos_t p1, world_pos_t p2);

// Get base block info at position p from seed
block_t world_base_block(const world_t* w, world_pos_t p);

// Run hash function to get hash index in edits table
uint32_t block_hash_index(const world_t* w, world_pos_t p, uint32_t cap);

// Helper to get next open index in table with or without collision. Returns -1 if table is full.
uint32_t get_next_index(world_entry_t* entries, uint32_t cap, uint32_t key);

// Lookup entry in edits table: returns ptr to entry if found, null if not found
world_entry_t* world_get_entry(const world_t* w, world_pos_t p);

// Insert entry into edits table: returns T if successful, F if failed
bool world_set_entry(world_t* w, block_t block, world_pos_t p);

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







