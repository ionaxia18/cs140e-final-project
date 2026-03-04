#include "pending.h"



// Add entry to pending buffer: returns T if successful, F if failed
bool pending_add(pending_table_t* pending, world_entry_t entry) {
    if (pending->size == pending->cap) {
        return false;
    }
    uint32_t index = table_empty_index(pending->entries, pending->cap, entry.pos);  
    pending->indices[pending->size] = index;
    pending->entries[index] = entry;
    pending->size++;
    return true;
}

// Flush pending buffer: returns T if successful, F if failed
bool pending_flush(pending_table_t* pending) {
    for (uint32_t i = 0; i < pending->size; i++) {
        world_entry_t entry = pending->entries[pending->indices[i]];
        // send uart message to bridge program
    }
    pending->size = 0;
    memset(pending->entries, 0, pending->cap * sizeof(world_entry_t));
    memset(pending->indices, 0, pending->cap * sizeof(uint32_t));
    return true;
}