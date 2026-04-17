# 09 - プリプロセッサ・マクロ (Preprocessor / Macros)

C言語のプリプロセッサとマクロに関する基本概念と操作をまとめたクイックレビュー用ドキュメントです。

---

## 目次

1. [定数マクロ（#define）](#1-定数マクロdefine)
2. [関数マクロと副作用の罠](#2-関数マクロと副作用の罠)
3. [do-while(0) イディオム — SWAP マクロ](#3-do-while0-イディオム--swap-マクロ)
4. [条件コンパイル — LOG マクロ](#4-条件コンパイル--log-マクロ)
5. [文字列化（#）とトークン結合（##）](#5-文字列化とトークン結合)
6. [事前定義マクロ](#6-事前定義マクロ)
7. [assert](#7-assert)
8. [まとめ：プリプロセッサ・マクロチートシート](#8-まとめプリプロセッサマクロチートシート)

---

## 1. 定数マクロ（`#define`）

```c
#define PI        3.14159265358979
#define MAX_BUF   256
#define APP_NAME  "MyApp"
```

### プリプロセッサとは

コンパイルの前段階で動作するテキスト置換エンジン。  
`#define`・`#include`・`#ifdef` などの `#` で始まるディレクティブを処理する。

```
ソースコード
    ↓ プリプロセッサ（テキスト置換）
展開済みコード
    ↓ コンパイラ
オブジェクトファイル
```

### `#define` 定数の特徴

- **型情報がない** → コンパイラの型チェックが効かない
- コンパイラのエラーメッセージにマクロ名が表示されず原因特定が難しい

### より安全な代替手段

| 方法 | 型安全 | スコープ | 推奨度 |
|------|--------|----------|--------|
| `#define PI 3.14` | **なし** | グローバル（ファイル全体） | △ |
| `static const double PI = 3.14;` | あり | ファイルスコープ | ◎ |
| `enum { MAX = 256 };` | あり（整数のみ） | ブロックスコープ可 | ◎ |

整数定数には `enum`、浮動小数点・文字列定数には `static const` を使うのが現代 C の推奨スタイル。  
ただし `ARRAY_LEN` のような**型に依存しないユーティリティマクロ**は `#define` が適切。

---

## 2. 関数マクロと副作用の罠

```c
#define SQUARE(x)    ((x) * (x))   // OK: 括弧で保護
#define SQUARE_NG(x) x * x         // NG: 括弧なし
#define MAX(a, b)    ((a) > (b) ? (a) : (b))
#define ABS(x)       ((x) < 0 ? -(x) : (x))
```

### 括弧がない場合の誤動作

```c
SQUARE_NG(v + 1)
// 展開後: v + 1 * v + 1
//       = 3 + 1 * 3 + 1 = 7  ← 期待値 16 と異なる！

SQUARE(v + 1)
// 展開後: ((v + 1) * (v + 1))
//       = (3 + 1) * (3 + 1) = 16  ← 正しい
```

**ルール①** 引数は必ず `()` で囲む。  
**ルール②** マクロ全体も `()` で囲む。

### 副作用による多重評価の罠

```c
int x = 3;
int result = MAX(x++, 2);
// 展開後: ((x++) > (2) ? (x++) : (2))
//         x++ が 2 回評価される → x = 5 になる（期待値 4）
```

関数マクロは**引数を複数回展開する**ため、`++`・`--`・関数呼び出しなど副作用のある式を渡すと意図しない動作になる。

> **対策:** 副作用のある式をマクロ引数に渡さない。必要なら `inline` 関数を使う。

```c
// inline 関数なら副作用の問題がない
static inline int max_int(int a, int b) { return a > b ? a : b; }
```

### `ARRAY_LEN` マクロ

```c
#define ARRAY_LEN(arr) (sizeof(arr) / sizeof((arr)[0]))

int arr[] = {10, 20, 30, 40, 50};
ARRAY_LEN(arr)  // → 5
```

- `sizeof(arr)` は配列全体のバイト数
- `sizeof(arr[0])` は要素 1 個のバイト数
- 割り算で要素数が求まる
- **ポインタに decay した後では使えない**（`sizeof(ptr)` はポインタサイズになる）

---

## 3. `do-while(0)` イディオム — SWAP マクロ

```c
#define SWAP(type, a, b) do { \
    type _tmp = (a);          \
    (a) = (b);                \
    (b) = _tmp;               \
} while(0)

int a = 1, b = 2;
SWAP(int, a, b);  // a=2, b=1
```

### なぜ `do { ... } while(0)` が必要か

複文マクロを `if/else` の中で使う場合に問題が起きる。

```c
// NG: 単純な { } ブロックの場合
#define SWAP_BAD(type, a, b) { type _t = a; a = b; b = _t; }

if (cond)
    SWAP_BAD(int, x, y);  // ← セミコロンで if の外に出てしまう
else
    foo();
// 展開後:
// if (cond)
//     { type _t = x; x = y; y = _t; };  ← ここで if が終わる
//     // else がぶら下がれず コンパイルエラー
```

```c
// OK: do-while(0) を使う場合
#define SWAP(type, a, b) do { \
    type _tmp = (a);          \
    (a) = (b);                \
    (b) = _tmp;               \
} while(0)

if (cond)
    SWAP(int, x, y);  // セミコロンが do-while(0) のセミコロンになる
else
    foo();
// 展開後:
// if (cond)
//     do { ... } while(0);  ← 1つの文として扱われる
// else
//     foo();  ← 正しく else に続く
```

`do { ... } while(0)` はループせず（条件が常に偽）、**複文を 1 つの文として扱う**ためのイディオム。

### `_tmp` の先頭アンダースコア

マクロ内のローカル変数名に `_tmp` のようなアンダースコアプレフィックスを使うのは、  
呼び出し側の変数名との衝突を避けるための慣習。

---

## 4. 条件コンパイル — LOG マクロ

```c
#ifdef DEBUG
    #define LOG(fmt, ...) \
        fprintf(stderr, "[DEBUG] %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
    #define LOG(fmt, ...)  ((void)0)  // no-op（リリースビルドでは完全に消える）
#endif
```

```bash
gcc -DDEBUG main.c -o main   # DEBUG を定義してコンパイル → LOG が有効
gcc main.c -o main           # 定義なし → LOG は何もしない
```

### 条件コンパイルディレクティブ

| ディレクティブ | 意味 |
|----------------|------|
| `#ifdef NAME` | `NAME` が定義されていれば有効 |
| `#ifndef NAME` | `NAME` が定義されていなければ有効 |
| `#if 式` | 式が真なら有効（数値比較も可能）|
| `#elif 式` | else if |
| `#else` | それ以外 |
| `#endif` | ブロックの終了 |

### 可変長引数マクロ（`__VA_ARGS__`）

```c
#define LOG(fmt, ...) fprintf(stderr, fmt "\n", ##__VA_ARGS__)
//                                     ↑
//              fmt に続く任意の引数が ##__VA_ARGS__ に展開される

LOG("hello");             // fprintf(stderr, "hello\n")
LOG("x=%d", x);          // fprintf(stderr, "x=%d\n", x)
LOG("x=%d y=%d", x, y);  // fprintf(stderr, "x=%d y=%d\n", x, y)
```

`##__VA_ARGS__` の `##` は引数が空の場合に前のカンマを削除する GCC 拡張。  
C23 では標準化された `__VA_OPT__(,)` が使える。

### ヘッダガード（重複インクルード防止）

```c
// header.h
#ifndef HEADER_H
#define HEADER_H

// ヘッダの内容

#endif  // HEADER_H
```

`#ifndef` を使ったヘッダガードは、同じヘッダが複数回インクルードされても  
内容が 1 度しか展開されないことを保証する最も基本的なパターン。

---

## 5. 文字列化（`#`）とトークン結合（`##`）

```c
#define STRINGIFY(x) #x    // x をそのまま文字列リテラルにする
#define CONCAT(a, b) a##b  // a と b を結合して 1 つのトークンにする

STRINGIFY(hello)    // → "hello"
STRINGIFY(42)       // → "42"
STRINGIFY(a + b)    // → "a + b"（式がそのまま文字列化）

int CONCAT(my_, value) = 42;  // → int my_value = 42;
```

### `#`（文字列化演算子）

マクロ引数を**文字列リテラル**に変換する。  
値ではなく**ソースコード上のテキスト**がそのまま文字列になる。

```c
int x = 5;
printf("%s = %d\n", STRINGIFY(x), x);  // "x = 5"（変数名が文字列に）
```

主な用途：デバッグ出力でマクロ引数の式をそのまま表示する。

```c
// 例: assert の実装で使われる
#define MY_ASSERT(expr) \
    ((expr) ? (void)0 : \
     fprintf(stderr, "Assertion failed: %s at %s:%d\n", \
             #expr, __FILE__, __LINE__))
```

### `##`（トークン結合演算子）

2 つのトークンを**結合して 1 つの識別子**を作る。

```c
#define FIELD(name) my_##name

FIELD(x) = 10;   // → my_x = 10;
FIELD(y) = 20;   // → my_y = 20;
```

主な用途：コードの自動生成、登録マクロ、型ごとの関数名生成など。

---

## 6. 事前定義マクロ

```c
printf("__FILE__ = %s\n", __FILE__);  // "main.c"（ファイル名）
printf("__LINE__ = %d\n", __LINE__);  // 行番号（整数）
printf("__func__ = %s\n", __func__);  // "main"（関数名、C99以降）
printf("__DATE__ = %s\n", __DATE__);  // "Apr 17 2026"（コンパイル日）
printf("__TIME__ = %s\n", __TIME__);  // "12:34:56"（コンパイル時刻）
```

### 主な事前定義マクロ一覧

| マクロ | 型 | 内容 |
|--------|----|------|
| `__FILE__` | `const char *` | 現在のソースファイル名 |
| `__LINE__` | `int` | 現在の行番号 |
| `__func__` | `const char *` | 現在の関数名（C99以降） |
| `__DATE__` | `const char *` | コンパイル日（`"Mon DD YYYY"` 形式）|
| `__TIME__` | `const char *` | コンパイル時刻（`"HH:MM:SS"` 形式）|
| `__STDC__` | `int` | C 標準準拠なら `1` |
| `__STDC_VERSION__` | `long` | C 標準のバージョン（C11 なら `201112L`）|

### 実用的な使い方

```c
// エラーログにファイル・行番号を含める
#define ERR(msg) \
    fprintf(stderr, "[ERROR] %s:%d: %s\n", __FILE__, __LINE__, msg)

// デバッグ用の変数ダンプ
#define DUMP(x) \
    printf(#x " = %d (at %s:%d)\n", (x), __FILE__, __LINE__)

DUMP(my_var);  // "my_var = 42 (at main.c:55)"
```

---

## 7. `assert`

```c
#include <assert.h>

int val = 5;
assert(val > 0);    // OK: 条件が真なので通過
assert(val == 5);   // OK: 条件が真なので通過
// assert(val == 0); // NG: 条件が偽 → プログラムが中断

printf("assert passed\n");
```

### `assert` の動作

```
assert(条件) の展開イメージ:
  if (!(条件)) {
      fprintf(stderr, "Assertion failed: 条件 at ファイル:行番号\n");
      abort();  // プログラムを強制終了
  }
```

- 条件が**真**なら何もしない
- 条件が**偽**ならエラーメッセージを出力して `abort()` でクラッシュさせる

### `NDEBUG` で無効化する

```bash
gcc -DNDEBUG main.c -o main   # リリースビルド: assert が完全に消える
```

`NDEBUG`（No Debug）マクロを定義すると、すべての `assert` がコンパイル時に  
空の式に置き換えられ、実行時コストがゼロになる。

### プログラミングエラー vs 実行時エラー

| エラーの種類 | 対処法 |
|-------------|--------|
| **プログラミングエラー**（仕様違反・前提条件違反） | `assert` で即クラッシュ → バグを早期発見 |
| **実行時エラー**（ファイルなし・OOM・ユーザー入力） | 戻り値・エラーコードで処理継続 |

```c
// assert: 「ここに来るはずがない」前提条件の検証
void process(int *ptr) {
    assert(ptr != NULL);  // NULL は呼び出し側のバグ → assert でよい
    // ...
}

// 戻り値: 実行時に起こりうるエラー
FILE *f = fopen("data.txt", "r");
if (!f) { perror("fopen"); return -1; }  // ファイルがないのは実行時エラー
```

---

## 8. まとめ：プリプロセッサ・マクロチートシート

```c
// --- 定数マクロ（型なし・非推奨）→ static const / enum を優先 ---
#define MAX_BUF  256          // 型なし整数定数
#define PI       3.14159      // 型なし浮動小数点定数
static const int  MAX = 256;  // 推奨: 型あり
enum { LIMIT = 1024 };        // 推奨: 整数定数に

// --- 関数マクロの 3 ルール ---
#define SQUARE(x) ((x) * (x))        // 1. 引数を () で囲む
                                      // 2. 全体を () で囲む
                                      // 3. 副作用のある引数を渡さない
#define ARRAY_LEN(arr) (sizeof(arr) / sizeof((arr)[0]))  // decay 前のみ有効

// --- do-while(0): 複文マクロを安全に使う ---
#define SWAP(type, a, b) do { \
    type _tmp = (a); (a) = (b); (b) = _tmp; \
} while(0)  // セミコロン付きで 1 文として扱われる

// --- 条件コンパイル ---
#ifdef DEBUG
    // デバッグビルド時のみ有効
#else
    // リリースビルド時
#endif
// コンパイル時に定義: gcc -DDEBUG ...

// --- 可変長引数マクロ ---
#define LOG(fmt, ...) fprintf(stderr, fmt "\n", ##__VA_ARGS__)

// --- 文字列化（#）とトークン結合（##）---
#define STRINGIFY(x) #x        // x を "x" に変換
#define CONCAT(a, b) a##b      // ab という 1 つのトークンに結合

// --- 事前定義マクロ ---
__FILE__          // ファイル名（文字列）
__LINE__          // 行番号（整数）
__func__          // 関数名（文字列、C99以降）
__DATE__          // コンパイル日
__TIME__          // コンパイル時刻

// --- assert: プログラミングエラーの早期発見 ---
#include <assert.h>
assert(ptr != NULL);    // 偽ならクラッシュ → バグを即発見
// -DNDEBUG でリリースビルド時に完全に無効化

// --- ヘッダガード ---
#ifndef MY_HEADER_H
#define MY_HEADER_H
// ヘッダの内容（重複インクルード防止）
#endif
```
