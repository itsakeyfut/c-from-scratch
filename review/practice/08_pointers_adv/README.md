# 08 - ポインタ応用 (Advanced Pointers)

`void*`・関数ポインタ・バイト単位のポインタ演算など、C言語のポインタ応用テクニックをまとめたクイックレビュー用ドキュメントです。

---

## 目次

1. [void* — 汎用ポインタ](#1-void--汎用ポインタ)
2. [void* + elem_size によるジェネリック操作](#2-void--elem_size-によるジェネリック操作)
3. [関数ポインタの宣言と呼び出し](#3-関数ポインタの宣言と呼び出し)
4. [関数ポインタを使ったソート](#4-関数ポインタを使ったソート)
5. [関数ポインタの配列（ジャンプテーブル）](#5-関数ポインタの配列ジャンプテーブル)
6. [char* によるバイト単位のポインタ演算](#6-char-によるバイト単位のポインタ演算)
7. [restrict キーワード](#7-restrict-キーワード)
8. [まとめ：ポインタ応用チートシート](#8-まとめポインタ応用チートシート)

---

## 1. `void*` — 汎用ポインタ

```c
int    i_val = 42;
double d_val = 3.14;
void  *vp;

vp = &i_val;
printf("%d\n", *(int *)vp);      // キャストしてからデリファレンス

vp = &d_val;
printf("%.2f\n", *(double *)vp);

// void* のままでは使えない操作
// *(vp)    // NG: コンパイルエラー（型がないのでサイズ不明）
// vp + 1   // NG: 演算不可（GCC 拡張では char* 扱いになるが移植性なし）

char *cp = (char *)vp + 1;  // OK: char* にキャストしてバイト演算
```

### `void*` とは

**型情報を持たない汎用ポインタ。** どんな型のポインタでも代入できる。

- `malloc` の戻り値が `void *` なのもこのため（どんな型にでも使える）
- デリファレンス・ポインタ演算をするには、必ず具体的な型にキャストする
- C では `void *` ↔ 任意ポインタの変換は暗黙キャスト可（C++ では明示キャストが必要）

### `void*` の主な用途

| 用途 | 例 |
|------|-----|
| 汎用メモリ確保 | `malloc` / `calloc` の戻り値 |
| 型消去による汎用関数 | `qsort`、`bsearch` のコールバック引数 |
| ジェネリックなデータ構造 | `Vec`（要素サイズを別途管理）|
| バイト単位のメモリ操作 | `memcpy`、`memset` の引数 |

---

## 2. `void*` + `elem_size` によるジェネリック操作

```c
// 要素 i を取得するマクロ（Vec の vec_get 相当）
#define ELEM(base, i, sz)  ((void *)((char *)(base) + (i) * (sz)))

int    int_arr[4]  = {10, 20, 30, 40};
double dbl_arr[3]  = {1.1, 2.2, 3.3};

void *elem2 = ELEM(int_arr, 2, sizeof(int));
printf("%d\n", *(int *)elem2);       // → 30

void *elem1 = ELEM(dbl_arr, 1, sizeof(double));
printf("%.1f\n", *(double *)elem1);  // → 2.2
```

### 仕組みの解説

C の配列はメモリ上で連続しているため、先頭アドレスと要素サイズさえあれば任意の要素にアクセスできる。

```
ELEM(base, i, sz) の計算:

  base（先頭アドレス）
    ↓
  (char *)base          // バイト単位で演算するために char* にキャスト
  + i * sz              // i 番目の要素までのバイト数を加算
  → (void *)            // void* として返す（呼び出し側が型キャストして使う）
```

```
int_arr のメモリ:
  [10][20][30][40]
   0   4   8   12  ← バイトオフセット（sizeof(int) = 4 の場合）

ELEM(int_arr, 2, 4):
  (char*)int_arr + 2 * 4 = 先頭 + 8 バイト → [30] の位置
```

### なぜ `char*` にキャストするのか

`void*` はポインタ演算が禁止されている（型のサイズが不明なため）。  
`char*` は 1 バイト単位で演算できるので、**バイト単位の任意のオフセット計算**に使う。  
これは `Vec` や `HashMap` など型消去を伴うデータ構造の内部実装の基本パターン。

---

## 3. 関数ポインタの宣言と呼び出し

```c
// typedef なし（宣言が読みにくい）
int (*raw_cmp)(const void *, const void *) = cmp_asc;

// typedef あり（推奨）
typedef int (*CmpFn)(const void *, const void *);
CmpFn fn = cmp_asc;

int a = 5, b = 3;
fn(&a, &b);   // 関数ポインタを通じて呼び出す
fn = cmp_desc; // 別の関数に差し替え可能
fn(&a, &b);
```

### 関数ポインタの構文

```
戻り値型 (*変数名)(引数型, ...) = 関数名;
  ↑       ↑                      ↑
  int    (*cmp)  (const void*, const void*) = cmp_asc;
```

`*` を括弧で囲む点が特徴。括弧がないと「関数を返す関数」の宣言になってしまう。

```c
int *f(void);       // int* を返す関数 f（関数ポインタではない）
int (*fp)(void);    // void を受け取って int を返す関数へのポインタ fp
```

### typedef で読みやすくする

```c
// typedef の書き方
typedef 戻り値型 (*型名)(引数型, ...);

typedef int  (*CmpFn)(const void *, const void *);  // 比較関数
typedef void (*PrintFn)(int);                        // 出力関数
typedef void (*Callback)(void *ctx, int event);      // コールバック
```

---

## 4. 関数ポインタを使ったソート

```c
static int cmp_asc(const void *a, const void *b) {
    return *(const int *)a - *(const int *)b;
}
static int cmp_desc(const void *a, const void *b) {
    return *(const int *)b - *(const int *)a;
}

// 高階関数: ソート戦略を引数で受け取る
static void my_sort(int *arr, size_t n, CmpFn cmp) {
    qsort(arr, n, sizeof(int), cmp);
}

int nums[] = {5, 2, 8, 1, 9, 3};
my_sort(nums, n, cmp_asc);   // 昇順ソート
my_sort(nums, n, cmp_desc);  // 降順ソート（関数だけ差し替え）
```

### `qsort` の比較関数の規約

`qsort` のコールバックは必ず `int f(const void *, const void *)` の形でなければならない。

| 戻り値 | 意味 |
|--------|------|
| 負の値 | 第1引数 < 第2引数（第1引数を前に置く）|
| `0` | 第1引数 == 第2引数 |
| 正の値 | 第1引数 > 第2引数（第2引数を前に置く）|

```c
// 昇順: a - b（a > b なら正 → b を前に置く）
return *(const int *)a - *(const int *)b;

// 降順: b - a（b > a なら正 → a を前に置く）
return *(const int *)b - *(const int *)a;
```

> **注意:** `a - b` の減算は `INT_MIN` 付近でオーバーフローする可能性がある。  
> 安全な書き方は `(a > b) - (a < b)` だが、実用上は問題になることは少ない。

### 高階関数パターン

```c
// 関数ポインタを引数に取る → 振る舞いを外から注入できる
void my_sort(int *arr, size_t n, CmpFn cmp) { ... }

// apply: 各要素に任意の関数を適用
void apply(const int *arr, size_t n, PrintFn fn) {
    for (size_t i = 0; i < n; i++) fn(arr[i]);
}

apply(nums, n, print_int);     // 通常表示
apply(nums, n, print_hex);     // 16進表示
apply(nums, n, print_squared); // 二乗表示（関数だけ差し替え）
```

---

## 5. 関数ポインタの配列（ジャンプテーブル）

```c
PrintFn fns[] = { print_int, print_hex, print_squared };
const char *names[] = { "int", "hex", "squared" };

for (int f = 0; f < 3; f++) {
    printf("%-8s: ", names[f]);
    for (int i = 0; i < 3; i++) {
        fns[f](sample[i]);   // インデックスで関数を切り替え
    }
    printf("\n");
}
```

### ジャンプテーブルとは

関数ポインタの配列を使って、**インデックスで呼び出す関数を動的に切り替える**パターン。

```
fns[0] → print_int      ← インデックス 0 で int 表示
fns[1] → print_hex      ← インデックス 1 で hex 表示
fns[2] → print_squared  ← インデックス 2 で二乗表示

fns[f](x) で f の値によって異なる関数が呼ばれる
```

### `switch` との比較

```c
// switch の場合（ケース追加のたびに変更が必要）
switch (mode) {
    case 0: print_int(x);     break;
    case 1: print_hex(x);     break;
    case 2: print_squared(x); break;
}

// ジャンプテーブルの場合（配列に追加するだけ）
PrintFn fns[] = { print_int, print_hex, print_squared };
fns[mode](x);  // 追加時は配列に足すだけ
```

ジャンプテーブルは `switch` より**O(1) で呼び出せ、ケース追加が容易**。  
ゲームのステートマシン、コマンドパーサー、プラグインシステムなどで活用される。

---

## 6. `char*` によるバイト単位のポインタ演算

```c
typedef struct { int x; int y; } Point;
Point pts[3] = {{1,2},{3,4},{5,6}};

char  *base      = (char *)pts;   // 先頭を char* として取得
size_t elem_size = sizeof(Point);

for (int i = 0; i < 3; i++) {
    Point *p = (Point *)(base + i * elem_size);  // バイトオフセットで要素を取得
    printf("pts[%d] = (%d, %d)\n", i, p->x, p->y);
}
```

### なぜこのパターンが重要か

通常の配列アクセス `pts[i]` は型が決まっている場合にしか使えない。  
`void*` でデータを保持する汎用コンテナ（`Vec` など）では、型情報がないため  
`char*` + バイトオフセットで要素位置を計算する必要がある。

```
pts のメモリ（Point = 8 バイトの場合）:

バイトオフセット: 0        8        16
                 +--------+--------+--------+
                 | (1, 2) | (3, 4) | (5, 6) |
                 +--------+--------+--------+
                 ↑
                 base（char*）

base + 0 * 8 → pts[0] = (1, 2)
base + 1 * 8 → pts[1] = (3, 4)
base + 2 * 8 → pts[2] = (5, 6)
```

### アライメントの注意

`char*` からキャストして構造体ポインタを作る場合、**アライメントが合っていること**が前提。  
`malloc` が返すアドレスは最大アライメントが保証されているので問題ない。  
スタック配列や `malloc` 確保済みバッファからのオフセット計算も、要素サイズ単位で進む限り安全。

---

## 7. `restrict` キーワード

```c
// memcpy の宣言（標準ライブラリ）
void *memcpy(void * restrict dst, const void * restrict src, size_t n);
//                   ↑                          ↑
//           「dst はここでしかこの領域を指さない」というコンパイラへのヒント
```

### `restrict` とは

**「このポインタだけがその指すメモリ領域にアクセスする」** というコンパイラへの約束（C99以降）。

コンパイラはこの約束を元に、ポインタ間のエイリアシング（別名参照）を考慮せずに積極的な最適化を行える。

```c
// restrict なし: dst と src が同じ領域を指す可能性があるため
//                コンパイラは毎回メモリから読み直す必要がある
void add(int *dst, const int *src, int n) { ... }

// restrict あり: エイリアシングがないことが保証されるため
//                レジスタへのキャッシュなど積極的な最適化が可能
void add(int * restrict dst, const int * restrict src, int n) { ... }
```

### `memcpy` vs `memmove` との関係

| 関数 | restrict | 重複領域の扱い |
|------|----------|---------------|
| `memcpy` | あり（重複なし前提） | 重複すると UB |
| `memmove` | なし（重複考慮）| 重複しても安全 |

`memcpy` の `restrict` 指定により「src と dst は重ならない」ことが前提になっている。  
重なる可能性がある場合は `memmove` を使う。

---

## 8. まとめ：ポインタ応用チートシート

```c
// --- void*: 汎用ポインタ ---
void *vp = &some_val;
*(int *)vp          // キャストしてからデリファレンス
(char *)vp + n      // バイト演算には char* にキャスト

// --- void* + elem_size: ジェネリックアクセス ---
#define ELEM(base, i, sz) ((void *)((char *)(base) + (i) * (sz)))
void *p = ELEM(arr, i, sizeof(int));
int val = *(int *)p;

// --- 関数ポインタ ---
// 宣言（typedef なし）
int (*cmp)(const void *, const void *) = cmp_asc;

// 宣言（typedef あり・推奨）
typedef int (*CmpFn)(const void *, const void *);
CmpFn fn = cmp_asc;
fn(&a, &b);       // 呼び出し
fn = cmp_desc;    // 差し替え

// 高階関数の引数として渡す
void my_sort(int *arr, size_t n, CmpFn cmp) {
    qsort(arr, n, sizeof(int), cmp);
}

// --- ジャンプテーブル ---
typedef void (*PrintFn)(int);
PrintFn fns[] = { print_int, print_hex, print_squared };
fns[mode](x);  // mode に応じた関数を O(1) で呼び出す

// --- char* によるバイト演算 ---
char *base = (char *)arr;
Point *p = (Point *)(base + i * sizeof(Point));  // i 番目の要素へ

// --- qsort の比較関数パターン ---
int cmp_asc(const void *a, const void *b) {
    return *(const int *)a - *(const int *)b;  // 昇順
}
int cmp_desc(const void *a, const void *b) {
    return *(const int *)b - *(const int *)a;  // 降順
}

// --- restrict: エイリアシングなしのコンパイラヒント ---
void copy(void * restrict dst, const void * restrict src, size_t n);
// → dst と src が重ならないことをコンパイラに保証 → 最適化が効く
// 重なりがある場合は memmove を使う
```
