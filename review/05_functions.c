// 05_functions.c — 関数・引数の渡し方・プロトタイプ
// ビルド: clang -std=c11 -Wall -Wextra -o 05_functions 05_functions.c && ./05_functions

#include <stdio.h>
#include <stdlib.h>  // malloc, free

typedef struct { int x; int y; } Point;

// =========================================================
// プロトタイプ宣言（前方宣言）
// 定義より前に呼ぶ場合に必要
// ヘッダファイルに書くのが慣習
// =========================================================
int add(int a, int b);
Point point_add(Point a, Point b);
int  *alloc_array(size_t n);

// =========================================================
// 基本的な関数
// =========================================================
int add(int a, int b) {
    return a + b;
}

// void 関数（Rust の -> () に相当）
void print_point(const Point *p) {  // const * = 読み取り専用
    printf("(%d, %d)", p->x, p->y);
}

// =========================================================
// 構造体の値渡し（コピーされる）
// 小さい構造体なら OK、大きい場合はポインタ渡しを検討
// =========================================================
Point point_add(Point a, Point b) {
    return (Point){ a.x + b.x, a.y + b.y };
}

// =========================================================
// ポインタ渡しで結果を返す（out パラメータ）
// Rust には `&mut` があるが C では慣習的に使われる
// =========================================================
void point_add_out(const Point *a, const Point *b, Point *out) {
    out->x = a->x + b->x;
    out->y = a->y + b->y;
}

// =========================================================
// ヒープからメモリを確保して返す
// 呼び出し側が free() する責任を持つ
// =========================================================
int *alloc_array(size_t n) {
    int *arr = malloc(n * sizeof(int));
    if (!arr) return NULL;    // 確保失敗
    for (size_t i = 0; i < n; i++) arr[i] = (int)i;
    return arr;
}

// =========================================================
// static 関数 — このファイル内のみ有効（非公開）
// ヘッダに書かない内部実装に使う
// =========================================================
static int internal_helper(int x) {
    return x * 2;
}

// =========================================================
// inline ヒント（コンパイラへの提案）
// =========================================================
static inline int square(int x) {
    return x * x;
}

// =========================================================
// 再帰
// =========================================================
int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

// =========================================================
// 複数の値を返す — Rust の tuple 相当はないため選択肢がある
// ① 構造体で返す
// ② out パラメータを使う
// ③ グローバル変数（悪い習慣）
// =========================================================
typedef struct { int quot; int rem; } DivResult;

DivResult divide(int a, int b) {
    return (DivResult){ a / b, a % b };
}

int main(void) {

    printf("=== 基本関数 ===\n");
    printf("add(3, 4) = %d\n", add(3, 4));

    printf("\n=== 構造体の渡し方 ===\n");
    Point p1 = {1, 2};
    Point p2 = {3, 4};

    // ① 値渡し（コピー） → 戻り値で返す
    Point r1 = point_add(p1, p2);
    printf("値渡し: "); print_point(&r1); printf("\n");

    // ② ポインタ渡し（out パラメータ）
    Point r2;
    point_add_out(&p1, &p2, &r2);
    printf("ポインタ: "); print_point(&r2); printf("\n");

    printf("\n=== ヒープ確保した配列を返す ===\n");
    int *arr = alloc_array(5);
    if (arr) {
        for (int i = 0; i < 5; i++) printf("%d ", arr[i]);
        printf("\n");
        free(arr);   // 呼び出し側が解放する
        arr = NULL;  // ダングリングポインタ防止（dangling pointer）
    }

    printf("\n=== static / inline ===\n");
    printf("internal_helper(7) = %d\n", internal_helper(7));
    printf("square(9) = %d\n", square(9));

    printf("\n=== 再帰 ===\n");
    for (int i = 1; i <= 6; i++) {
        printf("%d! = %d\n", i, factorial(i));
    }

    printf("\n=== 複数値を返す ===\n");
    DivResult dr = divide(17, 5);
    printf("17 / 5 = %d 余り %d\n", dr.quot, dr.rem);

    printf("\n=== 関数に配列を渡す（崩壊に注意）===\n");
    int nums[] = {5, 3, 8, 1, 9};
    size_t len = sizeof(nums) / sizeof(nums[0]);
    // 関数の中では sizeof(nums) はポインタサイズになる → len を別で渡す
    printf("長さ %zu の配列: ", len);
    for (size_t i = 0; i < len; i++) printf("%d ", nums[i]);
    printf("\n");

    return 0;
}
