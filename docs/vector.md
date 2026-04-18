# Vector (動的配列) — 仕様・設計

## 概要

要素サイズを実行時に指定する汎用動的配列。
`void *` + `elem_size` による型消去で任意の型を格納できる。

**学習目標:**
- `realloc` の正しい使い方とポインタ無効化を体験する
- `void *` ポインタ演算（キャストの必要性）を理解する
- 償却 O(1) の成長戦略を実装する

**Tier 2 以降の予定:** Arena Allocator 完成後にアロケータパラメータを追加してリファクタリングする。

---

## ファイル構成

```
src/
  vec.h        // 型定義・関数宣言・インラインマクロ
  vec.c        // 実装
  main.c       // デモ
```

---

## データ構造

```c
typedef struct {
    void   *data;       // 要素配列へのポインタ。未確保時は NULL
    size_t  len;        // 現在格納している要素数
    size_t  cap;        // 確保済みの容量（要素数単位）
    size_t  elem_size;  // 1 要素のバイトサイズ
} Vec;
```

**メモリレイアウト:**

```
data → [ elem0 ][ elem1 ][ elem2 ][      ][      ]
        |<------ len * elem_size ------>|
        |<------------ cap * elem_size ---------->|
```

---

## 定数

```c
#define VEC_INITIAL_CAP 8   // 最初の grow 時に確保する要素数
```

---

## API 仕様

### `vec_init`

```c
void vec_init(Vec *v, size_t elem_size);
```

| 項目 | 内容 |
|------|------|
| 概要 | Vec を初期化する。メモリ確保は行わない（遅延確保） |
| `elem_size` | 1 要素のバイトサイズ。0 を渡してはならない |
| 戻り値 | なし |
| 計算量 | O(1) |

```c
// 実装イメージ
v->data      = NULL;
v->len       = 0;
v->cap       = 0;
v->elem_size = elem_size;
```

---

### `vec_free`

```c
void vec_free(Vec *v);
```

| 項目 | 内容 |
|------|------|
| 概要 | 内部バッファを解放し Vec をゼロ初期化する |
| 戻り値 | なし |
| 事後条件 | `v->data == NULL`, `v->len == 0`, `v->cap == 0` |
| 計算量 | O(1) |

> **注意:** 要素が内部にポインタを持つ場合、呼び出し側が事前に各要素を処理すること。
> Vec 自身は要素のデストラクタを呼ばない。

---

### `vec_push`

```c
int vec_push(Vec *v, const void *elem);
```

| 項目 | 内容 |
|------|------|
| 概要 | 末尾に要素を 1 つ追加する |
| `elem` | コピー元。`elem_size` バイトが読まれる |
| 戻り値 | 成功: `0` / メモリ確保失敗: `-1` |
| 計算量 | 償却 O(1) |

```
cap == len のとき grow() を呼ぶ
grow() が失敗したら v を変更せず -1 を返す
コピー先: (char *)v->data + v->len * v->elem_size
memcpy でコピーしてから len を +1 する
```

> **警告:** grow が発生すると、push 前に `vec_get` で取得済みのポインタはすべて無効になる。

---

### `vec_pop`

```c
int vec_pop(Vec *v, void *out);
```

| 項目 | 内容 |
|------|------|
| 概要 | 末尾要素を取り出す |
| `out` | 取り出した要素のコピー先。`NULL` の場合は値を捨てて削除のみ |
| 戻り値 | 成功: `0` / 空: `-1` |
| 計算量 | O(1) |

- `cap` は変更しない（メモリ解放しない）

---

### `vec_get`

```c
void *vec_get(const Vec *v, size_t i);
```

| 項目 | 内容 |
|------|------|
| 概要 | インデックス `i` の要素へのポインタを返す |
| 戻り値 | 要素へのポインタ / `i >= len` の場合 `NULL` |
| 計算量 | O(1) |

```c
// 実装イメージ
if (i >= v->len) return NULL;
return (char *)v->data + i * v->elem_size;
```

> **警告:** 返したポインタは `vec_push` / `vec_insert` / `vec_reserve` 呼び出し後に無効になる可能性がある。

