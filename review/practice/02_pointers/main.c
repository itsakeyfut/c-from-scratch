// Basic Pointers

#include <stdio.h>
#include <stddef.h> // NULL

typedef struct {
    int x;
    int y;
} Point;

// Example: Swap values using pointer parameters
void swap (int *a, int *b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

// Example: Swap values by value (does not affect caller)
void try_swap_by_value (int a, int b) {
    int tmp = a;
    a = b;
    b = tmp;
    printf("  Inside function: a=%d, b=%d\n", a, b);
}

int main(void) {
    // =========================================================
    // Basics of & and *
    // Similar to Rust's &, but without safety guarantees
    // =========================================================
    printf("=== & and * ===\n");
    int x = 42;
    int *p = &x; // &x: Get the address of x
    printf("Address of x:  %p\n", (void *)&x);
    printf("Value of p (address): %p\n", (void *)p);
    printf("*p (dereference): %d\n", *p);

    *p = 100; // Write through pointer
    printf("*p = 100, x after: %d\n", x); // x also becomes 100

    // =========================================================
    // NULL pointer
    // Similar to Rust's None, represents "pointing to nothing"
    // Dereferencing it will cause a crash (UB)
    // =========================================================
    printf("\n=== NULL pointer ===\n");
    int *null_p = NULL;
    printf("null_p == NULL: %d\n", null_p == NULL);
    // *null_p = 1; // ← NG: Segmentation fault

    // =========================================================
    // Struct pointers and -> operator
    // =========================================================
    printf("\n=== Struct pointer and -> ===\n");
    Point pt = {3, 4};
    Point *pp = &pt;

    printf("Dot:        pt.x = %d\n", pt.x);
    printf("Arrow:      pp->x = %d\n", pp->x); // Same as (*pp).x
    printf("Explicit: (*pp).x = %d\n", (*pp).x);

    pp->x = 10; // Modify through pointer
    printf("After pp->x = 10, pt.x = %d\n", pt.x);

    // =========================================================
    // Pass by value vs. pass by pointer
    // =========================================================
    printf("\n=== Pass by value vs. pass by pointer ===\n");
    int a = 1, b = 2;

    printf("Before: a=%d, b=%d\n", a, b);
    try_swap_by_value(a, b);
    printf("After: a=%d, b=%d\n", a, b); // Still unchanged

    printf("Before: a=%d, b=%d\n", a, b);
    swap(&a, &b); // Pass addresses to swap
    printf("After swap: a=%d, b=%d\n", a, b); // Now swapped

    // =========================================================
    // Pointer arithmetic (not shown here, but important to understand)
    // =========================================================
    printf("\n=== Pointer arithmetic ===\n");
    int arr[3] = {10, 20, 30};
    int *parr = arr; // Points to arr[0]
    printf("parr points to arr[0]: %d\n", *parr); // 10
    parr++; // Move to next element
    printf("parr points to arr[1]: %d\n", *parr); // 20
    parr++; // Move to next element
    printf("parr points to arr[2]: %d\n", *parr); // 30

    // =========================================================
    // Pointer to pointer (double pointer)
    // =========================================================
    printf("\n=== Pointer to pointer ===\n");
    int val = 99;
    int *ptr = &val;
    int **pptr = &ptr; // Address of ptr

    **pptr = 77; // Modify val through double pointer
    printf("After **pptr = 77, val = %d\n", val);

    // ========================================================
    // const and pointers (4 combinations)
    // =========================================================
    printf("\n=== const and pointers ===\n");
    int n = 5;
    int m = 10;

    const int *cp = &n; // Pointer to const int: can change pointer, not value
    int *const pc = &n; // Const pointer to int: can change value, not pointer
    // cp = &m;         // OK: can change pointer
    // *cp = 99;        // NG: cannot change value through cp
    *pc = 99;           // OK: can change value through pc
    // pc = &m;         // NG: cannot change pointer pc
    printf("n after *pc=99: %d\n", n);
    (void)cp; (void)m;  // Suppress unused variable warnings

    // Using `const int *` in function parameters indicates "read-only" input
    // → Common practice to use const for read-only parameters in API design

    return 0;
}