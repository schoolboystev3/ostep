/*### DS05: Sparse Set For Huge ID Domains

Problem statement:

Design and implement a set of integer IDs where the possible ID domain is huge, but only a small fraction of IDs are present at any time.

The interviewer cares about:

- fast `add`
- fast `contains`
- compact/cache-friendly iteration over active values
- avoiding expensive upfront initialization
- not treating uninitialized memory as meaningful data
- memory/performance tradeoffs for a large sparse domain
*/

#include <stdbool.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define LEVEL_SIZE (0x1 << 16)

struct sparse_set {
    uint32_t *dense;   // length max_active; active values packed contiguously
    uint32_t **sparse;  // length universe_size; value -> candidate dense index
    uint32_t *level2_counts; // number of active entries in level 2 
    uint32_t count;
    uint32_t max_active;
    uint32_t universe_size;
};

static inline uint32_t get_lvl1_idx(uint32_t value) {
    return (value >> 16);
}

static inline uint32_t get_lvl2_idx(uint32_t value) {
    return (value & 0xFFFF);
}

// 2 Tier "Page Table" like approach
struct sparse_set *ss_create(uint32_t universe_size,
                             uint32_t max_active) {
    struct sparse_set *ss = malloc(sizeof(struct sparse_set));
    if (!ss) return NULL;

    ss->dense = malloc(max_active * (sizeof(uint32_t)));
    ss->sparse = calloc(LEVEL_SIZE, sizeof(uint32_t *));
    ss->level2_counts = calloc(LEVEL_SIZE, sizeof(uint32_t));
    ss->count = 0;
    ss->max_active = max_active;
    ss->universe_size = universe_size;

    // Initialize the pointers to 0 (NULL)
    return ss;
}

bool ss_contains(const struct sparse_set *s, uint32_t value) {
    uint32_t lvl1_idx = get_lvl1_idx(value);
    if (s->sparse[lvl1_idx] == NULL) {
        return false;
    }
    uint32_t lvl2_idx = get_lvl2_idx(value);
    if (s->sparse[lvl1_idx][lvl2_idx] == UINT32_MAX) {
        return false;
    }
    return true;
}

int ss_add(struct sparse_set *s, uint32_t value) {
    
    // skip duplicates
    if (ss_contains(s, value)) {
        return 0;
    }

    // Reject if at max or invalid value
    if ((s->count >= s->max_active) || (value > s->universe_size)) {
        return -1;
    }

    // Add new level2 if needed
    uint32_t lvl1_idx = get_lvl1_idx(value);
    if (s->sparse[lvl1_idx] == NULL) {
        s->sparse[lvl1_idx] = malloc(LEVEL_SIZE * sizeof(uint32_t));
        memset(s->sparse[lvl1_idx], -1, LEVEL_SIZE * sizeof(uint32_t));
    }
    
    // Insert new entry
    uint32_t lvl2_idx = get_lvl2_idx(value);
    s->sparse[lvl1_idx][lvl2_idx] = s->count;
    s->dense[s->count] = value;
    s->count++;
    s->level2_counts[lvl1_idx]++;

    return 0;
}

int ss_remove(struct sparse_set *s, uint32_t value) {

    // Invalid value
    if (!ss_contains(s, value)) {
        return -1;
    }

    // Find idx of value to be removed
    uint32_t lvl1_idx = get_lvl1_idx(value);
    uint32_t lvl2_idx = get_lvl2_idx(value);
    uint32_t dense_idx = s->sparse[lvl1_idx][lvl2_idx];

    // Rearrange dense array
    s->dense[dense_idx] = s->dense[s->count-1];
    uint32_t moved_val = s->dense[dense_idx];
    s->sparse[get_lvl1_idx(moved_val)][get_lvl2_idx(moved_val)] = dense_idx;

    // Do this last in case dense_idx == count - 1
    s->sparse[lvl1_idx][lvl2_idx] = UINT32_MAX; 
    s->level2_counts[lvl1_idx]--;

    s->count--;

    // Free the leaf table if it's empty
    if (s->level2_counts[lvl1_idx] == 0) {
        free(s->sparse[lvl1_idx]);
        s->sparse[lvl1_idx] = 0;
    }
    
    return 0;
}

void ss_clear(struct sparse_set *s) {
    for (uint32_t i = 0; i < LEVEL_SIZE; i++) {
        if (s->sparse[i] != NULL) {
            free(s->sparse[i]);
            s->sparse[i] = NULL;
        }
    }

    memset(s->level2_counts, 0, LEVEL_SIZE);
    s->count = 0;
}

void ss_destroy(struct sparse_set *s) {
    ss_clear(s);

    free(s->dense);
    free(s->sparse);
    free(s->level2_counts);
    free(s);

    return;
}

/*
Key invariant:

```c
ss_contains(s, x):
    idx = s->sparse[x];
    return idx < s->count && s->dense[idx] == x;
```

Why this is safe with uninitialized `sparse[x]`:

- `sparse[x]` may contain garbage if you skipped initialization.
- You do not trust it by itself.
- You only accept it if it indexes into the active dense range and round-trips back to `x`.
- The meaningful data lives in `dense[0..count)`.

Requirements:

- `contains`, `add`, and `remove` should be `O(1)` for the classic representation.
- Iteration over active values should be cache-friendly by walking `dense[0..count)`.
- `clear` should be `O(1)` by setting `count = 0`; no full array clear.
- `add` should reject values outside the universe, duplicates, and capacity overflow.
- `remove` may use swap-with-last to keep `dense` packed.
- Discuss memory cost: classic sparse set is `O(universe_size + max_active)`.

Remove algorithm:

```text
idx = sparse[value]
if idx invalid or dense[idx] != value: not present
last = dense[count - 1]
dense[idx] = last
sparse[last] = idx
count--
```
*/
