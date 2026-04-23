/* 
 * The Scenario: You are building a high-performance, in-memory key-value store. 
 * To keep writes O(1) and sequential, you decide to use a Log-Structured approach.
 *
 * 1. Append only writes.
 * 2. Imap maintains where the latest value of key is.
 * 3. Once disk is 80% full, do a segment clean removing old versions of kv pairs & consolidate.
 * 4. Concurrency (bonus)
 *
 */

#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include "logstore.h"
#include "logstore_internal.h"

void logstore_put(LogStore* store, int key, int value) {

    pthread_rwlock_wrlock(&store->lock);

    // Add new entry
    Entry *new = &store->buffer[store->head];
    new->key = key;
    new->value = value;

    // Update imap and head
    store->imap[key] = store->head;
    store->head += 1;

    pthread_rwlock_unlock(&store->lock);
}


int logstore_get(LogStore* store, int key) {
    int value = 0;

    pthread_rwlock_rdlock(&store->lock);
    int buffer_offset = store->imap[key];
    if (buffer_offset < 0) {
        value = -1; // TODO assuming values always > 0
    } else {
        value = store->buffer[buffer_offset].value;
    }
    pthread_rwlock_unlock(&store->lock);

    return value;
}

// Build a new buffer and imap and atomically switch the pointer
void logstore_clean(LogStore* store) {

    // Only clean if the store is 80% full
    if (store->head < (store->capacity * 0.8)) {
        return;
    }

    // Make new buffer and imap
    Entry *new_buffer = malloc(sizeof(Entry) * store->capacity);
    int *new_imap = malloc(sizeof(int) * store->key_range);
    memset(new_imap, -1, sizeof(int) * store->key_range);
    int new_head = 0;

    // Add all valid entries
    for (int i = 0; i < store->key_range; i++) {
        if (store->imap[i] >= 0) {
            new_buffer[new_head] = store->buffer[store->imap[i]];
            new_imap[i] = new_head;
            new_head += 1;
        }
    }

    Entry *old_buffer = store->buffer;
    int *old_imap = store->imap;

    pthread_rwlock_wrlock(&store->lock);

    // Switch pointers (logically atomic with locks)
    store->buffer = new_buffer;
    store->imap = new_imap;
    store->head = new_head;

    pthread_rwlock_unlock(&store->lock);

    free(old_buffer);
    free(old_imap);
}

// LogStore Init
LogStore* logstore_create(int capacity, int key_range) {
    LogStore *ls = malloc(sizeof(LogStore));
    ls->capacity = capacity;
    ls->key_range = key_range;
    ls->head = 0;
    ls->buffer = calloc(capacity, sizeof(Entry));
    ls->imap = malloc(sizeof(int) * key_range);
    memset(ls->imap, -1, sizeof(int) * key_range);
    pthread_rwlock_init(&ls->lock, NULL);

    return ls;
}

void logstore_destroy(LogStore *store) {
    free(store->buffer);
    free(store->imap);
    pthread_rwlock_destroy(&store->lock);
    free(store);
}

Entry logstore_get_buffer_index(LogStore *store, int index) {
    return store->buffer[index];
}

void print_logstore_buffer(LogStore *store) {
    for (int i = 0; i < store->capacity; i++) {
        printf("(%d %d) ", store->buffer[i].key, store->buffer[i].value);
    }
    printf("\n");
}
