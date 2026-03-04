
#include "rpi.h"
#include "world.h"
#include "hashtable.h"

uint32_t interval_ms;

// Create pending buffer: returns NULL if failed
pending_table_t* pending_table_create(uint32_t cap);

// Add entry to pending buffer: returns T if successful, F if failed
bool pending_add(pending_table_t* pending, world_entry_t entry);

// Flush pending buffer: returns T if successful, F if failed
bool pending_flush(pending_table_t* pending);