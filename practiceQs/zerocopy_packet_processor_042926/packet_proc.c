/*
 * Goal: Build a user-space "Packet Processor" that receives "packets" (simulated) and 
 *       processes them without ever copying the data from the initial buffer.
 * Requirements:
 *   1. Shared Memory Pool: fixed size buffer pool (mimick hw mem).
 *   2. Metadata Header: Each packet contains timestamp, priority, payload length.
 *   3. The "Hardware" Simulator: Write a producer thread that "DMAs" data into the pool
 *                                use a ring buffer to track free.
 *   4. Zero Copy: Consumer thread should process the packet in place.
 *   5. Concurrency: C11 Atomics to synchronize instead of mutexes.
 */

#include <inttypes.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdint.h>
#include <stdalign.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>

#define PAYLOAD_SIZE 64 // 16B header + 48B max payload
#define BUFFER_SIZE 128 // Power of 2 to avoid using Modulo
#define HIGH_PRIORITY 0 // For now everything set to this priority
#define EXPECTED_PACKET_COUNT 1000

typedef struct packet {
    uint64_t timestamp;
    uint32_t length;
    uint32_t priority;
    uint8_t payload[];
} packet_t;

// Align to avoid false sharing
typedef struct ringbuf {
    alignas(64) volatile uint32_t _Atomic head;  // write index
    alignas(64) volatile uint32_t _Atomic tail;  // read index
    uint32_t mask;                               // For bitwise AND to avoid modulo, cache mask
    alignas(64) uint32_t buffer[];               // Align first element
} ringbuf_t;

// Use Mirror Bit logic
bool ring_is_full(ringbuf_t *rb) {
    return (rb->head == (rb->tail ^ BUFFER_SIZE));
}

bool ring_is_empty(ringbuf_t *rb) {
    return (rb->head == rb->tail);
}

uint64_t current_nanoseconds() {
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    return ((uint64_t)ts.tv_sec * 1000000000ULL) + ts.tv_nsec;
}

size_t read_next_packet_from_file(uint8_t *file, packet_t *packet) {
    (void) file;
    (void) packet;
    return 0;
}

void process_packet(packet_t *curr) {
    return;
}

int main() {

    // Shared Buffer
    size_t packet_buf_size = PAYLOAD_SIZE * BUFFER_SIZE;
    packet_t *packet_buf = (packet_t*) mmap(NULL, packet_buf_size, PROT_READ | PROT_WRITE,
            MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (packet_buf == MAP_FAILED) goto cleanup;

    // Ready Ring
    size_t ring_size = sizeof(ringbuf_t) + BUFFER_SIZE * sizeof(uint32_t);
    ringbuf_t *ready_ring = (ringbuf_t*) mmap(NULL, ring_size, PROT_READ | PROT_WRITE, 
            MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (ready_ring == MAP_FAILED) goto cleanup;
    ready_ring->head = 0;
    ready_ring->tail = 0;
    ready_ring->mask = BUFFER_SIZE - 1;

    // Free Ring
    ringbuf_t *free_ring = (ringbuf_t*) mmap(NULL, ring_size, PROT_READ | PROT_WRITE, 
            MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (free_ring == MAP_FAILED) goto cleanup;

    // Add all indicies to free ring TODO shuffle this
    for (int i = 0; i < BUFFER_SIZE; i++) {
        free_ring->buffer[i] = i;
    }
    free_ring->head = BUFFER_SIZE;
    free_ring->tail = 0;
    free_ring->mask = BUFFER_SIZE - 1;


    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return 1;
    }
    else if (pid == 0) {
        // Child Producer
        int fd = open("traffic.bin", O_RDONLY); // TODO

        struct stat st;
        fstat(fd, &st);
        size_t file_len = st.st_size;

        uint8_t *mapped_data = (uint8_t*) mmap(NULL, file_len, PROT_READ,
                MAP_PRIVATE, fd, 0);
        uint8_t *eof_addr = mapped_data + (file_len * sizeof(uint8_t));
        
        if (mapped_data == MAP_FAILED) {
            perror("Map Failed");
            _exit(1); // _exit to not flush standard I/O twice
        }

        // Tell Kernel planning to read sequentially
        madvise(mapped_data, file_len, MADV_SEQUENTIAL);

        // While there is still data to send
        while(mapped_data < eof_addr) {
            if (ring_is_empty(free_ring)) continue; // busy wait

            uint32_t free_ring_tail = atomic_load_explicit(&free_ring->tail, memory_order_acquire);
            uint32_t packet_idx = free_ring->buffer[free_ring_tail & free_ring->mask];
            free_ring->tail += 1;

            // Setup new packet
            packet_t *new = &packet_buf[packet_idx];
            size_t bytes_read = read_next_packet_from_file(mapped_data, new); // TODO
            new->length = bytes_read;
            new->timestamp = current_nanoseconds();
            new->priority = HIGH_PRIORITY;

            ready_ring->buffer[ready_ring->head] = packet_idx;
            atomic_store_explicit(&ready_ring->head, ready_ring->head + 1, memory_order_release);
        }
    } else {
        // Parent Consumer

        uint32_t packet_count = 0;
        while (packet_count < EXPECTED_PACKET_COUNT) {
            if (ring_is_empty(ready_ring)) continue;

            uint32_t ready_ring_tail = atomic_load_explicit(&ready_ring->tail, memory_order_acquire);
            uint32_t packet_idx = ready_ring->buffer[ready_ring_tail & ready_ring->mask];
            ready_ring_tail += 1;

            packet_t *curr = &packet_buf[packet_idx];
            process_packet(curr); // TODO

            free_ring->buffer[free_ring->head] = packet_idx;
            atomic_store_explicit(&free_ring->head, free_ring->head + 1, memory_order_release);
        }
    }

cleanup:
    if (packet_buf != MAP_FAILED) munmap(packet_buf, packet_buf_size);
    if (ready_ring != MAP_FAILED) munmap(ready_ring, ring_size);
    if (free_ring != MAP_FAILED) munmap(ready_ring, ring_size);
    if (errno != 0) perror("mmap failure");

    return 0;
}
