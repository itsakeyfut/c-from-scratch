// Primitive Types and Variables in C

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <limits.h>

int main(void) {
    // =========================================================
    // Basic type sizes (platform-dependent)
    // =========================================================
    printf("=== Basic Types and Their Sizes ===\n");
    printf("char:      %zu byte\n", sizeof(char));      // Always 1 byte
    printf("short:     %zu byte\n", sizeof(short));     // Typically 2 bytes
    printf("int:       %zu byte\n", sizeof(int));       // Typically 4 bytes
    printf("long:      %zu byte\n", sizeof(long));      // 4 bytes on 32-bit, 8 bytes on 64-bit
    printf("long long: %zu byte\n", sizeof(long long)); // Always 8 bytes
    printf("float:     %zu byte\n", sizeof(float));     // Typically 4 bytes
    printf("double:    %zu byte\n", sizeof(double));    // Typically 8 bytes
    printf("void*      %zu byte\n", sizeof(void*));     // 8 bytes on 64-bit

    // =========================================================
    // Fixed-width integer types (stdint.h) — size is guaranteed
    // Prefer these over plain int in real implementations
    // =========================================================
    printf("\n=== Fixed-width Integer Types (Recommended) ===\n");
    uint8_t  u8  = 255;
    int8_t   i8  = -128;
    uint32_t u32 = 0xDEADBEEF;
    int64_t  i64 = -9000000000LL;
    printf("uint8_t  255 -> %u\n",   u8);
    printf("int8_t  -128 -> %d\n",   i8);
    printf("uint32_t hex -> 0x%X\n", u32);
    printf("int64_t      -> %lld\n", i64);

    // =========================================================
    // size_t — always use this for array indices and memory sizes
    // It is unsigned, so it cannot hold negative values
    // =========================================================
    printf("\n=== size_t / ptrdiff_t ===\n");
    size_t len = 100;    // Length of an array or size in bytes
    ptrdiff_t diff = -5; // Difference between two pointers (signed) 
    printf("size_t:     %zu\n", len);
    printf("ptrdiff_t:  %td\n", diff);

    // =========================================================
    // bool (requires stdbool.h, available since C99)
    // 0 = false, any other value = true
    // =========================================================
    printf("\n=== bool ===\n");
    bool t = true;
    bool f = false;
    printf("true=%d, false=%d\n", t, f);
    printf("(bool)42  -> %d\n", (bool)42); // Non-zero becomes 1
    printf("(bool)-1  -> %d\n", (bool)-1); // Non-zero becomes 1
    printf("(bool)0   -> %d\n", (bool)0);  // Zero becomes 0

    // =========================================================
    // Implicit integer promotion pitfall (caught by -Wall)
    // Mixing signed and unsigned can produce unexpected results
    // =========================================================
    printf("\n=== Implicit Integer Promotion Pitfall ===\n");
    unsigned int u = 1;
    int          i = -1;
    // -1 is int, but when compared with unsigned int it is converted to unsigned
    // → -1 becomes 4294967295, which is greater than 1
    if ((unsigned)i > u) {
        printf("This will print because -1 is converted to a large unsigned value.\n");
    }
    // Fix: always compare values of the same type
    if ((size_t)i > len) { /* len is size_t, so be careful */}

    // =========================================================
    // const — prevents the value from being modified
    // =========================================================
    printf("\n=== const ===\n");
    const int MAX = 100;       // A constant value that cannot be changed
    const char *s = "hello";   // Pointer to an immutable string literal
    char *const p = (char*)""; // Constant pointer — the pointer itself cannot change
    printf("MAX=%d, s=%s\n", MAX, s);
    (void)p; // Suppress unused variable warning

    // =========================================================
    // Variable initialization rules
    // Global variables: automatically zero-initialized
    // Local variables: garbage value if not initialized (undefined behavior!)
    // =========================================================
    printf("\n=== Variable Initialization Rules ===\n");
    int initialized = 0;
    // int uninitialized;             // Undefined behavior if read before initialization
    // printf("%d\n", uninitialized); // Do not do this!
    printf("initialized: %d\n", initialized);

    return 0;
}
