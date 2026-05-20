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

#include <stddef.h>
#include <stdint.h>

#define INVALID_ADDR ((uintptr_t)-1)
uintptr_t head;

// Initializes the pool with a raw chunk of memory.
// 'raw_memory' is a pointer to a statically allocated byte array of size 'pool_size'.
// 'block_size' is the requested size of each buffer block before alignment.
void buffer_pool_init(uint8_t *raw_memory, size_t pool_size, size_t block_size) {
    // Round up to first aligned address
    head = (uintptr_t)raw_memory;
    head = (head + 63) & ~63;

    // Minimum block size is 64b to at least store address
    if (block_size < 8) {
        block_size = 8;
    }
    block_size = (block_size + 63) & ~63;

    uintptr_t curr = head;
    uintptr_t next = head;
    uintptr_t pool_end = (uintptr_t)raw_memory + pool_size;
    while (curr < pool_end) {
        if (curr + block_size <= pool_end) {
            *(uintptr_t*)curr = curr + block_size;
        } else {
            // Last block
            *(uintptr_t*)curr = INVALID_ADDR;
        }
        curr += block_size;
    }
}

// Allocates a single 64-byte aligned block from the pool. 
// Returns a pointer to the usable block payload, or NULL if empty.
void* buffer_pool_alloc(void) {
    if (head == INVALID_ADDR) {
        return NULL;
    } else {
        void* block = (void*)head;
        head = *(uintptr_t*)head;
        return block;
    }
}

// Returns a block back to the pool for reuse.
void buffer_pool_free(void *ptr) {
    uintptr_t next = head;
    head = (uintptr_t)ptr;
    *(uintptr_t*)head = next;
}
