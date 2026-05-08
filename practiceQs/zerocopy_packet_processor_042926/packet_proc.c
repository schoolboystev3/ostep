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
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>

#define PAYLOAD_HEADER_SIZE 16
#define MAX_PAYLOAD_DATA_SIZE 48
#define PAYLOAD_SIZE PAYLOAD_HEADER_SIZE + MAX_PAYLOAD_DATA_SIZE
#define BUFFER_SIZE 128 // Power of 2 to avoid using Modulo
#define HIGH_PRIORITY 0 // For now everything set to this priority
#define EXPECTED_PACKET_COUNT 1000
#define PACKET_DATA_FILE "traffic.bin"
#define DUMMY_DATA 0xDEADBEEFBABA // 6 bytes

typedef struct __attribute__((packed)) packet_payload {
    uint32_t len;
    uint8_t data[MAX_PAYLOAD_DATA_SIZE / 8];
} packet_payload_t;

typedef struct packet {
    uint64_t timestamp;
    uint32_t length;
    uint32_t priority;
    uint8_t payload[MAX_PAYLOAD_DATA_SIZE / 8];
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

void process_packet(packet_t *curr, uint64_t *checksum) {
    
    int payload_num_bytes = MAX_PAYLOAD_DATA_SIZE / 8;

    for (int i = 0; i < payload_num_bytes; i++) {
        *checksum += curr->payload[i];
    }
    return;
}

// The check sum is a sum of payload values byte by byte
uint64_t calc_expected_checksum() {
    uint64_t expected_checksum = 0;
    uint64_t dummy_val = DUMMY_DATA;
    int payload_num_bytes = MAX_PAYLOAD_DATA_SIZE / 8;

    // add up dummy val byte by byte
    for (int i = 0; i < payload_num_bytes; i++) {
        expected_checksum += (dummy_val & 0xFF);
        dummy_val >>= 8;
    }

    expected_checksum *= EXPECTED_PACKET_COUNT;
    return expected_checksum;
}


size_t read_next_packet_from_file(uint8_t *file, packet_t *packet) {
    packet_payload_t *payload = (packet_payload_t*) file;
    size_t payload_len = payload->len;
    memcpy(&packet->payload, &payload->data, payload_len);

    return payload_len;
}

int create_packet_file() {
    int fd = open(PACKET_DATA_FILE, O_RDWR);

    // If file does not exist, create it
    if (fd == -1) {
        if (errno == ENOENT) {
            fd = open(PACKET_DATA_FILE, O_CREAT | O_RDWR, 0666);
            if (fd == -1) {
                perror("Packet data file creation failure.\n");
                goto end;
            }
        } else {
            perror("Error opening packet data file");
            goto end; // open failed for other reasons
        }
    }

    // Build packet payload data buffer
    size_t buffer_size = sizeof(packet_payload_t) * EXPECTED_PACKET_COUNT;
    packet_payload_t *pp = (packet_payload_t*) malloc(buffer_size);
    if (pp == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for packet data\n");
        free(pp);
        return -1;
    }

    for (int i = 0; i < EXPECTED_PACKET_COUNT; i++) {
        pp[i].len = MAX_PAYLOAD_DATA_SIZE / 8;
        // Copy dummy data byte by byte
        for (int j = 0; j <= pp[i].len; j++) {
            pp[i].data[j] = (DUMMY_DATA >> ((5-j) * 8)) & 0xFF;
        }
    }

    size_t total_written = 0;
    size_t wlen = 0;
    while (total_written < buffer_size) {
        wlen = write(fd, pp + total_written, buffer_size - total_written);

        if (wlen < 0) {
            perror("Failed while writing\n");
            return -1;
        }
        total_written += wlen;
    }

end:
    return fd;
}

// TODO: Needs to be adjusted such that if either producer or consumer
//       fails & exits, the other is not waiting indefinitely
int main() {

    // Packet Data File Creation
    int fd = create_packet_file();
    if(fd == -1) goto cleanup;

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
        
        struct stat st;
        if (fstat(fd, &st) < 0) {
            fprintf(stderr, "producer fstat failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        size_t file_len = st.st_size;

        uint8_t *mapped_data = (uint8_t*) mmap(NULL, file_len, PROT_READ, MAP_PRIVATE, fd, 0);
        if (mapped_data == MAP_FAILED) {
            fprintf(stderr, "producer mmap failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

            
        uint8_t *eof_addr = mapped_data + (file_len * sizeof(uint8_t));
        
        if (mapped_data == MAP_FAILED) {
            perror("Map Failed");
            _exit(1); // _exit to not flush standard I/O twice
        }

        // Tell Kernel planning to read sequentially
        madvise(mapped_data, file_len, MADV_SEQUENTIAL);

        // While there is still data to send
        uint8_t *curr = mapped_data;
        while(curr < eof_addr) {
            if (ring_is_empty(free_ring)) continue; // busy wait

            uint32_t free_ring_tail = atomic_load_explicit(&free_ring->tail, memory_order_acquire);
            uint32_t packet_idx = free_ring->buffer[free_ring_tail & free_ring->mask];
            free_ring->tail += 1;

            // Setup new packet
            packet_t *new = &packet_buf[packet_idx];
            size_t payload_sz = read_next_packet_from_file(curr, new);
            new->length = payload_sz;
            new->timestamp = current_nanoseconds();
            new->priority = HIGH_PRIORITY;
            curr += sizeof(packet_payload_t);

            ready_ring->buffer[ready_ring->head & ready_ring->mask] = packet_idx;
            atomic_store_explicit(&ready_ring->head, ready_ring->head + 1, memory_order_release);
        }

        munmap(mapped_data, file_len * sizeof(uint8_t));
    } else {
        // Parent Consumer

        uint32_t packet_count = 0;
        uint64_t checksum = 0;
        while (packet_count < EXPECTED_PACKET_COUNT) {
            if (ring_is_empty(ready_ring)) continue;

            uint32_t ready_ring_tail = atomic_load_explicit(&ready_ring->tail, memory_order_acquire);
            uint32_t packet_idx = ready_ring->buffer[ready_ring_tail & ready_ring->mask];
            ready_ring->tail += 1;

            packet_t *curr = &packet_buf[packet_idx];
            process_packet(curr, &checksum);

            packet_count += 1;
            free_ring->buffer[free_ring->head & free_ring->mask] = packet_idx;
            atomic_store_explicit(&free_ring->head, free_ring->head + 1, memory_order_release);
        }

        int wstatus;
        if (!waitpid(pid, &wstatus, 0)) {
            perror("Wait for child failed.\n");
            goto cleanup;
        }

        uint64_t expected_checksum = calc_expected_checksum();
        printf("expected = %lu, actual = %lu\n", expected_checksum, checksum);

cleanup:
    if (packet_buf != MAP_FAILED) munmap(packet_buf, packet_buf_size);
    if (ready_ring != MAP_FAILED) munmap(ready_ring, ring_size);
    if (free_ring != MAP_FAILED) munmap(free_ring, ring_size);
    close(fd);
    if (errno != 0) perror("mmap failure");

    return 0;
    }
}
