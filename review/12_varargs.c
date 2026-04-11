// 12_varargs.c — 可変長引数（va_list）
// ビルド: clang -std=c11 -Wall -Wextra -o 12_varargs 12_varargs.c && ./12_varargs

#include <stdio.h>
#include <stdarg.h>   // va_list, va_start, va_arg, va_end

// =========================================================
// 可変長引数関数の基本構造
//
// ① 最低 1 つの固定引数が必要（va_start に渡す）
// ② va_start(args, last_fixed) で開始
// ③ va_arg(args, type) で 1 つずつ取り出す
// ④ va_end(args) で終了（必須）
// ⑤ 型は呼び出し側の責任（間違えると UB）
// =========================================================

// 例1: int の合計（引数の個数を count で受け取るパターン）
int sum_ints(int count, ...) {
    va_list args;
    va_start(args, count);   // count が最後の固定引数

    int total = 0;
    for (int i = 0; i < count; i++) {
        total += va_arg(args, int);  // int として取り出す
    }

    va_end(args);
    return total;
}

// 例2: 文字列を連結（NULL 番兵パターン）
// 引数の終端を NULL で示す
void print_all(const char *first, ...) {
    va_list args;
    va_start(args, first);

    const char *s = first;
    while (s != NULL) {
        printf("%s ", s);
        s = va_arg(args, const char *);  // 次の文字列を取り出す
    }
    printf("\n");

    va_end(args);
}

// 例3: 独自ログ関数（フォーマット文字列パターン）
// printf と同じ引数を受け取って vprintf に転送する
void my_log(const char *level, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    printf("[%s] ", level);
    vprintf(fmt, args);  // va_list をそのまま渡せる vprintf を使う
    printf("\n");

    va_end(args);
}

// 例4: va_list を別の関数に転送する
// va_list を受け取る関数を作ることで再利用できる
int my_vsnprintf_len(const char *fmt, va_list args) {
    // vsnprintf は書き込まれた（または書き込まれるべき）バイト数を返す
    return vsnprintf(NULL, 0, fmt, args);  // NULL バッファで長さだけ計算
}

int my_format_len(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int len = my_vsnprintf_len(fmt, args);
    va_end(args);
    return len;
}

// =========================================================
// 型安全でない危険なパターン
// =========================================================

// NG: 引数を float で渡しても double に昇格するので va_arg では double で受ける
void wrong_float(int count, ...) {
    va_list args;
    va_start(args, count);
    for (int i = 0; i < count; i++) {
        // float は渡すと double に昇格する
        // va_arg(args, float) は NG → va_arg(args, double) が正しい
        double v = va_arg(args, double);  // OK
        printf("%.2f ", v);
    }
    va_end(args);
    printf("\n");
}

int main(void) {

    printf("=== sum_ints（引数の個数を渡すパターン）===\n");
    printf("sum(1,2,3)      = %d\n", sum_ints(3, 1, 2, 3));
    printf("sum(10,20,30,40)= %d\n", sum_ints(4, 10, 20, 30, 40));

    printf("\n=== print_all（NULL番兵パターン）===\n");
    print_all("hello", "world", "foo", NULL);
    print_all("one", NULL);

    printf("\n=== my_log（フォーマット文字列パターン）===\n");
    my_log("INFO",  "サーバー起動 port=%d", 8080);
    my_log("ERROR", "ファイルが見つかりません: %s", "config.toml");
    my_log("DEBUG", "x=%d, y=%.2f", 42, 3.14);

    printf("\n=== フォーマット後の文字列長を事前計算 ===\n");
    int len = my_format_len("value=%d, name=%s", 42, "Alice");
    printf("フォーマット後の長さ: %d\n", len);

    printf("\n=== float は double に昇格する ===\n");
    wrong_float(3, 1.1f, 2.2f, 3.3f);  // float で渡しても double で受ける

    printf("\n=== 型を間違えると UB（コメントアウト）===\n");
    printf("例: sum_ints(2, 1.0, 2.0) で va_arg(int) → UB\n");
    // 実際に実行すると -fsanitize=undefined で検出できる

    return 0;
}
