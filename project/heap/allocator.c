#include "allocator.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>

#define BYTES_PER_LINE 32
#define HEADER_BYTES 8
#define ALIGNMENT 8
#define TYPE_BYTES 8
#define STATUS_BIT 1
#define MAX_REQUEST_SIZE (1 << 30)

typedef struct freelist {
    size_t payload;
    struct freelist *previous;
    struct freelist *next;
} freelist;

static freelist *segment_start;
static freelist *segment_end;
static size_t segment_size;
static size_t malloc_calls;
static size_t free_calls;
static size_t nused;
static freelist *list_head;


/* Function: myinit
 * -----------------
 * This function initializes the heap segment before a client can use it and resets it to an empty state. 
 * It also initializes the free list, which contains the entire heap as one freelist struct. 
 * It returns true if initialization was successful or false if an error occured.
*/
bool myinit(void *heap_start, size_t heap_size) {
    segment_start = heap_start;
    segment_end = (freelist *)((char *)segment_start + heap_size);
    segment_size = heap_size;
    freelist *head = segment_start;
    // freelist contains the entire heap
    head->previous = NULL;
    head->next = NULL;
    head->payload = segment_size - HEADER_BYTES;
    list_head = head;
    if (segment_size < sizeof(freelist)) {
        // heap size must be at least one payload and one header
        return false;
    } 
    nused = 0;
    malloc_calls = 0;
    free_calls = 0;
    return true;
}

/* Function: roundup
 * -----------------
 * This function rounds up the given number to the given multiple, which
 * must be a power of 2, and returns the result.
 */
size_t roundup(size_t sz, size_t mult) {
    return (sz + mult - 1) & ~(mult - 1);
}

/* Function: check_status
 * -----------------
 * This function takes in a size_t payload and extracts the last bit of the eight bits (1 = used, 0 = unused). 
 * It returns that last bit. 
 */
size_t check_status(size_t payload) {
    return payload & STATUS_BIT;
}

/* Function: check_size
 * -----------------
 * This function takes in a size_t payload and extracts the first 7 bytes of the payload. 
 * It returns this value as a size_t.
 */
size_t check_size(size_t payload) {
    return payload & ~STATUS_BIT;
}

/* Function: next_block
 * -----------------
 * This function takes in a freelist struct ptr, which it assumes points to the start of a block of at least 24 bytes
 * containing a header and payload in the heap. 
 * It analyzes the header to return a pointer to the right adjacent block in the heap. 
 */
freelist *next_block(freelist *block) {
    size_t offset = check_size(block->payload) + HEADER_BYTES;
    return (freelist *)((char *)block + offset);  // cast char * to perform pointer arith
}

/* Function: remove_node
 * -----------------
 * This function takes in a freelist struct ptr, which it assumes points to the start of a block of at least 24 bytes
 * containing a header and payload in the heap. 
 * It wires the adjacent nodes in the free list to remove the given node. The function returns nothing.
 */
void remove_node(freelist *cur_block) {
    freelist *prev_node = cur_block->previous;
    freelist *next_node = cur_block->next;
    if (prev_node) {  // check if the node is the first or last node in the list, which leads to different wiring
        if (next_node) {
            prev_node->next = cur_block->next;
            next_node->previous = prev_node;
        } else {
            prev_node->next = NULL;
        }
    } else if (next_node) {
        next_node->previous = NULL;
        list_head = next_node;
    } else {
        list_head = NULL;
    }
}

/* Function: add_head
 * -----------------
 * This function takes in a freelist struct ptr new_block, 
 * which it assumes points to the start of a block of at least 24 bytes containing a header and payload in the heap.
 * It wires the nodes in the free list to add the node to the front, in a LIFO order. 
 */
void add_head(freelist *new_block) {
    new_block->next = list_head; 
    new_block->previous = NULL;
    if (list_head) { 
        list_head->previous = new_block;
    }
    list_head = new_block;
}

/* Function: coalesce
 * -----------------
 * This function takes in a freelist struct ptr block, 
 * which it assumes points to the start of a block of at least 24 bytes containing a header and payload in the heap.
 * It checks the adjacent right block to see if it is free and can be combined with the given block.
 * It returns nothing but updates the size of the given block accordingly depending on if coalesce is possible.  
 */
void coalesce(freelist *block) {
    // find the next block and check if free
    freelist *right_block = next_block(block);
    if (right_block < segment_end && !check_status(right_block->payload)) {
        block->payload += right_block->payload + HEADER_BYTES;
        remove_node(right_block);  // remove right block if coalesce successful
    }
}

/* Function: update_headers
 * -----------------
 * This function takes in a freelist struct ptr block, which it assumes points to the start of 
 * a block of at least 24 bytes containing a header and payload in the heap. It also takes in two size_t parameters.
 * This function performs housekeeping for when a new block of size needed needs to be allocated at cur_block,
 * including updating the sizes of the adjacent headers, updating statuses, and checking for right coalesces.
 * It returns nothing.
 */
