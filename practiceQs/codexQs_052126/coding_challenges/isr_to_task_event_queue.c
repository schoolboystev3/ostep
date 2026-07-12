/* 
Requirements:
 
 - Use power-of-two indexing.
 - Return false if full/empty.
 - Do not block in the ISR push path.
 - Assume single ISR producer and single task consumer.


 ### Challenge 5B: ISR-To-Task Event Queue
 
 You have a tiny firmware event queue shared by an ISR producer and a task consumer.
 
 ```c
  Follow-ups:

 - What changes if multiple ISRs can push?
    Answer: we would need a synchronization between ISRs so they
            do not step on each others feet in race conditions.
            Introduce a lock.
 - Where would you place memory barriers?
    Answer: When reading or writing to head, we will need to 
            place barriers so things are done in the proper order
            and all tasks get updated values.
 - How does the task get woken after an event is pushed?
    Answer: I assume the task is polling on this event q?
 - What happens if the queue overflows during an interrupt storm?
    Answer: With the current implementation, the interrupts will be
            lost. How do we prevent that from happening? I forget.
 - Which events are safe to drop, and which are not?
    Answer: Not sure how to answer this. All these events seem
            quite crucial.
 */
#define EVT_Q_CAP 64
#define EVT_Q_MASK (EVT_Q_CAP - 1)

enum event_type {
    EVT_LINK_UP,
    EVT_LINK_DOWN,
    EVT_CMD_DONE,
    EVT_ERROR,
};

struct event {
    enum event_type type;
    uint32_t data;
};

struct event_q {
    _Atomic uint32_t head; // written by ISR
    _Atomic uint32_t tail; // written by task
    struct event entries[EVT_Q_CAP];
};

// TODO: Use c11 atomics to make this safe.
//       ISR could interrupt the task at anytime, we need to make
//       sure that these operations happen in the correct order.
//       ie we don't increment tail before grabbing data.

bool event_q_push_from_isr(struct event_q *q, struct event e) {
    if ((q->head - q->tail) == EVT_Q_CAP) {
        return false;
    }
    q->entries[q->head & EVT_Q_MASK] = e;
    q->head++;
    return true;
}

bool event_q_pop_from_task(struct event_q *q, struct event *out) {
    if (q-> head == q->tail) {
        return false;
    }
    *out = q->entries[q->tail & EVT_Q_MASK];
    q->tail++;
    return true;
}
