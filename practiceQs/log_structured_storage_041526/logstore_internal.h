/* 
 * Internals of logstore & debugging/testing functions
 * Wouldn't want to expose to user.
 * TODO Could #ifdef DEBUG in future
 */

#ifndef LOGSTORE_INTERNAL_H
#define LOGSTORE_INTERNAL_H

#include "logstore.h"
#include <pthread.h>

struct Entry {
    int key;
    int value;
};

struct LogStore {
    Entry* buffer;              // The "Log"
    int* imap;                  // Key -> Offset map
    int head;                   // Next write position
    int capacity;               // Max entries
    int key_range;              // Size of range of possible keys
    pthread_rwlock_t lock;      // Read Write Lock
};

Entry logstore_get_buffer_index(LogStore *store, int index);
void print_logstore_buffer(LogStore *store);

#endif
