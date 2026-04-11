// 03_arrays.c — 配列とポインタの関係
// ビルド: clang -std=c11 -Wall -Wextra -o 03_arrays 03_arrays.c && ./03_arrays

#include <stdio.h>
#include <string.h>  // memcpy

// 配列を関数に渡すと「ポインタに崩壊」してサイズが失われる
// → 長さを別引数で渡す必要がある
void print_array(const int *arr, size_t len) {
    for (size_t i = 0; i < len; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
}

// 配列サイズをマクロで取得（ポインタに崩壊した後は使えない）
#define ARRAY_LEN(arr)  (sizeof(arr) / sizeof((arr)[0]))

int main(void) {

    // =========================================================
    // 固定長配列の宣言と初期化
    // =========================================================
    printf("=== 配列の宣言 ===\n");
    int a[5] = {1, 2, 3, 4, 5};     // 完全初期化
    int b[5] = {1, 2};              // 残りはゼロ初期化
    int c[5] = {0};                 // 全要素ゼロ
    int d[]  = {10, 20, 30};        // サイズ自動推論 → d[3]

    printf("a: "); print_array(a, ARRAY_LEN(a));
    printf("b: "); print_array(b, ARRAY_LEN(b));
    printf("c: "); print_array(c, ARRAY_LEN(c));
    printf("d: "); print_array(d, ARRAY_LEN(d));

    // =========================================================
    // 配列名はポインタに「崩壊（decay）」する
    // 配列名 == 先頭要素のアドレス
    // =========================================================
    printf("\n=== 配列とポインタの関係 ===\n");
    int  arr[4] = {10, 20, 30, 40};
    int *p      = arr;          // &arr[0] と同じ
    printf("arr[0]  = %d\n", arr[0]);
    printf("*p      = %d\n", *p);         // 同じ値
    printf("p[1]    = %d\n", p[1]);       // arr[1] と同じ
    printf("*(p+2)  = %d\n", *(p + 2));   // arr[2] と同じ

    // sizeof は崩壊前でしか使えない
    printf("sizeof(arr) = %zu（配列全体）\n", sizeof(arr));
    printf("sizeof(p)   = %zu（ポインタのサイズ）\n", sizeof(p));
    // → 関数の引数で受け取った後は sizeof でサイズを得られない

    // =========================================================
    // ポインタ演算
    // p + n は (char*)p + n * sizeof(*p) と等価
    // =========================================================
    printf("\n=== ポインタ演算 ===\n");
    int nums[5] = {0, 10, 20, 30, 40};
    int *start  = nums;
    int *end    = nums + 5;  // 末尾の「1つ先」（デリファレンス不可）

    for (int *q = start; q < end; q++) {
        printf("%d ", *q);
    }
    printf("\n");

    // ポインタ間の差 → ptrdiff_t
    ptrdiff_t dist = end - start;  // = 5
    printf("end - start = %td\n", dist);

    // =========================================================
    // 2次元配列
    // =========================================================
    printf("\n=== 2次元配列 ===\n");
    int matrix[3][4] = {
        {1,  2,  3,  4},
        {5,  6,  7,  8},
        {9, 10, 11, 12},
    };
    // メモリ上は行優先（row-major）で連続している
    // matrix[r][c] == *(matrix[0] + r * 4 + c)
    for (int r = 0; r < 3; r++) {
        for (int c = 0; c < 4; c++) {
            printf("%3d", matrix[r][c]);
        }
        printf("\n");
    }

    // =========================================================
    // VLA（可変長配列）— C99以降、スタック上に確保
    // スタックオーバーフローの危険があるため実務では避けるべき
    // 大きなデータには malloc を使う
    // =========================================================
    printf("\n=== VLA（可変長配列）===\n");
    int n = 5;
    int vla[n];                     // n はコンパイル時定数でなくてよい
    for (int i = 0; i < n; i++) vla[i] = i * i;
    print_array(vla, (size_t)n);
    // sizeof(vla) は実行時に評価される

    // =========================================================
    // memcpy での配列コピー
    // 配列の代入（arr2 = arr1）はできない → memcpy を使う
    // =========================================================
    printf("\n=== 配列のコピー（memcpy）===\n");
    int src[4] = {1, 2, 3, 4};
    int dst[4];
    memcpy(dst, src, sizeof(src));
    // dst = src;  // ← NG: コンパイルエラー
    printf("dst: "); print_array(dst, ARRAY_LEN(dst));

    // =========================================================
    // 配列の範囲外アクセス — コンパイルエラーにならない（UB）
    // -fsanitize=address で検出可能
    // =========================================================
    printf("\n=== 範囲外アクセス（コメントアウト済み）===\n");
    int safe[3] = {1, 2, 3};
    printf("safe[2] = %d（OK）\n", safe[2]);
    // printf("safe[3] = %d（UB！）\n", safe[3]);   // ← 絶対にやらない

    return 0;
}
