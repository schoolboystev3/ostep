/*
 *Requirements:

Return the original destination pointer.
my_memcpy may assume non-overlapping regions.
my_memmove must handle overlapping regions correctly.
my_memset writes the low byte of value.
Handle n == 0.
Follow-ups:

What happens if dst == NULL or src == NULL and n == 0?
    Answer: Null pointers not allowed return, n == 0 means do nothing.
Why is overlap undefined for memcpy but supported by memmove?
    Answer: I guess it's a API decision, memcpy is a special case of 
            memmove. And memcpy is the more common case which can be optimized.
When should memmove copy forward versus backward?
    Answer: If there is overlap, when the src is "in front" of the dst
How would you optimize with word-sized copies?
    Answer: This was done below, to dwords
What alignment concerns appear when optimizing?
    Answer: If n was not a multiple of the size of the writes
            corner cases needed to be managed. If dst and src were also
            not multiples of writes it would also incur performance loss
            or faults.
Why can overly clever optimized code introduce undefined behavior?
    Answer: optimizations introduce many corner cases like I mentioned above
 */

void *my_memcpy(void *dst, const void *src, size_t n) {
    if (!dst || !src || n == 0) {
        return dst;
    }

    uint32_t *dst32 = (uint32_t*)dst;
    uint32_t *src32 = (uint32_t*)src;

    // Move 4bytes at a time
    for (size_t i = 0; i < (n >> 2); i++) {
       *dst32 = *src32;
        dst32 += 1;
        src32 += 1;
    }

    uint8_t *dst8 = (uint8_t*)dst32;
    uint8_t *src8 = (uint8_t*)src32;

    for (size_t i = 0; i < (n & 0x3); i++) {
        *dst8 = *src8;
        dst8 += 1;
        src8 += 1;
    }
    return dst;
}

void *my_memmove(void *dst, const void *src, size_t n) {
    if (!dst || !src || n == 0 || src == dst) {
        return dst;
    }

    // dst: D----D
    // src: S----S
    // case 1: D--S--D--S (L->R)
    // case 2: S--D--S--D (R->L)
    // case 3: D----D S----S (memcpy)

    uint8_t *dst8 = (uint8_t*)dst;
    uint8_t *dst_end = dst8 + (n-1);
    uint8_t *src8 = (uint8_t*)src;
    uint8_t *src_end = src8 + (n-1);

    // Case 2 (otherwise can memcpy)
    if (dst8 > src8 && dst8 <= src_end) {
        for (size_t i = 0; i < n; i++) {
            *dst_end = *src_end;
            dst_end--;
            src_end--;
        }
        return dst;
    } else {
        return my_memcpy(dst, src, n);
    }
}
    
// set n bytes at dst to value (which 1 byte value)
void *my_memset(void *dst, int value, size_t n) {
    if (!dst || value > 255 || n == 0) {
        return dst;
    }

    uint32_t val = (uint32_t) value;
    uint32_t *dst32 = (uint32_t*)dst;
    uint32_t val32 = ((val << 24) | (val << 16) | (val << 8) | val);

    for (size_t i = 0; i < (n >> 2); i++) {
        *dst32 = val32;
        dst32++;
    }

    uint8_t *dst8 = (uint8_t*)dst32;
    uint8_t val8 = (uint8_t)value;

    for (size_t i = 0; i < (n & 0x3); i++) {
        *dst8 = val8;
        dst8++;
    }

    return dst;

}

