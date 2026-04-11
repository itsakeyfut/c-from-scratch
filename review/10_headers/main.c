// 10_headers/main.c — ヘッダファイル分割のデモ
// ビルド: clang -std=c11 -Wall -Wextra -o 10_headers main.c mylib.c && ./10_headers
//         ※ Windows では -lm 不要（Linux/macOS では -lm が必要）

#include <stdio.h>
#include "mylib.h"   // ダブルクォートで相対パス（<> はシステムヘッダ用）

int main(void) {

    printf("=== ヘッダ分割のデモ ===\n");

    // mylib.h で定義した型・関数を使う
    Point p1 = point_new(3, 4);
    Point p2 = point_new(6, 8);

    printf("p1 = "); point_print(&p1); printf("\n");
    printf("p2 = "); point_print(&p2); printf("\n");

    Point sum = point_add(p1, p2);
    printf("p1 + p2 = "); point_print(&sum); printf("\n");

    double dist = point_dist(p1, p2);
    printf("dist(p1, p2) = %.4f\n", dist);

    printf("p1 == p1: %d\n", point_eq(p1, p1));
    printf("p1 == p2: %d\n", point_eq(p1, p2));

    printf("MYLIB_VERSION = %d\n", MYLIB_VERSION);

    // =========================================================
    // インクルードガードの意味
    // =========================================================
    // mylib.h を 2 回 include してもエラーにならない
    // #ifndef MYLIB_H が 2 回目のインクルードをスキップする
    #include "mylib.h"  // 2 回目: 何も起きない（ガードが防ぐ）

    printf("\n完了\n");
    return 0;
}
