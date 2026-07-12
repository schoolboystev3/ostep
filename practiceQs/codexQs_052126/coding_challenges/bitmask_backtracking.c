/*### Challenge 23: Bitmask Backtracking
 
 Implement a bitmask/backtracking helper. This is included because public reports mention bit/array-style algorithm questions, and low-level roles often like bit manipulation fluency.
 
 Requirements:
 
 - Return `0` on success.
 - Return `-1` if `out_count` is `NULL`.
 - Set `*out_count = 0` before any other validation.
 - Return `-2` if `out` is `NULL` and `mask != 0`.
 - Generate every submask of `mask`, including `0` and `mask`.
 - Return `-3` if `out_cap` is too small.
 - Do not generate duplicate submasks.
 - It is acceptable to generate submasks in any order.
 - Do not use recursion if you can explain the iterative trick.
 
 Hint: 
 
 ```c
 // Common submask iteration pattern:
 // for (uint32_t s = mask; ; s = (s - 1) & mask) { ... if (s == 0) break; }
 ```
 */

int generate_submasks(uint32_t mask, uint32_t *out,
                      size_t out_cap, size_t *out_count) {
   if (!out_count) {
       return -1;
   }

   *out_count = 0;

   if (!out && mask != 0) {
       return -2;
   }
   
   if (!out && mask == 0) {
        *out_count = 1;
        return 0;
   }
   
   int k = 0;
   uint32_t i = mask;
   while (i != 0) {
       i &= (i-1);
       k++;
   }

   if (out_cap < (1 << k)) {
       return -3;
   }

   out[0] = 0;
   int idx = 1;
   uint32_t s = mask;
   while (s != 0) {
       out[idx] = s;
       idx++;
       s = ((s - 1) & mask);
   }

   *out_count = idx;
   return 0;
}
