/*
 * Requirements:

Preserve unrelated bits.
mode must fit in 3 bits.
timeout must fit in 8 bits.
Return 0 on success and -1 for invalid input.
Follow-ups:

What if reg points to MMIO?
    Answer: Probably want to mark this reg as volatile. Would need special function to access these registers?
What if CTRL_RESET_REQ is write-one-to-start and should not be preserved during read-modify-write?
    Answer: During any RMW always set CTRL_RESET_REQ to 0 as well.
What locking is needed if an ISR or another task can update the same register?
    Answer: If it's an ISR, disable interrupts and re-enable
            If it's another task, disable interrupts and grab spin lock
                Prevent ISR from causing deadlock
*/

#define CTRL_ENABLE          (1u << 0)
#define CTRL_RESET_REQ       (1u << 1)
#define CTRL_MODE_MASK       (0x7u << 4)    // bits [6:4]
#define CTRL_MODE_SHIFT      4
#define CTRL_TIMEOUT_MASK    (0xffu << 8)   // bits [15:8]
#define CTRL_TIMEOUT_SHIFT   8

int ctrl_set_mode(uint32_t *reg, uint32_t mode) {
    if (mode > 7) {
        return -1;
    }

    // use tmp in case volatile, for atomicity, minimize hw writes
    uint32_t tmp = *reg;
    tmp &= ~(CTRL_MODE_MASK);
    tmp |= (mode << CTRL_MODE_SHIFT);
    *reg = tmp;

    return 0;
    
}

int ctrl_set_timeout(uint32_t *reg, uint32_t timeout) {
    if (timeout > 255) {
        return -1;
    }

    // use tmp in case volatile, for atomicity, minimize hw writes
    uint32_t tmp = *reg;
    tmp &= ~(CTRL_TIMEOUT_MASK);
    tmp |= (timeout << CTRL_TIMEOUT_MASK);
    *reg = tmp;
    
    return 0;
}


