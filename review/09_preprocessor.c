// 09_preprocessor.c — プリプロセッサ・マクロ
// ビルド: clang -std=c11 -Wall -Wextra -o 09_preprocessor 09_preprocessor.c && ./09_preprocessor
// デバッグビルド: clang -std=c11 -DDEBUG -o 09_preprocessor 09_preprocessor.c && ./09_preprocessor

#include <stdio.h>
#include <assert.h>

// =========================================================
// 定数マクロ（#define）
// const int よりも型なし。enum や static const を使う方が安全
// =========================================================
#define PI        3.14159265358979
#define MAX_BUF   256
#define APP_NAME  "MyApp"

// =========================================================
// 関数マクロ — 展開時にそのまま置換される
// ① 引数は必ず () で囲む
// ② マクロ全体も () で囲む
// ③ 複数回評価される副作用に注意
// =========================================================
#define SQUARE(x)    ((x) * (x))          // OK: 括弧あり
#define SQUARE_NG(x)  x * x               // NG: 括弧なし
#define MAX(a, b)    ((a) > (b) ? (a) : (b))
#define MIN(a, b)    ((a) < (b) ? (a) : (b))
#define ABS(x)       ((x) < 0 ? -(x) : (x))

// 配列長（崩壊前のみ有効）
#define ARRAY_LEN(arr)  (sizeof(arr) / sizeof((arr)[0]))

// =========================================================
// do { ... } while(0) イディオム
// 複文マクロを if/else の中で安全に使うために必要
// =========================================================
// 型を明示するバージョン（標準 C11 で動作）
#define SWAP(type, a, b)  do { \
    type _tmp = (a);           \
    (a) = (b);                 \
    (b) = _tmp;                \
} while(0)

// =========================================================
// 条件コンパイル（#ifdef / #ifndef / #if）
// =========================================================
#ifdef DEBUG
    #define LOG(fmt, ...) \
        fprintf(stderr, "[DEBUG] %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
    #define LOG(fmt, ...)  ((void)0)  // 何もしない（最適化で消える）
#endif

// =========================================================
// 文字列化（#）とトークン結合（##）
// =========================================================
#define STRINGIFY(x)   #x              // x を文字列リテラルにする
#define CONCAT(a, b)   a##b            // a と b をトークンとして結合

// =========================================================
// 事前定義マクロ（コンパイラが提供）
// =========================================================
// __FILE__   : 現在のファイル名（文字列）
// __LINE__   : 現在の行番号（整数）
// __func__   : 現在の関数名（C99以降）
// __DATE__   : コンパイル日（"Jan  1 2025" 形式）
// __TIME__   : コンパイル時刻（"12:34:56" 形式）
// __STDC__   : C標準準拠なら 1

int main(void) {

    printf("=== 定数マクロ ===\n");
    printf("PI      = %f\n", PI);
    printf("MAX_BUF = %d\n", MAX_BUF);
    printf("APP_NAME = %s\n", APP_NAME);

    // =========================================================
    // 関数マクロの副作用の罠
    // =========================================================
    printf("\n=== マクロの副作用の罠 ===\n");
    int v = 3;

    printf("SQUARE(v)      = %d\n", SQUARE(v));     // 9（OK）
    printf("SQUARE(v + 1)  = %d\n", SQUARE(v + 1)); // 16（OK: 括弧あり）
    printf("SQUARE_NG(v+1) = %d\n", SQUARE_NG(v+1));// 7（NG: 3+1*3+1 = 7）

    // 副作用: ++ が 2 回評価される
    int x = 3;
    int result = MAX(x++, 2);  // x++ が 2 回評価されて x = 5 になる
    printf("MAX(x++, 2): result=%d, x=%d（x が 2 回インクリメント）\n", result, x);
    // → 副作用のある式をマクロ引数に渡さない

    printf("\n=== MAX / MIN / ABS ===\n");
    printf("MAX(4, 7)  = %d\n", MAX(4, 7));
    printf("MIN(4, 7)  = %d\n", MIN(4, 7));
    printf("ABS(-5)    = %d\n", ABS(-5));

    printf("\n=== ARRAY_LEN ===\n");
    int arr[] = {10, 20, 30, 40, 50};
    printf("ARRAY_LEN(arr) = %zu\n", ARRAY_LEN(arr));

    printf("\n=== SWAP（do-while マクロ）===\n");
    int a = 1, b = 2;
    printf("before: a=%d, b=%d\n", a, b);
    SWAP(int, a, b);
    printf("after:  a=%d, b=%d\n", a, b);

    // =========================================================
    // 条件コンパイル
    // =========================================================
    printf("\n=== 条件コンパイル ===\n");
    LOG("デバッグメッセージ: x=%d", x);  // -DDEBUG 時のみ出力
#ifdef DEBUG
    printf("DEBUG ビルドです\n");
#else
    printf("RELEASE ビルドです（-DDEBUG でデバッグビルド）\n");
#endif

    // =========================================================
    // 文字列化とトークン結合
    // =========================================================
    printf("\n=== 文字列化（#）===\n");
    printf("STRINGIFY(hello) = %s\n", STRINGIFY(hello));
    printf("STRINGIFY(42)    = %s\n", STRINGIFY(42));
    printf("STRINGIFY(a + b) = %s\n", STRINGIFY(a + b));

    printf("\n=== トークン結合（##）===\n");
    int CONCAT(my_, value) = 42;   // int my_value = 42; に展開
    printf("my_value = %d\n", my_value);

    // =========================================================
    // 事前定義マクロ
    // =========================================================
    printf("\n=== 事前定義マクロ ===\n");
    printf("__FILE__ = %s\n", __FILE__);
    printf("__LINE__ = %d\n", __LINE__);
    printf("__func__ = %s\n", __func__);
    printf("__DATE__ = %s\n", __DATE__);
    printf("__TIME__ = %s\n", __TIME__);

    // =========================================================
    // assert — 条件が偽の場合にプログラムを中断（NDEBUG で無効化）
    // =========================================================
    printf("\n=== assert ===\n");
    int val = 5;
    assert(val > 0);         // OK: 通過する
    assert(val == 5);        // OK: 通過する
    printf("assert 通過\n");
    // assert(val == 0);     // NG: ここに来ると中断される

    return 0;
}
