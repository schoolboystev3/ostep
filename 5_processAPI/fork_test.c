#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>

// Last exercise child1 stdout -> child2 stdin

int main() {
    int pipe_fd[2];
    char* msg1 = "hellooo";

    if (pipe(pipe_fd) < 0) {
        exit(1);
    }

    int rc1 = fork();

    if(rc1 < 0) {
        fprintf(stderr, "fork failed\n");
        exit(1);
    } else if(rc1 == 0) {
        //child1 writer
        if (dup2(pipe_fd[1], STDOUT_FILENO) == -1) {
            exit(1);
        }
        write(STDOUT_FILENO, msg1, strlen(msg1));
        close(STDOUT_FILENO);
    } else {
        //parent
        int rc2 = fork();

        if (rc2 < 0) {
            fprintf(stderr, "fork failed\n");
            exit(1);
        } else if (rc2 == 0) {
            //child2
            if (dup2(pipe_fd[0], STDIN_FILENO) == -1) {
                exit(1);
            }
            char* buff = (char*)calloc(strlen(msg1), sizeof(char));
            read(STDIN_FILENO, buff, strlen(msg1));
            printf("This is what child2 read: %s\n", buff);
        } else {
            //parent
            int status1, status2;
            waitpid(rc1, &status1, 0);
            waitpid(rc2, &status2, 0);
        }
    }
}
//int main() {
//    printf("hello (pid:%d)\n", (int) getpid());
//    pipe(pipe_fd);
//    int rc = fork();
//    int fd = open("test.txt", O_RDWR | O_APPEND);
//    int wait_rc = 11;
//
//    if(rc < 0) {
//        fprintf(stderr, "fork failed\n");
//        exit(1);
//    } else if(rc == 0) {
//        // Child
//        close(STDOUT_FILENO);
//        printf("child (pid:%d, fd:%d)\n", (int) getpid(), fd);
//        write(fd, "child says hello.\n", strlen("child says hello.\n"));
//        execl("/bin/ls" , "", NULL);
//    } else {
//        // Parent
//        wait_rc = wait(NULL);
//        printf("parent of %d (pid:%d, fd:%d, wait_rc:%d)\n", rc, (int) getpid(), fd, wait_rc);
//        write(fd, "parent says hello.....\n", strlen("parent says hello.....\n"));
//        close(fd);
//    }
//}

