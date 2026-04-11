// 07_memory.c — 動的メモリ管理（malloc / calloc / realloc / free）
// ビルド: clang -std=c11 -Wall -Wextra -o 07_memory 07_memory.c && ./07_memory
// UBを検出する場合: clang -std=c11 -fsanitize=address,undefined -o 07_memory 07_memory.c

#include <stdio.h>
#include <stdlib.h>  // malloc, calloc, realloc, free
#include <string.h>  // memset, memcpy

int main(void) {

    // =========================================================
    // malloc — 初期化なしでメモリを確保
    // 戻り値は void* → 使う型にキャストして使う
    // 失敗時は NULL を返す → 必ず確認する
    // =========================================================
    printf("=== malloc ===\n");
    int *arr = malloc(5 * sizeof(int));  // int 5 個分
    if (!arr) {                          // NULL チェック必須
        fprintf(stderr, "malloc failed\n");
        return 1;
    }
    // 初期化されていない（ゴミ値）→ 必ず自分で初期化する
    for (int i = 0; i < 5; i++) arr[i] = i * 10;
    for (int i = 0; i < 5; i++) printf("%d ", arr[i]);
    printf("\n");
    free(arr);
    arr = NULL;  // ダングリングポインタ防止（慣習）

    // =========================================================
    // calloc — ゼロ初期化して確保
    // malloc(n * size) + memset(0) の組み合わせ
    // =========================================================
    printf("\n=== calloc ===\n");
    double *dbls = calloc(4, sizeof(double));  // (要素数, サイズ)
    if (!dbls) { return 1; }
    // calloc はゼロ初期化を保証する
    for (int i = 0; i < 4; i++) printf("%.1f ", dbls[i]);  // 0.0 0.0 0.0 0.0
    printf("\n");
    free(dbls);
    dbls = NULL;

    // =========================================================
    // realloc — サイズを変更して再確保
    //
    // NG: v->data = realloc(v->data, new_size);
    //     → 失敗時に NULL が代入されて元のポインタが失われる
    //
    // OK: 必ず一時変数を使う
    // =========================================================
    printf("\n=== realloc ===\n");
    int *buf  = malloc(3 * sizeof(int));
    if (!buf) { return 1; }
    buf[0] = 1; buf[1] = 2; buf[2] = 3;
    printf("before realloc: ");
    for (int i = 0; i < 3; i++) printf("%d ", buf[i]);
    printf("\n");

    // NG パターン（コメントアウト）
    // buf = realloc(buf, 6 * sizeof(int));  // 失敗時に buf が NULL になる

    // OK パターン: 一時変数で受け取る
    int *tmp = realloc(buf, 6 * sizeof(int));
    if (!tmp) {
        free(buf);   // 元のポインタを解放してエラー処理
        return 1;
    }
    buf = tmp;  // 成功したら buf を更新
    buf[3] = 4; buf[4] = 5; buf[5] = 6;
    printf("after  realloc: ");
    for (int i = 0; i < 6; i++) printf("%d ", buf[i]);
    printf("\n");
    free(buf);
    buf = NULL;

    // =========================================================
    // free の規則
    // ① malloc/calloc/realloc で確保したポインタのみ解放可能
    // ② NULL を free するのは安全（何もしない）
    // ③ 二重解放は UB
    // ④ スタック変数のアドレスを free するのは UB
    // =========================================================
    printf("\n=== free の規則 ===\n");
    free(NULL);  // OK: 安全
    printf("free(NULL) は安全\n");

    int *p = malloc(sizeof(int));
    *p = 42;
    free(p);
    p = NULL;
    // free(p);  // OK: NULL の free は安全
    // *p = 1;   // NG: free 後のアクセス（Use-after-free）

    // =========================================================
    // 構造体の動的確保
    // =========================================================
    printf("\n=== 構造体の動的確保 ===\n");
    typedef struct { int x; int y; } Point;

    Point *pt = malloc(sizeof(Point));
    if (!pt) { return 1; }
    pt->x = 10;
    pt->y = 20;
    printf("pt = (%d, %d)\n", pt->x, pt->y);
    free(pt);
    pt = NULL;

    // =========================================================
    // 構造体配列の動的確保
    // =========================================================
    printf("\n=== 構造体配列の動的確保 ===\n");
    int n = 3;
    Point *points = calloc((size_t)n, sizeof(Point));
    if (!points) { return 1; }
    for (int i = 0; i < n; i++) {
        points[i].x = i;
        points[i].y = i * i;
    }
    for (int i = 0; i < n; i++) {
        printf("points[%d] = (%d, %d)\n", i, points[i].x, points[i].y);
    }
    free(points);
    points = NULL;

    // =========================================================
    // メモリ操作関数（string.h）
    // =========================================================
    printf("\n=== memset / memcpy / memmove ===\n");
    char data[8];

    // memset: バイト単位でセット（ゼロクリアによく使う）
    memset(data, 0, sizeof(data));
    printf("memset(0): ");
    for (int i = 0; i < 8; i++) printf("%02X ", (unsigned char)data[i]);
    printf("\n");

    // memcpy: 重なりのないコピー
    char src[] = "ABCDEFG";
    char dst[8];
    memcpy(dst, src, sizeof(src));
    printf("memcpy: %s\n", dst);

    // memmove: 重なりのある領域でも安全なコピー
    // 例: 配列内で要素をシフト（vec_insert等で使う）
    char shift[] = "ABCDE";
    // [1..4] を [2..5] にシフト（重なりあり）
    memmove(shift + 2, shift + 1, 4);
    shift[1] = 'X';
    printf("memmove: %s\n", shift);  // AXBCDE

    // =========================================================
    // スタック vs ヒープ
    // =========================================================
    printf("\n=== スタック vs ヒープ ===\n");
    // スタック: 関数終了時に自動解放、高速、サイズ制限あり
    int stack_arr[100];  // スタック（OK）
    (void)stack_arr;
    // int big[1000000];  // 大きすぎるとスタックオーバーフロー

    // ヒープ: 明示的に解放が必要、低速、サイズ制限大
    int *heap_arr = malloc(100 * sizeof(int));  // ヒープ（OK）
    if (heap_arr) {
        printf("ヒープに %zu bytes 確保\n", 100 * sizeof(int));
        free(heap_arr);
    }

    printf("\n完了\n");
    return 0;
}
