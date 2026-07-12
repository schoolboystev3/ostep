/*### C07: Worker Queue Shutdown

Problem statement:

Design and implement the core worker-loop logic for a thread pool consuming jobs from a blocking queue.

Requirements:

- Reject new jobs after stop is requested.
- Workers drain existing jobs or exit immediately; choose one policy and state it.
- Do not run job callbacks while holding the pool mutex.
- Wake all workers on shutdown.
- Join worker threads in the owner code; explain why.
*/

#define Q_MAX 16u
#define Q_MASK (Q_MAX - 1)

typedef void (*job_fn)(void *arg);

struct job {
    job_fn fn;
    void *arg;
};

struct worker_pool {
    struct job queue[16];
    size_t head;
    size_t tail; 
    size_t count;
    bool stopping;
    pthread_mutex_t mu;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
};

static inline get_queue_count(struct worker_pool *p) {
    return (p->tail - p->head);
}

int submit_job(struct worker_pool *p, job_fn fn, void *arg) {
    if (!p || !fn || !arg) {
        return -1;
    }

    // Fast fail here for cleanup
    if (p->stopping) {
        return -2;
    }

    pthread_mutex_lock(&p->mu);
    while ((get_queue_count(p) == Q_MAX) && !p->stopping) {
        pthread_cond_wait(&p->not_full, &p->mu);
    }
    
    if (p->stopping) {
        pthread_mutex_unlock(&p->mu);
        return -2;
    }

    p->queue[p->tail & Q_MASK].fn = fn;
    p->queue[p->tail & Q_MASK].arg = arg;
    p->tail++;

    pthread_cond_signal(&p->not_empty, &p->mu);
    pthread_mutex_unlock(&p->mu);

}

void request_stop(struct worker_pool *p) {
    if (!p) return;

    pthread_mutex_lock(&p->mu);
    p->stopping = true;
    pthread_cond_broadcast(&p->not_empty);
    pthread_cond_broadcast(&p->not_full);
    pthread_mutex_unlock(&p->mu);
}

// On stop, drain existing jobs
void *worker_main(void *arg) {
    if (!arg) return;

    struct worker_pool *p = (struct worker_pool *)arg;
    while (1) {
        pthread_mutex_lock(&p->mu);

        while ((get_queue_count(p) == 0) && !p->stopping) {
            pthread_cond_wait(&p->not_empty, &p->mu);
        }

        if (p->stopping && (get_queue_count == 0)) {
            pthread_mutex_unlock(&p->mu);
            break;
        }

        struct job curr;

        curr.fn = p->queue[p->head & Q_MASK].fn;
        curr.arg = p->queue[p->head & Q_MASK].arg;
        p->head++;

        pthread_cond_signal(&p->not_full, &p->mu);
        pthread_mutex_unlock(&p->mu);

        curr.fn(curr.arg);
    }
}
/*
 Follow-ups:

 - What if a job submits another job?
    Answer: unless we have a recursive mutex, this will deadlock
 - What if a job hangs?
    Answer: Right now we don't have a mechanism to handle that.
            It may be helpful to have a timer/watchdog to kill
            threads that are hanging? How is this usuall handled?
 - How would priority work?
    Answer: If we wanted to include priority in jobs, we could separate
            jobs into different queues organized by priority, which
            would make it easy to schedule jobs.
 */
