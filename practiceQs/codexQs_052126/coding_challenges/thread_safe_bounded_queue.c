/*### Challenge 20: Thread-Safe Bounded Queue

Implement a blocking producer/consumer queue. This is a concurrency interview staple and maps well to firmware/runtime queues, completion queues, and background work scheduling.


Requirements:

- `bq_push` returns `0` on success.
- `bq_push` returns `-1` if `q` is `NULL`.
- `bq_push` blocks while the queue is full.
- `bq_push` returns `-2` if the queue is closed before it can push.
- `bq_pop` returns `0` on success.
- `bq_pop` returns `-1` if `q` or `out_value` is `NULL`.
- `bq_pop` blocks while the queue is empty and not closed.
- `bq_pop` returns `-2` if the queue is closed and empty.
- `bq_close` marks the queue closed and wakes all waiting producers/consumers.
- Use `while`, not `if`, around condition-variable waits.
- Do not busy-wait.
- Hold the mutex only while checking or modifying shared state.
*/
#define BQ_CAPACITY 8u

struct blocking_queue {
    uint32_t data[BQ_CAPACITY];
    size_t head;
    size_t tail; 
    size_t count;
    bool closed;
    pthread_mutex_t mu;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
};

int bq_push(struct blocking_queue *q, uint32_t value) {
    if (!q) return -1;

    pthread_mutex_lock(&q->mu);

    while (q->count == BQ_CAPACITY && !q->closed) {
        pthread_cond_wait(&q->not_full, &mu);
    }

    if (q->closed) {
        pthread_mutex_unlock(&q->mu);
        return -2;
    }

    q->data[q->head] = value;
    q->head = (q->head + 1) % BQ_CAPACITY;
    q->count++;

    pthread_cond_signal(&q->not_empty, &q->mu);

    pthread_mutex_unlock(&q->mu);
    return 0;
}

int bq_pop(struct blocking_queue *q, uint32_t *out_value) {
    if (!q || !out_value) return -1;

    pthread_mutex_lock(&q->mu);

    while (q->count == 0 && !q->closed) {
        pthread_cond_wait(&q->not_empty, &q->mu);
    }

    if (q->closed && (q->count == 0)) {
        pthread_mutex_unlock(&q->mu);
        return -2;
    }

    *out_value = q->data[tail];
    q->tail = (q->tail + 1) % BQ_CAPACITY;
    q->count--;

    pthread_cond_signal(&q->not_full, &q->mu);

    pthread_mutex_unlock(&q->mu);
    return 0;
}

int bq_close(struct blocking_queue *q) {
    if (!q) return -1;

    pthread_mutex_lock(&q->mu);

    q->closed = true;
    pthread_cond_broadcast(&q->not_full);
    pthread_cond_broadcast(&q->not_empty);

    pthread_mutex_unlock(&q->mu);
    return 0;
}

/*
Follow-ups:

- Why do condition-variable waits need a loop?
    Answer: The scheduler is unpredictable. Between the signaling of a thread
            to wake up and actually waking and acquiring the lock state
            might have changed. We must check state again once awake
            and having acquired the lock.
- What happens if `bq_close` only signals one waiter?
    Answer: Thread leaks, waiting forever for cond vars.
            Maybe other bad things?
- Where could deadlock happen?
    Answer: If the producer held the lock waiting for the queue
            to have space and busy waited. But the consumer
            was waiting on the same lock so that it could dequeue?
- How would you test two producers and two consumers?
    Answer: The real test is to see if the cond vars are working 
            as expected. So I would have 1 producer fill the queue
            then start the two consumers. After it's empty begin
            the other producer and at that point run all threads.
- How would this differ for a lock-free SPSC queue?
    Answer: If it's SPSC we don't need cond vars because there is 
            only one consumer and one producer. Instead of a mutex
            we could ensure consistency with memory barriers. Logically
            there is no need to synchronize as long as actions are
            atomic and follow a deterministic order.
*/
