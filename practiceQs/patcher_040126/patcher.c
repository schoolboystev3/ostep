/* 
 * The Wednesday Challenge: "The Binary Surgeon"
 * The Goal: Write a program called patcher that takes three arguments: 
 * a filename, a byte offset, and a single character. 
 * The program must swap the character at that exact offset with the new
 * one without truncating the file or rewriting the whole thing.
 *
 * The Twist: You must use the pwrite() system call and not lseek().
 *
 * ./patcher my_file.txt 1024 'Z' // change byte 1024 to Z
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>

int main(int argc, char* argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Input arguements Invalid");
        return 1;
    }

    int fd = open(argv[1], O_WRONLY);
    if (fd < 0) {
        perror("Failed to open file");
        return 1;
    }

    struct stat sb = {0};
    if (fstat(fd, &sb)) {
        perror("Failed to get file stats");
        return 1;
    }

    char *endptr;
    errno = 0;
    long offset = strtol(argv[2], &endptr, 10);
    if (errno == ERANGE || (errno != 0 && offset == 0)) {
        perror("strol overflow/underflow");
        return 1;
    }

    if (endptr == argv[2] || *endptr != '\0') {
        fprintf(stderr, "Error: '%s' is not a valid numeric offset \n", argv[2]);
        return 1;
    }

    if (offset < 0) {
        fprintf(stderr, "Error: Offset cannot be negative\n");
        return 1;
    }

    if (pwrite(fd, argv[3], 1, offset) < 0) {
        perror("Failed to write to file");
        return 1;
    }

    close(fd);
    return 0;
}
