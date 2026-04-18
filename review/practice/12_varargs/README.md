# 12 - 可変長引数 (Variadic Functions)

C言語の可変長引数（`stdarg.h`）に関する基本概念と実用パターンをまとめたクイックレビュー用ドキュメントです。

---

## 目次

1. [可変長引数関数の基本構造](#1-可変長引数関数の基本構造)
2. [例1: 引数の個数を明示するパターン（sum_ints）](#2-例1-引数の個数を明示するパターンsum_ints)
3. [例2: NULL 番兵パターン（print_all）](#3-例2-null-番兵パターンprint_all)
4. [例3: フォーマット文字列パターン（my_log）](#4-例3-フォーマット文字列パターンmy_log)
5. [例4: va_list を別関数に転送する](#5-例4-va_list-を別関数に転送する)
6. [float は double に昇格する](#6-float-は-double-に昇格する)
7. [型を間違えると UB](#7-型を間違えると-ub)
8. [まとめ：可変長引数チートシート](#8-まとめ可変長引数チートシート)

---

## 1. 可変長引数関数の基本構造

```c
#include <stdarg.h>

void example(int count, ...) {  // ... が可変長引数を表す
    va_list args;               // 引数リストを保持する型
    va_start(args, count);      // count（最後の固定引数）から開始

    for (int i = 0; i < count; i++) {
        int v = va_arg(args, int);  // 1つずつ取り出す（型を指定）
    }

    va_end(args);               // 必ず呼ぶ（リソース解放）
}
```

### 4 つのマクロ

| マクロ | 役割 |
|--------|------|
| `va_list args` | 可変長引数のリストを保持する変数を宣言 |
| `va_start(args, last)` | `last`（最後の固定引数）の次から取り出せるように初期化 |
| `va_arg(args, type)` | 次の引数を `type` として取り出して返す |
| `va_end(args)` | 後処理（必ず呼ぶ）|

### 5 つのルール

1. **最低 1 つの固定引数が必要** — `va_start` に渡すため
2. **`va_start` → `va_arg` → `va_end` の順序を守る**
3. **`va_end` は必ず呼ぶ** — 省略するとスタック破壊などの UB になりうる
4. **引数の型は呼び出し側の責任** — C はランタイムに型情報を持たない
5. **引数の個数も呼び出し側の責任** — 終端を示す仕組みを自分で決める

---

## 2. 例1: 引数の個数を明示するパターン（`sum_ints`）

```c
int sum_ints(int count, ...) {
    va_list args;
    va_start(args, count);

    int total = 0;
    for (int i = 0; i < count; i++) {
        total += va_arg(args, int);
    }

    va_end(args);
    return total;
}

sum_ints(3, 1, 2, 3);        // → 6
sum_ints(4, 10, 20, 30, 40); // → 100
```

### 仕組みの解説

```
呼び出し: sum_ints(3, 1, 2, 3)

スタック上の引数:
  [count=3] [1] [2] [3]
      ↑
  va_start(args, count) で count の次から取り出し開始

va_arg(args, int) を 3 回呼ぶ:
  1回目: 1
  2回目: 2
  3回目: 3
```

### このパターンの特徴

- 引数の個数を **第1引数で明示的に渡す**
- `printf` の `%d` の数をカウントするような「個数が自明」な状況に向く
- 個数を間違えると範囲外アクセスになる（UB）

---

## 3. 例2: NULL 番兵パターン（`print_all`）

```c
void print_all(const char *first, ...) {
    va_list args;
    va_start(args, first);

    const char *s = first;
    while (s != NULL) {           // NULL が来たら終了
        printf("%s ", s);
        s = va_arg(args, const char *);
    }
    printf("\n");

    va_end(args);
}

print_all("hello", "world", "foo", NULL);  // NULL で終端を示す
```

### 仕組みの解説

```
呼び出し: print_all("hello", "world", "foo", NULL)

引数の流れ:
  first = "hello"         ← 最初の固定引数として受け取る
  va_arg → "world"
  va_arg → "foo"
  va_arg → NULL           ← ループを抜ける
```

### NULL 番兵パターンの特徴

- 引数の終端を **`NULL`（ポインタ）で示す**
- 個数を別途管理する必要がない
- **最後の `NULL` を忘れると UB**（範囲外を読み続ける）
- 文字列の配列など、ポインタ型の引数リストに適している

```c
// 呼び出し時に NULL を忘れると危険
print_all("a", "b");       // NG: NULL がない → スタック上のゴミを読む
print_all("a", "b", NULL); // OK
```

---

## 4. 例3: フォーマット文字列パターン（`my_log`）

```c
void my_log(const char *level, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    printf("[%s] ", level);
    vprintf(fmt, args);  // va_list をそのまま渡せる vprintf を使う
    printf("\n");

    va_end(args);
}

my_log("INFO",  "server started on port=%d", 8080);
my_log("ERROR", "file not found: %s", "config.toml");
// → [INFO] server started on port=8080
// → [ERROR] file not found: config.toml
```

### `printf` 系の `v` バージョン

`printf` などの標準関数には `va_list` を受け取る `v` バージョンがある。  
可変長引数を受け取ってそのまま別の printf 系関数に渡すときに使う。

| 通常版 | va_list 版 | 用途 |
|--------|-----------|------|
| `printf(fmt, ...)` | `vprintf(fmt, args)` | 標準出力 |
| `fprintf(fp, fmt, ...)` | `vfprintf(fp, fmt, args)` | ファイル出力 |
| `sprintf(buf, fmt, ...)` | `vsprintf(buf, fmt, args)` | バッファ出力（危険）|
| `snprintf(buf, n, fmt, ...)` | `vsnprintf(buf, n, fmt, args)` | 安全なバッファ出力 |

### `vprintf` を使う理由

```c
// NG: printf に ... を転送することはできない
void my_log(const char *fmt, ...) {
    printf(fmt, ...);  // コンパイルエラー: ... はそのまま渡せない
}

// OK: va_list に格納してから vprintf に渡す
void my_log(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);  // va_list を渡せる
    va_end(args);
}
```

---

## 5. 例4: `va_list` を別関数に転送する

```c
// va_list を受け取るヘルパー関数
int my_vsnprintf_len(const char *fmt, va_list args) {
    // NULL バッファ・サイズ 0 で呼ぶと「書き込まれるべきバイト数」を返す
    return vsnprintf(NULL, 0, fmt, args);
}

// 可変長引数を受け取って va_list に変換し、ヘルパーに渡す
int my_format_len(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int len = my_vsnprintf_len(fmt, args);  // va_list を転送
    va_end(args);
    return len;
}

int len = my_format_len("value=%d, name=%s", 42, "Alice");
// → 22（"value=42, name=Alice" の文字数）
```

### `vsnprintf(NULL, 0, fmt, args)` のテクニック

```c
int n = vsnprintf(NULL, 0, fmt, args);
// 出力先: NULL（何も書かない）
// サイズ: 0（1バイトも書かない）
// 戻り値: 実際に必要なバイト数（'\0' を含まない）
```

これを使って **`malloc` するサイズを事前計算**できる：

```c
va_list args;
va_start(args, fmt);
int len = vsnprintf(NULL, 0, fmt, args);  // 必要サイズを計算
va_end(args);

char *buf = malloc(len + 1);  // +1 は '\0'
va_start(args, fmt);          // va_list は使い切ったので再度 start
vsnprintf(buf, len + 1, fmt, args);
va_end(args);
```

### `va_list` は使い切り

一度 `va_arg` を呼び出すと位置が進む。  
同じ `va_list` を2つの関数に渡すことはできない（2番目は壊れた状態から始まる）。

```c
// NG: 同じ args を2つの関数で共有
vprintf(fmt, args);
vsnprintf(buf, n, fmt, args);  // args はすでに使い切られている

// OK: va_copy で複製してから使う
va_list args_copy;
va_copy(args_copy, args);  // args を args_copy にコピー
vprintf(fmt, args);
vsnprintf(buf, n, fmt, args_copy);
va_end(args_copy);
```

---

## 6. `float` は `double` に昇格する

```c
void wrong_float(int count, ...) {
    va_list args;
    va_start(args, count);
    for (int i = 0; i < count; i++) {
        // va_arg(args, float) は NG → UB
        double v = va_arg(args, double);  // OK: double で受け取る
        printf("%.2f ", v);
    }
    va_end(args);
}

wrong_float(3, 1.1f, 2.2f, 3.3f);  // float で渡しても double で受ける
```

### デフォルト引数昇格（Default Argument Promotions）

`...` に渡された引数は自動的に型変換される。これを**デフォルト引数昇格**という。

| 渡した型 | `va_arg` で受け取る型 |
|----------|----------------------|
| `float` | `double` |
| `char` | `int` |
| `short` | `int` |
| `int` | `int`（変化なし） |
| `double` | `double`（変化なし） |
| ポインタ | ポインタ（変化なし） |

```c
// 間違いやすいパターン
va_arg(args, float)  // NG: float は渡した瞬間に double に変換済み
va_arg(args, char)   // NG: char は渡した瞬間に int に変換済み

// 正しいパターン
va_arg(args, double) // OK
va_arg(args, int)    // OK
```

---

## 7. 型を間違えると UB

```c
// NG: int の引数を渡したのに double で受け取ろうとする
sum_ints(2, 1.0, 2.0);   // 1.0, 2.0 は double → va_arg(args, int) と不一致 → UB
```

### なぜ UB になるのか

C の可変長引数は**実行時に型情報を持たない**。  
`va_arg(args, type)` は「ここに `type` のサイズのデータがある」と信じて読み出すだけ。  
実際のデータが異なる型なら、**正しいビットパターンを読めずに壊れた値が返る**。

```
渡したデータ（double 1.0 = 8バイト）:
  3F F0 00 00 | 00 00 00 00

va_arg(args, int) で読むと（4バイト）:
  3F F0 00 00  ← double の上位4バイトをそのまま int として解釈
  = 1072693248  ← 期待値 1 と全く異なる
```

### 検出方法

```bash
gcc -fsanitize=undefined main.c -o main && ./main
# UndefinedBehaviorSanitizer が実行時に検出してくれる
```

---

## 8. まとめ：可変長引数チートシート

```c
#include <stdarg.h>

// --- 基本構造 ---
void func(int count, ...) {
    va_list args;
    va_start(args, count);   // 最後の固定引数を渡す
    int v = va_arg(args, int); // 型を指定して取り出す
    va_end(args);             // 必ず呼ぶ
}

// --- 終端の示し方 3 パターン ---
// 1. 個数を引数で渡す
void sum(int count, ...)        // count 個の引数を取り出す

// 2. NULL 番兵
void print_all(const char *s, ...) {
    while (s != NULL) { s = va_arg(args, const char *); }
}                               // NULL を最後に渡す（必須）

// 3. フォーマット文字列（printf スタイル）
void log(const char *fmt, ...) { vprintf(fmt, args); }
// fmt の % の数で引数の個数が決まる

// --- v バージョン（va_list を受け取る）---
vprintf(fmt, args)              // printf の va_list 版
vfprintf(fp, fmt, args)         // fprintf の va_list 版
vsnprintf(buf, n, fmt, args)    // snprintf の va_list 版（推奨）

// --- 長さ事前計算 ---
int len = vsnprintf(NULL, 0, fmt, args);  // 書き込みサイズを計算
char *buf = malloc(len + 1);              // 必要量を確保

// --- va_copy: 同じ va_list を2回使いたいとき ---
va_list copy;
va_copy(copy, args);
// ... copy を使う ...
va_end(copy);

// --- デフォルト引数昇格（型を間違えると UB）---
// float → double で受ける: va_arg(args, double)
// char / short → int で受ける: va_arg(args, int)

// --- 検出: -fsanitize=undefined でランタイムチェック ---
```
