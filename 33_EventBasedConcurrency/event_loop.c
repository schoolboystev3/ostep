#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/timerfd.h>
#include <unistd.h>
#include <poll.h>
#include <fcntl.h>

/*
 * Gemini Coding Assignment: Write a single-threaded C program that monitors two different inputs
 * at the same time without using any threads and whitout "blocking."
 * 
 */

int main() {
    // 1. Open the slow file for reading (this file will be written to every 5 secs)
    int fd_file = open("/tmp/slow_file", O_RDONLY | O_NONBLOCK);
    lseek(fd_file, 0, SEEK_END); // move offset to end of file

    // 2. Setup the pollfd array (one for stdin, one for the file)
    struct pollfd fds[2];

    // 3. Don't watch the file directly, use a timer to prevent spinning on it
    int tfd = timerfd_create(CLOCK_MONOTONIC, 0);
    struct itimerspec its;
    its.it_value.tv_sec = 5; // First expiration
    its.it_value.tv_nsec = 0;
    its.it_interval.tv_sec = 5; // 5 second period
    its.it_interval.tv_nsec = 0;
    timerfd_settime(tfd, 0, &its, NULL);

    // Watch STDIN for data
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;

    // Watch the slow file for data
    fds[1].fd = tfd;
    fds[1].events = POLLIN;

    printf("Event Loop Starting... Type something or wait for the file!\n");

    while (1) {
        int ret = poll(fds, 2, -1);

        if (ret < 0) {perror("poll"); break; }

        // Check STDIN
        if (fds[0].revents & POLLIN) {
            char buf[1024];
            int n = read(STDIN_FILENO, buf, sizeof(buf) - 1);
            if (n > 0) {
                buf[n] = '\0';
                printf("Keyboard: %s", buf);
            }
        }

        // Check the Timer
        if (fds[1].revents & POLLIN) {
            // Clear the timer
            uint64_t expirations;
            read(tfd, &expirations, 8);

            // Now check the file
            char buf[1024];
            int n = read(fd_file, buf, sizeof(buf));
            if (n > 0) {
                printf("File Updated: %s", buf);
            }
        }
    }
    return 0;
}
