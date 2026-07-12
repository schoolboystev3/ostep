/*
 * Requirements:

Validate each field fits.
Compose one 32-bit value.
Set DB_VALID.
Write exactly once to *db.
Follow-ups:

Why might a memory barrier be needed before ringing the doorbell?
    Answer: Doorbell registers are often used to signal to HW that data is ready
            Before ringing the doorbell we want to ensure the data is written.
            This can be done with a memory barrier
What data must be visible before the doorbell write?
    Answer: The data that needed to be written prior to ringing the doorbell.
How would the host/firmware acknowledge command completion?
    Answer: Host can be notified via interrupt of status polling maybe
            in the same queue where the data is was placed. 
 */

#define DB_QUEUE_MASK      (0xffu << 0)    // bits [7:0]
#define DB_QUEUE_SHIFT     0
#define DB_OPCODE_MASK     (0xffu << 8)    // bits [15:8]
#define DB_OPCODE_SHIFT    8
#define DB_SEQ_MASK        (0xfffu << 16)  // bits [27:16]
#define DB_SEQ_SHIFT       16
#define DB_VALID           (1u << 31)

int ring_doorbell(volatile uint32_t *db,
                  uint32_t queue_id,
                  uint32_t opcode,
                  uint32_t seq) {

    if (((queue_id > (DB_QUEUE_MASK >> DB_QUEUE_SHIFT))) || 
        ((opcode > (DB_OPCODE_MASK >> DB_OPCODE_SHIFT))) || 
        ((seq > (DB_SEQ_MASK >> DB_SEQ_SHIFT)))) {
        return -1;
    }

    uint32_t val = 0;
    val = (((queue_id << DB_QUEUE_SHIFT) & DB_QUEUE_MASK) |
          ((opcode << DB_OPCODE_SHIFT) & DB_OPCODE_MASK) |
          ((seq << DB_SEQ_SHIFT) & DB_SEQ_MASK) |
          DB_VALID);

    *db = val;
    return 0;
}

