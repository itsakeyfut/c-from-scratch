// Array in C

#include <stdio.h>
#include <string.h> // memcpy

// The array decays to a pointer when passed to a function, so we need to pass the length separately
void print_array(const int *arr, size_t len) {
    for (size_t i = 0; i < len; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
}

// Macro to get the length of an array (cannot be used after decay to pointer)
#define ARRAY_LEN(arr)  (sizeof(arr) / sizeof((arr[0])))

int main(void) {
    // =========================================================
    // Declaration and initialization of fixed-length arrays
    // =========================================================
    printf("=== Array Declaration ===\n");
    int a[5] = {1, 2, 3, 4, 5}; // Fully initialized
    int b[5] = {1, 2};          // The rest are zero-initialized
    int c[5] = {0};             // All elements are zero
    int d[]  = {10, 20, 30};    // Size is automatically inferred → d[3]

    printf("a: "); print_array(a, ARRAY_LEN(a));
    printf("b: "); print_array(b, ARRAY_LEN(b));
    printf("c: "); print_array(c, ARRAY_LEN(c));
    printf("d: "); print_array(d, ARRAY_LEN(d));

    // =========================================================
    // Array names decay to pointers
    // Array name == address of the first element
    // =========================================================
    printf("\n=== Relationship between Arrays and Pointers ===\n");
    int  arr[4] = {10, 20, 30, 40};
    int *p      = arr; // Same as &arr[0]
    printf("arr[0]  = %d\n", arr[0]);
    printf("*p      = %d\n", *p);         // Same value
    printf("p[1]    = %d\n", p[1]);       // Same as arr[1]
    printf("*(p+2)  = %d\n", *(p + 2));   // Same as arr[2]

    // sizeof can only be used before decay
    printf("sizeof(arr) = %zu (size of the entire array)\n", sizeof(arr));
    printf("sizeof(p)   = %zu (size of the pointer)\n", sizeof(p));
    // → After being passed to a function, sizeof cannot be used to get the size

    // =========================================================
    // Pointer arithmetic
    // p + n is equivalent to (char*)p + n * sizeof(*p)
    // =========================================================
    printf("\n=== Pointer Arithmetic ===\n");
    int nums[5] = {0, 10, 20, 30, 40};
    int *start  = nums;
    int *end    = nums + 5; // "One past the end" (cannot be dereferenced)

    for (int *q = start; q < end; q++) {
        printf("%d ", *q);
    }
    printf("\n");

    ptrdiff_t dist = end - start; // = 5
    printf("end - start = %td\n", dist);

    // =========================================================
    // Multidimensional arrays
    // =========================================================
    printf("\n=== Multidimensional Arrays ===\n");
    int matrix[3][4] = {
        {1,  2,  3,  4},
        {5,  6,  7,  8},
        {9, 10, 11, 12},
    };
    // In memory, it is stored in row-major order (row by row)
    // maxtrix[r][c] == *(matrix[0] + r * 4 + c)
    for (int r = 0; r < 3; r++) {
        for (int c = 0; c < 4; c++) {
            printf("%3d", matrix[r][c]);
        }
        printf("\n");
    }

    // =========================================================
    // VLA (Variable Length Arrays) — Available since C99, allocated on the stack
    // Avoid using VLAs in practice due to potential stack overflow and lack of support in some compilers
    // Instead, consider using dynamic memory allocation (malloc) for variable-sized arrays
    // =========================================================
    printf("\n=== VLA (Variable Length Arrays) ===\n");
    int n = 5;
    int vla[n]; // VLA declaration (size determined at runtime)
    for (int i = 0; i < n; i++) vla[i] = i * i;
    print_array(vla, (size_t)n);
    // Evaluated at runtime, so the size of the array can be determined during execution rather than at compile time.
    // Note: VLAs are not supported in C++, 
    // and their use is generally discouraged in C due to potential issues with stack overflow and lack of support in some compilers.
    // For variable-sized arrays, consider using dynamic memory allocation (malloc) instead.

    // =========================================================
    // Array copying using memcpy (shallow copy)
    // Assignment of arrays is not allowed in C, we can use memcpy to copy the contents of one array to another
    // =========================================================
    printf("\n=== Array Copying with memcpy ===\n");
    int src[5] = {1, 2, 3, 4};
    int dst[4];
    memcpy(dst, src, sizeof(src)); // Copy contents of src to dst (shallow copy)
    // dst = src; ← This is not allowed in C, as arrays cannot be assigned directly
    printf("dst: "); print_array(dst, ARRAY_LEN(dst));

    // =========================================================
    // Accessing out-of-bounds elements leads to undefined behavior
    // -fsanitize=address can help detect such issues during runtime
    // =========================================================
    printf("\n=== Out-of-Bounds Access (Commented Out) ===\n");
    int safe[3] = {1, 2, 3};
    printf("safe[2] = %d (OK)\n", safe[2]);
    // printf("safe[3] = %d (UB!)\n", safe[3]); // ← This is undefined behavior and should never be done

    return 0;
}