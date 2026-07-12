/* ## C06: Thread-Safe Pub/Sub Dispatcher

Problem statement:

Implement a small thread-safe pub/sub dispatcher.

Requirements:

- Return `0` on success.
- Return `-1` for invalid required pointers.
- `subscribe` returns `-2` if full.
- `unsubscribe` returns `-2` if not found.
- `publish` calls every active matching subscriber.
- Do not hold `d->mu` while invoking callbacks.
- Copy matching callbacks into a temporary array under lock, then unlock and invoke.

*/

#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>

#define MAX_SUBS 16

typedef void (*event_cb)(uint32_t event_id, void *ctx);

struct subscriber {
    bool active;
    uint32_t topic;
    event_cb cb;
    void *ctx;
};

struct dispatcher {
    pthread_mutex_t mu;
    struct subscriber subs[MAX_SUBS];
};

int subscribe(struct dispatcher *d, uint32_t topic, event_cb cb, void *ctx) {
    if (!d || !ctx || !cb) {
        return -1;
    }

    bool found_slot = false;
    pthread_mutex_lock(&d->mu);
    for (size_t i = 0; i < MAX_SUBS; i++) {
        if (!d->subs[i].active) {
            found_slot = true;
            d->subs[i].active = true;
            d->subs[i].topic = topic;
            d->subs[i].cb = cb;
            d->subs[i].ctx = ctx;
            break;
        }
    }
    pthread_mutex_unlock(&d->mu);
    if (found_slot) {
        return 0;
    } else {
        return -2;
    }
}

int unsubscribe(struct dispatcher *d, event_cb cb, void *ctx) {
    if (!d || !ctx || !cb) {
        return -1;
    }

    bool found_sub = false;
    pthread_mutex_lock(&d->mu);
    for (size_t i = 0; i < MAX_SUBS; i++) {
        if ((d->subs[i].cb == cb) && (d->subs[i].ctx == ctx) && d->subs[i].active) {
            found_sub = true;
            d->subs[i].active = false;
        }
    }
    pthread_mutex_unlock(&d->mu);
    if (found_sub) {
        return 0;
    } else {
        return -2;
    }
}

int publish(struct dispatcher *d, uint32_t topic, uint32_t event_id) {
    if (!d) return -1;

    event_cb cbs[MAX_SUBS];
    void *ctxs[MAX_SUBS];
    size_t cb_idx = 0;

    pthread_mutex_lock(&d->mu);
    for (size_t i = 0; i < MAX_SUBS; i++) {
        if (d->subs[i].active && (d->subs[i].topic == topic)) {
            cbs[cb_idx] = d->subs[i].cb;
            ctxs[cb_idx] = d->subs[i].ctx;
            cb_idx++;
        }
    }
    pthread_mutex_unlock(&d->mu);

    for (size_t i = 0; i < cb_idx; i ++) {
        cbs[i](event_id, ctxs[i]);
    }

    return 0;
}
/*
 Follow-ups:
 
 - What races exist between publish and unsubscribe?
    Answer: the race is on active.
 - What if callbacks must be strictly disabled after unsubscribe returns?
    Answer: Publish may have already copied the function pointer into
            its local stack array. We we want the heaviest synchronization
            we can have the callbacks in the lock too but there may be
            deadlock if the callbacks call the dispatcher.
    Answer: Need a mechanism to queue subscribers instead of failing 
            when the array is full. Use a cond var or semaphore.
*/
