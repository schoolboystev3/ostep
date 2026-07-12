/* ## Scenario 7: Interrupt Storm After Error

### Prompt

A firmware block starts generating an interrupt storm after an error event. CPU time is consumed by the ISR, and the recovery task barely runs.

Debug the handler.

### Artifact

```c
*/
#include <stdint.h>
#include <stdbool.h>

#define IRQ_DONE      (1u << 0) // W1C
#define IRQ_ERROR     (1u << 1) // W1C
#define IRQ_FATAL     (1u << 2) // W1C

#define MASK_DONE     (1u << 0)
#define MASK_ERROR    (1u << 1)
#define MASK_FATAL    (1u << 2)

struct regs {
    volatile uint32_t status;
    volatile uint32_t mask;
};

struct state {
    uint32_t done_count;
    uint32_t error_count;
    bool recovery_needed;
};

void wake_recovery_task(void);
void irq_handler(struct regs *r, struct state *s)
{
    uint32_t st = r->status;

    if (st & IRQ_DONE)
        s->done_count++;

    if (st & IRQ_ERROR) {
        s->error_count++;
        s->recovery_needed = true;
        wake_recovery_task();
    }

    if (st & IRQ_FATAL) {
        s->recovery_needed = true;
        wake_recovery_task();
    }

    /* Clear done events. Error recovery task will clear errors later. */
    r->status = IRQ_DONE;
}
/*
```

Observed:

```text
status repeatedly reads 0x2
mask   reads 0x7
ISR entry count increases rapidly
recovery task is ready but barely runs
```

### Practice Questions
 - Which status bit is repeatedly asserted?
    Answer: Error
 - Why does the interrupt keep firing?
    Answer: Might be because the recovery task is not clearing the
            bits or taking too long to do so
 - What work is too heavy or risky inside the ISR?
    Answer: recovery should not be done as is in the top half
 - Should the handler clear, mask, or defer the error?
    Answer: Clear the error and wake up the recovery task
 - What is the role of the recovery task?
    Answer: Should not be clearing bits, instead disable ints and recover

    I understand now. You don't have to clear the error bit necessairly
    You can adjust the mask so that these errors are filtered until
    heavy recovery is performed. The problem is there's a storm because
    the bits aren't cleared or mask so it continously refires the handler
    and the the recovery task is starved.
*/
