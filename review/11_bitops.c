// 11_bitops.c — ビット操作
// ビルド: clang -std=c11 -Wall -Wextra -o 11_bitops 11_bitops.c && ./11_bitops

#include <stdio.h>
#include <stdint.h>

// ビット列を表示するユーティリティ
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
    // 基本的なビット演算子
    // =========================================================
    printf("=== 基本ビット演算 ===\n");
    uint8_t a = 0b10110011;  // 0xB3
    uint8_t b = 0b11001010;  // 0xCA

    printf("a     = "); print_bits8(a); printf("\n");
    printf("b     = "); print_bits8(b); printf("\n");
    printf("a & b = "); print_bits8(a & b);  printf("  AND\n");
    printf("a | b = "); print_bits8(a | b);  printf("  OR\n");
    printf("a ^ b = "); print_bits8(a ^ b);  printf("  XOR\n");
    printf("~a    = "); print_bits8((uint8_t)~a); printf("  NOT\n");

    // =========================================================
    // シフト演算
    // 左シフト: × 2^n  右シフト: ÷ 2^n
    // 符号あり整数の右シフトは処理系依存 → uint を使う
    // =========================================================
    printf("\n=== シフト演算 ===\n");
    uint8_t v = 0b00001100;  // 12
    printf("v      = "); print_bits8(v); printf("\n");
    printf("v << 2 = "); print_bits8((uint8_t)(v << 2)); printf("  (*4 = 48)\n");
    printf("v >> 1 = "); print_bits8((uint8_t)(v >> 1)); printf("  (/2 = 6)\n");

    // =========================================================
    // ビット操作の定番パターン
    // =========================================================
    printf("\n=== ビット操作パターン ===\n");
    uint8_t flags = 0b00000000;
    printf("初期値:       "); print_bits8(flags); printf("\n");

    // ビットをセット（1 にする）: |= (1 << n)
    flags |= (1 << 3);   // ビット3をセット
    flags |= (1 << 5);   // ビット5をセット
    printf("bit3,5 SET:   "); print_bits8(flags); printf("\n");

    // ビットをクリア（0 にする）: &= ~(1 << n)
    flags &= ~(1 << 3);  // ビット3をクリア
    printf("bit3 CLEAR:   "); print_bits8(flags); printf("\n");

    // ビットを反転: ^= (1 << n)
    flags ^= (1 << 5);   // ビット5を反転
    printf("bit5 TOGGLE:  "); print_bits8(flags); printf("\n");

    // ビットを確認: (v >> n) & 1
    flags = 0b10110100;
    printf("\n確認対象:     "); print_bits8(flags); printf("\n");
    for (int i = 7; i >= 0; i--) {
        printf("  bit%d = %d\n", i, (flags >> i) & 1);
    }

    // =========================================================
    // マスク操作（特定のビット範囲を取り出す）
    // =========================================================
    printf("\n=== マスク操作 ===\n");
    uint32_t pixel = 0xABCDEF12;
    uint8_t r = (pixel >> 24) & 0xFF;  // 上位8ビット
    uint8_t g = (pixel >> 16) & 0xFF;
    uint8_t bv = (pixel >>  8) & 0xFF;
    uint8_t al = (pixel >>  0) & 0xFF;  // 下位8ビット
    printf("pixel = 0x%08X\n", pixel);
    printf("R=%02X, G=%02X, B=%02X, A=%02X\n", r, g, bv, al);

    // =========================================================
    // 2の冪の判定: n & (n-1) == 0
    // =========================================================
    printf("\n=== 2の冪の判定 ===\n");
    uint32_t vals[] = {1, 2, 3, 4, 8, 12, 16, 0};
    for (int i = 0; i < 8; i++) {
        uint32_t n = vals[i];
        int is_pow2 = (n != 0) && ((n & (n - 1)) == 0);
        printf("%3u: %s\n", n, is_pow2 ? "2の冪" : "違う");
    }

    // =========================================================
    // 上位ビットへの切り上げ（アロケータでよく使う）
    // 例: 次の 8 の倍数に切り上げ
    // =========================================================
    printf("\n=== アライメント切り上げ（n を align の倍数に）===\n");
    #define ALIGN_UP(n, align)  (((n) + (align) - 1) & ~((align) - 1))
    for (size_t sz = 0; sz <= 20; sz += 3) {
        printf("ALIGN_UP(%2zu, 8) = %zu\n", sz, ALIGN_UP(sz, 8));
    }

    // =========================================================
    // エンディアン確認
    // =========================================================
    printf("\n=== エンディアン確認 ===\n");
    uint32_t test = 0x01020304;
    uint8_t *bytes = (uint8_t *)&test;
    printf("0x01020304 のバイト順: %02X %02X %02X %02X\n",
           bytes[0], bytes[1], bytes[2], bytes[3]);
    if (bytes[0] == 0x04) {
        printf("→ リトルエンディアン（x86/x64）\n");
    } else {
        printf("→ ビッグエンディアン\n");
    }

    // =========================================================
    // バイトスワップ（エンディアン変換）
    // =========================================================
    printf("\n=== バイトスワップ（エンディアン変換）===\n");
    uint32_t be = 0xAABBCCDD;
    uint32_t le = ((be & 0xFF000000) >> 24) |
                  ((be & 0x00FF0000) >>  8) |
                  ((be & 0x0000FF00) <<  8) |
                  ((be & 0x000000FF) << 24);
    printf("元:          "); print_bits32(be); printf("\n");
    printf("バイトスワップ後: "); print_bits32(le); printf("\n");

    return 0;
}
