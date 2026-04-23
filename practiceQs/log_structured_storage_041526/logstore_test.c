/* 
 * File to test logstore code.
 */

#include "logstore_internal.h"
#include "logstore.h"
#include "testing.h"
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

// Loop through all keys and add them to the store twice.
// Clean the store and verify that it was cleaned.
// One thread doing both.
void simple_put_clean_test() {

    printf("Running %s\n", __func__);

    int key_range = 10;
    LogStore *store = logstore_create(2 * key_range, key_range);

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < key_range; j++) {
           logstore_put(store, j, i*j);
        }
    }

    logstore_clean(store);

    //print_logstore_buffer(store);

    int correct_store_vals = true;
    for (int j = 0; j < key_range; j++) {
        if (logstore_get_buffer_index(store, j).value != j) {
            correct_store_vals = false;
        }
    }

    EXPECT_TRUE(correct_store_vals == true);
    
    logstore_clean(store);
        
}

typedef struct t_args {
    LogStore *store;
    int reps;
} t_args;

// Put repeatedly to all keys
void* put_thread(void* args) {

    t_args *thread_args = (t_args*)args;
    LogStore *store = thread_args->store;
    int range = store->key_range;

    for (int i = 0; i < thread_args->reps; i++) {
        logstore_put(store, i % range, i);
    }

    return NULL;
}

// Get repeatedly to all keys
void* get_thread(void* args) {

    t_args *thread_args = (t_args*)args;
    LogStore *store = thread_args->store;
    int range = store->key_range;

    for (int i = 0; i < thread_args->reps; i++) {
        logstore_get(store, i % range);
    }

    return NULL;
}
    
void multi_put_get_clean_test() {
    printf("Running %s\n", __func__);

    int key_range = 10;
    int capacity = 5000;

    LogStore *store = logstore_create(capacity, key_range);
    pthread_t tids[4] = {0};
    t_args *thread_args = malloc(sizeof(t_args));
    thread_args->store = store;
    thread_args->reps = 2000;


    pthread_create(&tids[0], NULL, put_thread, thread_args);
    pthread_create(&tids[1], NULL, put_thread, thread_args);
    pthread_create(&tids[2], NULL, get_thread, thread_args);
    pthread_create(&tids[3], NULL, get_thread, thread_args);

    for (int i = 0; i < 4; i++) {
        pthread_join(tids[i], NULL);
    }

    // Store should be at exactly at 80% capacity so this should clean
    logstore_clean(store);

    //print_logstore_buffer(store);

    int correct_store_vals = true;
    for (int j = 0; j < store->key_range; j++) {
        if (logstore_get_buffer_index(store, j).value != 
                (thread_args->reps - store->key_range + j)) {
            correct_store_vals = false;
        }
    }

    EXPECT_TRUE(correct_store_vals == true);

    free(thread_args);
    logstore_destroy(store);
}

int main() {

    simple_put_clean_test();
    multi_put_get_clean_test();

}
