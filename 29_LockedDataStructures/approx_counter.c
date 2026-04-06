#include "counter.h"
#include <stdlib.h>
#include <pthread.h>

// Gobal update threshold
static int threshold = 10000;

typedef struct __counter_t {
    int value;
    int num_threads;
    int *local_value; // array of local values
    pthread_mutex_t glock;
    pthread_mutex_t *llock; // array of local locks
} counter_t;

counter_t* counter_init(int num_threads) {
    counter_t *c = malloc(sizeof(counter_t));
    c->value = 0;
    c->num_threads = num_threads;
    pthread_mutex_init(&c->glock, NULL);
    c->local_value = malloc(num_threads*sizeof(int));
    c->llock = malloc(num_threads*sizeof(pthread_mutex_t));
    for(int i = 0; i < num_threads; i++) {
        pthread_mutex_init(&c->llock[i], NULL);
    }
    return c;
}

void counter_destroy(counter_t *c) {
    free(c->local_value);
    free(c->llock);
    free(c);
}

void counter_increment(counter_t *c, int thread_id) {
    pthread_mutex_lock(&c->llock[thread_id]);
    c->local_value[thread_id] += 1;
    if(c->local_value[thread_id] >= threshold) {
        pthread_mutex_lock(&c->glock);
        c->value += c->local_value[thread_id];
        pthread_mutex_unlock(&c->glock);
        c->local_value[thread_id] = 0;
    }
    pthread_mutex_unlock(&c->llock[thread_id]);
}

int counter_get(counter_t *c) {
    int tmp;
    pthread_mutex_lock(&c->glock);
    tmp = c->value;
    pthread_mutex_unlock(&c->glock);
    return tmp;
}


