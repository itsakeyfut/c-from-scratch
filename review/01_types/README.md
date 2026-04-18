# 01 — Primitive Types and Variables / 基本型と変数

C言語の型・変数に関するクイックリファレンス。  
`main.c` の各セクションに対応している。

---

## 目次

1. [必要なヘッダー](#必要なヘッダー)
2. [基本型とサイズ](#基本型とサイズ)
3. [固定幅整数型](#固定幅整数型)
4. [size_t / ptrdiff_t](#size_t--ptrdiff_t)
5. [bool](#bool)
6. [暗黙の整数昇格](#暗黙の整数昇格)
7. [const](#const)
8. [変数の初期化ルール](#変数の初期化ルール)
9. [printf フォーマット指定子](#printf-フォーマット指定子)
10. [まとめ：型と変数チートシート](#まとめ型と変数チートシート)

---

## 必要なヘッダー

| ヘッダー | 提供するもの |
|---|---|
| `<stdio.h>` | `printf`, `scanf` など入出力関数 |
| `<stdint.h>` | `uint8_t`, `int32_t` など固定幅整数型 |
| `<stdbool.h>` | `bool`, `true`, `false`（C99以降） |
| `<stddef.h>` | `size_t`, `ptrdiff_t`, `NULL` |
| `<limits.h>` | `INT_MAX`, `CHAR_MIN` など各型の限界値 |

---

## 基本型とサイズ

C の基本型はサイズが **環境依存**（OSやコンパイラによって変わる）。  
サイズを保証したい場合は [固定幅整数型](#固定幅整数型) を使う。

| 型 | 符号 | 典型的なサイズ | 用途 |
|---|---|---|---|
| `char` | 実装依存 | 1 バイト | 文字、または小さな整数 |
| `unsigned char` | なし | 1 バイト | バイト列の操作 |
| `short` | あり | 2 バイト | 小さな整数 |
| `int` | あり | 4 バイト | 汎用整数（最も一般的） |
| `long` | あり | 4 or 8 バイト | 環境によって変わるので注意 |
| `long long` | あり | 8 バイト | 大きな整数 |
| `float` | — | 4 バイト | 単精度浮動小数点数 |
| `double` | — | 8 バイト | 倍精度浮動小数点数（推奨） |
| `void*` | — | 8 バイト（64bit） | 任意のポインタ |

```c
printf("int: %zu byte\n", sizeof(int)); // sizeof はバイト数を返す（型は size_t）
```

> **注意**: `sizeof` の返り値は `size_t` 型なので、`%zu` で出力する。`%d` は不正。

---

## 固定幅整数型

`<stdint.h>` で定義。**サイズが環境によらず保証される**ため、実装では `int` より優先して使う。

| 型 | 符号 | サイズ | 範囲 |
|---|---|---|---|
| `uint8_t` | なし | 8 bit | 0 〜 255 |
| `int8_t` | あり | 8 bit | -128 〜 127 |
| `uint16_t` | なし | 16 bit | 0 〜 65535 |
| `int16_t` | あり | 16 bit | -32768 〜 32767 |
| `uint32_t` | なし | 32 bit | 0 〜 4,294,967,295 |
| `int32_t` | あり | 32 bit | -2,147,483,648 〜 2,147,483,647 |
| `uint64_t` | なし | 64 bit | 0 〜 18,446,744,073,709,551,615 |
| `int64_t` | あり | 64 bit | -9,223,372,036,854,775,808 〜 … |

```c
uint8_t  u8  = 255;
int32_t  i32 = -100;
uint64_t u64 = 0xDEADBEEFCAFEBABEULL; // ULL サフィックスで unsigned long long リテラル
```

> `long long` リテラルには `LL`、`unsigned long long` には `ULL` サフィックスをつける。

---

## size_t / ptrdiff_t

| 型 | 符号 | 用途 | printf |
|---|---|---|---|
| `size_t` | なし | 配列インデックス、`sizeof` の返り値、メモリサイズ | `%zu` |
| `ptrdiff_t` | あり | 2つのポインタの差（`ptr2 - ptr1`） | `%td` |

```c
size_t len = 100;
ptrdiff_t diff = ptr2 - ptr1; // ポインタ同士の引き算の結果は ptrdiff_t
```

> **落とし穴**: `size_t` は unsigned なので、`int` と比較すると暗黙の型変換が起きる。  
> 常に同じ型どうしで比較する。

---

## bool

`<stdbool.h>` が必要（C99以降）。

| 値 | 内部表現 |
|---|---|
| `true` | `1` |
| `false` | `0` |
| ゼロ以外の任意の値を `(bool)` にキャスト | `1` |
| `0` を `(bool)` にキャスト | `0` |

```c
bool flag = true;
printf("%d\n", flag);      // 1
printf("%d\n", (bool)42);  // 1（非ゼロはすべて true）
printf("%d\n", (bool)0);   // 0
```

> `bool` は内部的に `int` なので、`printf` では `%d` を使う。`%b` はC23以降。

---

## 暗黙の整数昇格

`signed` と `unsigned` を混在させると**予期しない挙動**が起きる。

```c
unsigned int u = 1;
int          i = -1;

if (i > u) { ... }          // 危険: i が unsigned に変換され 4294967295 になる
if ((int)u > i) { ... }     // 安全: 同じ型で比較
```

**昇格のルール（C標準）**:

| 演算の組み合わせ | 結果の型 |
|---|---|
| `int` op `unsigned int` | `unsigned int`（signed が unsigned に変換される） |
| `int` op `long` | `long` |
| `long` op `unsigned long` | `unsigned long` |
| 小さい型（`char`, `short`）が演算に使われる | まず `int` に昇格してから演算 |

> **対策**: `-Wall` フラグでコンパイルすると警告が出る。常に同じ型どうしで比較する。

---

## const

`const` は値や参照先の変更を禁止する。**ポインタとの組み合わせ**に注意。

| 宣言 | 意味 |
|---|---|
| `const int x = 10;` | `x` の値を変更できない |
| `const char *p = s;` | `p` が指す先の値を変更できない（ポインタ自体は変更可） |
| `char *const p = s;` | `p` 自体を変更できない（ポインタが指す先の値は変更可） |
| `const char *const p = s;` | ポインタも指す先の値も両方変更できない |

```c
const int MAX = 100;
// MAX = 200; // コンパイルエラー

const char *s = "hello";
// s[0] = 'H'; // 未定義動作（文字列リテラルは変更不可）
s = "world";   // OK（ポインタ自体は変更可）
```

---

## 変数の初期化ルール

| 変数の種類 | 宣言場所 | 自動初期化 | 初期値 |
|---|---|---|---|
| グローバル変数 | 関数の外 | される | `0` / `NULL` / `0.0` |
| 静的ローカル変数 | 関数内に `static` | される | `0` / `NULL` / `0.0` |
| ローカル変数 | 関数内 | **されない** | ゴミ値（未定義動作） |

```c
int g = 0;           // グローバル: 明示的初期化（推奨）
static int s;        // static: 自動的に 0

void func(void) {
    int x;           // 危険: ゴミ値が入っている
    int y = 0;       // 安全: 明示的に初期化
    printf("%d\n", x); // 未定義動作！
}
```

> **ルール**: ローカル変数は**必ず宣言と同時に初期化**する。

---

## printf フォーマット指定子

### 整数

| 指定子 | 型 | 出力形式 | 例 |
|---|---|---|---|
| `%d` | `int` | 符号付き10進数 | `-42` |
| `%i` | `int` | 符号付き10進数（`%d` と同じ） | `-42` |
| `%u` | `unsigned int` | 符号なし10進数 | `42` |
| `%o` | `unsigned int` | 8進数 | `52` |
| `%x` | `unsigned int` | 16進数（小文字） | `2a` |
| `%X` | `unsigned int` | 16進数（大文字） | `2A` |
| `%lld` | `long long` | 符号付き64bit整数 | `-9000000000` |
| `%llu` | `unsigned long long` | 符号なし64bit整数 | `9000000000` |
| `%zu` | `size_t` | `sizeof` / 配列インデックス | `8` |
| `%td` | `ptrdiff_t` | ポインタ差分 | `-5` |

### 浮動小数点数

| 指定子 | 型 | 出力形式 | 例 |
|---|---|---|---|
| `%f` | `double` | 小数表記（デフォルト6桁） | `3.141593` |
| `%e` | `double` | 指数表記（小文字） | `3.141593e+00` |
| `%g` | `double` | 短い方を自動選択 | `3.14159` |
| `%.2f` | `double` | 小数点以下2桁 | `3.14` |

### 文字・文字列・その他

| 指定子 | 型 | 出力形式 | 例 |
|---|---|---|---|
| `%c` | `char` | 1文字 | `A` |
| `%s` | `char*` | 文字列（null終端まで） | `hello` |
| `%p` | `void*` | ポインタアドレス（16進数） | `0x7ffd1234` |
| `%%` | — | `%` 文字そのもの | `%` |

### 幅・精度の指定

```c
printf("%5d\n",   42);    //    42  （右寄せ、幅5）
printf("%-5d|\n", 42);    // 42   |（左寄せ、幅5）
printf("%05d\n",  42);    // 00042 （ゼロ埋め）
printf("%.3f\n",  3.14159); // 3.142（小数点以下3桁）
printf("%8.2f\n", 3.14159); //     3.14（幅8、小数点以下2桁）
```

---

## まとめ：型と変数チートシート

```c
// --- ヘッダー ---
#include <stdint.h>   // 固定幅整数型
#include <stdbool.h>  // bool
#include <stddef.h>   // size_t, ptrdiff_t, NULL
#include <limits.h>   // INT_MAX など

// --- 基本型（サイズは環境依存） ---
char c = 'A';
int  i = 42;
double d = 3.14;

// --- 固定幅整数型（サイズ保証あり） ---
uint8_t  u8  = 255;
int32_t  i32 = -100;
uint64_t u64 = 0xDEADBEEFULL;

// --- size_t / ptrdiff_t ---
size_t    len  = sizeof(arr) / sizeof(arr[0]); // %zu
ptrdiff_t diff = ptr2 - ptr1;                  // %td

// --- bool ---
bool flag = true;   // 内部は 1
bool zero = false;  // 内部は 0
// printf: %d を使う（%b は C23以降）

// --- const ---
const int MAX = 100;        // 値を変更不可
const int *cp = &n;         // 指す先を変更不可（ポインタは可）
int *const pc = &n;         // ポインタを変更不可（指す先は可）
const int *const cpc = &n;  // 両方変更不可

// --- 初期化ルール ---
// グローバル・static → 自動的に 0
// ローカル変数      → 必ず明示的に初期化する（ゴミ値に注意）

// --- printf 主要フォーマット指定子 ---
// %d   int              符号付き10進数
// %u   unsigned int     符号なし10進数
// %lld long long
// %llu unsigned long long
// %zu  size_t
// %td  ptrdiff_t
// %f   double           小数表記
// %e   double           指数表記
// %g   double           短い方を自動選択
// %c   char
// %s   char*
// %p   void*            アドレス（16進数）

// --- 暗黙の整数昇格（落とし穴） ---
// signed と unsigned を混在させると signed が unsigned に変換される
// → 常に同じ型どうしで比較する
// → -Wall フラグで警告を有効にする
```
