// 04_structs.c — 構造体・typedef・enum・union
// ビルド: clang -std=c11 -Wall -Wextra -o 04_structs 04_structs.c && ./04_structs

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>   // memset

// =========================================================
// struct の基本定義
// =========================================================
struct Point {
    int x;
    int y;
};

// =========================================================
// typedef struct — 毎回 struct を書かずに済む（慣習）
// =========================================================
typedef struct {
    float x;
    float y;
    float z;
} Vec3;

// =========================================================
// 自己参照（linked list ノードなど）
// typedef と struct タグを両方使う必要がある
// =========================================================
typedef struct Node {
    int          value;
    struct Node *next;  // 自分自身の型を参照するため struct タグが必要
} Node;

// =========================================================
// enum — 名前付き整数定数
// Rust の enum と違い、値を持てない（ただのint）
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
// union — 同じメモリを複数の型で解釈する
// sizeof は最大メンバのサイズ
// =========================================================
typedef union {
    uint32_t as_u32;
    uint8_t  bytes[4];
} U32Bytes;

// =========================================================
// ビットフィールド — 構造体のビット単位割り当て
// =========================================================
typedef struct {
    uint8_t r : 5;  // 5ビット（0〜31）
    uint8_t g : 6;  // 6ビット（0〜63）
    uint8_t b : 5;  // 5ビット（0〜31）
} RGB565;

// =========================================================
// 構造体の初期化方法
// =========================================================
Vec3 vec3_add(Vec3 a, Vec3 b) {
    // C99 指示付き初期化子（designated initializer）で返す
    return (Vec3){ a.x + b.x, a.y + b.y, a.z + b.z };
}

int main(void) {

    // --- struct の使い方 ---
    printf("=== struct ===\n");
    struct Point p1 = {3, 4};          // 順序通り初期化
    struct Point p2 = {.x = 10, .y = 20}; // 指示付き初期化子（推奨）
    printf("p1: (%d, %d)\n", p1.x, p1.y);
    printf("p2: (%d, %d)\n", p2.x, p2.y);

    // コピーは代入でOK（memcpy 不要）
    struct Point p3 = p1;
    p3.x = 99;
    printf("p3.x=99 後: p1.x=%d（コピーなので変わらない）\n", p1.x);

    // --- typedef struct ---
    printf("\n=== typedef struct ===\n");
    Vec3 v1 = {1.0f, 2.0f, 3.0f};
    Vec3 v2 = {.x = 4.0f, .y = 5.0f, .z = 6.0f};
    Vec3 v3 = vec3_add(v1, v2);
    printf("v3 = (%.1f, %.1f, %.1f)\n", v3.x, v3.y, v3.z);
    printf("sizeof(Vec3) = %zu\n", sizeof(Vec3));

    // ゼロ初期化（C では {} が使えないので memset か {0} を使う）
    Vec3 zero = {0};           // すべてゼロ
    memset(&zero, 0, sizeof(zero)); // 同じ意味
    printf("zero = (%.1f, %.1f, %.1f)\n", zero.x, zero.y, zero.z);

    // --- ポインタ経由のアクセス ---
    printf("\n=== 構造体ポインタ ===\n");
    Vec3 *vp = &v1;
    printf("vp->x = %.1f\n", vp->x);   // (*vp).x と同じ
    vp->x = 100.0f;
    printf("v1.x after vp->x=100: %.1f\n", v1.x);

    // --- enum ---
    printf("\n=== enum ===\n");
    Direction dir = DIR_NORTH;
    printf("DIR_NORTH = %d\n", dir);

    // switch で enum を使う（フォールスルーに注意）
    switch (dir) {
        case DIR_NORTH: printf("North\n"); break;
        case DIR_EAST:  printf("East\n");  break;
        case DIR_SOUTH: printf("South\n"); break;
        case DIR_WEST:  printf("West\n");  break;
        // default を省くと -Wall で警告が出る（全ケースを網羅すれば OK）
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
    // リトルエンディアンなら bytes[0] = 0x78

    printf("sizeof(union) = %zu（最大メンバのサイズ）\n", sizeof(u));

    // --- ビットフィールド ---
    printf("\n=== ビットフィールド ===\n");
    RGB565 rgb = {.r = 31, .g = 63, .b = 31};  // 最大値
    printf("R=%u, G=%u, B=%u\n", rgb.r, rgb.g, rgb.b);
    printf("sizeof(RGB565) = %zu\n", sizeof(rgb));  // 通常 2 バイト

    // --- パディングとアライメント ---
    printf("\n=== パディングとアライメント ===\n");
    typedef struct { char a; int b; char c; } Padded;
    typedef struct { int b; char a; char c; } Packed;
    // Padded: a(1) + pad(3) + b(4) + c(1) + pad(3) = 12
    // Packed: b(4) + a(1) + c(1) + pad(2) = 8
    printf("Padded: %zu bytes\n", sizeof(Padded));
    printf("Packed: %zu bytes\n", sizeof(Packed));
    // → フィールドの順序でサイズが変わる

    return 0;
}
