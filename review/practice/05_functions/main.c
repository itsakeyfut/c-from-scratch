// Function in C

#include <stdio.h>
#include <stdlib.h> // malloc, free

typedef struct { int x; int y; } Point;

// =========================================================
// Prototype declaration (forward declaration)
// Needed if you call before definition
// It's customary to write in header files
// =========================================================
int add(int a, int b);
Point point_add(Point a, Point b);
int *alloc_array(size_t n);

// =========================================================
// Basic function
// =========================================================
int add(int a, int b) {
    return a + b;
}

// void function (equivalent to Rust's -> ())
void print_point(const Point *p) { // const * = read-only
    printf("(%d, %d)", p->x, p->y);
}

// =========================================================
// Struct pass by value (copied)
// OK for small structs, consider pointer pass for large ones
// =========================================================
Point point_add(Point a, Point b) {
    return (Point){ a.x + b.x, a.y + b.y };
}

// =========================================================
// Pass by pointer to return result (out parameter)
// Rust has `&mut`, but it's commonly used in C
// =========================================================
void point_add_out(const Point *a, const Point *b, Point *out) {
    out->x = a->x + b->x;
    out->y = a->y + b->y;
}

// =========================================================
// Allocate memory from heap and return
// Caller is responsible for free()
// =========================================================
int *alloc_array(size_t n) {
    int *arr = malloc(n * sizeof(int));
    if (!arr) return NULL; // Allocation failed
    for (size_t i = 0; i < n; i++) arr[i] = (int)i;
    return arr;
}

// =========================================================
// static function — only valid within this file (private)
// Used for internal implementation that is not written in header
// =========================================================
static int internal_helper(int x) {
    return x * 2;
}

// =========================================================
// inline hint (suggestion to compiler)
// =========================================================
static inline int square(int x) {
    return x * x;
}

// =========================================================
// Recursion
// =========================================================
int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

// =========================================================
// Returning multiple values — No tuple equivalent in Rust, so options are:
// ① Return a struct
// ② Use out parameters
// ③ Global variables (bad practice)
// =========================================================
typedef struct { int quot; int rem; } DivResult;

DivResult divide(int a, int b) {
    return (DivResult){ a/ b, a % b };
}

int main(void) {
    printf("=== Basic Functions ===\n");
    printf("add(3, 4) = %d\n", add(3, 4));

    printf("\n=== How to pass structs ===\n");
    Point p1 = {1, 2};
    Point p2 = {3, 4};

    // Pass by value (Copy)
    Point r1 = point_add(p1, p2);
    printf("Pass by value: "); print_point(&r1); printf("\n");

    // Pass by pointer (out parameter)
    Point r2;
    point_add_out(&p1, &p2, &r2);
    printf("Pass by pointer (out param): "); print_point(&r2); printf("\n");

    printf("\n=== Return allocated array ===\n");
    int *arr = alloc_array(5);
    if (arr) {
        for (int i = 0; i < 5; i++) printf("%d ", arr[i]);
        printf("\n");
        free(arr); // Remember to free the allocated memory
        arr = NULL; // Avoid dangling pointer
    }

    printf("\n=== Static and Inline Functions ===\n");
    printf("internal_helper(7) =%d\n", internal_helper(7));
    printf("square(9) = %d\n", square(9));

    printf("\n=== Recursion ===\n");
    for (int i = 0; i <= 5; i++) {
        printf("%d! = %d\n", i, factorial(i));
    }

    printf("\n=== Returning Multiple Values ===\n");
    DivResult dr = divide(17, 5);
    printf("17 / 5 = %d remainder %d\n", dr.quot, dr.rem);

    printf("\n=== Pass array to function (Careful of Array Decay) ===\n");
    int nums[] = {5, 3, 8, 1, 9};
    size_t len = sizeof(nums) / sizeof(nums[0]);
    // sizeof(nums) can be a pointer size if passed to a function, so we calculate length here
    printf("Array length: %zu\n", len);
    for (size_t i = 0; i < len; i++) printf("%d ", nums[i]);
    printf("\n");

    return 0;
}