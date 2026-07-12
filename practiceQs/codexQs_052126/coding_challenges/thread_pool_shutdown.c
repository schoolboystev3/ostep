/*### C11: Thread Pool Shutdown - Drain Or Cancel

Problem statement:

Design and implement the shutdown path for a small C thread pool. The interviewer is testing whether you can reason about states, blocked workers, pending jobs, active jobs, memory ownership, and         joining threads.

Ownership model:

- Before `tp_submit`, the caller owns `arg`.
- If `tp_submit` succeeds, the pool owns the queued job and `arg`.
- If a job runs, the worker calls `fn(arg)` outside the pool lock, then calls `destroy(arg)` if `destroy != NULL`.
- If a queued job is canceled before running, the shutdown path calls `destroy(arg)` if `destroy != NULL`.
- If `tp_submit` fails, ownership remains with the caller.
requirements:

- `tp_submit` returns `0` on success, `-1` for invalid input, `-2` if shutdown has started.
- `tp_submit` may block while the queue is full, but must wake and fail if shutdown starts.
- `TP_SHUTDOWN_DRAIN`: reject new jobs, let workers finish all queued and active work, then exit.
- `TP_SHUTDOWN_CANCEL_PENDING`: reject new jobs, destroy queued-but-not-running jobs, let active jobs finish, then exit.
- `tp_shutdown` wakes all blocked workers and blocked submitters.
- `tp_shutdown` joins all worker threads. Worker threads do not join themselves.
- Job callbacks must never run while holding `p->mu`.
- After `tp_shutdown` returns, no worker is running and `p->state == TP_STOPPED`.

Worker exit conditions:

- In drain mode, a worker exits when `state == TP_DRAINING && count == 0`.
- In cancel mode, a worker exits when `state == TP_CANCELING`.
- In either mode, active jobs already removed from the queue are allowed to finish.

Invariants:

- `count <= TP_CAP`
- `active` equals the number of jobs currently running outside the lock.
- `state`, `head`, `tail`, `count`, and `active` are protected by `mu`.
- A job record is owned by exactly one place: caller, queue, worker, or cancel path.

Tests:

- Submit one job and observe it runs.
- Fill the queue and verify a submitter blocks.
- Start drain shutdown with queued work; all queued jobs run before workers exit.
- Start cancel-pending shutdown with queued work; queued jobs are destroyed, not run.
- Shutdown while workers are sleeping on an empty queue.
- Shutdown while a submitter is blocked on a full queue.
- A job submits another job during drain; decide and explain whether that should fail.
- One job hangs forever; explain what your API can and cannot guarantee.
*/

#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>

#define TP_CAP 16

typedef void (*tp_job_fn)(void *arg);
typedef void (*tp_destroy_fn)(void *arg);

enum tp_state {
    TP_RUNNING,
    TP_DRAINING,
    TP_CANCELING,
    TP_STOPPED
};

enum tp_shutdown_mode {
    TP_SHUTDOWN_DRAIN,
    TP_SHUTDOWN_CANCEL_PENDING
};

struct tp_job {
    tp_job_fn fn;
    void *arg;
    tp_destroy_fn destroy;
};

struct thread_pool {
    pthread_t *threads;
    size_t nthreads;

    struct tp_job queue[TP_CAP];
    size_t head;
    size_t tail;
    size_t count;
    size_t active;

    enum tp_state state;
    pthread_mutex_t mu;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
};

int tp_submit(struct thread_pool *p,
              tp_job_fn fn,
              void *arg,
              tp_destroy_fn destroy) {
    if (!p || !fn || !arg || !destroy) {
        return -1;
    }

    pthread_mutex_lock(&p->mu);

    if (p->state != TP_RUNNING) {
        return -2;
    }

    // shutdown
    while (p->count == TP_CAP && ) {
        pthread_cond_wait(&p->not_full, &p->mu);
    }

    if (p->state != TP_RUNNING) {
        return -2;
    }

    p->queue[p->tail].fn = fn;
    p->queue[p->tail].arg = arg;
    p->queue[p->tail].destory = destroy;
    p->count++;
    p->tail = (p->tail + 1) % TP_CAP;

    pthread_cond_signal(&p->not_empty, &p-mu);

    pthread_mutex_unlock(&p->mu);
    return 0;
}

void *tp_worker_main(void *arg) {
    if (!arg) return -1;
    struct thread_pool *p = (struct thread_pool*)arg;

    pthread_mutex_lock(&p->mu);


    // shutdown
    while ((p->count == 0) && (p->state == TP_RUNNING)) {
        pthread_cond_wait(&p->not_empty, &p->mu);
    }

    if (p->state == TP_CANCELING) {
        pthread_mutex_unlock(&p-mu);
        return;
    }

    struct tp_job curr;
    curr.fn = p->queue[p->head].fn;
    curr.arg = p->queue[p->head].fn;
    curr.destroy = p->queue[p->head].destroy;
    p->count--;
    p->head = (p->head + 1) % TP_CAP;

    pthread_cond_signal(&p->not_full, &p->mu);
    
    p->active++;
    pthread_mutex_unlock(&p->mu);

    curr->fn(curr.arg);
    pthread_mutex_lock(&p->mu);
    p->active--;
    pthread_mutex_unlock(&p->mu);
}

int tp_shutdown(struct thread_pool *p, enum tp_shutdown_mode mode) {
}