---

### `vec_set`

```c
int vec_set(Vec *v, size_t i, const void *elem);
```

| 項目 | 内容 |
|------|------|
| 概要 | インデックス `i` の要素を上書きする |
| 戻り値 | 成功: `0` / `i >= len`: `-1` |
| 計算量 | O(1) |

---

### `vec_reserve`

```c
int vec_reserve(Vec *v, size_t new_cap);
```

| 項目 | 内容 |
|------|------|
| 概要 | 少なくとも `new_cap` 要素分の容量を確保する |
| 戻り値 | 成功: `0` / 失敗: `-1` |
| 計算量 | O(n)（realloc によるコピー） |

- `new_cap <= v->cap` の場合は何もしない
- `len` は変更しない

---

### `vec_clear`

```c
void vec_clear(Vec *v);
```

| 項目 | 内容 |
|------|------|
| 概要 | `len` を 0 にリセットする。メモリは解放しない |
| 計算量 | O(1) |

---

### `vec_len`

```c
size_t vec_len(const Vec *v);
```

| 項目 | 内容 |
|------|------|
| 概要 | 現在の要素数を返す |
| 戻り値 | `v->len` |
| 計算量 | O(1) |

---

### `vec_insert`

```c
int vec_insert(Vec *v, size_t i, const void *elem);
```

| 項目 | 内容 |
|------|------|
| 概要 | インデックス `i` に要素を挿入する。`i` 以降の要素は後ろにシフト |
| 戻り値 | 成功: `0` / `i > len`: `-1` / OOM: `-1` |
| 計算量 | O(n) |

```
必要なら grow()
memmove で [i .. len-1] を [i+1 .. len] にずらす
インデックス i にコピー
len を +1
```

---

### `vec_remove`

```c
int vec_remove(Vec *v, size_t i);
```

| 項目 | 内容 |
|------|------|
| 概要 | インデックス `i` の要素を削除する。順序を保つ（後の要素を前にシフト） |
| 戻り値 | 成功: `0` / `i >= len`: `-1` |
| 計算量 | O(n) |

```
memmove で [i+1 .. len-1] を [i .. len-2] にずらす
len を -1
```

---

### `vec_swap_remove`

```c
int vec_swap_remove(Vec *v, size_t i);
```

| 項目 | 内容 |
|------|------|
| 概要 | インデックス `i` の要素を末尾要素と入れ替えてから削除する。順序を保たない代わりに O(1) |
| 戻り値 | 成功: `0` / `i >= len`: `-1` |
| 計算量 | O(1) |

> `i == len - 1`（末尾）の場合は単純に `len--` するだけでよい。

---

### `vec_sort`

```c
void vec_sort(Vec *v, int (*cmp)(const void *, const void *));
```

| 項目 | 内容 |
|------|------|
| 概要 | 比較関数を使ってソートする |
| `cmp` | `qsort` と同じシグネチャ。負: a < b / 0: a == b / 正: a > b |
| 計算量 | O(n log n)（内部で `qsort` を使う） |

---

## 内部設計

### `vec_grow`（非公開）

```c
static int vec_grow(Vec *v);
```

```
new_cap = (v->cap == 0) ? VEC_INITIAL_CAP : v->cap * 2
tmp = realloc(v->data, new_cap * v->elem_size)
if tmp == NULL → return -1  （v->data は変更しない）
v->data = tmp
v->cap  = new_cap
return 0
```

### 成長戦略

| 現在の cap | grow 後の cap |
|-----------|--------------|
| 0         | 8            |
| 8         | 16           |
| 16        | 32           |
| n         | n * 2        |

2 倍成長を選ぶ理由: シンプルで push の償却コストが O(1) に収まる。
1.5 倍はメモリ効率が良いが複雑になるため Tier 1 では採用しない。

---

## エラー処理方針

| エラーの種類 | 扱い方 |
|------------|--------|
| プログラミングエラー（`v == NULL`, `elem_size == 0`, `i >= len` の書き込み等） | `assert` で即死 |
| 実行時エラー（OOM） | 戻り値 `-1` で通知。Vec の状態は変更しない |

