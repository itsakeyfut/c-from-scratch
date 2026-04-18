// Undefined Behavior (UB) and Pitfalls in C

// All UB patterns in this file are commented out.
// The goal is to read and understand WHY each one is dangerous.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>

int main(void) {

    // =========================================================
    // UB 1: Reading an uninitialized variable
    // =========================================================
    printf("=== UB-1: Uninitialized Variable ===\n");
    {
        int x = 0;      // OK: initialized
        printf("x = %d (initialized)\n", x);

        // int y;
        // printf("y = %d\n", y);   // NG: garbage value or UB
        //                          // detectable with -fsanitize=memory
        printf("-> always initialize local variables\n");
    }

    // =========================================================
    // UB 2: Dereferencing a NULL pointer
    // =========================================================
    printf("\n=== UB-2: NULL Dereference ===\n");
    {
        int *p = NULL;
        printf("p = %p (NULL)\n", (void *)p);

        // *p = 42;         // NG: segmentation fault (crashes)
        // int x = *p;      // NG: same

        // Correct pattern: check for NULL before use
        if (p != NULL) {
            printf("*p = %d\n", *p);
        } else {
            printf("p is NULL, skipping read\n");
        }
    }

    // =========================================================
    // UB 3: Use-after-free (accessing freed memory)
    // =========================================================
    printf("\n=== UB-3: Use-after-free ===\n");
    {
        int *p = malloc(sizeof(int));
        *p = 42;
        printf("*p = %d (before free)\n", *p);

        free(p);
        p = NULL;   // assign NULL after free to prevent accidental reuse (convention)

        // *p = 100;          // NG: write to freed memory (UB)
        // printf("%d", *p);  // NG: read from freed memory (UB)
        // free(p);           // OK: free(NULL) is safe

        printf("-> assign NULL after free (convention)\n");
    }

    // =========================================================
    // UB 4: Double free
    // =========================================================
    printf("\n=== UB-4: Double Free ===\n");
    {
        int *p = malloc(sizeof(int));
        *p = 1;
        free(p);
        p = NULL;
        free(p);   // OK: free(NULL) is safe (no-op)
        printf("-> assigning NULL makes a second free safe\n");

        // Dangerous pattern:
        // int *q = malloc(sizeof(int));
        // free(q);
        // free(q);    // NG: freeing the same pointer twice (crash or heap corruption)
    }

    // =========================================================
    // UB 5: Out-of-bounds array access
    // =========================================================
    printf("\n=== UB-5: Out-of-Bounds Array Access ===\n");
    {
        int arr[5] = {1, 2, 3, 4, 5};
        printf("arr[4] = %d (last valid index)\n", arr[4]);

        // printf("%d\n", arr[5]);    // NG: UB (detectable with -fsanitize=address)
        // arr[5] = 99;               // NG: buffer overflow

        // Mitigation: keep track of the size and use it in loops
        size_t len = 5;
        for (size_t i = 0; i < len; i++) {
            printf("arr[%zu] = %d\n", i, arr[i]);
        }
    }

    // =========================================================
    // UB 6: Signed integer overflow
    // Unsigned (uint) overflow is well-defined (wraps around).
    // Signed (int) overflow is UB.
    // =========================================================
    printf("\n=== UB-6: Signed Integer Overflow ===\n");
    {
        int max = INT_MAX;
        printf("INT_MAX = %d\n", max);

        // int overflow = max + 1;    // NG: signed integer overflow (UB)
        //                            // detectable with -fsanitize=undefined

        // Safe addition: check before adding
        if (max < INT_MAX) {
            int ok = max + 1;
            printf("ok = %d\n", ok);
        } else {
            printf("-> INT_MAX + 1 would overflow, skipping\n");
        }

        // Unsigned wraps around (not UB)
        uint32_t u = UINT32_MAX;
        printf("UINT32_MAX + 1 = %u (wraps around)\n", u + 1);  // 0
    }

    // =========================================================
    // UB 7: Pointer arithmetic on void*
    // =========================================================
    printf("\n=== UB-7: void* Arithmetic ===\n");
    {
        int arr[3] = {10, 20, 30};
        void *vp = arr;

        // vp + 1;              // NG: arithmetic on void* is UB (works as a GCC extension, but not standard)
        // *(int *)vp + 1;      // OK: arithmetic after dereferencing

        // Correct pattern: cast to char* first, then do byte-level arithmetic
        char *cp = (char *)vp;
        int  *ip = (int *)(cp + 1 * sizeof(int));   // address of arr[1]
        printf("arr[1] via char* arithmetic = %d\n", *ip);
    }

    // =========================================================
    // UB 8: Comparing signed and unsigned values
    // =========================================================
    printf("\n=== Pitfall: signed/unsigned comparison ===\n");
    {
        int           i = -1;
        unsigned int  u = 1;
        (void)u;

        // -Wall will warn about this
        // if (i < u) { ... }  // -1 is converted to UINT_MAX, making it larger than u

        // Safe comparison
        if (i < 0) {
            printf("-1 is negative, so it is less than 1u (correct)\n");
        }

        // Common mistake when comparing against size_t
        size_t len = 5;
        int    idx = -1;
        // if (idx < len) { ... }  // NG: idx is converted to size_t and becomes a huge positive number
        if (idx >= 0 && (size_t)idx < len) {
            printf("safe index check passed\n");
        } else {
            printf("idx = -1 -> out of range\n");
        }
    }

    // =========================================================
    // UB 9: Strict aliasing violation
    // =========================================================
    printf("\n=== Pitfall: strict aliasing ===\n");
    {
        // Accessing the same memory through pointers of different types can cause
        // the compiler to assume they refer to different objects and misoptimize.
        // Exception: char* is allowed to alias any type.

        uint32_t val = 0x12345678;

        // NG: aliasing via int* and float* is UB
        // float *fp = (float *)&val;
        // printf("%f\n", *fp);

        // OK: use memcpy to reinterpret bit patterns (type punning)
        float f;
        memcpy(&f, &val, sizeof(f));
        printf("type punning via memcpy: %f\n", f);

        // OK: use a union (valid in C99 and later)
        union { uint32_t u; float f; } pun = { .u = val };
        printf("type punning via union:  %f\n", pun.f);
    }

    // =========================================================
    // Summary: using -fsanitize=address,undefined
    // =========================================================
    printf("\n=== Using -fsanitize=address,undefined ===\n");
    printf("Most of the UB patterns above can be caught at runtime with sanitizers.\n");
    printf("Always compile with -fsanitize=address,undefined during development.\n");

    return 0;
}
