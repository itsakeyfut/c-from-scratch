// Structs / typedefs / Unions / Enums in C

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>  // memset

// =========================================================
// Basic struct definition
// =========================================================
struct Point {
    int x;
    int y;
};

// =========================================================
// typedef struct — Avoid writing struct every time (convention)
// =========================================================
typedef struct {
    float x;
    float y;
    float z;
} Vec3;

// =========================================================
// Self-referential struct (e.g., linked list node)
// Both typedef and struct tag are needed
// =========================================================
typedef struct Node {
    int          value;
    struct Node *next; // struct tag is needed to refer to itself
} Node;

// =========================================================
// enum — Named integer constants
// Unlike Rust's enum, C enums cannot hold values (just ints)
// =========================================================
typedef enum {
    DIR_NORTH = 0,
    DIR_EAST  = 1,
    DIR_SOUTH = 2,
    DIR_WEST  = 3,
} Direction;

typedef enum {
    COLOR_RED   = 0xFF0000,
    COLOR_GREEN = 0x00FF00,
    COLOR_BLUE  = 0x0000FF,
} Color;

// =========================================================
// union — Interpret the same memory as different types
// sizeof is the size of the largest member
// =========================================================
typedef union {
    uint32_t as_u32;
    uint8_t  bytes[4];
} U32Bytes;

// =========================================================
// Bit fields — Assign bits within a struct
// =========================================================
typedef struct {
    uint8_t r : 5; // 5 bits (0-31)
    uint8_t g : 6; // 6 bits (0-63)
    uint8_t b : 5; // 5 bits (0-31)
} RGB565;

// =========================================================
// Struct Initialization and usage
// =========================================================
Vec3 vec3_add(Vec3 a, Vec3 b) {
    // Return C99 designated initializer style
    return (Vec3){ a.x + b.x, a.y + b.y, a.z + b.z };
}

int main(void) {
    // --- Basic struct ---
    printf("=== Basic struct ===\n");
    struct Point p1 = {3, 4};             // Initialize in order
    struct Point p2 = {.x = 10, .y = 20}; // Designated initializer (recommended)
    printf("p1: (%d, %d)\n", p1.x, p1.y);
    printf("p2: (%d, %d)\n", p2.x, p2.y);

    // Copying is done by assignment (no need for memcpy)
    struct Point p3 = p1;
    p3.x = 99;
    printf("p3.x=99 after: p1.x=%d (unchanged because it's a copy)\n", p1.x);

    // --- typedef struct ---
    printf("\n=== typedef struct ===\n");
    Vec3 v1 = {1.0f, 2.0f, 3.0f};
    Vec3 v2 = {.x = 4.0f, .y = 5.0f, .z = 6.0f};
    Vec3 v3 = vec3_add(v1, v2);
    printf("v3 = (%.1f, %.1f, %.1f)\n", v3.x, v3.y, v3.z);
    printf("sizeof(Vec3) = %zu\n", sizeof(Vec3));

    // Zero initialization (C doesn't allow {} for this, so use memset or {0})
    Vec3 zero = {0};                // All fields set to 0
    memset(&zero, 0, sizeof(zero)); // Alternative way to zero-initialize
    printf("zero = (%.1f, %.1f, %.1f)\n", zero.x, zero.y, zero.z);

    // --- Accessing through pointers ---
    printf("\n=== Struct pointers ===\n");
    Vec3 *vp = &v1;
    printf("vp->x = %.1f\n", vp->x); // Same as (*vp).x
    vp->x = 100.0f; // Modify through pointer
    printf("v1.x after vp->x=100: %.1f\n", v1.x); // v1.x is now 100.0f

    // --- enum ---
    printf("\n=== enum ===\n");
    Direction dir = DIR_NORTH;
    printf("DIR_NORTH = %d\n", dir);

    // Using enum in switch (be careful with fall-through)
    switch (dir) {
        case DIR_NORTH: printf("North\n"); break;
        case DIR_EAST:  printf("East\n"); break;
        case DIR_SOUTH: printf("South\n"); break;
        case DIR_WEST:  printf("West\n"); break;
        // -Wall warnings if default is omitted (but it's fine if all cases are covered)
    }

    Color c = COLOR_RED;
    printf("RED = 0x%06X\n", c);

    // --- union ---
    printf("\n=== union ===\n");
    U32Bytes u;
    u.as_u32 = 0x12345678;
    printf("as_u32 = 0x%08X\n", u.as_u32);
    printf("bytes  = %02X %02X %02X %02X\n",
           u.bytes[0], u.bytes[1], u.bytes[2], u.bytes[3]);
    // bytes[0] will be 0x78 on little-endian, 0x12 on big-endian

    printf("sizeof(union) = %zu (should be 4, the size of the largest member)\n", sizeof(u));

    // --- bit fields ---
    printf("\n=== bit fields ===\n");
    RGB565 rgb = { .r = 31, .g = 63, .b = 31 }; // Max values for each field
    printf("R=%u, G=%u, B=%u\n", rgb.r, rgb.g, rgb.b);
    printf("sizeof(RGB565) = %zu\n", sizeof(rgb)); // Size may be larger than 2 bytes due to padding

    // --- Padding and alignment ---
    printf("\n=== Padding and alignment ===\n");
    typedef struct { char a; int b; char c; } Padded;
    typedef struct { int b; char a; char c; } Packed;
    // Padded: a(1) + pad(3) + b(4) + c(1) + pad(3) = 12 bytes
    // Packed: b(4) + a(1) + c(1) + pad(2) = 8 bytes (if packed tightly)
    printf("Padded: %zu bytes\n", sizeof(Padded));
    printf("Packed: %zu bytes\n", sizeof(Packed));
    // Note: Packed may still have padding depending on the compiler and architecture.
    // Use #pragma pack if you need to control this.

    return 0;
}