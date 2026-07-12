 //### Lab 7: Missing Barrier Before Doorbell 
 
// Prompt:
 
// > Firmware sometimes sees a doorbell but reads an old or partially written command descriptor.
 
// ```c
 #include <stdint.h>
 
 struct cmd_desc {
     uint32_t opcode;
     uint32_t len;
     uint64_t addr;
 };
 
 struct queue {
     struct cmd_desc descs[256];
     volatile uint32_t *doorbell;
     uint32_t head;
 };
 
 void submit_cmd(struct queue *q, struct cmd_desc *cmd)
 {
     q->descs[q->head & 255] = *cmd;
     q->head++;
     *q->doorbell = q->head; 
 } 

/*
 ```
 
 Questions:
 
 - What must be visible before ringing the doorbell?
    Answer: The command must first be written to the q->descs
 - Why is `volatile` not enough?
    Answer: volatile indicates to the compiler that doorbell may
            be r/w by the HW. But it doesn't prevent the compiler
            from performing code reordering optimizations.
 - Where might a memory barrier belong?
    Answer: We could use a atomic_store_explicit when setting the
            doorbell so that the writes prior complete before the
            doorbell is rung.
 - What if hardware and CPU have different cache/coherency rules?
    Answer: I'm not exactly sure what this means.
 
 Expected fix direction:
 
 - Write descriptor.
 - Ensure descriptor writes are visible with the platform-appropriate barrier/cache operation.
 - Publish head / ring doorbell after that.
 - Use driver/OS-provided MMIO accessors where applicable.
 */
