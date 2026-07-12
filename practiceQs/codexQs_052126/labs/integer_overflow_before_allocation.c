 //### Lab 2: Integer Overflow Before Allocation
 
 //Prompt:
 
 //> A command handler allocates a buffer for an array of descriptors, then copies host data into it. It passes tests with small inputs but is exploitable.
 
 //```c
 #include <stdint.h>
 #include <stdlib.h>
 #include <string.h>
 
 struct desc {
     uint64_t addr;
     uint32_t len;
     uint32_t flags;
 };
 
 int load_descs(const void *host_buf, uint32_t count)
 {
     uint32_t bytes = count * sizeof(struct desc);
     struct desc *descs = malloc(bytes);
 
     if (!descs)
         return -1;
 
     memcpy(descs, host_buf, (size_t)count * sizeof(struct desc));
 
     /* validate and use descs... */
 
     free(descs);
     return 0;
 }
/*
 ```
 
 Questions:
 
 - What happens if `count * sizeof(struct desc)` overflows `uint32_t`?
    Answer: the size of the heap allocation will be incorrect which will
            result in a buffer overflow when we perform the memcpy
 - Why is the allocation size different from the copy size?
    Answer: The calulation will result in the same value but "bytes"
            is a uin32_t which is potentially smaller than the result
            of the multiplication which will be size_t due to type promotion.
 - What if `host_buf == NULL` and `count > 0`?
    Answer: For memcpy I believe this will result in undefined behavior.
            Or it will just immediately return and not perform the copying.
 - Where should the maximum descriptor count be enforced?
    Answer: At the beginning of this function we could enforce a maximum
            size for the count. At a very maximum it would need to be
            the maximum value of size_t divded by (sizeof(struct desc))
*/
