// 13_ub.c — 未定義動作（UB）と落とし穴
// ビルド（通常）:    clang -std=c11 -Wall -Wextra -o 13_ub 13_ub.c
// ビルド（検出用）:  clang -std=c11 -fsanitize=address,undefined -o 13_ub 13_ub.c
//
// ★ このファイルの UB パターンはすべてコメントアウトしてある
//   「なぜ危険か」を読んで理解することが目的

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>

int main(void) {

    // =========================================================
    // UB 1: 初期化されていない変数の読み取り
    // =========================================================
    printf("=== UB-1: 未初期化変数 ===\n");
    {
        int x = 0;      // OK: 初期化済み
        printf("x = %d（初期化済み）\n", x);

        // int y;
        // printf("y = %d\n", y);   // NG: ゴミ値 or UB
        //                          // -fsanitize=memory で検出
        printf("→ ローカル変数は必ず初期化する\n");
    }

    // =========================================================
    // UB 2: NULL ポインタのデリファレンス
    // =========================================================
    printf("\n=== UB-2: NULL デリファレンス ===\n");
    {
        int *p = NULL;
        printf("p = %p（NULL）\n", (void *)p);

        // *p = 42;         // NG: セグフォルト（クラッシュする）
        // int x = *p;      // NG: 同様

        // 正しいパターン: 使う前に NULL チェック
        if (p != NULL) {
            printf("*p = %d\n", *p);
        } else {
            printf("p が NULL のため読み取りをスキップ\n");
        }
    }

    // =========================================================
    // UB 3: Use-after-free（解放済みメモリへのアクセス）
    // =========================================================
    printf("\n=== UB-3: Use-after-free ===\n");
    {
        int *p = malloc(sizeof(int));
        *p = 42;
        printf("*p = %d（解放前）\n", *p);

        free(p);
        p = NULL;   // 解放後は NULL を代入して誤使用を防ぐ（慣習）

        // *p = 100;    // NG: 解放済みメモリへの書き込み（UB）
        // printf("%d", *p);  // NG: 解放済みメモリの読み取り（UB）
        // free(p);            // OK: NULL の free は安全

        printf("→ free 後は NULL を代入する慣習\n");
    }

    // =========================================================
    // UB 4: 二重解放（double free）
    // =========================================================
    printf("\n=== UB-4: 二重解放 ===\n");
    {
        int *p = malloc(sizeof(int));
        *p = 1;
        free(p);
        p = NULL;
        free(p);   // OK: NULL の free は安全（何もしない）
        printf("→ NULL を代入しておけば二重解放しても安全\n");

        // 危険なパターン:
        // int *q = malloc(sizeof(int));
        // free(q);
        // free(q);    // NG: 同じポインタを 2 回 free（クラッシュまたは heap 破壊）
    }

    // =========================================================
    // UB 5: 配列の範囲外アクセス
    // =========================================================
    printf("\n=== UB-5: 配列の範囲外アクセス ===\n");
    {
        int arr[5] = {1, 2, 3, 4, 5};
        printf("arr[4] = %d（最後の有効インデックス）\n", arr[4]);

        // printf("%d\n", arr[5]);    // NG: UB（-fsanitize=address で検出）
        // arr[5] = 99;               // NG: バッファオーバーフロー

        // 対策: サイズを保持してループで確認
        size_t len = 5;
        for (size_t i = 0; i < len; i++) {
            printf("arr[%zu] = %d\n", i, arr[i]);
        }
    }

    // =========================================================
    // UB 6: 符号付き整数オーバーフロー
    // 符号なし（uint）のオーバーフローは well-defined（ラップアラウンド）
    // 符号あり（int）のオーバーフローは UB
    // =========================================================
    printf("\n=== UB-6: 符号付き整数オーバーフロー ===\n");
    {
        int max = INT_MAX;
        printf("INT_MAX = %d\n", max);

        // int overflow = max + 1;    // NG: 符号あり整数オーバーフロー（UB）
        //                            // -fsanitize=undefined で検出

        // 安全な加算（オーバーフロー前にチェック）
        if (max < INT_MAX) {
            int ok = max + 1;
            printf("ok = %d\n", ok);
        } else {
            printf("→ INT_MAX + 1 はオーバーフローするためスキップ\n");
        }

        // 符号なしはラップアラウンド（UBではない）
        uint32_t u = UINT32_MAX;
        printf("UINT32_MAX + 1 = %u（ラップアラウンド）\n", u + 1);  // 0
    }

    // =========================================================
    // UB 7: void* のポインタ演算
    // =========================================================
    printf("\n=== UB-7: void* 演算 ===\n");
    {
        int arr[3] = {10, 20, 30};
        void *vp = arr;

        // vp + 1;              // NG: void* への加算は UB（GCC 拡張では動くが）
        // *(int *)vp + 1;      // OK: デリファレンス後の演算

        // 正しいパターン: char* に変換してからバイト単位で計算
        char *cp = (char *)vp;
        int  *ip = (int *)(cp + 1 * sizeof(int));   // arr[1] のアドレス
        printf("arr[1] via char* 演算 = %d\n", *ip);
    }

    // =========================================================
    // UB 8: 符号あり vs 符号なしの比較
    // =========================================================
    printf("\n=== 落とし穴: signed/unsigned 比較 ===\n");
    {
        int           i = -1;
        unsigned int  u = 1;
        (void)u;

        // -Wall で警告が出る
        // if (i < u) { ... }  // -1 が UINT_MAX になり u より大きくなる

        // 安全な比較
        if (i < 0) {
            printf("-1 は負数なので 1u より小さい（正しい判断）\n");
        }

        // size_t との比較でよくやるミス
        size_t len = 5;
        int    idx = -1;
        // if (idx < len) { ... }  // NG: idx が size_t に変換され巨大な正数に
        if (idx >= 0 && (size_t)idx < len) {
            printf("安全なインデックスチェック\n");
        } else {
            printf("idx = -1 → 範囲外\n");
        }
    }

    // =========================================================
    // UB 9: strict aliasing 違反
    // =========================================================
    printf("\n=== 落とし穴: strict aliasing ===\n");
    {
        // 異なる型のポインタで同じメモリを読み書きするとコンパイラが
        // 「別のオブジェクト」と仮定して最適化し誤動作することがある
        // 例外: char* はどんな型のエイリアスにもなれる

        uint32_t val = 0x12345678;

        // NG: int* と float* でエイリアスするのは UB
        // float *fp = (float *)&val;
        // printf("%f\n", *fp);

        // OK: memcpy を使ってビット表現を変換する（type punning）
        float f;
        memcpy(&f, &val, sizeof(f));
        printf("type punning via memcpy: %f\n", f);

        // OK: union を使う（C99以降）
        union { uint32_t u; float f; } pun = { .u = val };
        printf("type punning via union:  %f\n", pun.f);
    }

    // =========================================================
    // まとめ: -fsanitize=address,undefined の活用
    // =========================================================
    printf("\n=== -fsanitize=address,undefined の活用 ===\n");
    printf("上記の UB のほとんどはサニタイザで実行時に検出できる\n");
    printf("開発中は常に -fsanitize=address,undefined をつけること\n");
    printf("（docs/vector.md の「実装の壁と対策」参照）\n");

    return 0;
}
