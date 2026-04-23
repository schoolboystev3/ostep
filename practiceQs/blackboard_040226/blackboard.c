/*
 * The Goal: Create two separate processes that communicate by writing to 
 * the same physical page of memory.
 * Write a program where a prent process and a child process share a single
 * integer in memory. 
 * Parent should sit in a loop, incrementing the int once per sec
 * Child should sit in a loop, printing the value of the int once per sec
 *
 * Constraints: Cannot use a file or pipe(), must use mmap()
 *
 */

#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <stdio.h>

int main() {
    int *blackboard = (int *)mmap(NULL, 4096, PROT_READ | PROT_WRITE,
            MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    int num_seconds = 5;

    pid_t pid = fork();
    if (pid < 0) {
        fprintf(stderr, "Fork failed\n");
        return 1;
    } else if (pid == 0) {
        // Child
        for (int i = 0; i < num_seconds; i++) {
            printf("Blackboard says %d\n", *blackboard);
            sleep(1);
        }

    } else {
        // Parent
        for (int i = 0; i < num_seconds; i++) {
            *blackboard += 1;
            sleep(1);
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
