// 01_types.c — 基本型・変数宣言
// ビルド: clang -std=c11 -Wall -Wextra -o 01_types 01_types.c && ./01_types

#include <stdio.h>
#include <stdint.h>   // uint8_t, int32_t など固定幅型
#include <stdbool.h>  // bool, true, false
#include <stddef.h>   // size_t, ptrdiff_t
#include <limits.h>   // INT_MAX など

int main(void) {

    // =========================================================
    // 基本型のサイズ（環境依存に注意）
    // =========================================================
    printf("=== 基本型のサイズ ===\n");
    printf("char:       %zu byte\n", sizeof(char));       // 常に 1
    printf("short:      %zu byte\n", sizeof(short));      // 通常 2
    printf("int:        %zu byte\n", sizeof(int));        // 通常 4
    printf("long:       %zu byte\n", sizeof(long));       // 32bit=4, 64bit=8
    printf("long long:  %zu byte\n", sizeof(long long));  // 常に 8
    printf("float:      %zu byte\n", sizeof(float));      // 通常 4
    printf("double:     %zu byte\n", sizeof(double));     // 通常 8
    printf("void*:      %zu byte\n", sizeof(void *));     // 64bit環境=8

    // =========================================================
    // 固定幅整数型（stdint.h） — サイズが保証される
    // 実装では int の代わりにこれを使う
    // =========================================================
    printf("\n=== 固定幅整数型（推奨） ===\n");
    uint8_t  u8  = 255;
    int8_t   i8  = -128;
    uint32_t u32 = 0xDEADBEEF;
    int64_t  i64 = -9000000000LL;
    printf("uint8_t  255  → %u\n",   u8);
    printf("int8_t  -128  → %d\n",   i8);
    printf("uint32_t hex  → 0x%X\n", u32);
    printf("int64_t       → %lld\n", i64);

    // =========================================================
    // size_t — 配列インデックスやメモリサイズは常にこれを使う
    // unsigned なので負の値を持てない（signed との比較に注意）
    // =========================================================
    printf("\n=== size_t / ptrdiff_t ===\n");
    size_t    len  = 100;   // 配列の長さ・バイト数
    ptrdiff_t diff = -5;    // ポインタ間の差（signed）
    printf("size_t:     %zu\n", len);
    printf("ptrdiff_t:  %td\n", diff);

    // =========================================================
    // bool（C99以降、stdbool.h が必要）
    // 0 = false、それ以外は true
    // =========================================================
    printf("\n=== bool ===\n");
    bool t = true;
    bool f = false;
    printf("true=%d, false=%d\n", t, f);
    printf("(bool)42  → %d\n", (bool)42);   // 1
    printf("(bool)-1  → %d\n", (bool)-1);   // 1
    printf("(bool)0   → %d\n", (bool)0);    // 0

    // =========================================================
    // 整数の暗黙昇格の罠（-Wall で警告が出る）
    // signed と unsigned を混在させると予期しない結果になる
    // =========================================================
    printf("\n=== 符号付き vs 符号なしの罠 ===\n");
    unsigned int u = 1;
    int          i = -1;
    // -1 は int だが、unsigned int と比較すると unsigned に変換される
    // → -1 が 4294967295 になり、1 より大きくなる
    if ((unsigned)i > u) {
        printf("-1 > 1u になる（-1 が %u に変換される）\n", (unsigned)i);
    }
    // 対策: 常に同じ型で比較する
    if ((size_t)i > len) { /* len は size_t なので注意 */ }

    // =========================================================
    // const — 値の変更を禁止する
    // =========================================================
    printf("\n=== const ===\n");
    const int  MAX = 100;           // 変更不可の値
    const char *s  = "hello";       // ポインタが指す先を変更不可
    char       *const p = (char*)""; // ポインタ自体を変更不可（指す先はOK）
    printf("MAX=%d, s=%s\n", MAX, s);
    (void)p;  // 未使用警告を抑止

    // =========================================================
    // 変数の初期化ルール
    // グローバル変数: 自動的にゼロ初期化
    // ローカル変数:   初期化しないとゴミ値（UB！）
    // =========================================================
    printf("\n=== 初期化 ===\n");
    int initialized = 0;
    // int uninitialized;         // ← これを読むと UB
    // printf("%d\n", uninitialized); // NG
    printf("initialized: %d\n", initialized);

    return 0;
}
