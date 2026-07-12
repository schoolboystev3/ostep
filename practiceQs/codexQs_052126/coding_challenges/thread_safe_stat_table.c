/* ### C08: Thread-Safe Stats Table                                                                     
                                                                                                     
Problem statement:                                                                                   
                                                                                                     
Implement a stats table where many readers query counters and occasional writers update/reset them.  

Requirements:                                                                                        
                                                                                                     
- Validate pointers and indices.                                                                     
- Readers use read lock.                                                                             
- Writers use write lock.                                                                            
- Snapshot must be internally consistent.                                                            
- Discuss overflow behavior for `stats_add`. 
*/
                                                                                                     
#include <pthread.h>                                                                                 
#include <stdint.h>                                                                                  
#include <stdbool.h>                                                                                 
                                                                                                     
#define STAT_COUNT 32                                                                                
                                                                                                     
struct stats_table {                                                                                 
    uint64_t counters[STAT_COUNT];                                                                   
    pthread_rwlock_t lock;                                                                           
};                                                                                                   
                                                                                                     
int stats_read(struct stats_table *t, size_t idx, uint64_t *out) {
    if (!t || idx >= STAT_COUNT || !out) {
        return -1;
    }

    pthread_rwlock_rdlock(&t->lock);
    *out = t->counters[idx];
    pthread_rwlock_unlock(&t->lock);
    return 0;
}
int stats_add(struct stats_table *t, size_t idx, uint64_t delta) {
    if (!t || idx >= STAT_COUNT) {
        return -1;
    }
    // Adding delta to the counter may overflow so only add to MAX
    pthread_rwlock_wrlock(&t->lock);
    if (delta > (UINT64_MAX - t->counters[idx])) {
        t->counters[idx] = UINT64_MAX;
    } else {
        t->counters[idx] += delta;
    }
    pthread_rwlock_unlock(&t->lock);
    return 0;
}
int stats_snapshot(struct stats_table *t, uint64_t *out, size_t out_count) {
    if (!t || !out || out_count < STAT_COUNT) {
        return -1;
    }
    pthread_rwlock_rdlock(&t->lock);
    for (size_t i = 0; i < STAT_COUNT; i++) {
        out[i] = t->counters[i];
    }
    pthread_rwlock_unlock(&t->lock);
    return 0;
}
int stats_reset_all(struct stats_table *t) {
    if (!t) {
        return -1;
    }
    pthread_rwlock_wrlock(&t->lock);
    for (size_t i = 0; i < STAT_COUNT; i++) {
        t->counters[i] = 0;
    }
    pthread_rwlock_unlock(&t->lock);
    return 0;
}
/*
Follow-ups:                                                                                          

- When would atomics be simpler?
    Answer: Things like snapshot wouldn't work with atomics right?
            The scheduler would mess things up. Seems like atomics
            might be useful when data and control planes are separated?
            Need more help to understand
            Is there also some breakeven point with atomics
            because they enforce barriers it may cause a lot of
            overhead?
- When is a plain mutex better?
    Answer: If writers are getting starved then a plain mutex might
            be better. This brings up a good question. How does the
            scheduler decide which blocked thread to run? Say if we
            had a writer and reader blocked on the same mutex?
- Can writers starve?
    Answer: I forget how rwlocks work internally, if there is an
            internal prevention of this but if not then yes
            definitely they can be starved. If the lock is
            constantly swarmed by readers, writers won't be
            able to update.
- How does this map to device telemetry?
    Answer: Live device telemetry requires synchronization 
            just like this. If we want a coherent understanding
            of hardware/device behavior the logs and telemetry
            gathered has to consistent.
*/
