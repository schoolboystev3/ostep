#ifndef COUNTER_H
#define COUNTER_H

struct __counter_t;
typedef struct __counter_t counter_t;

counter_t* counter_init(int num_threads);
void counter_destroy(counter_t *c);
void counter_increment(counter_t *c, int thread_id);
int counter_get(counter_t *c);

#endif
