#include "rpi.h"
#include "world.h"
#include "hashtable.h"
#include "world-gen.h"
#include "../heap/allocator.h"

// Run hash function to get index of table. Returns -1 if table is full.
// Linear probe if collision.
uint32_t table_hash_index(pos_t p, uint32_t cap) { 
    uint32_t key = world_make_key(p);
    // Knuth multiplicative hash function
    return (key * 2654435761u) >> (32 - __builtin_ctz(cap)); 
    
}

// // Helper to get next open index in table with or without collision. Returns -1 if table is full.
// uint32_t table_empty_index(world_entry_t* entries, uint32_t cap, pos_t p) {
//     uint32_t start = table_hash_index(p, cap);
//     for (int i = 0; i < cap; i++) {
//         uint32_t index = (start + i) & (cap - 1);
//         if (!(entries[index].full)) {
//             return index;
//         }
//     }
//     return -1;
// }

/* Assumes pos is valid. 
Returns null if not found or pointer to entry if found*/
world_entry_t* table_get_entry(const world_table_t* t, pos_t p) {
    uint32_t idx = table_hash_index(p, t->cap);
    world_entry_t* cur = t->entries[idx];
    while (cur) {
        if (block_pos_equal(cur->pos, p)) {
            return cur;
        }
        cur = cur->next;
    }
    return NULL;
}

bool table_set_entry(world_table_t* t, block_t block, pos_t p) {
    uint32_t idx = table_hash_index(p, t->cap);
    world_entry_t* cur = t->entries[idx];

    // Update existing node if present.
    while (cur) {
        if (block_pos_equal(cur->pos, p)) {
            cur->block = block;
            return true;
        }
        cur = cur->next;
    }

    // otherwise, allocate a new node and insert at bucket head.
    world_entry_t* node = mymalloc(sizeof(world_entry_t));
    if (!node) {
        return false;
    }
    node->block = block;
    node->pos   = p;
    node->next  = t->entries[idx];
    t->entries[idx] = node;
    t->size++;
    return true;
}

bool table_delete_entry(world_table_t* t, pos_t p) {
    uint32_t idx = table_hash_index(p, t->cap);
    world_entry_t* cur  = t->entries[idx];
    world_entry_t* prev = NULL;

    while (cur) {
        if (block_pos_equal(cur->pos, p)) {
            if (prev) {
                prev->next = cur->next;
            } else {
                t->entries[idx] = cur->next;
            }
            myfree(cur);
            t->size--;
            return true;
        }
        prev = cur;
        cur  = cur->next;
    }
    return false;
}