/*
 *  Requirements:

 - Use monotonically increasing `head` and `tail`.
 - Use `counter & RING_MASK` for indexing.
 - Use all 1024 entries.
 - Do not reset `head` or `tail` at 1024.

 1. modulo is expensive
    - power of two -> use the mask
 2. without extra space, hard to tell empty from full
    - monotonically increasing

     Follow-ups:

 - Why does this avoid leaving one empty slot?
    Answer: by allowing the head and tail to monotonically increase
            we avoid the need to leave an empty slot to tell the 
            difference between a full and empty ring.
 - What happens when counters wrap?
    Answer: It does not require anything special to happen because
            the hw inherently can handle overflow/underflow math.
 - How would you make this single-producer/single-consumer safe?
    Answer: If the spsc is handled by different tasks then we will
            need to enforce memory barriers to make sure stale
            data is not read between the two tasks.
 - What memory ordering is needed if hardware consumes the ring?
    Answer: memory barriers enforce cpu cache coherency but what 
            happens if hw is used that bypasses the cache? Will
            need methods to flush this cash to RAM. For things like
            DMA engines.
 */

#define RING_CAP 1024
#define RING_MASK (RING_CAP - 1)
                                                                                                     
struct ring {
    uint32_t head;                                                                                   
    uint32_t tail; 
    uint32_t entries[RING_CAP];
};

bool ring_empty(const struct ring *r) {
    return (r->head == r->tail);
}

bool ring_full(const struct ring *r) {
    return ((r->tail - r->head) == RING_CAP);
}

uint32_t ring_size(const struct ring *r) {
    return (r->tail - r->head); 
}

bool ring_push(struct ring *r, uint32_t value) {
    if (ring_full(r)) {
        return false;
    }

    r->entries[r->tail & RING_MASK] = value;
    r->tail++;
    return true;
}

bool ring_pop(struct ring *r, uint32_t *value) {
    if (ring_empty(r)) {
        return false;
    }

    *value = r->entries[r->head & RING_MASK];
    r->head++;
    return true;
}