> Tier 1 では `<assert.h>` の `assert()` を使う。
> 独自 assert マクロ（`assert.md`）完成後に差し替える。

---

## ポインタ無効化

`vec_get` が返すポインタは内部バッファへの直接参照。
以下の操作で **grow が発生した場合** に無効になる:

- `vec_push`
- `vec_insert`
- `vec_reserve`

```c
// バグの例
int *p = vec_get(&v, 0);  // 内部バッファを指す
vec_push(&v, &x);         // grow → realloc → 旧バッファが解放される
*p = 10;                  // Use-after-free!

// 安全な使い方: push の後にポインタを取得する
vec_push(&v, &x);
int *p = vec_get(&v, 0);  // 新しいバッファの先頭を指す
```

---

## テストチェックリスト

### 基本動作
- [ ] `vec_push` → `vec_get` で同じ値が取れる
- [ ] `vec_pop` で末尾要素が取り出せる
- [ ] 空の Vec に `vec_pop` すると `-1` が返る
- [ ] `vec_clear` 後に `len == 0` かつ `cap > 0`

### 容量・成長
- [ ] `VEC_INITIAL_CAP + 1` 個 push した後も正しく動作する（grow が起きる）
- [ ] cap が 0 → 8 → 16 → 32 と推移する
- [ ] `vec_reserve(v, 100)` の後に 100 回 push しても grow が起きない
- [ ] `vec_free` 後に `data == NULL`

### 境界条件
- [ ] `vec_get(v, len)` が `NULL` を返す
- [ ] `vec_set(v, len, ...)` が `-1` を返す
- [ ] `vec_insert(v, 0, ...)` で先頭挿入が機能する
- [ ] `vec_remove(v, 0)` で先頭削除後に順序が保たれる
- [ ] `vec_swap_remove` で末尾要素が削除対象の位置に移動する

### さまざまな型
- [ ] `sizeof(int)` で動作する
- [ ] `sizeof(double)` で動作する
- [ ] 構造体（`sizeof(struct { int x; int y; })`）で動作する

### OOM 対策
- [ ] grow 失敗時に元のデータが保持されている（`realloc` の一時変数パターンが正しい）

---

## 実装の壁と対策

### 壁 1: `realloc` の罠

```c
// NG: 失敗時に NULL が代入されて元のポインタが失われる
v->data = realloc(v->data, new_size);

// OK: 必ず一時変数を使う
void *tmp = realloc(v->data, new_size);
if (!tmp) return -1;
v->data = tmp;
```

### 壁 2: `void *` のポインタ演算

C では `void *` に対する加算は未定義動作（UB）。
内部では必ず `char *` にキャストしてから演算する。

```c
// NG: GCC 拡張では動くが UB
void *elem = v->data + i * v->elem_size;

// OK
void *elem = (char *)v->data + i * v->elem_size;
```

### 壁 3: `memmove` vs `memcpy`

`vec_insert` / `vec_remove` では要素をシフトする際にコピー元とコピー先が重なる。
このケースでは `memcpy` ではなく `memmove` を使うこと。

```c
// insert でのシフト（重なりあり → memmove）
memmove(
    (char *)v->data + (i + 1) * v->elem_size,  // dst
    (char *)v->data + i       * v->elem_size,  // src
    (v->len - i) * v->elem_size                // size
);
```

### 壁 4: ポインタ無効化に気づかない

`-fsanitize=address`（AddressSanitizer）を使うと Use-after-free を実行時に検出できる。

```makefile
CFLAGS += -fsanitize=address,undefined
```

---

## 将来の拡張（Tier 2: Arena Allocator 完成後）

```c
// 変更後のシグネチャ
void vec_init(Vec *v, size_t elem_size, Allocator *alloc);
```

変更点:
- `Vec` に `Allocator *alloc` フィールドを追加
- 内部の `malloc` / `realloc` / `free` をアロケータ経由に置き換え
- `stdlib_allocator` をデフォルトとして用意すれば既存コードへの影響を最小化できる

この変更により、Arena 上で Vec を使えるようになる（JSON の AST 構築等で威力を発揮）。
