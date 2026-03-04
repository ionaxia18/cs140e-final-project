#ifndef HASHTABLE_H
#define HASHTABLE_H


#include <stdint.h>
#include <stdbool.h>
#include "rpi.h"
#include "world.h"
#include "world-gen.h"


// Run hash function to get index of table. Returns -1 if table is full.
// Linear probe if collision.
uint32_t table_hash_index(world_pos_t p, uint32_t cap);

// Helper to get next open index in table with or without collision. Returns -1 if table is full.
uint32_t table_empty_index(world_entry_t* entries, uint32_t cap, world_pos_t p);

/* Assumes pos is valid. 
Returns null if not found or pointer to entry if found*/
world_entry_t* table_get_entry(const world_table_t* t, world_pos_t p);    

// Insert entry into table: returns T if successful, F if failed
bool table_set_entry(world_table_t* t, block_t block, world_pos_t p);

#endif // HASHTABLE_H