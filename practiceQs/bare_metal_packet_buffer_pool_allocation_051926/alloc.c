/*
 * You are writing firmware for a high-throughput network interface layer on an embedded microcontroller. 
 * Standard malloc is unavailable because it's non-deterministic and can cause heap fragmentation.
 * Instead, you need to implement a fixed-size block memory allocator (a buffer pool) 
 * from a statically allocated chunk of hardware RAM.
 *
 * Constraints:
 * 1. Every buffer block must be 64 byte aligned
 * 2. O(1) alloc & free
 * 3. No External tracking structures
 */

uint8_t *first_free;
uint8_t *last_free;

// Initializes the pool with a raw chunk of memory.
// 'raw_memory' is a pointer to a statically allocated byte array of size 'pool_size'.
// 'block_size' is the requested size of each buffer block before alignment.
void buffer_pool_init(uint8_t *raw_memory, size_t pool_size, size_t block_size) {
    // figure out first aligned address
    first_free = NULL;
    last_free = NULL;
    uint64_t mask = 0b11111111;
    if (raw_memory & mask) {
        first_free &= ~mask; // Round down to nearest 8byte
        first_free += 1;     // Add 8 bytes
    }
    else {
        first_free = raw_memory;
    }

    // Minimum block size is 64b to at least store address
    if (block_size < 8) {
        block_size = 8;
    } else if (block_size & mask) {
        // round up
        block_size &= ~mask; // round down to nearest 8byte
        block_size += 8;     // Add 8 bytes
    }

    uint8_t *curr = first_free;
    uint8_t *next_free = first_free;
    uint8_t *pool_end = curr + pool_size; // remainder doesn't matter
    while (curr < pool_end) {
        // Do I need the struct
        // Write next address and move on to the next
        if (curr + block_size < pool_end) {
            *(uint64_t)curr = curr + block_size;
        } else {
            *(uint64_t)curr = curr; // For last free block, set address to itself
            last_free = curr;
        }
        curr += block_size;
    }
}

// Allocates a single 64-byte aligned block from the pool. 
// Returns a pointer to the usable block payload, or NULL if empty.
void* buffer_pool_alloc(void) {

    if (frist_free == NULL) {
        // None Free
        return NULL;
    } else if (first_free == last_free) {
        // Last Free
        uint8_t* buffer = first_free;
        first_free = NULL;
        last_free = NULL;
        return buffer
    } else {
        uint8_t* buffer = first_free;
        first_free = *first_free;
        return buffer;
    }
}

// Returns a block back to the pool for reuse.
void buffer_pool_free(void *ptr) {
    if (first_free == NULL) {
        first_free = ptr;
        last_free = ptr;
    } else {
        *(uint64_t)last_free = ptr;
        last_free = ptr;
    }
}


