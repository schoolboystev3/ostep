/* ### Challenge 5C: Reset Race State Guard

A firmware module tracks whether commands may be submitted:

Requirements:

-  New commands may start only from `MOD_READY` or `MOD_ACTIVE`.
- `try_submit_command` increments `inflight` if accepted.
- `command_completed` decrements `inflight`.
- `begin_reset` prevents new submissions.
- `begin_reset` succeeds immediately if no commands are in flight.
- If commands are in flight, it moves the module to `MOD_RESETTING` and returns false to indicate reset is pending.
*/

enum module_state {
    MOD_READY,
    MOD_ACTIVE,
    MOD_RESETTING,
    MOD_FAULTED,
};

struct module {
    enum module_state state;
    uint32_t inflight;
    spinlock_t lock;
};

// Implement pseudocode or C-like logic:
bool try_submit_command(struct module *m) {
    bool success = false;

    lock(&m->lock);
    if (m->state == MOD_READY || m->state == MOD_ACTIVE) {
        success = true;
        m->inflight++;
    }
    unlock(&m->lock);

    return success;
}

void command_completed(struct module *m) {
    
    lock(&m->lock);
    m->inflight--;
    if (m->state == MOD_RESETTING && m->inflight == 0) {
        // perform reset now
    }
    unlock(&m->lock);
}

bool begin_reset(struct module *m) {
    bool success = true;

    lock(&m->lock);
    if (m->inflight > 0) {
        m->state = MOD_RESETTING;
        success = false;
    }
    unlock(&m->lock);
    
    return success;
}

/*
 *  Follow-ups:

 - What locks or atomics are needed?
    Answer: The state and inflight counter are shared resources so require
            special concurrency handling. Either a spinlock, mutex or atomics.
 - What if `command_completed` runs from interrupt context?
    Answer: We would need to disable interrupts for all these functions before 
            changing state and locking and reneable interrupts afterwards.
 - What if reset times out while commands remain in flight?
    Answer: We shouldn't have commands in flight before or during resets
            this policy needs to be enforced.
 - Should in-flight commands complete normally, abort, or return errors?
    Answer: Commands shouldn't be executed during resets so they should
            be aborted and return the appropriate and informative errors.
*/
