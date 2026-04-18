// void* / Function Pointers / Pointer Arithmetic in C

#include <stdio.h>
#include <stdlib.h>  // qsort
#include <string.h>  // memcpy

// =========================================================
// typedef for function pointers (improves readability)
// =========================================================
typedef int (*CmpFn)(const void *, const void *);
typedef void (*PrintFn)(int);

// Comparison functions (for qsort)
static int cmp_asc(const void *a, const void *b) {
    return *(const int *)a - *(const int *)b;
}
static int cmp_desc(const void *a, const void *b) {
    return *(const int *)b - *(const int *)a;
}

// Higher-order function that takes a function pointer
static void my_sort(int *arr, size_t n, CmpFn cmp) {
    qsort(arr, n, sizeof(int), cmp);
}

static void print_int(int x)     { printf("%d", x); }
static void print_hex(int x)     { printf("0x%X", x); }
static void print_squared(int x) { printf("%d", x * x); }

static void apply(const int *arr, size_t n, PrintFn fn) {
    for (size_t i = 0; i < n; i++) {
        fn(arr[i]);
        if (i < n - 1) printf(", ");
    }
    printf("\n");
}

int main(void) {
    // =========================================================
    // void* — A generic pointer that carries no type information
    // This is why malloc returns void*
    // =========================================================
    printf("=== void* ===\n");
    int    i_val  = 42;
    double d_val  = 3.14;
    void  *vp;

    vp = &i_val;
    printf("void* -> int*: %d\n", *(int *)vp);    // cast before dereferencing

    vp = &d_val;
    printf("void* -> double*: %.2f\n", *(double *)vp);

    // void* cannot be dereferenced or used in pointer arithmetic directly
    // *(vp)       // NG: compile error
    // vp + 1      // NG: UB (cast to char* first for byte-level arithmetic)
    char *cp = (char *)vp + 1;  // OK: convert to char* first
    (void)cp;

    // =========================================================
    // void* + elem_size for generic operations
    // This is the pattern used inside Vec
    // =========================================================
    printf("\n=== void* + elem_size (Vec internal pattern) ===\n");
    int   int_arr[4]  = {10, 20, 30, 40};
    double dbl_arr[3] = {1.1, 2.2, 3.3};

    // Utility macro to get element i (equivalent to vec_get in Vec)
    #define ELEM(base, i, sz)  ((void *)((char *)(base) + (i) * (sz)))

    void *elem2 = ELEM(int_arr, 2, sizeof(int));
    printf("int_arr[2] = %d\n", *(int *)elem2);

    void *elem1 = ELEM(dbl_arr, 1, sizeof(double));
    printf("dbl_arr[1] = %.1f\n", *(double *)elem1);

    // =========================================================
    // Declaring and calling function pointers
    // =========================================================
    printf("\n=== Function Pointers ===\n");

    // Without typedef (harder to read)
    int (*raw_cmp)(const void *, const void *) = cmp_asc;
    printf("raw_cmp function pointer: %p\n", (void *)raw_cmp);

    // With typedef (recommended)
    CmpFn fn = cmp_asc;
    int a = 5, b = 3;
    printf("cmp_asc(5, 3) = %d\n", fn(&a, &b));   // positive -> 5 > 3

    fn = cmp_desc;
    printf("cmp_desc(5, 3) = %d\n", fn(&a, &b));  // negative -> 5 comes after 3 in descending order

    // =========================================================
    // Sorting with function pointers
    // =========================================================
    printf("\n=== Switching behavior with function pointers ===\n");
    int nums[] = {5, 2, 8, 1, 9, 3};
    size_t n = sizeof(nums) / sizeof(nums[0]);

    my_sort(nums, n, cmp_asc);
    printf("ascending:  "); apply(nums, n, print_int);

    my_sort(nums, n, cmp_desc);
    printf("descending: "); apply(nums, n, print_int);

    printf("hex:        "); apply(nums, n, print_hex);
    printf("squared:    "); apply(nums, n, print_squared);

    // =========================================================
    // Array of function pointers (jump table)
    // =========================================================
    printf("\n=== Array of function pointers (jump table) ===\n");
    PrintFn fns[] = { print_int, print_hex, print_squared };
    const char *names[] = { "int", "hex", "squared" };
    int sample[] = {10, 255, 7};

    for (int f = 0; f < 3; f++) {
        printf("%-8s: ", names[f]);
        for (int i = 0; i < 3; i++) {
            fns[f](sample[i]);
            if (i < 2) printf(", ");
        }
        printf("\n");
    }

    // =========================================================
    // Pointer arithmetic with char* (byte-level)
    // =========================================================
    printf("\n=== char* pointer arithmetic (byte-level operations) ===\n");
    typedef struct { int x; int y; } Point;
    Point pts[3] = {{1,2},{3,4},{5,6}};

    // Traverse a Point array as a byte sequence
    char  *base      = (char *)pts;
    size_t elem_size = sizeof(Point);

    for (int i = 0; i < 3; i++) {
        Point *p = (Point *)(base + i * elem_size);
        printf("pts[%d] = (%d, %d)\n", i, p->x, p->y);
    }

    // =========================================================
    // restrict — A compiler hint meaning "only this pointer aliases this region"
    // memcpy declaration: void *memcpy(void * restrict dst, const void * restrict src, ...)
    // -> guarantees that dst and src do not overlap
    // =========================================================
    printf("\n=== restrict (explanation only) ===\n");
    printf("restrict: an optimization hint guaranteeing dst and src do not overlap\n");
    printf("use memmove when the regions may overlap\n");

    return 0;
}
