#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[]) {

    if (argc < 3) {
        fprintf(stderr, "Incorrect Args");
        return 1;
    }

    // File usage example mytail -2 file1
    int num_lines = atoi(argv[1]+1) + 1; // Go till N+1
    int lines_found = 0;
    char *filepath = argv[2];
    int fd = open(filepath, O_RDONLY);

    struct stat sb = {0};
    if (stat(filepath, &sb))
        perror("Error getting file stats");

    int file_size = sb.st_size;
    int curr_pos = file_size;
    int tail_start_pos = 0; // default print whole file
    char buffer[4096] = {0};
    int bytes_read = 0;
    
    // Find the tail_start_pos
    while ((lines_found < num_lines) && (curr_pos > 0)) {
        int read_size = (file_size < 4096) ? file_size : 4096;
        curr_pos -= read_size;

        lseek(fd, (-1 * read_size), SEEK_END);
        bytes_read = read(fd, buffer, read_size);

        //printf("%d\n", bytes_read); // HMM
        for (int i = bytes_read; i >= 0; i--) {
            if (buffer[i] == '\n') {
                //printf("found: %d, goal %d\n", lines_found, num_lines);
                lines_found++;
            }
            if (lines_found == num_lines) {
                tail_start_pos = curr_pos + i + 1;
                break;
            }
        }
    }
    // Print starting at tail_start_pos
    lseek(fd, tail_start_pos, SEEK_SET);
    while((bytes_read = read(fd, buffer, 4096)) > 0) {
        write(STDOUT_FILENO, buffer, bytes_read);
    }
    
    return 0;
}
