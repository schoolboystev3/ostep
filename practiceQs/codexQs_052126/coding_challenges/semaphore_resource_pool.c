/*### C01: Semaphore Resource Pool

Problem statement:

Implement a small pool of reusable resources. A semaphore counts available resources. A mutex protects the table that says which resource IDs are in use.

Requirements:

- Return `0` on success.
- Return `-1` for invalid pointers.
- Return `-2` if the pool is closed.
- `rp_acquire` waits for an available permit, then chooses one unused ID under the mutex.
- `rp_release` marks the ID free and posts one permit.
- The mutex protects `closed` and `in_use`.
- The semaphore counts resources, but does not protect the table.
- Discuss the shutdown limitation: POSIX semaphores do not have a natural broadcast wakeup like condition variables.
*/

#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define RP_CAP 8

struct resource_pool {
    uint32_t ids[RP_CAP];
    bool in_use[RP_CAP]; 
    bool closed;
    pthread_mutex_t mu;
    sem_t available;
};

int rp_acquire(struct resource_pool *p, uint32_t *out_id) {
    if (!p || !out_id) {
        return -1;
    }

    sem_wait(&p->available);

    pthread_mutex_lock(&p->mu);
    if (p->closed) {
        pthread_mutex_unlock(&p->mu);
        sem_post(&p->available);
        return -2;
    }

    int free_idx = 0;
    for (int i = 0; i < RP_CAP; i++) {
        if (!p->in_use[i]) {
            free_idx = i;
            p->in_use[i] = true;
            break;
        }
    }
    *out_id = p->ids[free_idx];
    pthread_mutex_unlock(&p->mu);
    
    return 0;
}

int rp_release(struct resource_pool *p, uint32_t id) {
    if (!p) {
        return -1;
    }

    pthread_mutex_lock(&p->mu);

    int release_idx = -1;
    for (int i = 0; i < RP_CAP; i++) {
        if (id == p->ids[i]) {
            release_idx = i;
            break;
        }
    }
    if (release_idx == -1) {
        return -1; // invalid id
    }

    p->in_use[release_idx] = false;
    pthread_mutex_unlock(&p->mu);

    sem_post(&p->available);
    return 0;
}

void rp_close(struct resource_pool *p) {
    if (!p) {
        return -1;
    }

    pthread_mutex_lock(&p->mu);
    p->closed = true;
    // Thread clean up limitation: There's no way to wake up
    // all waiting threads so that we can prevent leftover
    // dangling threads.
    pthread_mutex_unlock(&p->mu);
}

/*
Follow-ups:

- When would a condition variable be clearer?
    Answer: Going to need help with this one.. They both seem good in this
            situation? perhaps there is some nuance in different situaitons.
- How would you track waiters for clean shutdown?
    Answer: Do semaphores allow some type of counting? If not we could add
            a count before waiting on semaphores but that counter would
            also require synchronization which adds complexity.
- How does this map to device queue slots, DMA buffers, or processor resources?
    Answer: This is one way these sort of implementations can be achieved.
            Condition variables are another way to implement these shared 
            resources.
*/
