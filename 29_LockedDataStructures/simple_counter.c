#include "counter.h"
#include <stdlib.h>
#include <pthread.h>

typedef struct __counter_t {
    int value;
    pthread_mutex_t glock;
} counter_t;

counter_t* counter_init(int num_threads) {
    counter_t *c = malloc(sizeof(counter_t));
    c->value = 0;
    pthread_mutex_init(&c->glock, NULL);

    (void)num_threads;
    return c;
}

void counter_destroy(counter_t *c) {
    free(c);
}

void counter_increment(counter_t *c, int thread_id) {
    pthread_mutex_lock(&c->glock);
    c->value++;
    pthread_mutex_unlock(&c->glock);
    (void)thread_id;
}

int counter_get(counter_t *c) {
    int tmp;
    pthread_mutex_lock(&c->glock);
    tmp = c->value;
    pthread_mutex_unlock(&c->glock);
    return tmp;
}

