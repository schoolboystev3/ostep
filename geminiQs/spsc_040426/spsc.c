/*
 * The Scenario: You are building a high-frequency trading (HFT) logger. 
 * You have a Producer (Parent) generating "Trade" data and a Consumer (Child) 
 * that needs to process it. You want to pass data without using a slow Pipe or Socket.
 *
 * Ring Buffer: array of 10 ints
 * Parent: Writes nums into buffer. If buffer full, spin-wait
 * Child: Read nums from buffer and prints. If buffer empty, spin-wait
 *
 * Constraint: Cannot use Mutex, must use Atomics.
 */

/* Release (for Atmoic Store): Preceding operations cannot be reordered past it.
 *                             I'm done with data; here's the signal.
 * Acquire (for Atomic Load) : Following operations cannot be reordered before it
 *                             I see the signal; now I can safely read the data.
 */

#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdatomic.h>
#include <stdint.h>

#define RB_LEN 10

typedef struct rb {
    int buff[RB_LEN];
    atomic_int head;
    atomic_int tail;
} rb_t;

int main() {

    rb_t *rb = (rb_t*)mmap(NULL, sizeof(rb_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    uint64_t max_count = 10000000;

    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork Failed.\n");
        return 1;
    } else if (pid == 0) {
        // Child (Consumer)
        int child_counter = 0;
        int local_tail = 0; // local copy to avoid constant syncing across cores
        uint64_t entry_sum = 0;

        // TODO batching production and consumption
        while(child_counter < max_count) {
            // Check if rb is not Empty
            if (local_tail != atomic_load_explicit(&rb->head, memory_order_acquire)) {
                entry_sum += rb->buff[local_tail];
                child_counter++;
                local_tail = ((local_tail + 1) % RB_LEN);
                atomic_store_explicit(&rb->tail, local_tail, memory_order_release);
            }
        }
        printf("Final Sum is %lu, should be %lu\n", entry_sum, ((max_count-1) * ((max_count-1) + 1)/2));
    } else {
        // Parent (producer)
        int parent_counter = 0;
        int local_head = 0; // local copy to avoid constant syncing across cores

        while (parent_counter < max_count) {
            // Check if rb is not full
            if (((local_head + 1) % RB_LEN) != atomic_load_explicit(&rb->tail, memory_order_acquire)) {
               rb->buff[local_head] = parent_counter;
               parent_counter++;
               local_head = ((local_head + 1) % RB_LEN);
               atomic_store_explicit(&rb->head, local_head, memory_order_release);
            }
        }
        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            printf("Child %d exited with code %d.\n", 
                    pid, WEXITSTATUS(status));
        }
    }
    return 0;
}
