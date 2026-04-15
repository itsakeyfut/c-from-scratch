# 07 - 動的メモリ管理 (Dynamic Memory Management)

C言語の動的メモリ管理（`malloc` / `calloc` / `realloc` / `free`）に関する基本概念と操作をまとめたクイックレビュー用ドキュメントです。

---

## 目次

1. [malloc — 初期化なしで確保](#1-malloc--初期化なしで確保)
2. [calloc — ゼロ初期化して確保](#2-calloc--ゼロ初期化して確保)
3. [realloc — サイズを変更して再確保](#3-realloc--サイズを変更して再確保)
4. [free のルール](#4-free-のルール)
5. [構造体の動的確保](#5-構造体の動的確保)
6. [構造体配列の動的確保](#6-構造体配列の動的確保)
7. [メモリ操作関数（memset / memcpy / memmove）](#7-メモリ操作関数memset--memcpy--memmove)
8. [スタック vs ヒープ](#8-スタック-vs-ヒープ)
9. [まとめ：動的メモリ管理チートシート](#9-まとめ動的メモリ管理チートシート)

---

## 1. `malloc` — 初期化なしで確保

```c
int *arr = malloc(5 * sizeof(int));  // int 5 個分のメモリを確保
if (!arr) {                          // NULL チェック必須
    fprintf(stderr, "malloc failed\n");
    return 1;
}
// 初期化されていない（ゴミ値）→ 必ず自分で初期化する
for (int i = 0; i < 5; i++) arr[i] = i * 10;
free(arr);
arr = NULL;  // ダングリングポインタ防止（慣習）
```

### 重要なポイント

- `malloc` は**初期化しない**。確保直後のメモリにはゴミ値が入っている
- 戻り値は `void *` → 代入先のポインタ型に暗黙キャストされる（C では不要、C++ では必要）
- **確保失敗時は `NULL` を返す** → 必ず `NULL` チェックをする
- 使い終わったら `free` する。`free` 後は `NULL` を代入してダングリングポインタを防ぐ

### メモリイメージ

```
malloc(5 * sizeof(int)):

  ヒープ:
  +------+------+------+------+------+
  |  ??  |  ??  |  ??  |  ??  |  ??  |  ← ゴミ値（未初期化）
  +------+------+------+------+------+
  ↑
  arr

初期化後:
  +------+------+------+------+------+
  |  0   |  10  |  20  |  30  |  40  |
  +------+------+------+------+------+
```

### 確保サイズの計算方法

```c
// 型のサイズに要素数を掛ける（sizeof は必ず使う）
malloc(n * sizeof(int))      // int n 個
malloc(sizeof(Point))        // 構造体 1 個
malloc(n * sizeof(Point))    // 構造体 n 個
```

---

## 2. `calloc` — ゼロ初期化して確保

```c
double *dbls = calloc(4, sizeof(double));  // (要素数, 1要素のサイズ)
if (!dbls) { return 1; }
// calloc はゼロ初期化を保証する → 0.0 0.0 0.0 0.0
free(dbls);
dbls = NULL;
```

### `malloc` との比較

| | `malloc(n * size)` | `calloc(n, size)` |
|--|-------------------|-------------------|
| 初期化 | なし（ゴミ値） | **ゼロ初期化** |
| 引数 | バイト数 1 つ | 要素数 + サイズの 2 つ |
| 速度 | 若干速い | ゼロ埋め分だけ遅い |
| 用途 | すぐに上書きする場合 | ゼロ状態から使いたい場合 |

```c
// 以下の 2 つは等価（ただし calloc はオーバーフローに安全）
malloc(n * size)         // n * size がオーバーフローする危険がある
calloc(n, size)          // 内部でオーバーフローを検出できる（実装依存）
```

---

## 3. `realloc` — サイズを変更して再確保

```c
int *buf = malloc(3 * sizeof(int));
buf[0] = 1; buf[1] = 2; buf[2] = 3;

// BAD: 失敗時に buf が NULL になり元のポインタが失われる（メモリリーク）
// buf = realloc(buf, 6 * sizeof(int));

// GOOD: 一時変数で受け取る
int *tmp = realloc(buf, 6 * sizeof(int));
if (!tmp) {
    free(buf);  // 元のポインタを解放してエラー処理
    return 1;
}
buf = tmp;  // 成功したら buf を更新
buf[3] = 4; buf[4] = 5; buf[5] = 6;
free(buf);
buf = NULL;
```

### `realloc` の動作

```
realloc(ptr, new_size) の挙動:

1. 現在の位置で拡張できる場合:
   [1][2][3][ ][ ][ ]   ← 後ろに拡張（元のアドレスのまま）

2. 現在の位置で拡張できない場合:
   旧: [1][2][3]         ← 旧アドレスに解放
   新: [1][2][3][ ][ ][ ] ← 新アドレスにコピー&拡張

3. 縮小の場合:
   [1][2][3][4][5][6] → [1][2][3]  （余分な領域は解放）

4. 失敗の場合:
   NULL を返す。元の ptr は変更されない（まだ有効）
```

### 一時変数パターンが必須な理由

```c
// 危険なパターン
buf = realloc(buf, 6 * sizeof(int));
// realloc が NULL を返した場合:
//   buf = NULL になる → 元のメモリのアドレスが失われる
//   → free できなくなり、メモリリーク確定
```

---

## 4. `free` のルール

```c
free(NULL);    // 1. OK: 安全（何もしない）

int *p = malloc(sizeof(int));
free(p);
p = NULL;
// free(p);   // 2. OK: free(NULL) は安全
// *p = 1;    // 3. NG: use-after-free（未定義動作）

int x = 42;
// free(&x);  // 4. NG: スタック変数を free するのは UB
```

### free の 4 つのルール

| # | ルール | 違反した場合 |
|---|--------|-------------|
| 1 | `malloc`/`calloc`/`realloc` で確保したポインタのみ解放可能 | UB（クラッシュ、メモリ破壊） |
| 2 | `free(NULL)` は安全（何もしない） | 問題なし |
| 3 | 二重 free は UB | クラッシュ、セキュリティ脆弱性 |
| 4 | スタック変数のアドレスを free するのは UB | クラッシュ、スタック破壊 |

### ダングリングポインタとは

```c
int *p = malloc(sizeof(int));
*p = 42;
free(p);
// この時点で p は「解放済みの領域を指すポインタ」= ダングリングポインタ
// *p = 1; は未定義動作（クラッシュするかもしれないし、しないかもしれない）

p = NULL;  // NULL を代入することでダングリングポインタを無効化
// *p = 1; は確実にクラッシュ（セグフォ）→ バグが早期に発見できる
```

---

## 5. 構造体の動的確保

```c
typedef struct { int x; int y; } Point;

Point *pt = malloc(sizeof(Point));
if (!pt) { return 1; }
pt->x = 10;
pt->y = 20;
printf("pt = (%d, %d)\n", pt->x, pt->y);
free(pt);
pt = NULL;
```

### 重要なポイント

- `sizeof(Point)` で構造体のサイズ（パディング込み）を正確に取得できる
- 確保後は `->` 演算子でメンバにアクセスする（`(*pt).x` と同じ）
- `malloc` が返すのはポインタ1つだけ → `free` も1回でよい

### スタック確保との比較

```c
// スタック確保（関数を抜けると自動解放）
Point pt = {10, 20};
process(&pt);  // ポインタを渡す

// ヒープ確保（明示的に free が必要）
Point *pt = malloc(sizeof(Point));
pt->x = 10; pt->y = 20;
process(pt);
free(pt);
```

ヒープ確保が必要なのは：
- 関数を越えて生存させたい場合
- 確保するサイズが実行時に決まる場合
- 大きすぎてスタックに置けない場合

---

## 6. 構造体配列の動的確保

```c
int n = 3;
Point *points = calloc((size_t)n, sizeof(Point));
if (!points) { return 1; }

for (int i = 0; i < n; i++) {
    points[i].x = i;
    points[i].y = i * i;
}
free(points);
points = NULL;
```

### メモリレイアウト

```
calloc(3, sizeof(Point)):  （各 Point は {int x, int y} = 8 バイト）

  ヒープ:
  +--------+--------+--------+--------+--------+--------+
  | x=0    | y=0    | x=1    | y=1    | x=2    | y=4    |
  +--------+--------+--------+--------+--------+--------+
  ↑                 ↑                 ↑
  points[0]         points[1]         points[2]
```

構造体配列はメモリ上に連続して並ぶ（配列と同じ）。  
`free` は1回だけ呼べばよい（配列全体がまとめて解放される）。

### `calloc` vs `malloc` の選択

```c
// ゼロ初期化が不要な場合（すぐに代入する）
Point *pts = malloc(n * sizeof(Point));

// ゼロ初期化が必要な場合（デフォルト状態を 0 にしたい）
Point *pts = calloc(n, sizeof(Point));  // 全フィールドが 0 に
```

---

## 7. メモリ操作関数（`memset` / `memcpy` / `memmove`）

```c
#include <string.h>

// memset: バイト単位でメモリを埋める
char data[8];
memset(data, 0, sizeof(data));    // 全バイトを 0 に
memset(data, 0xFF, sizeof(data)); // 全バイトを 0xFF に

// memcpy: 重ならない領域間のコピー
char src[] = "ABCDEFG";
char dst[8];
memcpy(dst, src, sizeof(src));    // src を dst にコピー

// memmove: 重なる領域でも安全なコピー
char shift[] = "ABCDE";
memmove(shift + 2, shift + 1, 4); // [1..4] → [2..5] にシフト
shift[1] = 'X';
// → "AXBCDE"
```

### 3関数の比較

| 関数 | 用途 | 重複領域 | 速度 |
|------|------|----------|------|
| `memset(dst, val, n)` | n バイトを val で埋める | — | 速い |
| `memcpy(dst, src, n)` | n バイトをコピー | **不可**（UB） | 最速 |
| `memmove(dst, src, n)` | n バイトをコピー | **可** | やや遅い |

### `memcpy` vs `memmove` の使い分け

```
memcpy: src と dst が重ならない場合（通常のコピー）
  src: [A][B][C][D]
  dst:             [A][B][C][D]  ← 重なっていない → memcpy OK

memmove: src と dst が重なる場合（配列内シフトなど）
  "ABCDE" の [1..4] を [2..5] にシフト
  src: _[B][C][D][E]_
  dst: __[B][C][D][E]  ← 1バイトずれて重なっている → memmove 必須
```

### `memset` の注意点

```c
// OK: ゼロ埋め（0 はすべての型で「ゼロ」を表す）
memset(ptr, 0, sizeof(*ptr));

// NG: int 配列に 1 を入れたい場合（1 バイトずつ埋めるため意図通りにならない）
int arr[4];
memset(arr, 1, sizeof(arr));  // 各バイトが 0x01 → int は 0x01010101 = 16843009
// 正しくはループで初期化する
for (int i = 0; i < 4; i++) arr[i] = 1;
```

---

## 8. スタック vs ヒープ

```c
// スタック: 関数終了時に自動解放、高速、サイズ制限あり（通常 1〜8MB）
int stack_arr[100];    // OK
// int big[1000000];   // NG: スタックオーバーフローの可能性

// ヒープ: 明示的に解放が必要、低速、サイズ制限が大きい
int *heap_arr = malloc(100 * sizeof(int));
// ...
free(heap_arr);
```

### スタックとヒープの比較

| | スタック | ヒープ |
|--|----------|--------|
| 確保・解放 | 自動（関数の出入りで管理） | 手動（`malloc`/`free`） |
| 速度 | **非常に速い**（スタックポインタの移動のみ） | 遅い（OS に要求、断片化あり） |
| サイズ上限 | 小さい（通常 1〜8 MB） | 大きい（物理メモリ・仮想メモリに依存） |
| 生存期間 | 関数スコープ内 | `free` するまで（または終了まで） |
| 主な用途 | ローカル変数・関数引数 | 大きなデータ・動的サイズ・長期間保持 |

### 使い分けの指針

```c
// スタックで十分なケース
int buf[256];              // 小さく固定サイズ
Point pt = {1, 2};        // 構造体も小さければスタックでOK

// ヒープが必要なケース
int n;
scanf("%d", &n);
int *arr = malloc(n * sizeof(int));  // サイズが実行時に決まる

typedef struct { char data[1024 * 1024]; } LargeData;
LargeData *ld = malloc(sizeof(LargeData));  // 大きすぎるものはヒープへ
```

---

## 9. まとめ：動的メモリ管理チートシート

```c
// --- malloc: 初期化なしで確保 ---
int *arr = malloc(n * sizeof(int));  // void* → 型に暗黙変換
if (!arr) { /* エラー処理 */ }       // 必ず NULL チェック
// ...使用...
free(arr);
arr = NULL;                          // ダングリングポインタ防止

// --- calloc: ゼロ初期化して確保 ---
int *arr = calloc(n, sizeof(int));   // (要素数, サイズ) → 全バイトが 0
if (!arr) { /* エラー処理 */ }

// --- realloc: サイズ変更（必ず一時変数を使う）---
int *tmp = realloc(buf, new_n * sizeof(int));
if (!tmp) { free(buf); return 1; }   // 失敗時は元のポインタを自分で解放
buf = tmp;                           // 成功時のみ更新

// --- free のルール ---
free(NULL);          // OK: 安全（何もしない）
free(ptr); ptr=NULL; // free 後は NULL 代入
// free(ptr) x2;     // NG: 二重 free → UB
// free(&local_var); // NG: スタック変数 → UB
// *ptr after free;  // NG: use-after-free → UB

// --- 構造体の動的確保 ---
Point *pt = malloc(sizeof(Point));
pt->x = 10; pt->y = 20;
free(pt); pt = NULL;

// --- 構造体配列の動的確保 ---
Point *pts = calloc(n, sizeof(Point));  // n 個、ゼロ初期化
pts[i].x = ...; pts[i].y = ...;
free(pts); pts = NULL;

// --- メモリ操作関数 ---
memset(ptr, 0, sizeof(*ptr));      // ゼロ埋め（int に 0 以外はループで）
memcpy(dst, src, n);               // n バイトコピー（重複不可）
memmove(dst, src, n);              // n バイトコピー（重複可・配列シフト等）

// --- 使い分けの指針 ---
// スタック: 小さく固定サイズ、関数スコープ内で完結
// ヒープ:   動的サイズ、関数を越えて使用、大きなデータ
```
