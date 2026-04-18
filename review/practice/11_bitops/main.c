// Bitwise Operations

#include <stdio.h>
#include <stdint.h>

static void print_bits8(uint8_t v) {
    for (int i = 7; i >= 0; i--) {
        printf("%d", (v >> i) & 1);
        if (i == 4) printf("_");
    }
    printf(" (0x%02X = %3u)", v, v);
}

static void print_bits32(uint32_t v) {
    for (int i = 31; i >= 0; i--) {
        printf("%d", (v >> i) & 1);
        if (i % 4 == 0 && i > 0) printf("_");
    }
    printf(" (0x%08X)", v);
}

int main(void) {
    // =========================================================
    // Basic bitwise operators
    // =========================================================
    printf("=== Basic Bitwise Operations ===\n");
    uint8_t a = 0b10110011;  // 0xB3
    uint8_t b = 0b11001010;  // 0xCA

    printf("a     = "); print_bits8(a); printf("\n");
    printf("b     = "); print_bits8(b); printf("\n");
    printf("a & b = "); print_bits8(a & b);       printf("  AND\n");
    printf("a | b = "); print_bits8(a | b);       printf("  OR\n");
    printf("a ^ b = "); print_bits8(a ^ b);       printf("  XOR\n");
    printf("~a    = "); print_bits8((uint8_t)~a); printf("  NOT\n");

    // =========================================================
    // Shift operations
    // Left shift: × 2^n   Right shift: ÷ 2^n
    // Right shift of signed integers is implementation-defined -> use uint
    // =========================================================
    printf("\n=== Shift Operations ===\n");
    uint8_t v = 0b00001100;  // 12
    printf("v      = "); print_bits8(v); printf("\n");
    printf("v << 2 = "); print_bits8((uint8_t)(v << 2)); printf("  (*4 = 48)\n");
    printf("v >> 1 = "); print_bits8((uint8_t)(v >> 1)); printf("  (/2 = 6)\n");

    // =========================================================
    // Common bit manipulation patterns
    // =========================================================
    printf("\n=== Bit Manipulation Patterns ===\n");
    uint8_t flags = 0b00000000;
    printf("initial:      "); print_bits8(flags); printf("\n");

    // Set a bit (to 1): |= (1 << n)
    flags |= (1 << 3);   // set bit 3
    flags |= (1 << 5);   // set bit 5
    printf("bit3,5 SET:   "); print_bits8(flags); printf("\n");

    // Clear a bit (to 0): &= ~(1 << n)
    flags &= ~(1 << 3);  // clear bit 3
    printf("bit3 CLEAR:   "); print_bits8(flags); printf("\n");

    // Toggle a bit: ^= (1 << n)
    flags ^= (1 << 5);   // toggle bit 5
    printf("bit5 TOGGLE:  "); print_bits8(flags); printf("\n");

    // Check a bit: (v >> n) & 1
    flags = 0b10110100;
    printf("\ncheck target: "); print_bits8(flags); printf("\n");
    for (int i = 7; i >= 0; i--) {
        printf("  bit%d = %d\n", i, (flags >> i) & 1);
    }

    // =========================================================
    // Mask operations (extract a specific range of bits)
    // =========================================================
    printf("\n=== Mask Operations ===\n");
    uint32_t pixel = 0xABCDEF12;
    uint8_t r  = (pixel >> 24) & 0xFF;  // top 8 bits
    uint8_t g  = (pixel >> 16) & 0xFF;
    uint8_t bv = (pixel >>  8) & 0xFF;
    uint8_t al = (pixel >>  0) & 0xFF;  // bottom 8 bits
    printf("pixel = 0x%08X\n", pixel);
    printf("R=%02X, G=%02X, B=%02X, A=%02X\n", r, g, bv, al);

    // =========================================================
    // Power-of-two check: n & (n-1) == 0
    // =========================================================
    printf("\n=== Power-of-Two Check ===\n");
    uint32_t vals[] = {1, 2, 3, 4, 8, 12, 16, 0};
    for (int i = 0; i < 8; i++) {
        uint32_t n = vals[i];
        int is_pow2 = (n != 0) && ((n & (n - 1)) == 0);
        printf("%3u: %s\n", n, is_pow2 ? "power of 2" : "not a power of 2");
    }

    // =========================================================
    // Round up to the next multiple (commonly used in allocators)
    // Example: round up to the next multiple of 8
    // =========================================================
    printf("\n=== Alignment Round-Up (round n up to a multiple of align) ===\n");
    #define ALIGN_UP(n, align)  (((n) + (align) - 1) & ~((align) - 1))
    for (size_t sz = 0; sz <= 20; sz += 3) {
        printf("ALIGN_UP(%2zu, 8) = %zu\n", sz, ALIGN_UP(sz, 8));
    }

    // =========================================================
    // Endianness detection
    // =========================================================
    printf("\n=== Endianness Detection ===\n");
    uint32_t test = 0x01020304;
    uint8_t *bytes = (uint8_t *)&test;
    printf("byte order of 0x01020304: %02X %02X %02X %02X\n",
           bytes[0], bytes[1], bytes[2], bytes[3]);
    if (bytes[0] == 0x04) {
        printf("-> little-endian (x86/x64)\n");
    } else {
        printf("-> big-endian\n");
    }

    // =========================================================
    // Byte swap (endianness conversion)
    // =========================================================
    printf("\n=== Byte Swap (Endianness Conversion) ===\n");
    uint32_t be = 0xAABBCCDD;
    uint32_t le = ((be & 0xFF000000) >> 24) |
                  ((be & 0x00FF0000) >>  8) |
                  ((be & 0x0000FF00) <<  8) |
                  ((be & 0x000000FF) << 24);
    printf("original:   "); print_bits32(be); printf("\n");
    printf("byte-swapped: "); print_bits32(le); printf("\n");

    return 0;
}
