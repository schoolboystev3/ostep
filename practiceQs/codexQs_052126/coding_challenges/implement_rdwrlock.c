/*
 * Question: Implement a Read-Write Lock using only Mutexes and Condition Variables
What it tests: Ability to manage complex thread state and coordinate multiple wait queues manually.

The Core Challenge: A candidate must implement rwlock_rdlock, rwlock_wrlock, and their respective unlocks. They have to correctly handle tracking the count of active readers and waiting writers, deal with the conditional predicates safely, and choose a starvation policy (e.g., how to prevent a continuous stream of readers from locking out a writer permanently).
*/

// Behavior: unclaimed -- rd/wr thread can lcok
//           If rd has -- more rd can lcok, wr cannot lock
//           If wr has -- No one else can lock

typedef struct rwlock {
    size_t num_readers;
    bool locked;
    pthread_cond_t lock_is_free;
    pthread_mutex_t lock; // protects access to fields
} pthread_rwlock_t;

int pthread_rwlock_rdlock(pthread_rwlock_t *p) {
    if (!p) {
        return -1;
    }
    
    pthread_mutex_lock(&p->mu);
    if (p->num_readers > 0) {
        p->num_readers++;
        pthread_mutex_unlock(&p->mu);
        return 0;
    }

    while (p->locked) {
        pthread_cond_wait(&p->lock_is_free, &p->mu);
    }
    
    p->num_readers++;
    p->locked = true;
    pthread_mutex_unlock(&p->mu);

    return 0;
}

int pthread_rwlock_wrlock(pthread_rwlock_t *p) {
    if (!p) {
        return -1;
    }

    pthread_mutex_lock(&p->mu);
    while(p->locked) {
        pthread_cond_wait(&p->lock_is_free, &p->mu);
    }

    p->locked = true;

    pthread_mutex_unlock(&p->mu);
    return 0;
}
