/* 
Requirements:
 
 - Read `status` once.
 - Ignore masked-off events. A mask bit of 1 means the event is enabled.
 - Increment the appropriate counters.
 - Set `needs_recovery` for `IRQ_FATAL` or `IRQ_LINK_DOWN`.
 - Clear only the events you handled.
 - Do not use read-modify-write to clear W1C bits.
 
 ### Challenge 5D: Interrupt Status Handler
 
 A firmware block reports events through a status register. Bits are write-one-to-clear.

 Follow-ups:

 - What if a new event arrives between the status read and clear?
    Answer: We read the status once at the beginning and only clear set
            bits during that read. If new events arrive subsequently
            the hw interrupt line will remain high and we will 
            immediately jump back into this handler to handle it.
 - Should fatal/link-down mask further interrupts until recovery runs?
    Answer: Yes, we may want to turn off interrupts from this hw because
            if there is a fatal error or the link is down, those interrupts
            are second to recovering the link.
 - Which work belongs in the ISR versus a deferred task?
    Answer: The ISR should be fast. It can collect some basic telemetry
            and clear the event bits but handling actual errors or
            running the recovery sequence should be deffered to a task.
 - How would you avoid losing diagnostic state?
    Answer: I'm not sure what "losing" means in this case or what diagnostic
            state refers to.
            I see -- there may be peripheral useful metadata registers to
            help debug issues. They should also be snapshotted by the
            interrupt handler.
 - What if this handler races with reset?
    Answer: If we are performing a reset, it feels safe to assume we can
            turn off interrupts until the device is back up.
            "Race Conditions" remember to use this word.
 
 ```c
 */

#define IRQ_CMD_DONE    (1u << 0) // W1C
#define IRQ_ERROR       (1u << 1) // W1C
#define IRQ_FATAL       (1u << 2) // W1C 
#define IRQ_LINK_DOWN   (1u << 3) // W1C

struct irq_regs {
    volatile uint32_t status;
    volatile uint32_t mask;
};

struct irq_events {
    uint32_t cmd_done_count;
    uint32_t error_count;
    uint32_t fatal_count;
    uint32_t link_down_count;
    bool needs_recovery;
};

void handle_irq(struct irq_regs *regs, struct irq_events *events) {
    uint32_t active_interrupts = regs->status & regs->mask;
    uint32_t bits_to_clear = 0;

    if (active_interrupts & IRQ_FATAL) {
        events->fatal_count++;
        events->needs_recovery = true;
        bits_to_clear |= IRQ_FATAL;
    }
    if (active_interrupts & IRQ_LINK_DOWN) {
        events->link_down_count++;
        events->needs_recovery = true;
        bits_to_clear |= IRQ_LINK_DOWN;
    }
    if (active_interrupts & IRQ_ERROR) {
        events->error_count++;
        bits_to_clear |= IRQ_ERROR;
    }
    if (active_interrupts & IRQ_CMD_DONE) {
        events->cmd_done_count++;
        bits_to_clear |= IRQ_CMD_DONE;
    }

    regs->status = bits_to_clear;
}

