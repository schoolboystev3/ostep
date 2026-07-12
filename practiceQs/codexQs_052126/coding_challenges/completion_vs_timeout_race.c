/*### C12: Completion vs Timeout Race
 
 Problem statement:
 
 Design a completion object for an operation that may finish normally, time out, or be canceled/reset. This models a common firmware/driver race: one thread waits for a command to complete, while another  thread, interrupt handler, or deferred worker reports completion at nearly the same time as a timeout/reset path.
 
Return values:
 
 - `comp_wait_timeout`: `0` if completed, `-1` for invalid input, `-2` if timed out, `-3` if canceled, `-4` if the generation is stale.
 - `comp_complete`: `0` if it won the race and completed the pending operation, `-1` for invalid input, `-2` if completion was stale or the operation already left `COMP_PENDING`.
 - `comp_cancel`: `0` if it canceled a pending operation, `-1` for invalid input, `-2` if stale or already completed/timed out.
 
 Requirements:
 
 - `comp_arm` starts a new operation by incrementing `generation` and setting `state = COMP_PENDING`.
 - Exactly one terminal transition wins for each generation:
   - `COMP_PENDING -> COMP_DONE`
   - `COMP_PENDING -> COMP_TIMED_OUT`
   - `COMP_PENDING -> COMP_CANCELED`
 - `comp_wait_timeout` waits in a `while` loop and re-checks state after every wakeup.
 - If timeout expires while state is still `COMP_PENDING`, the waiter transitions it to `COMP_TIMED_OUT`.
 - `comp_complete` must ignore stale completions from older generations.
 - `comp_complete` wakes waiters after setting `COMP_DONE`.
 - `comp_cancel` wakes waiters after setting `COMP_CANCELED`.
 - All state and generation checks happen under `mu`.
*/
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>

enum completion_state {
    COMP_IDLE,
    COMP_PENDING,
    COMP_DONE,
    COMP_TIMED_OUT,
    COMP_CANCELED
};

struct completion {
    pthread_mutex_t mu;
    pthread_cond_t cv;
    enum completion_state state; 
    uint64_t generation;
};

uint64_t comp_arm(struct completion *c) {
    uint64_t gen;
    pthread_mutex_lock(&c->mu);
    c->generation++;
    c->state = COMP_PENDING;
    gen = c->generation;
    pthread_mutex_unlock(&c->mu);
    return gen;
}

int comp_wait_timeout(struct completion *c,
                      uint64_t generation, int timeout_ms) {

    // TODO Absolute Timer Code
    struct timespec ts;

    pthread_mutex_lock(&c->mu);
    if (c->generation != generation || c->state != COMP_PENDING) {
        return -1;
    }

    while (c->state == COMP_PENDING) {
        rc = pthread_cond_timedwait(&c->cv, *c->mu, &ts);
    }

    if (c->state == COMP_DONE) {
        pthread_mutex_unlock(&c->mu);
        return 0;
    }

    if(c->state == COMP_CANCELED) {
        pthread_mutex_unlock(&c->mu);
        return -3;
    }

    if (rc == ETIMEDOUT && c->state == COMP_PENDING) {
        c->state = COMP_TIMED_OUT;
        pthread_mutex_unlock(&c->mu);
        return -2;
    }
    
}

int comp_complete(struct completion *c, uint64_t generation) {
    
}

int comp_cancel(struct completion *c, uint64_t generation);
