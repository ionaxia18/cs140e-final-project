#include "flatworld.h"
#include "rpi.h"

world_key_t world_make_key(world_pos_t p) {
    uint32_t mask = 0x3ff; 
    // mask to get rid of sign bit
    return ((p.x & mask) << 20) | ((p.y & mask) << 10) | (p.z & mask);
}

world_pos_t world_read_key(world_key_t k){
    world_pos_t p; 
    uint32_t mask = 0x3ff;
    uint32_t masked_z = k & mask;
    uint32_t masked_y = (k >> 10) & mask;
    uint32_t masked_x = (k >> 20) & mask;

    // Check if x, y, and z were negative via sign bit and fill with 1s if so
    p.x = (masked_x & 0x200) ? (masked_x | 0xFC00) : masked_x;
    p.y = (masked_y & 0x200) ? (masked_y | 0xFC00) : masked_y;
    p.z = (masked_z & 0x200) ? (masked_z | 0xFC00) : masked_z;

    return p;
}

bool world_pos_is_valid(world_pos_t p) {
    return p.x >= -512 && p.x <= 511 && p.y >= -512 && p.y <= 511 && p.z >= -512 && p.z <= 511;
}

world_t* world_create(const world_info_t* info) {
    world_t* w = kmalloc(sizeof(world_t));
    if (!w) {
        return NULL;
    }
    w->info = info;
    w->edits.entries = kmalloc(info->edits_cap * sizeof(world_entry_t));
    w->edits.cap = info->edits_cap;
    w->edits.size = 0;
    //w->pending.entries = kmalloc(info->pending_cap * sizeof(world_entry_t));
    w->pending.cap = info->pending_cap;
    w->pending.size = 0;

    return w;
}

void world_reset(world_t* w);

block_t world_base_block(const world_t* w, world_pos_t p) {
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



// Check if two positions are equal
bool block_pos_equal(world_pos_t p1, world_pos_t p2) {
    return p1.x == p2.x && p1.y == p2.y && p1.z == p2.z;
}

// Run hash function to get index of table. Returns -1 if table is full.
// Linear probe if collision.
uint32_t block_hash_index(const world_t* w, world_pos_t p, uint32_t cap) { 
    uint32_t key = world_make_key(p);
    // Knuth multiplicative hash function
    return (key * 2654435761u) >> (32 - __builtin_ctz(cap)); 
    
}

// Helper to get next open index in table with or without collision. Returns -1 if table is full.
uint32_t get_next_index(world_entry_t* entries, uint32_t cap, uint32_t key) {
    for (int i = 0; i < cap; i++) {
        uint32_t index = (key + i) & (cap - 1);
        if (!(entries[index].full)) {
            return index;
        }
    }
    return -1;
}

/* Assumes pos is valid. 
Returns null if not found or pointer to entry if found*/
world_entry_t* world_get_entry(const world_t* w, world_pos_t p) {
    uint32_t start = block_hash_index(w, p, w->edits.cap);
    world_entry_t* entries = w->edits.entries;

    // Linear probe until find entry with the same pos or first empty slot
    for (int i = 0; i < w->edits.cap; i++) {
        uint32_t index = (start + i) & (w->edits.cap - 1);
        if (!(entries[index].full)) {
            return NULL;
        }

        if (block_pos_equal(entries[index].pos, p)) {
            return &entries[index];
        }
       
    }
    return NULL;
}

bool world_set_entry(world_t* w, block_t block, world_pos_t p) {
    // Create new entry in table for block at pos p
    world_entry_t entry = (world_entry_t){block, p, true};

    uint32_t start = block_hash_index(w, p, w->edits.cap);
    world_entry_t* entries = w->edits.entries;

    uint32_t index = get_next_index(entries, w->edits.cap, start);
    if (index != -1) {
        entries[index] = entry;
        w->edits.size++;
        return true;
    }
    // failed to insert into table
    return false;
}

// Get current block type at position p 
block_t world_get_block(const world_t* w, world_pos_t p) {
    if (!world_pos_is_valid(p)) {
        trace("Invalid position %d, %d, %d\n", p.x, p.y, p.z);
        return BLOCK_AIR;
    }
    world_entry_t* entry = world_get_entry(w, p);
    if (entry) {
        return entry->block;
    } else {
        return world_base_block(w, p);
    }
}

/* Add block info at position p to hash tables.
Else, insert change into edits and pending tables
Returns F if out of bounds or if tables are full */
bool world_set_block(world_t* w, world_pos_t p, block_t new_block) {
    if (!world_pos_is_valid(p)) {
        trace("Invalid position %d, %d, %d\n", p.x, p.y, p.z);
        return false;
    }

    world_entry_t* entry = world_get_entry(w, p);
    if (entry) {
        trace("Updating existing entry for block at %d, %d, %d\n", p.x, p.y, p.z);
        entry->block = new_block;
        return true;
    }   

    if (world_set_entry(w, new_block, p)) {
        return true;
    } else {
        trace("Failed to set block in edits table");
    }
    return false;
}














