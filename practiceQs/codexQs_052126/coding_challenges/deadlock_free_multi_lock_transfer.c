/*
### Challenge 21: Deadlock-Free Multi-Lock Transfer

Implement a function that must lock two objects without deadlocking. This is a compact way to practice lock ordering and cleanup on error paths.

Requirements:

- Return `0` on success.
- Return `-1` if either pointer is `NULL`.
- Return `-2` if `amount <= 0`. 
- Return `-3` if `from->value < amount`. 
- If `from == to`, return `0` without changing the value.
- Lock both objects before reading or modifying either value.
- Prevent deadlock by always locking in increasing `id` order.
- Unlock every mutex on every return path.
- On success, subtract `amount` from `from->value` and add it to `to->value`.
*/

struct locked_counter {
    uint32_t id;
    int64_t value; 
    pthread_mutex_t mu;
};

int transfer_value(struct locked_counter *from,
                   struct locked_counter *to,
                   int64_t amount) {
    if (!from || !to) {
        return -1;
    }

    if (amount <= 0) {
        return -2;
    }

    if (from == to) {
        return 0;
    }

    pthread_mutex_t *first;
    pthread_mutex_t *second;
    if (from->id < to->id) {
        first = &from->mu;
        second = &to->mu;
    } else if (from->id > to->id) {
        first = &to->mu;
        second = &from->mu;
    } else {
        if ((uintptr)from < (uintptr) to) {
            first = &from->mu;
            second = &to->mu;
        } else {
            first = &to->mu;
            second = &from->mu;
        }
    }

    pthread_mutex_lock(first);
    pthread_mutex_lock(second);

    if (from->value < amount) {
        pthread_mutex_unlock(second);
        pthread_mutex_unlock(first);
        return -3;
    }

    from->value -= amount;
    to->value += amount;

    pthread_mutex_unlock(second);
    pthread_mutex_unlock(first);
    return 0;
}
/*
 *  Follow-ups:

 - Why does consistent lock ordering prevent deadlock?
    Answer: It attacks the circular wait dependency requirement for deadlock
 - What if two objects have the same `id`?
    Asnwer: If we can't gaurentee unique id, we can use another invariable
            the address of the objects we are interacting with.
 - How would you prevent overflow in `to->value + amount`?
    Answer: We can check explicitly if the value will overflow. If it did
            overflow the value would go into the negatives. We should 
            protect against this with a specific failure code.
 - How would this pattern apply to two device queues or two firmware states?
    Answer: The same pattern can be used in these cases where there are two
            shared objects and we want to prevent race conditions.
 - What are the tradeoffs between coarse-grained and fine-grained locking?
    Answer: coarse grain is obviously easier (and dead lock free) 
            but fine-grain allows granularity and flexibility if you only 
            need a subset of the shared resources.
*/
