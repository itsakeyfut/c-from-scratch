// Dynamic Memory Management in C (malloc / calloc / realloc / free)

#include <stdio.h>
#include <stdlib.h>  // malloc, calloc, realloc, free
#include <string.h>  // memset, memcpy

int main(void) {
    // =========================================================
    // malloc — Allocates memory without initialization
    // Returns void* -> cast to the desired type before use
    // Returns NULL on failure -> always check the return value
    // =========================================================
    printf("=== malloc ===\n");
    int *arr = malloc(5 * sizeof(int));  // allocate space for 5 ints
    if (!arr) {                          // NULL check is mandatory
        fprintf(stderr, "malloc failed\n");
        return 1;
    }
    // Not initialized (contains garbage values) -> always initialize manually
    for (int i = 0; i < 5; i++) arr[i] = i * 10;
    for (int i = 0; i < 5; i++) printf("%d ", arr[i]);
    printf("\n");
    free(arr);
    arr = NULL;  // prevent dangling pointer (convention)

    // =========================================================
    // calloc — Allocates memory and zero-initializes it
    // Equivalent to malloc(n * size) + memset(0)
    // =========================================================
    printf("\n=== calloc ===\n");
    double *dbls = calloc(4, sizeof(double));  // (count, size)
    if (!dbls) { return 1; }
    // calloc guarantees zero initialization
    for (int i = 0; i < 4; i++) printf("%.1f ", dbls[i]);  // 0.0 0.0 0.0 0.0
    printf("\n");
    free(dbls);
    dbls = NULL;

    // =========================================================
    // realloc — Resize a previously allocated memory block
    //
    // BAD: v->data = realloc(v->data, new_size);
    //      -> on failure, NULL is assigned and the original pointer is lost
    //
    // GOOD: always use a temporary variable
    // =========================================================
    printf("\n=== realloc ===\n");
    int *buf  = malloc(3 * sizeof(int));
    if (!buf) { return 1; }
    buf[0] = 1; buf[1] = 2; buf[2] = 3;
    printf("before realloc: ");
    for (int i = 0; i < 3; i++) printf("%d ", buf[i]);
    printf("\n");

    // BAD pattern (commented out)
    // buf = realloc(buf, 6 * sizeof(int));  // buf becomes NULL on failure

    // GOOD pattern: receive the result in a temporary variable
    int *tmp = realloc(buf, 6 * sizeof(int));
    if (!tmp) {
        free(buf);   // free the original pointer and handle the error
        return 1;
    }
    buf = tmp;  // update buf only on success
    buf[3] = 4; buf[4] = 5; buf[5] = 6;
    printf("after  realloc: ");
    for (int i = 0; i < 6; i++) printf("%d ", buf[i]);
    printf("\n");
    free(buf);
    buf = NULL;

    // =========================================================
    // Rules for free
    // 1. Only pointers from malloc/calloc/realloc can be freed
    // 2. free(NULL) is safe (no-op)
    // 3. Double free is undefined behavior (UB)
    // 4. Freeing the address of a stack variable is UB
    // =========================================================
    printf("\n=== Rules for free ===\n");
    free(NULL);  // OK: safe
    printf("free(NULL) is safe\n");

    int *p = malloc(sizeof(int));
    *p = 42;
    free(p);
    p = NULL;
    // free(p);  // OK: free(NULL) is safe
    // *p = 1;   // NG: access after free (use-after-free)

    // =========================================================
    // Dynamic allocation of a struct
    // =========================================================
    printf("\n=== Dynamic struct allocation ===\n");
    typedef struct { int x; int y; } Point;

    Point *pt = malloc(sizeof(Point));
    if (!pt) { return 1; }
    pt->x = 10;
    pt->y = 20;
    printf("pt = (%d, %d)\n", pt->x, pt->y);
    free(pt);
    pt = NULL;

    // =========================================================
    // Dynamic allocation of a struct array
    // =========================================================
    printf("\n=== Dynamic struct array allocation ===\n");
    int n = 3;
    Point *points = calloc((size_t)n, sizeof(Point));
    if (!points) { return 1; }
    for (int i = 0; i < n; i++) {
        points[i].x = i;
        points[i].y = i * i;
    }
    for (int i = 0; i < n; i++) {
        printf("points[%d] = (%d, %d)\n", i, points[i].x, points[i].y);
    }
    free(points);
    points = NULL;

    // =========================================================
    // Memory manipulation functions (string.h)
    // =========================================================
    printf("\n=== memset / memcpy / memmove ===\n");
    char data[8];

    // memset: fill memory byte by byte (commonly used for zero-clearing)
    memset(data, 0, sizeof(data));
    printf("memset(0): ");
    for (int i = 0; i < 8; i++) printf("%02X ", (unsigned char)data[i]);
    printf("\n");

    // memcpy: copy non-overlapping memory regions
    char src[] = "ABCDEFG";
    char dst[8];
    memcpy(dst, src, sizeof(src));
    printf("memcpy: %s\n", dst);

    // memmove: safe copy even when source and destination overlap
    // e.g., shifting elements within an array (used in vec_insert etc.)
    char shift[] = "ABCDE";
    // shift [1..4] to [2..5] (overlapping regions)
    memmove(shift + 2, shift + 1, 4);
    shift[1] = 'X';
    printf("memmove: %s\n", shift);  // AXBCDE

    // =========================================================
    // Stack vs Heap
    // =========================================================
    printf("\n=== Stack vs Heap ===\n");
    // Stack: automatically freed when the function returns, fast, limited size
    int stack_arr[100];  // stack (OK)
    (void)stack_arr;
    // int big[1000000];  // too large -> stack overflow

    // Heap: must be freed explicitly, slower, much larger size limit
    int *heap_arr = malloc(100 * sizeof(int));  // heap (OK)
    if (heap_arr) {
        printf("allocated %zu bytes on the heap\n", 100 * sizeof(int));
        free(heap_arr);
    }

    printf("\ndone\n");
    return 0;
}