void update_headers(freelist *cur_block, size_t needed, size_t cur_size) {
    size_t needed_block = needed + HEADER_BYTES;
    // cast to char * first to perform pointer arithmetic
    freelist *next_block = (freelist *)((char *)cur_block + needed_block);
    if (cur_size - needed >= sizeof(freelist)) {  // check if cur_block can be further split into two blocks
        next_block->payload = cur_size - needed_block;
        // add 1 to signify allocated status
        cur_block->payload = needed + 1;                   
    } else {
        cur_block->payload = cur_size + 1;
        next_block = NULL;
    }  
    if (next_block) {  // if next_block is not null, add as head of free list and check if right coalesce is possible
        coalesce(next_block);
        add_head(next_block);
    }   
}

/* Function: mymalloc
 * -----------------
 * This function takes in a size_t size, which is the client's requested allocation size.
 * It performs some error checking for the size and checks if there is space on the heap to 
 * accomodate this request. If there is, mymalloc returns a pointer to the first adequate segment, otherwise
 * it returns null if the request is invalid or if the heap could not accomodate the request.
 */
void *mymalloc(size_t requested_size) {
    size_t needed = roundup(requested_size, 2 * ALIGNMENT);
    if (needed > MAX_REQUEST_SIZE || requested_size == 0) {
        return NULL;
    }
    freelist *cur_block = list_head;
    while (cur_block != NULL) {  // run through free list only 
        size_t payload = cur_block->payload;
        size_t cur_size = check_size(payload);
        if (cur_size >= needed) {
            malloc_calls++;
            update_headers(cur_block, needed, cur_size);          
            remove_node(cur_block);        
            nused += check_size(cur_block->payload);           
            return (void *)((char *)cur_block + HEADER_BYTES);  // return void * as payload to client
            // pointer char * cast for pointer arithmetic to get location of payload
        }
        cur_block = cur_block->next;
    }
    return NULL;
}

/* Function: myfree
 * -----------------
 * This function receives a payload pointer from the client.
 * It frees this pointer by setting the allocation status to free and checks if the free block can
 * coalesce with its right neighbor.
 * It returns nothing.
 */
void myfree(void *ptr) {
    if (ptr == NULL) {
        return;
    }
    free_calls++;
    freelist *free_block = (freelist *)((char *)ptr - HEADER_BYTES);  // get header from payload ptr
    nused -= check_size(free_block->payload);
    coalesce(free_block);
    free_block->payload--;
    add_head(free_block);  // becomes head of free list
}

/* Function: absorb_realloc
 * -----------------
 * This function takes in two size_t parameters and a freelist struct ptr cur_block, which it assumes points to 
 * the start of a block of at least 24 bytes containing a header and payload in the heap.
 * This is a helper function for realloc when the new size is larger than the old size.
 * The function attempts to right coalesce until an adequate block to accomodate the new size is created or until no
 * more coalescing is possible.
 * It returns the location of the payload if coalescing successfully created a block or null if not successful.
 * The return is void *, as it becomes the pointer given to the client.
 */
void *absorb_realloc(size_t new_size, freelist *cur_block) {  
    freelist *right_block = next_block(cur_block);
    while (right_block < segment_end && !check_status(right_block->payload)) {
        coalesce(cur_block);
        if (check_size(cur_block->payload) >= new_size) {  // adequate block created
            // - 1 to remove status bit for update_headers, which updates the status              
            update_headers(cur_block, new_size, cur_block->payload - 1);          
            return (char *)cur_block + HEADER_BYTES;  // cast to char * for pointer arithmetic
        }
        right_block = next_block(right_block);
    }    
    return NULL;    
}

/* Function: myrealloc
 * -----------------
 * This function receives a payload pointer from the client and a size_t new size.
 * It performs some error checking for null pointers. If new size is smaller than the old size, myrealloc
 * performs in place realloc and returns the original pointer. Otherwise, it tries to find a new block for the new size 
 * and copies over the original data to the new location if needed. 
 * It returns the pointer to the new location if allocation was successful.
 */
void *myrealloc(void *old_ptr, size_t new_size) {
    if (old_ptr && new_size == 0) {
        myfree(old_ptr);
        return NULL;
    }
    if (!old_ptr) {    
        return mymalloc(new_size);
    }
    new_size = roundup(new_size, 2 * ALIGNMENT);
    freelist *cur_block = (freelist *)((char *)old_ptr - HEADER_BYTES);  // get header from payload ptr
    size_t old_size = check_size(cur_block->payload);
    // if new size is smaller, in place realloc
    if (new_size <= old_size) {      
        update_headers(cur_block, new_size, old_size);
        nused -= (old_size - check_size(cur_block->payload));  // check_size is needed to check for changes in size
        return old_ptr;
    }
    // attempt to coalesce adjacent free blocks first
    void *absorbed_block = absorb_realloc(new_size, cur_block);
    if (absorbed_block) {       
        nused += check_size(cur_block->payload) - old_size;
        return absorbed_block;
    }
    // coalesce failed to produce an adequate block, turn to mymalloc
    freelist *malloc_ptr = mymalloc(new_size);  
    if (!malloc_ptr) {
        return NULL;
    }  
    memcpy(malloc_ptr, old_ptr, old_size);       
    myfree(old_ptr);
    nused += check_size(cur_block->payload) - old_size;
    return malloc_ptr;
}