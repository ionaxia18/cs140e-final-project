#include "world.h"
#include "hashtable.h"
#include "pending.h"


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
    if (!w->edits.entries) {
        return NULL;
    }
    w->edits.cap = info->edits_cap;
    w->edits.size = 0;


    w->pending.cap = info->pending_cap;
    w->pending.size = 0;
    w->pending.entries = kmalloc(info->pending_cap * sizeof(world_entry_t));
    w->pending.indices = kmalloc(info->pending_cap * sizeof(uint32_t));
    if (!w->pending.entries || !w->pending.indices) {
        return NULL;
    }

    return w;
}

void world_reset(world_t* w);


// Check if two positions are equal
bool block_pos_equal(world_pos_t p1, world_pos_t p2) {
    return p1.x == p2.x && p1.y == p2.y && p1.z == p2.z;
}


// Get current block type at position p 
block_t world_get_block(const world_t* w, world_pos_t p) {
    if (!world_pos_is_valid(p)) {
        trace("Invalid position %d, %d, %d\n", p.x, p.y, p.z);
        return BLOCK_AIR;
    }
    world_entry_t* entry = table_get_entry(&w->edits, p);
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

    world_entry_t* entry = table_get_entry(&w->edits, p);
    if (!entry) {
        if (!table_set_entry(&w->edits, new_block, p)) {
            trace("Failed to set block in edits table");
            return false;
        }
    } else {
        trace("Updating existing entry for block at %d, %d, %d\n", p.x, p.y, p.z);
        entry->block = new_block;
    }

    if (!pending_add(&w->pending, (world_entry_t){new_block, p, true})) {
        trace("Failed to add entry to pending table");
        return false;
    }
    return true;
    
}














