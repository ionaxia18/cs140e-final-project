#include "world.h"
#include "hashtable.h"
#include "pending.h"
#include "../heap/allocator.h"
#include "../boot/world-gen.h"

void print_pos(pos_t p) {
    trace("position at x=%d, y=%d, z=%d\n", (int)p.x, (int)p.y, (int)p.z);
}
static char heap[64 * 1500];
static size_t heap_size = sizeof(heap);
static void* heap_start = heap;
void world_print(world_t* w) {
    trace("printing world now\n");
    for (uint32_t i = 0; i < w->edits.cap; i++) {
        // trace("i = %d", i);
        world_entry_t* cur = w->edits.entries[i];
        while (cur) {
            world_entry_t* next = cur->next;
            print_pos(cur->pos);
            trace("block is %d\n", (int)cur->block);
            cur = next;
        }
    }
    trace("edit size=%d\n", w->edits.size);

}
world_key_t world_make_key(pos_t p) {

    // Cast to int first to preserve sign
    int16_t ix = (int16_t)(p.x < -512 ? -512 : (p.x > 511 ? 511 : p.x));
    int16_t iy = (int16_t)(p.y < -512 ? -512 : (p.y > 511 ? 511 : p.y));
    int16_t iz = (int16_t)(p.z < -512 ? -512 : (p.z > 511 ? 511 : p.z));
    uint32_t ux = (uint32_t)(ix + 512) & 0x3ff;
    uint32_t uy = (uint32_t)(iy + 512) & 0x3ff;
    uint32_t uz = (uint32_t)(iz + 512) & 0x3ff;
    return (ux << 20) | (uy << 10) | iz;
}

pos_t world_read_key(world_key_t k){
    pos_t p; 
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

bool world_pos_is_valid(pos_t p) {
    return p.x >= -512 && p.x <= 511 && p.y >= -512 && p.y <= 511 && p.z >= -512 && p.z <= 511;
}

// to do: this should somehow link to how the fruitjuice server is being started up
world_t* world_create(const world_info_t* info) {
    world_t* w = mymalloc(sizeof(world_t));
    if (!w) {
        trace("Failed to allocate memory for world");
        return NULL;
    }
    w->info = info;

    w->edits.cap = info->edits_cap;
    w->edits.size = 0;
    w->edits.entries = mymalloc(w->edits.cap * sizeof(world_entry_t*));
    if (!w->edits.entries) {
        trace("Failed to allocate memory for edits table");
        return NULL;
    }
    memset(w->edits.entries, 0, w->edits.cap * sizeof(world_entry_t*));

    w->pending.cap = info->pending_cap;
    w->pending.size = 0;
    w->pending.entries = mymalloc(w->pending.cap * sizeof(world_entry_t));
    w->pending.indices = mymalloc(w->pending.cap * sizeof(uint32_t));
    if (!w->pending.entries || !w->pending.indices) {
        trace("Failed to allocate memory for pending table");
        return NULL;
    }
    memset(w->pending.entries, 0, w->pending.cap * sizeof(world_entry_t));
    memset(w->pending.indices, 0, w->pending.cap * sizeof(uint32_t));

    return w;
}

void world_destroy(world_t* w) {
    for (uint32_t i = 0; i < w->edits.cap; i++) {
        world_entry_t* cur = w->edits.entries[i];
        while (cur) {
            world_entry_t* next = cur->next;
            myfree(cur);
            cur = next;
        }
    }
    myfree(w->edits.entries);
    myfree(w->pending.entries);
    myfree(w);
}


// Check if two positions are equal
bool block_pos_equal(pos_t p1, pos_t p2) {
    return p1.x == p2.x && p1.y == p2.y && p1.z == p2.z;
}


// Get current block type at position p 
block_t world_get_block(const world_t* w, pos_t p) {
    if (!world_pos_is_valid(p)) {
        trace("Invalid position %d, %d, %d\n", p.x, p.y, p.z);
        return BLOCK_AIR;
    }
    world_entry_t* entry = table_get_entry(&w->edits, p);
    if (entry != NULL) {
        return entry->block;
    } else {
        return world_base_block(w, p);
    }
}
/* Add block info at position p to hash tables.
Else, insert change into edits and pending tables
Returns F if out of bounds or if tables are full */
bool world_set_block(world_t* w, pos_t p, block_t new_block) {
    if (!world_pos_is_valid(p)) {
        trace("Invalid position %d, %d, %d\n", p.x, p.y, p.z);
        return false;
    }


    if (!table_set_entry(&w->edits, new_block, p)) {
        trace("Failed to set block in edits table");
        return false;
    }

    if (!pending_add(&w->pending, (world_entry_t){ new_block, p, true, NULL })) {
        trace("Failed to add entry to pending table");
        return false;
    }
    return true;
    
}