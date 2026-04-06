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

#define RB_LEN 10

typedef struct rb {
    int buff[RB_LEN];
    atomic_int head;
    atomic_int tail;
} rb_t;

int main() {

    rb_t *rb = (rb_t*)mmap(NULL, sizeof(rb_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    double max_count = 10000000;

    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork Failed.\n");
        return 1;
    } else if (pid == 0) {
        // Child (Consumer)
        int child_counter = 0;
        int local_head = 0;
        double entry_sum = 0;

        // TODO how to make this just consume until it seems like there's none left
        while(child_counter < max_count) {
            // Check if rb is not Empty
            if (local_head != atomic_load_explicit(&rb->tail, memory_order_acquire)) {
                printf("Consuming %d\n", rb->buff[local_head]);
                entry_sum += rb->buff[local_head];
                child_counter++;
                local_head = ((local_head + 1) % 10);
                atomic_store_explicit(&rb->head, local_head, memory_order_release);
            }
        }
        printf("Final Sum is %f, should be %f\n", entry_sum, ((max_count-1) * ((max_count-1) + 1)/2));
    } else {
        // Parent (producer)
        int parent_counter = 0;
        int local_tail = 0; // local copy to avoid constant syncing across cores

        while (parent_counter < max_count) {
            // Check if rb is not full
            if (((local_tail + 1) % 10) != atomic_load_explicit(&rb->head, memory_order_acquire)) {
               rb->buff[local_tail] = parent_counter;
               parent_counter++;
               local_tail = ((local_tail + 1) % 10);
               atomic_store_explicit(&rb->tail, local_tail, memory_order_release);
            }
        }
        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            printf("Child %d exited with code %d\n", 
                    pid, WEXITSTATUS(status));
        }
    }
    return 0;
}
