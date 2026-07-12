/*
### Challenge 10: Safe Targeted Reset
 Requirements:
 
 - `target_id` must fit in 8 bits.
 - If `STATUS_BUSY` is set before reset, return `-1`.
 - Write target ID and `CTRL_RESET_REQ`.
 - Poll until `STATUS_RESET_DONE`.
 - Return `-2` on `STATUS_ERROR`.
 - Return `-3` on timeout.
 
 Registers:
 
 ```c 
*/ 
 #define CTRL_RESET_REQ       (1u << 0)
 #define CTRL_TARGET_MASK     (0xffu << 8)
 #define CTRL_TARGET_SHIFT    8
     
 #define STATUS_BUSY          (1u << 0)
 #define STATUS_RESET_DONE    (1u << 1) 
 #define STATUS_ERROR         (1u << 2)
 
 //Register block:
 
 struct regs {
     volatile uint32_t ctrl;
     volatile uint32_t status;
 };
 
 int reset_target(struct regs *r, uint32_t target_id, unsigned max_iters) {
    if (target_id > (CTRL_TARGET_MASK >> CTRL_TARGET_SHIFT)) {
        return -4; // Error target_id too large
    }
    uint32_t status = ioread32(r->status);
    if (status & STATUS_BUSY) { 
        return -1;
    }

    //write target id and reset req
    uint32_t tmp = ioread32(r->ctrl);
    tmp &= ~(CTRL_TARGET_MASK | CTRL_RESET_REQ);
    tmp |= (target_id << CTRL_TARGET_SHIFT) | CTRL_RESET_REQ;
    iowrite32(r->ctrl, tmp);

    for (unsinged i = 0; i < max_iters; i++ ) {
        status = ioread32(r->status);
        if (status & STATUS_ERROR) {
            return -2;
        }
        if (status & STATUS_RESET_DONE) {
            return 0;
        }
        sleep(1);
    }
    return -3; //timeout
 }
/*
 *  Follow-ups:
 
 - How does this relate to the broader class of management APIs you saw at SambaNova?
    Answer: Looks like the type of functionality that could be exposed in a user space
            daemon. Obviously with protections because resetting hardware is a
            privileged command.
 - What does the daemon validate before issuing this?
    Answer: We are told just to check target_id but all input arguments should be 
            validated and hardware state also needs to be validated.
 - What does the kernel/firmware validate?
    Answer: Will validate that hardware access is allowed for this user. That
            this register is within valid range. Forward the access to the driver
            properly.
 - How do you prevent external abuse of reset controls?
    Answer: Important to have careful state management and access/input validation
            so that user do not abuse. Safety limits set as per hardware spec.
 */
