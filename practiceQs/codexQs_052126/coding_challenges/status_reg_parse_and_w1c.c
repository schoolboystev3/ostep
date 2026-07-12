/*
 * Requirements:

Extract state correctly.
Clear only DONE, ERROR, and TIMEOUT.
Do not clear or write read-only bits.
Follow-ups:

Why is *status_reg &= ~STAT_ERROR wrong for W1C registers?
    Answer: to clear the register you must write 1 to it, not set it to 0.
What if reading status has side effects?
    Answer: If reading clears the register then make sure to save the state.
            Check ready and busy on the same read.
 */

#define STAT_READY       (1u << 0)  // read-only
#define STAT_BUSY        (1u << 1)  // read-only
#define STAT_DONE        (1u << 2)  // write 1 to clear
#define STAT_ERROR       (1u << 3)  // write 1 to clear
#define STAT_TIMEOUT     (1u << 4)  // write 1 to clear
#define STAT_STATE_MASK  (0xfu << 8) // bits [11:8]
#define STAT_STATE_SHIFT 8

bool status_ready(uint32_t status) {
    return (status & STAT_READY);
}
bool status_busy(uint32_t status) {
    return (status & STAT_BUSY);
}

uint32_t status_state(uint32_t status) {
    return ((status & STAT_STATE_MASK) >> STAT_STATE_SHIFT);
}

void clear_done_error_timeout(volatile uint32_t *status_reg) {
    *status_reg = (STAT_DONE | STAT_ERROR | STAT_TIMEOUT);
}
