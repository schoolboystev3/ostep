#include <sys/stat.h>
#include <stdio.h>


int main(int argc, char *argv[]) {

    if (argc != 2) {
        fprintf(stderr, "Only 1 filename Input allowed");
        return 1;
    }

    // filename can be a file or directory
    char *filename = argv[1];
    struct stat sb = {0};

    if (stat(filename, &sb) != 0)
        perror("stat failed");

    printf("Inode Number: %li\nSize: %li\nBlksize: %i\n"
           "Blocks:%li\n", sb.st_ino, sb.st_size,
           sb.st_blksize, sb.st_blocks);

    return 0;

}
