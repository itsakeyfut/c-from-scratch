# 05 - 関数 (Functions)

C言語の関数に関する基本概念と操作をまとめたクイックレビュー用ドキュメントです。

---

## 目次

1. [プロトタイプ宣言（前方宣言）](#1-プロトタイプ宣言前方宣言)
2. [基本的な関数](#2-基本的な関数)
3. [構造体の値渡し](#3-構造体の値渡し)
4. [出力引数（ポインタ渡しで結果を返す）](#4-出力引数ポインタ渡しで結果を返す)
5. [ヒープメモリを確保して返す関数](#5-ヒープメモリを確保して返す関数)
6. [static 関数と inline 関数](#6-static-関数と-inline-関数)
7. [再帰](#7-再帰)
8. [複数の値を返す方法](#8-複数の値を返す方法)
9. [配列の関数への渡し方と decay](#9-配列の関数への渡し方と-decay)
10. [まとめ：関数チートシート](#10-まとめ関数チートシート)

---

## 1. プロトタイプ宣言（前方宣言）

```c
int add(int a, int b);
Point point_add(Point a, Point b);
int *alloc_array(size_t n);
```

### 重要なポイント

- 関数の**定義より前**で呼び出すためには、プロトタイプ宣言が必要
- プロトタイプ宣言は「この関数が存在する」とコンパイラに伝えるだけで、実装は別の場所に書く
- 慣習として**ヘッダファイル（`.h`）に書き、`.c` ファイルから `#include` する**
- 引数名は省略可能だが、書いておくと可読性が上がる

```c
// ヘッダファイル (point.h) での典型的なスタイル
int add(int, int);          // 引数名省略（最低限）
int add(int a, int b);      // 引数名あり（推奨）
```

### ファイル構成の慣習

```
project/
  point.h    ← プロトタイプ宣言・型定義・マクロ
  point.c    ← 実装
  main.c     ← #include "point.h" して使う
```

---

## 2. 基本的な関数

```c
int add(int a, int b) {
    return a + b;
}

void print_point(const Point *p) {  // const * = 読み取り専用
    printf("(%d, %d)", p->x, p->y);
}
```

### 戻り値の型

| 戻り値型 | 意味 | Rust との対応 |
|----------|------|---------------|
| `int` など | 値を返す | `-> i32` など |
| `void` | 何も返さない | `-> ()` |
| `int *` | ポインタを返す | `-> *mut i32`（ unsafe） |

### `const Point *` を引数に使う理由

- 構造体をポインタで渡すとコピーコストがゼロになる
- `const` を付けることで「この関数は値を変更しない」とコンパイラと読み手に明示できる
- ポインタで渡しても `const` があれば誤変更をコンパイルエラーで防げる

---

## 3. 構造体の値渡し

```c
Point point_add(Point a, Point b) {
    return (Point){ a.x + b.x, a.y + b.y }; // 複合リテラルで返す
}

Point r = point_add(p1, p2); // p1, p2 のコピーが渡される
```

### 値渡しとポインタ渡しの使い分け

| 条件 | 推奨する渡し方 |
|------|---------------|
| 小さな構造体（数フィールド程度） | 値渡し（コードがシンプル） |
| 大きな構造体 | `const 型 *`（読み取り専用） |
| 呼び出し元の値を変更したい | `型 *`（書き込み可能ポインタ） |

値渡しでは関数内でコピーが作られるため、**呼び出し元の変数は変更されない**。

---

## 4. 出力引数（ポインタ渡しで結果を返す）

```c
void point_add_out(const Point *a, const Point *b, Point *out) {
    out->x = a->x + b->x;
    out->y = a->y + b->y;
}

Point r;
point_add_out(&p1, &p2, &r); // r に結果が書き込まれる
```

### 出力引数とは

C には多値返却（Rust のタプル返却など）がないため、**追加のポインタ引数**を使って呼び出し元の変数に結果を書き込む慣用パターン。

```
呼び出し元             関数内
  Point r  ← &r を渡す → out->x = ..., out->y = ...
  （未初期化）                ↓
  r.x = 4, r.y = 6     ← r が更新される
```

### 使い分け

| 方法 | 特徴 |
|------|------|
| 戻り値で返す | コードが直感的、小さな構造体向き |
| 出力引数 | 複数の結果を返したい場合、呼び出し元のバッファに書き込む場合 |

> **Rust との比較:** Rust の `&mut` 参照に相当するが、C では `NULL` チェックを関数側で行う責任がある。

---

## 5. ヒープメモリを確保して返す関数

```c
int *alloc_array(size_t n) {
    int *arr = malloc(n * sizeof(int));
    if (!arr) return NULL;  // 確保失敗時は NULL を返す
    for (size_t i = 0; i < n; i++) arr[i] = (int)i;
    return arr;
}

// 呼び出し側
int *arr = alloc_array(5);
if (arr) {
    // ... 使用 ...
    free(arr);  // 呼び出し側が解放する責任を持つ
    arr = NULL; // ダングリングポインタを防ぐ
}
```

### 重要なポイント

- `malloc` は失敗すると `NULL` を返す。**必ず `NULL` チェックをする**
- ヒープメモリの所有権は**呼び出し側**にある（C にはスマートポインタがない）
- `free` した後は `NULL` を代入してダングリングポインタを防ぐ慣習

### メモリ管理のルール

```
確保 (malloc)  →  使用  →  解放 (free)
                              ↑
                     呼び出し側の責任
```

| 問題 | 原因 | 防ぎ方 |
|------|------|--------|
| メモリリーク | `free` 忘れ | すべてのパスで `free` を呼ぶ |
| ダングリングポインタ | `free` 後に使用 | `free` 後に `NULL` 代入 |
| 二重 free | 同じポインタを 2 回 `free` | `NULL` 代入済みなら `free(NULL)` は安全 |

---

## 6. static 関数と inline 関数

```c
// static: このファイル内でのみ有効（他のファイルから参照できない）
static int internal_helper(int x) {
    return x * 2;
}

// inline: コンパイラへの展開ヒント（static と組み合わせるのが慣用）
static inline int square(int x) {
    return x * x;
}
```

### `static` 関数

- **ファイルスコープの private 関数**として機能する
- 他の `.c` ファイルからリンクされない（名前衝突を防げる）
- ヘッダファイルには書かない（内部実装として隠蔽）

```
point.c                     main.c
  static int helper()  ←── ここからは見えない
  int point_add()      ←── ここからは見える（ヘッダで宣言）
```

### `inline` 関数

- コンパイラに「関数呼び出しを展開してほしい」と**ヒント**を与える
- 実際に展開するかはコンパイラが判断する（保証はない）
- 小さく頻繁に呼ばれる関数（`square` など）に有効
- `static inline` の組み合わせがヘッダファイルに書く際の定番パターン

### マクロ関数との比較

```c
#define SQUARE(x) ((x) * (x))   // マクロ: 型なし、副作用の危険あり
static inline int square(int x) // inline 関数: 型安全、デバッグ可能
{ return x * x; }
```

`inline` 関数はマクロと異なり**型チェックが効き、デバッガで追える**ため現代 C では推奨される。

---

## 7. 再帰

```c
int factorial(int n) {
    if (n <= 1) return 1;        // 基底ケース
    return n * factorial(n - 1); // 再帰ケース
}
```

### 再帰の仕組み（コールスタック）

```
factorial(4)
  └─ 4 * factorial(3)
         └─ 3 * factorial(2)
                └─ 2 * factorial(1)
                       └─ 1（基底ケース、ここから戻り始める）
```

### 注意点

- **スタックオーバーフロー:** 再帰が深すぎるとスタック領域を使い切ってクラッシュする
- **基底ケースを必ず書く:** 終了条件がないと無限再帰になる
- **大きな入力には反復（ループ）を検討する**

```c
// 反復による階乗（スタックを消費しない）
int factorial_iter(int n) {
    int result = 1;
    for (int i = 2; i <= n; i++) result *= i;
    return result;
}
```

---

## 8. 複数の値を返す方法

C にはタプル返却がないため、以下の方法を使う。

```c
// ① 構造体で返す（推奨）
typedef struct { int quot; int rem; } DivResult;

DivResult divide(int a, int b) {
    return (DivResult){ a / b, a % b };
}

DivResult dr = divide(17, 5);
printf("%d remainder %d\n", dr.quot, dr.rem); // 3 remainder 2
```

### 方法の比較

| 方法 | コード例 | 評価 |
|------|----------|------|
| ① 構造体を返す | `DivResult divide(int, int)` | 推奨：型安全・可読性高 |
| ② 出力引数 | `void divide(int, int, int*, int*)` | 複数の大きな値を返す場合に有効 |
| ③ グローバル変数 | `g_quot = ...; g_rem = ...;` | **非推奨：スレッドセーフでない・密結合** |

### `(DivResult){ ... }` — 複合リテラル

```c
return (DivResult){ a / b, a % b };
// 型名を括弧で囲んで { } で初期化する C99 の機能
// その場で構造体のインスタンスを作って返せる
```

---

## 9. 配列の関数への渡し方と decay

```c
int nums[] = {5, 3, 8, 1, 9};
size_t len = sizeof(nums) / sizeof(nums[0]); // ここで長さを計算（decay 前）
```

### decay が起きる仕組み

配列を関数に渡すと**先頭要素へのポインタに変換（decay）**される。

```c
void process(int *arr, size_t len) {
    // ここで sizeof(arr) をしても arr はポインタなので 8 バイトになる
    // 配列の長さは len で受け取るしかない
}

int nums[] = {5, 3, 8, 1, 9};
size_t len = sizeof(nums) / sizeof(nums[0]); // decay 前に計算
process(nums, len);                           // ポインタと長さを別々に渡す
```

### decay 前後の `sizeof` の違い

```c
int nums[5] = {1, 2, 3, 4, 5};

// decay 前（配列のまま）
sizeof(nums)        // → 20 (5 * sizeof(int))
sizeof(nums[0])     // → 4  (sizeof(int))
sizeof(nums) / sizeof(nums[0]) // → 5（要素数）

// decay 後（ポインタになった）
void func(int *arr) {
    sizeof(arr);    // → 8 (64bit 環境のポインタサイズ)
}
```

### まとめ

- 長さの計算は**呼び出し側で decay が起きる前に行う**
- 関数には `(ポインタ, 長さ)` の組で渡すのが C の基本パターン
- C99 以降は `#define ARRAY_LEN(arr) (sizeof(arr) / sizeof(arr[0]))` マクロが便利

---

## 10. まとめ：関数チートシート

```c
// --- プロトタイプ宣言（定義より前に呼ぶ場合・ヘッダに書く場合）---
int add(int a, int b);

// --- 基本的な関数 ---
int add(int a, int b) { return a + b; }
void print_point(const Point *p) { ... }  // void: 戻り値なし / const *: 読み取り専用

// --- 値渡し（小さな構造体向き・コピーされる）---
Point point_add(Point a, Point b) {
    return (Point){ a.x + b.x, a.y + b.y }; // 複合リテラル
}

// --- 出力引数（複数の結果を返す・呼び出し元バッファに書き込む）---
void point_add_out(const Point *a, const Point *b, Point *out) {
    out->x = a->x + b->x;
}

// --- ヒープ確保して返す（呼び出し側が free する責任）---
int *alloc_array(size_t n) {
    int *arr = malloc(n * sizeof(int));
    if (!arr) return NULL;  // 必ず NULL チェック
    return arr;
}
// 呼び出し側: free(arr); arr = NULL;

// --- static: ファイルスコープの private 関数 ---
static int helper(int x) { return x * 2; }

// --- static inline: 小さく頻繁に呼ばれる関数（マクロの代替）---
static inline int square(int x) { return x * x; }

// --- 再帰（深すぎるとスタックオーバーフロー）---
int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

// --- 複数の値を返す → 構造体で返すのが推奨 ---
typedef struct { int quot; int rem; } DivResult;
DivResult divide(int a, int b) { return (DivResult){ a/b, a%b }; }

// --- 配列の渡し方（decay に注意）---
size_t len = sizeof(nums) / sizeof(nums[0]); // decay 前に計算
void process(int *arr, size_t len);           // ポインタ + 長さをセットで渡す
```
