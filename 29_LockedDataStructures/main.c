#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include "counter.h"

#define PER_THREAD_INCREMENT 10000000 //10mil

typedef struct _thread_arg {
    counter_t* counter;
    int thread_id;
} thread_args_t;

static double get_elapsed_time(struct timeval start, struct timeval end) {
    double elapsed_s = end.tv_sec - start.tv_sec;
    double elapsed_us = end.tv_usec - start.tv_usec;

    return (elapsed_s + ((double)elapsed_us/1e6));
}

static void* increase_counter(void *arg) {
    thread_args_t *thread_args = (thread_args_t*)arg;
    for(int i = 0; i < PER_THREAD_INCREMENT; i++) {
        counter_increment(thread_args->counter, thread_args->thread_id);
    }
}

//TODO: check all rc
int main() {

    // Will create a thread for each vCPU (on VM)
    // or each logical core (on BM) and add affinity to it
    int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    pthread_t threads[num_cores];
    pthread_attr_t attr[num_cores];
    cpu_set_t cpuset[num_cores];

    // For timing
    struct timeval start, end;
    gettimeofday(&start, NULL);

    counter_t *counter = counter_init(num_cores);
    thread_args_t thread_args[num_cores];

    for(int i = 0; i < num_cores; i++) {

        // Set affnity & attr
        CPU_ZERO(&cpuset[i]);
        CPU_SET(i, &cpuset[i]);
        pthread_attr_init(&attr[i]);
        pthread_attr_setaffinity_np(&attr[i], sizeof(cpu_set_t), &cpuset[i]);
        
        // Set thread_args
        thread_args[i].counter = counter;
        thread_args[i].thread_id = i;

        pthread_create(&threads[i], &attr[i], increase_counter, &thread_args[i]);
    }

    for(int i = 0; i < num_cores; i++) {
        pthread_join(threads[i], NULL);
        pthread_attr_destroy(&attr[i]);
    }
    gettimeofday(&end, NULL);

    printf("Num Threads: %d Final Counter: %d Elapsed Time: %f \n",
            num_cores, counter_get(counter), get_elapsed_time(start, end));

    counter_destroy(counter);
}
