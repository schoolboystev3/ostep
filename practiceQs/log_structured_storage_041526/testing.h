/* 
 * Test header file containing useful testing utilities
 */

#include <stdio.h>


#define EXPECT_TRUE(expr) do { \
    if (!(expr)) { \
        printf("  [FAIL] %s:%d: %s\n", __FILE__, __LINE__, #expr); \
    } \
} while (0)
