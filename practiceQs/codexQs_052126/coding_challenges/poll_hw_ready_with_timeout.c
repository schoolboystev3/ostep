/*
 * Requirements:

Read the register once per loop.
Return 0 if FW_READY is set.
Return -2 if FW_ERROR or FW_FATAL is seen.
Return -1 on timeout.
Follow-ups:

Should firmware spin, sleep, or yield to an RTOS scheduler?
    // Answer: Really depends on expectation of timing.
    //         For example, if max_iters is quick we can spin
    //         If expect time before status_reg changes can sleep
    //         Yield if time is very long and context switch is worthwhile
What telemetry would you log on timeout?
    // Answer: Probably want to print the status_reg value
    //         Are there other related registers we can read
    //         Think about if we should be expecting timeouts
    //         If not why, If so what's the reason
What if READY and ERROR are set simultaneously?
    // Answer: Error should be reported and function should return -2.
 */

#define FW_READY       (1u << 0)
#define FW_ERROR       (1u << 1)
#define FW_FATAL       (1u << 2)

int wait_fw_ready(volatile uint32_t *status_reg, unsigned max_iters) {

    uint32_t val = 0;
    
    for (unsigned i = 0; i < max_iters; i++) {
        val = *status_reg;
        if ((val & FW_FATAL) || (val & FW_ERROR)) {
            if (val & FW_FATAL) {
                // Log FW_FATAL
            }
            if (val & FW_ERROR) {
                // Log FW_ERROR
            }
            return -2;
        }
        if (val & FW_READY) {
            return 0;
        }
    }
    return -1;
}

