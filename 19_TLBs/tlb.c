#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

enum { NS_PER_SECOND = 1000000000 };

void sub_timespec(struct timespec t1, struct timespec t2, struct timespec *td)
{
    td->tv_nsec = t2.tv_nsec - t1.tv_nsec;
    td->tv_sec  = t2.tv_sec - t1.tv_sec;
    if (td->tv_sec > 0 && td->tv_nsec < 0)
    {
        td->tv_nsec += NS_PER_SECOND;
        td->tv_sec--;
    }
    else if (td->tv_sec < 0 && td->tv_nsec > 0)
    {
        td->tv_nsec -= NS_PER_SECOND;
        td->tv_sec++;
    }
}

double timespec_to_double(struct timespec *t)
{
    return (double)t->tv_sec + ((double)t->tv_nsec / NS_PER_SECOND);
}

int main(int argc, char* argv[]) {
    
    if (argc < 1) {
        return -1;
    }

    int page_size = getpagesize();
    int jump = page_size / sizeof(int);
    int num_pages = atoi(argv[1]);
    int num_trails = atoi(argv[2]);
    struct timespec start, finish, delta;
    double result;
    volatile int *a = malloc(num_pages * jump);
    int dummy_sum = 0;

    clock_gettime(CLOCK_REALTIME, &start);

    for (int i = 0; i < num_trails; i++) {
        for (int j = 0; j < num_pages; j+=jump) {
            dummy_sum += a[j];
        }
    }

    clock_gettime(CLOCK_REALTIME, &finish);
    sub_timespec(start, finish, &delta);
    result = timespec_to_double(&delta);
    if (dummy_sum != 123) {printf("hi\n");}
    printf("delta: %g Average Access Time: %g\n", result, (result / ((double)num_pages * num_trails)));
    free((void *) a);
    return 0;
}
