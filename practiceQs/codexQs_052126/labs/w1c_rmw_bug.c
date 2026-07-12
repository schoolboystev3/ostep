// ### Lab 8: W1C Read-Modify-Write Bug
 
// Prompt:
 
// > Error bits disappear before the diagnostic task can record them.
 
// ```c
 #include <stdint.h>
 
 #define STAT_DONE    (1u << 0) // W1C
 #define STAT_ERROR   (1u << 1) // W1C
 #define STAT_FATAL   (1u << 2) // W1C
 #define STAT_BUSY    (1u << 3) // read-only
 
 void clear_done(volatile uint32_t *status)
 {
     uint32_t v = *status;
     v &= ~STAT_DONE;
     *status = v;
 }

/*
 ```
 
 Questions:
 
 - Why does this accidentally clear other W1C bits?
    Answer: Between reading and writing, it's possible that the status register
            may have changed. When we write back to status, we may be writing
            1s that are clearing these bits.
 - What should be written to clear only `STAT_DONE`?
    Answer: This code should only write 1 to the STAT_DONE bit of status
            and always write 0s to everything else.
 - How should diagnostic code snapshot errors before clearing?
    Answer: Read the register just once. Clear only set bits.
            Handle errors on the stack after.
 
 Expected fix direction:
 
 ```c
 *status = STAT_DONE;
 ```
 
 For diagnostics, read/snapshot first, then clear only the bits intended.
 */
