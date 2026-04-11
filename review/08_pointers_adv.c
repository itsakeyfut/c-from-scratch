// 08_pointers_adv.c — void* / 関数ポインタ / ポインタ演算
// ビルド: clang -std=c11 -Wall -Wextra -o 08_pointers_adv 08_pointers_adv.c && ./08_pointers_adv

#include <stdio.h>
#include <stdlib.h>  // qsort
#include <string.h>  // memcpy

// =========================================================
// 関数ポインタの typedef（読みやすくする）
// =========================================================
typedef int (*CmpFn)(const void *, const void *);
typedef void (*PrintFn)(int);

// 比較関数（qsort 用）
static int cmp_asc(const void *a, const void *b) {
    return *(const int *)a - *(const int *)b;
}
static int cmp_desc(const void *a, const void *b) {
    return *(const int *)b - *(const int *)a;
}

// 関数ポインタを受け取る高階関数
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
    // void* — 型情報を持たない汎用ポインタ
    // malloc の戻り値が void* なのもこのため
    // =========================================================
    printf("=== void* ===\n");
    int    i_val  = 42;
    double d_val  = 3.14;
    void  *vp;

    vp = &i_val;
    printf("void* → int*: %d\n", *(int *)vp);   // キャストしてデリファレンス

    vp = &d_val;
    printf("void* → double*: %.2f\n", *(double *)vp);

    // void* のままではデリファレンス不可、演算も不可
    // *(vp)       // NG: コンパイルエラー
    // vp + 1      // NG: UB（char* にキャストしてから演算）
    char *cp = (char *)vp + 1;  // OK: char* に変換してから
    (void)cp;

    // =========================================================
    // void* + elem_size によるジェネリックな操作
    // Vec の内部で使うパターン
    // =========================================================
    printf("\n=== void* + elem_size（Vecの内部パターン）===\n");
    int   int_arr[4]    = {10, 20, 30, 40};
    double dbl_arr[3]   = {1.1, 2.2, 3.3};

    // 要素 i を取得するユーティリティ（Vec の vec_get 相当）
    #define ELEM(base, i, sz)  ((void *)((char *)(base) + (i) * (sz)))

    void *elem2 = ELEM(int_arr, 2, sizeof(int));
    printf("int_arr[2] = %d\n", *(int *)elem2);

    void *elem1 = ELEM(dbl_arr, 1, sizeof(double));
    printf("dbl_arr[1] = %.1f\n", *(double *)elem1);

    // =========================================================
    // 関数ポインタの宣言と呼び出し
    // =========================================================
    printf("\n=== 関数ポインタ ===\n");

    // typedef なし（読みにくい構文）
    int (*raw_cmp)(const void *, const void *) = cmp_asc;
    printf("raw_cmp 関数ポインタ: %p\n", (void *)raw_cmp);

    // typedef あり（推奨）
    CmpFn fn = cmp_asc;
    int a = 5, b = 3;
    printf("cmp_asc(5, 3) = %d\n", fn(&a, &b));  // 正 → 5 > 3

    fn = cmp_desc;
    printf("cmp_desc(5, 3) = %d\n", fn(&a, &b)); // 負 → 5 < 3（降順では）

    // =========================================================
    // 関数ポインタを使ったソート
    // =========================================================
    printf("\n=== 関数ポインタで動作を切り替え ===\n");
    int nums[] = {5, 2, 8, 1, 9, 3};
    size_t n = sizeof(nums) / sizeof(nums[0]);

    my_sort(nums, n, cmp_asc);
    printf("昇順: "); apply(nums, n, print_int);

    my_sort(nums, n, cmp_desc);
    printf("降順: "); apply(nums, n, print_int);

    printf("16進: "); apply(nums, n, print_hex);
    printf("二乗: "); apply(nums, n, print_squared);

    // =========================================================
    // 関数ポインタの配列（ジャンプテーブル）
    // =========================================================
    printf("\n=== 関数ポインタの配列（ジャンプテーブル）===\n");
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
    // char* でのポインタ演算（バイト単位）
    // =========================================================
    printf("\n=== char* ポインタ演算（バイト操作）===\n");
    typedef struct { int x; int y; } Point;
    Point pts[3] = {{1,2},{3,4},{5,6}};

    // Point 配列をバイト列として走査
    char  *base      = (char *)pts;
    size_t elem_size = sizeof(Point);

    for (int i = 0; i < 3; i++) {
        Point *p = (Point *)(base + i * elem_size);
        printf("pts[%d] = (%d, %d)\n", i, p->x, p->y);
    }

    // =========================================================
    // restrict — 「このポインタだけがその領域を指す」コンパイラヒント
    // memcpy の宣言: void *memcpy(void * restrict dst, const void * restrict src, ...)
    // → dst と src が重なっていないことを保証する
    // =========================================================
    printf("\n=== restrict（説明のみ）===\n");
    printf("restrict: 最適化ヒント。dst と src が重ならないことを保証\n");
    printf("重なりがある場合は memmove を使う\n");

    return 0;
}
