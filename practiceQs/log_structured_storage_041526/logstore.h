/* 
 * Log Structured Key Value Storage Header
 */

#ifndef LOGSTORE_H
#define LOGSTORE_H

typedef struct Entry Entry; // Opaque Pointer
typedef struct LogStore LogStore; // Opaque Pointer

LogStore* logstore_create(int capacity, int key_range);
void logstore_put(LogStore* store, int key, int value);
int logstore_get(LogStore* store, int key);
void logstore_clean(LogStore* store);
void logstore_destroy(LogStore* store);

#endif
