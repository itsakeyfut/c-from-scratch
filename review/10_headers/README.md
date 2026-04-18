# 10 - ヘッダファイル分割 (Header Files)

C言語のヘッダファイル・実装ファイルの分割方法と、関連する概念をまとめたクイックレビュー用ドキュメントです。

---

## 目次

1. [ファイル構成の概要](#1-ファイル構成の概要)
2. [ヘッダファイル（mylib.h）の書き方](#2-ヘッダファイルmylibhの書き方)
3. [インクルードガード](#3-インクルードガード)
4. [実装ファイル（mylib.c）の書き方](#4-実装ファイルmylibcの書き方)
5. [static による非公開関数](#5-static-による非公開関数)
6. [ヘッダの使い方（main.c）](#6-ヘッダの使い方mainc)
7. [コンパイルとリンク](#7-コンパイルとリンク)
8. [まとめ：ヘッダ分割チートシート](#8-まとめヘッダ分割チートシート)

---

## 1. ファイル構成の概要

```
10_headers/
  mylib.h   ← 公開インターフェース（型・定数・関数宣言）
  mylib.c   ← 実装（関数の中身）
  main.c    ← 利用側（mylib.h をインクルードして使う）
```

### なぜファイルを分割するのか

| 目的 | 説明 |
|------|------|
| **インターフェースと実装の分離** | ヘッダを見れば「何ができるか」がわかる |
| **コンパイル時間の短縮** | 変更があったファイルだけ再コンパイルできる |
| **カプセル化** | `static` で内部実装を隠蔽できる |
| **再利用性** | 他のプロジェクトから `mylib.h` + `mylib.c` をそのまま使える |

### C のビルドフロー

```
mylib.c ──┐
           ├─→ コンパイル → mylib.o ──┐
mylib.h ──┘                           ├─→ リンク → 実行ファイル
                                       │
main.c ──→ コンパイル → main.o  ──────┘
(mylib.h を #include して型・宣言を参照)
```

---

## 2. ヘッダファイル（`mylib.h`）の書き方

```c
// mylib.h の構成
#ifndef MYLIB_H
#define MYLIB_H

#include <stddef.h>   // このヘッダが必要とするヘッダを自分でインクルード
#include <stdbool.h>

// 定数マクロ
#define MYLIB_VERSION  1
#define POINT_MAX      1000

// 型定義
typedef struct { int x; int y; } Point;
typedef enum { MYLIB_OK=0, MYLIB_ERR=-1, MYLIB_OOM=-2 } MyLibResult;

// 関数プロトタイプ（宣言のみ）
Point  point_new(int x, int y);
Point  point_add(Point a, Point b);
double point_dist(Point a, Point b);
bool   point_eq(Point a, Point b);
void   point_print(const Point *p);

#endif  // MYLIB_H
```

### ヘッダファイルに書くべきもの・書かないもの

| 書くべきもの | 書かないもの |
|-------------|-------------|
| `typedef` による型定義 | 関数の実装（中身）|
| `#define` の定数・マクロ | グローバル変数の定義（`extern` 宣言は可）|
| 関数プロトタイプ（宣言） | `static` 関数（内部専用なので宣言不要）|
| 他のヘッダの `#include` | `static` でない変数の定義 |

### ヘッダ自体が必要なヘッダをインクルードする

```c
// NG: 利用側に依存する（mylib.h を使う人が先に stdbool.h を書く必要がある）
bool point_eq(Point a, Point b);  // ← bool が未定義だとエラー

// OK: mylib.h 自身で必要なものをインクルード（自己完結）
#include <stdbool.h>
bool point_eq(Point a, Point b);  // ← どこでインクルードしても動く
```

---

## 3. インクルードガード

```c
#ifndef MYLIB_H   // MYLIB_H が未定義ならコンパイル
#define MYLIB_H   // 定義して2回目以降をスキップ

// ヘッダの内容

#endif  // MYLIB_H
```

### なぜ必要か

同じヘッダが複数回インクルードされると、型や関数の**多重定義エラー**が起きる。

```c
// main.c が a.h と b.h をインクルードし、両方が mylib.h をインクルードする場合:
#include "a.h"     // a.h の中で #include "mylib.h" → Point 定義済み
#include "b.h"     // b.h の中で #include "mylib.h" → Point を再定義 → エラー！
```

インクルードガードがあれば 2 回目の `#include "mylib.h"` は何もしない：

```
1回目: MYLIB_H が未定義 → #ifndef が真 → 内容をコンパイル → MYLIB_H を定義
2回目: MYLIB_H が定義済み → #ifndef が偽 → #endif まで全スキップ
```

### ガードマクロの命名規則

```c
// 慣習: ファイル名を大文字にして _ に変換
// mylib.h  → MYLIB_H
// my_vec.h → MY_VEC_H
// foo/bar.h → FOO_BAR_H（ディレクトリ込みにする場合も）
```

### `#pragma once`（代替手段）

```c
#pragma once   // GCC / Clang / MSVC が対応（C 標準ではない）
// ↑ これだけで二重インクルードを防げる。ガードより簡潔
```

C 標準ではないが、現代的な環境では広く使われている。  
移植性を最重視する場合は `#ifndef` ガードを使う。

---

## 4. 実装ファイル（`mylib.c`）の書き方

```c
// mylib.c
#include "mylib.h"   // 対応ヘッダを先頭でインクルード（自己整合性チェック）
#include <stdio.h>
#include <math.h>

Point point_new(int x, int y) {
    return (Point){ clamp(x, -POINT_MAX, POINT_MAX),
                    clamp(y, -POINT_MAX, POINT_MAX) };
}

// ... 他の関数の実装
```

### 実装ファイルの 3 つのルール

**ルール 1: 対応ヘッダを先頭でインクルードする**

```c
#include "mylib.h"   // ← 必ず先頭
#include <stdio.h>
```

なぜ先頭か？  
`mylib.h` に書いた宣言と実装が**整合しているかをコンパイラが検証**できる。  
例えばヘッダで `int point_new(...)` と宣言したのに実装を `Point point_new(...)` にした場合、  
先頭でインクルードすることで即座にコンパイルエラーになる。

**ルール 2: 外部に公開しない関数は `static` にする**

```c
static int clamp(int val, int min, int max) { ... }  // 外部から呼べない
```

**ルール 3: グローバル変数を極力使わない**

グローバル変数はすべての `.c` ファイルから変更できるため、バグの原因になりやすい。

---

## 5. `static` による非公開関数

```c
// mylib.c 内部にのみ存在する private 関数
static int clamp(int val, int min, int max) {
    if (val < min) return min;
    if (val > max) return max;
    return val;
}
```

### `static` 関数のスコープ

```
mylib.c:
  static int clamp(...)    ← このファイル内からしか呼べない
  Point point_new(...)     ← mylib.h で宣言されているので外部から呼べる

main.c:
  clamp(...)   // ← コンパイルエラー（clamp は mylib.c 内部のみ）
  point_new()  // ← OK（mylib.h 経由で宣言されている）
```

### `static` 関数をヘッダに書かない理由

ヘッダに書くと、そのヘッダをインクルードした**すべての `.c` ファイルに static 関数のコピーが作られる**。  
内部実装は隠蔽するのが正しい設計。

### `static` の 2 つの意味

| 場所 | 意味 |
|------|------|
| 関数の前 | ファイルスコープの private（他のファイルから見えない）|
| ローカル変数の前 | 関数を跨いで値が保持される（静的ローカル変数）|

---

## 6. ヘッダの使い方（`main.c`）

```c
#include <stdio.h>
#include "mylib.h"  // "" で自分のヘッダ、<> でシステムヘッダ
```

### `""` と `<>` の違い

| 書き方 | 検索順序 |
|--------|----------|
| `#include "mylib.h"` | まず現在のディレクトリ、次にシステムのインクルードパス |
| `#include <stdio.h>` | システムのインクルードパスのみ |

自分で作ったヘッダには `""` を使うのが慣習。

### ヘッダのインクルード後に使えるもの

```c
#include "mylib.h"

// mylib.h で定義した型を使える
Point p1 = point_new(3, 4);

// mylib.h で定義したマクロを使える
printf("%d\n", MYLIB_VERSION);  // 1
printf("%d\n", POINT_MAX);       // 1000

// mylib.h で宣言した関数を使える
Point sum = point_add(p1, p2);
double dist = point_dist(p1, p2);
```

### インクルードガードの確認

```c
#include "mylib.h"   // 1回目: Point 型・関数が使えるようになる
// ...
#include "mylib.h"   // 2回目: ガードにより何も起きない（エラーにならない）
```

---

## 7. コンパイルとリンク

### 手動コンパイル

```bash
# 個別にコンパイル（オブジェクトファイルを生成）
gcc -c mylib.c -o mylib.o
gcc -c main.c  -o main.o

# リンク（実行ファイルを生成）
gcc main.o mylib.o -o main -lm   # -lm は math.h (sqrt) のため

# または一括コンパイル＆リンク
gcc main.c mylib.c -o main -lm
```

### ヘッダのみインクルード・実装のリンクが必要なもの

```c
#include <math.h>   // 宣言のみ → コンパイル時
                    // sqrt の実装は libm にある → リンク時に -lm が必要
```

`#include` はコンパイラへの宣言の提供。  
実際のコードはリンカが別途ライブラリ（`.a` / `.so` / `.lib`）から結合する。

### 依存関係の図

```
              コンパイル時             リンク時
main.c ───→ (mylib.hを参照) ───→ main.o ──┐
                                           ├──→ 実行ファイル
mylib.c ──→ (mylib.hを参照) ───→ mylib.o ─┘
```

---

## 8. まとめ：ヘッダ分割チートシート

```c
// =========================================================
// mylib.h（インターフェース）
// =========================================================
#ifndef MYLIB_H
#define MYLIB_H

#include <stdbool.h>  // このヘッダが依存するものは自前でインクルード

// 定数・マクロ
#define MYLIB_VERSION 1

// 型定義
typedef struct { int x; int y; } Point;

// 関数プロトタイプのみ（実装は .c に書く）
Point  point_new(int x, int y);
void   point_print(const Point *p);

// static 関数はここに書かない

#endif  // MYLIB_H

// =========================================================
// mylib.c（実装）
// =========================================================
#include "mylib.h"   // 対応ヘッダを必ず先頭で（自己整合性チェック）
#include <stdio.h>

// static: このファイル外から呼べない private 関数
static int helper(int x) { return x * 2; }

// 公開関数の実装（mylib.h の宣言と一致させる）
Point point_new(int x, int y) { return (Point){x, y}; }
void  point_print(const Point *p) { printf("(%d,%d)", p->x, p->y); }

// =========================================================
// main.c（利用側）
// =========================================================
#include "mylib.h"   // "" で自分のヘッダ
#include <stdio.h>   // <> でシステムヘッダ

Point p = point_new(3, 4);  // ヘッダで宣言された型・関数が使える
point_print(&p);

// =========================================================
// コンパイル
// =========================================================
// gcc main.c mylib.c -o main          // 一括ビルド
// gcc -c mylib.c && gcc -c main.c     // 個別コンパイル（差分ビルド向け）
// gcc main.o mylib.o -o main -lm      // リンク（math使う場合 -lm）

// =========================================================
// インクルードガードのパターン
// =========================================================
// #ifndef MYLIB_H / #define MYLIB_H / ... / #endif  // 標準 C（推奨）
// #pragma once                                        // 非標準だが広く使われる
```
