# 04 - 構造体・typedef・共用体・列挙型 (Structs / typedefs / Unions / Enums)

C言語の複合データ型に関する基本概念と操作をまとめたクイックレビュー用ドキュメントです。

---

## 目次

1. [基本的な struct 定義](#1-基本的な-struct-定義)
2. [typedef struct](#2-typedef-struct)
3. [自己参照構造体（linked list など）](#3-自己参照構造体linked-list-など)
4. [enum（列挙型）](#4-enum列挙型)
5. [union（共用体）](#5-union共用体)
6. [ビットフィールド](#6-ビットフィールド)
7. [構造体の初期化と使い方](#7-構造体の初期化と使い方)
8. [パディングとアライメント](#8-パディングとアライメント)
9. [まとめ：構造体・列挙型・共用体チートシート](#9-まとめ構造体列挙型共用体チートシート)

---

## 1. 基本的な struct 定義

```c
struct Point {
    int x;
    int y;
};

struct Point p1 = {3, 4};              // 順番通りに初期化
struct Point p2 = {.x = 10, .y = 20}; // 指示初期化子（推奨）
```

### 重要なポイント

- `struct` キーワードは型名の前に毎回必要（`typedef` なしの場合）
- 指示初期化子 `.フィールド名 =` を使うと順序に依存せず安全
- 構造体のコピーは**代入演算子 `=` だけで可能**（`memcpy` 不要）

```c
struct Point p3 = p1; // p1 の内容が p3 にコピーされる（独立したコピー）
p3.x = 99;            // p1.x は変わらない
```

> **Rust との比較:** Rust の `struct` と同様の概念だが、C には所有権・ムーブセマンティクスがなく、代入は常にコピー。

---

## 2. typedef struct

```c
typedef struct {
    float x;
    float y;
    float z;
} Vec3;

Vec3 v1 = {1.0f, 2.0f, 3.0f};
Vec3 v2 = {.x = 4.0f, .y = 5.0f, .z = 6.0f};
```

### `typedef` を使う理由

| 書き方 | 型として使う場合 |
|--------|-----------------|
| `struct` タグのみ | `struct Point p;` と毎回 `struct` が必要 |
| `typedef struct` | `Vec3 v;` と短く書ける（C の慣用スタイル） |

`typedef struct { ... } 型名;` とすると `struct` キーワードを省略できる。

### ゼロ初期化

```c
Vec3 zero = {0};                // フィールドをすべて 0 に初期化
memset(&zero, 0, sizeof(zero)); // 同等（ビット単位のゼロ埋め）
```

C では `{}` によるゼロ初期化はできない（C++ とは異なる）。  
`{0}` が最も簡潔なゼロ初期化の慣用表現。

### 構造体を返す関数

```c
Vec3 vec3_add(Vec3 a, Vec3 b) {
    return (Vec3){ a.x + b.x, a.y + b.y, a.z + b.z };
}
```

`(Vec3){ ... }` は**複合リテラル**（C99以降）。関数内でその場に構造体を作って返せる。

---

## 3. 自己参照構造体（linked list など）

```c
typedef struct Node {
    int          value;
    struct Node *next; // 自分自身を指すポインタ
} Node;
```

### なぜ `struct Node` タグが必要か

```
typedef struct Node { ... } Node;
                ↑              ↑
           struct タグ      typedef 名
```

- `typedef` の定義が完了する**前**に `next` の型を宣言する必要がある
- `typedef` 名 `Node` はまだ使えないため、`struct Node *` と struct タグで参照する
- **`typedef struct { ... } Node;`（タグなし）では自己参照できない**

### メモリイメージ（linked list）

```
Node a          Node b          Node c
+-------+---+   +-------+---+   +-------+------+
| val=1 | *─┼──>| val=2 | *─┼──>| val=3 | NULL |
+-------+---+   +-------+---+   +-------+------+
```

---

## 4. enum（列挙型）

```c
typedef enum {
    DIR_NORTH = 0,
    DIR_EAST  = 1,
    DIR_SOUTH = 2,
    DIR_WEST  = 3,
} Direction;

typedef enum {
    COLOR_RED   = 0xFF0000,
    COLOR_GREEN = 0x00FF00,
    COLOR_BLUE  = 0x0000FF,
} Color;
```

### 重要なポイント

- C の enum は**名前付き整数定数**に過ぎない。内部型は `int`
- 値を省略すると前の値 +1 が自動的に割り当てられる（最初は 0）
- 16進数など任意の整数値を割り当てられる（フラグ・カラーコードなどに便利）

```c
Direction dir = DIR_NORTH;
printf("%d\n", dir); // → 0（int として扱える）
```

### switch との組み合わせ

```c
switch (dir) {
    case DIR_NORTH: printf("North\n"); break;
    case DIR_EAST:  printf("East\n");  break;
    case DIR_SOUTH: printf("South\n"); break;
    case DIR_WEST:  printf("West\n");  break;
}
```

`-Wall` を付けると全 case を網羅していない場合に警告が出る。`default` を書かずに全 case を列挙すると、将来 enum に値を追加したときにコンパイラが警告してくれる。

> **Rust との比較:** Rust の enum は各バリアントがデータを持てるが、C の enum は整数定数のみ。C でデータ付き列挙型を実現するには `union` と組み合わせる（タグ付き共用体）。

---

## 5. union（共用体）

```c
typedef union {
    uint32_t as_u32;
    uint8_t  bytes[4];
} U32Bytes;

U32Bytes u;
u.as_u32 = 0x12345678;
printf("%02X %02X %02X %02X\n",
       u.bytes[0], u.bytes[1], u.bytes[2], u.bytes[3]);
// リトルエンディアン環境: 78 56 34 12
// ビッグエンディアン環境: 12 34 56 78
```

### union の仕組み

**すべてのメンバが同じメモリを共有する。**

```
同一メモリ領域（4バイト）:
  as_u32:  [ 12 | 34 | 56 | 78 ]  ← 32bit 整数として解釈
  bytes[]: [ 78   56   34   12 ]  ← 4つの 8bit 整数として解釈（LE）
```

- `sizeof(union)` は**最大メンバのサイズ**
- 書いたメンバとは別のメンバを読むと「型パニング」になる（C では合法）

### struct との違い

| | `struct` | `union` |
|--|----------|---------|
| メモリ | 各メンバが独立した領域を持つ | 全メンバが同じ領域を共有 |
| サイズ | 全メンバの合計 + パディング | 最大メンバのサイズ |
| 用途 | 複数のフィールドをまとめる | 同じデータを異なる型で解釈する |

### 主な用途

- バイト列 ↔ 整数の変換（エンディアン確認・シリアライズ）
- タグ付き共用体（`enum` と組み合わせて Rust の enum に相当する構造を作る）

```c
// タグ付き共用体の例
typedef struct {
    enum { INT_VAL, FLOAT_VAL } tag;
    union {
        int   i;
        float f;
    } data;
} Value;
```

---

## 6. ビットフィールド

```c
typedef struct {
    uint8_t r : 5; // 5 ビット（0〜31）
    uint8_t g : 6; // 6 ビット（0〜63）
    uint8_t b : 5; // 5 ビット（0〜31）
} RGB565;

RGB565 rgb = { .r = 31, .g = 63, .b = 31 }; // 各フィールドの最大値
```

### ビットフィールドの仕組み

`型 フィールド名 : ビット幅;` でビット単位のフィールドを定義できる。

```
RGB565 のビット配置（計 16 bit）:
  [ r:5 | g:6 | b:5 ]
    0-31  0-63  0-31
```

この例は **RGB565 フォーマット**（組み込みや GPU でよく使われる 16bit カラー）の表現。

### 注意点

- ビット配置の順序はコンパイラ・アーキテクチャ依存（移植性に注意）
- `sizeof` の結果もパディングにより期待より大きくなることがある
- ハードウェアレジスタや通信プロトコルの実装でよく使われる
- **ポータブルなビット操作が必要なら、マスクとシフトを使う方が確実**

---

## 7. 構造体の初期化と使い方

```c
// ポインタ経由のアクセス
Vec3 *vp = &v1;
vp->x = 100.0f; // (*vp).x = 100.0f と同じ
```

### 初期化方法のまとめ

| 方法 | 例 | 特徴 |
|------|----|------|
| 順番通り | `Vec3 v = {1.0f, 2.0f, 3.0f};` | フィールド順に依存 |
| 指示初期化子 | `Vec3 v = {.x=1.0f, .y=2.0f, .z=3.0f};` | 順序不問・可読性高（推奨） |
| ゼロ初期化 | `Vec3 v = {0};` | 全フィールドを 0 に |
| memset | `memset(&v, 0, sizeof(v));` | ビット単位のゼロ埋め |
| 複合リテラル | `(Vec3){1.0f, 2.0f, 3.0f}` | 式中で構造体を即席生成 |
| 代入コピー | `Vec3 v2 = v1;` | 独立したコピー |

### ポインタ渡しのパターン

```c
// 大きな構造体はポインタで渡してコピーコストを節約
void process(const Vec3 *v) { // const: 読み取り専用
    printf("%.1f\n", v->x);
}

// 値を変更したい場合はポインタで渡す
void scale(Vec3 *v, float s) {
    v->x *= s; v->y *= s; v->z *= s;
}
```

---

## 8. パディングとアライメント

```c
typedef struct { char a; int b; char c; } Padded; // 12 バイト
typedef struct { int b; char a; char c; } Packed;  //  8 バイト
```

### パディングが入る理由

CPU はメモリへのアクセスを**アライメント（境界整合）**単位で行う。  
`int`（4バイト）は 4 の倍数アドレスに配置する必要があるため、コンパイラが自動的にパディングを挿入する。

```
Padded のメモリレイアウト（合計 12 バイト）:
  [a:1][pad:3][b:4][c:1][pad:3]
   ^          ^         ^
   char       int       char の後ろにも struct のアライメントのためのパディング

Packed のメモリレイアウト（合計 8 バイト）:
  [b:4][a:1][c:1][pad:2]
   ^         ^
   int を先頭に持ってくることでパディングが減る
```

### フィールドの並び順による最適化

**大きい型から順に並べる**とパディングを最小化できる。

```c
// 悪い例（パディングが多い）
struct Bad  { char a; int b; char c; double d; }; // 24 バイト

// 良い例（パディングが少ない）
struct Good { double d; int b; char a; char c; };  // 16 バイト
```

### パディングを制御したい場合

```c
#pragma pack(1) // 1バイト境界に詰める（GCC/MSVC）
struct NoPad { char a; int b; char c; };
#pragma pack()  // 元に戻す
```

> **注意:** `#pragma pack` はアライメント違反を引き起こしパフォーマンス低下や UB の原因になる。通信プロトコルやバイナリフォーマットの厳密な制御が必要な場合のみ使う。

---

## 9. まとめ：構造体・列挙型・共用体チートシート

```c
// --- struct の定義と初期化 ---
struct Point { int x; int y; };          // struct タグのみ（毎回 struct が必要）
typedef struct { float x, y, z; } Vec3;  // typedef で短縮（慣用スタイル）

struct Point p = {3, 4};                 // 順番通り
struct Point p = {.x = 3, .y = 4};      // 指示初期化子（推奨）
Vec3 v = {0};                            // 全フィールドをゼロ初期化
Vec3 copy = original;                    // 代入でコピー（memcpy 不要）

// --- 自己参照構造体（タグが必要）---
typedef struct Node { int val; struct Node *next; } Node;

// --- enum ---
typedef enum { DIR_NORTH=0, DIR_EAST=1, DIR_SOUTH=2, DIR_WEST=3 } Direction;
Direction d = DIR_NORTH;
printf("%d\n", d);  // int として扱える

// --- ポインタ経由アクセス ---
Vec3 *vp = &v;
vp->x = 1.0f;    // (*vp).x と同じ

// --- union（同じメモリを異なる型で解釈）---
typedef union { uint32_t as_u32; uint8_t bytes[4]; } U32Bytes;
U32Bytes u;
u.as_u32 = 0x12345678;
u.bytes[0]; // リトルエンディアンなら 0x78

// --- ビットフィールド ---
typedef struct { uint8_t r:5; uint8_t g:6; uint8_t b:5; } RGB565;
RGB565 rgb = { .r=31, .g=63, .b=31 };  // 各フィールドの最大値

// --- パディング最小化：大きい型から順に並べる ---
struct Good { double d; int b; char a; char c; }; // 無駄なパディングが少ない

// --- ゼロ初期化 ---
Vec3 zero = {0};
memset(&zero, 0, sizeof(zero)); // 同等

// --- 関数への渡し方 ---
void read_only(const Vec3 *v);  // 読み取り専用: const ポインタ渡し
void modify(Vec3 *v);            // 変更あり: ポインタ渡し
Vec3 by_value(Vec3 v);           // 小さな構造体は値渡しも可
```
