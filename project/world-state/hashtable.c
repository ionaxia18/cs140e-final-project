#include "rpi.h"
#include "world.h"
#include "hashtable.h"
#include "world-gen.h"

// Run hash function to get index of table. Returns -1 if table is full.
// Linear probe if collision.
uint32_t table_hash_index(pos_t p, uint32_t cap) { 
    uint32_t key = world_make_key(p);
    // Knuth multiplicative hash function
    return (key * 2654435761u) >> (32 - __builtin_ctz(cap)); 
    
}

// Helper to get next open index in table with or without collision. Returns -1 if table is full.
uint32_t table_empty_index(world_entry_t* entries, uint32_t cap, pos_t p) {
    uint32_t start = table_hash_index(p, cap);
    for (int i = 0; i < cap; i++) {
        uint32_t index = (start + i) & (cap - 1);
        if (!(entries[index].full)) {
            return index;
        }
    }
    return -1;
}

/* Assumes pos is valid. 
Returns null if not found or pointer to entry if found*/
world_entry_t* table_get_entry(const world_table_t* t, pos_t p) {
    uint32_t start = table_hash_index(p, t->cap);
    world_entry_t* entries = t->entries;

    // Linear probe until find entry with the same pos or first empty slot
    for (int i = 0; i < t->cap; i++) {
        uint32_t index = (start + i) & (t->cap - 1);
        if (!(entries[index].full)) {
            return NULL;
        }

        if (block_pos_equal(entries[index].pos, p)) {
            return &entries[index];
        }
       
    }
    return NULL;
}

bool table_set_entry(world_table_t* t, block_t block, pos_t p) {
    // Create new entry in table for block at pos p
    world_entry_t entry = (world_entry_t){block, p, true};

    world_entry_t* entries = t->entries;

    uint32_t index = table_empty_index(entries, t->cap, p);
    if (index == -1) {
        return false;
    }
    entries[index] = entry;
    t->size++;
    return true;
}

