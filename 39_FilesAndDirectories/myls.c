#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <stdbool.h>

int main(int argc, char *argv[]) {
    char* filename;
    int opt;
    bool verbose;

    // Handle options
    while ((opt = getopt(argc, argv, "l")) != -1) {
        if (opt == 'l')
            verbose = true;
    }

    // Handle naked args
    if (optind != argc) {
        filename = argv[optind]; // if there is naked arg, use
    } else {
        filename = "."; // default CWD
    }

    DIR *d = opendir(filename);
    struct dirent *entry = readdir(d);
    struct stat sb = {0};

    while(entry) {
        if (verbose) {
            stat(entry->d_name, &sb);
            printf("%u  %u  %u  %s\n", sb.st_uid, sb.st_gid, sb.st_mode, entry->d_name);
        } else {
            printf("%s  ", entry->d_name);
        }
        entry = readdir(d);
    }
    printf("\n");

    closedir(d);
    return 0;
}
