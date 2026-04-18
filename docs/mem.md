# メモリユーティリティ — 仕様・設計

## 概要

`memcpy` / `memmove` / `memset` / `memcmp` を自前実装し、加えてプロジェクト全体で使う
assert 付き安全ラッパーと便利ユーティリティをまとめたモジュール。

**2 層構成:**

| 層 | 目的 | 関数プレフィックス |
|----|------|-----------------|
| 再実装層 | 標準関数の動作をアルゴリズムレベルで理解する | `my_` |
| ユーティリティ層 | Vec / Str / Arena 等で実際に使う安全版 | `mem_` |

**学習目標:**
- `memcpy` と `memmove` の違い（重なり判定）をアルゴリズムで理解する
- `unsigned char *` によるバイト操作の正しい書き方を身につける
- `restrict` 修飾子の意味を理解する

**依存関係:** Tier 1 の中で最もシンプル。Vector・String の実装より前に完成させる。

---

## ファイル構成

```
src/
  mem.h        // 宣言（再実装層 + ユーティリティ層）
  mem.c        // 実装
test/
  test_mem.c   // テスト
```

---

## API 仕様

### 再実装層（`my_` プレフィックス）

標準関数と同じシグネチャで自前実装する。
プロダクションコードでは使わず、理解確認のためだけに使う。

---

#### `my_memcpy`

```c
void *my_memcpy(void *restrict dst, const void *restrict src, size_t n);
```

| 項目 | 内容 |
|------|------|
| 概要 | `src` から `dst` へ `n` バイトコピーする。**重なり禁止** |
| `restrict` | `dst` と `src` が重ならないことをコンパイラに伝えるヒント |
| 戻り値 | `dst` |
| 計算量 | O(n) |

```c
// 実装イメージ
unsigned char       *d = (unsigned char *)dst;
const unsigned char *s = (const unsigned char *)src;
while (n--) *d++ = *s++;
return dst;
```

> **なぜ `unsigned char *` か:** `char` は符号付きか符号なしかが処理系依存。
> バイト操作は `unsigned char` が唯一 aliasing が定義されている型。

---

#### `my_memmove`

```c
void *my_memmove(void *dst, const void *src, size_t n);
```

| 項目 | 内容 |
|------|------|
| 概要 | `src` から `dst` へ `n` バイトコピーする。**重なりがあっても正しく動作する** |
| 戻り値 | `dst` |
| 計算量 | O(n) |

```c
// 実装イメージ
unsigned char       *d = (unsigned char *)dst;
const unsigned char *s = (const unsigned char *)src;

if (d <= s) {
    // dst が src より前 or 同じ → 前からコピーして安全
    for (size_t i = 0; i < n; i++) d[i] = s[i];
} else {
    // dst が src より後ろ → 前からコピーすると src の後半を上書きしてしまう
    // → 後ろからコピー
    for (size_t i = n; i > 0; i--) d[i - 1] = s[i - 1];
}
return dst;
```

重なりの可視化（`restrict` なしで重なりがある場合）:

```
ケース1: dst < src で重なる
  メモリ: [  dst  ][  src  ]
          [===dst+n===]
                 [===src+n===]
  → 前から後ろへコピーしても、読む前に上書きしない → 安全

ケース2: dst > src で重なる
  メモリ: [  src  ][  dst  ]
          [===src+n===]
                 [===dst+n===]
  → 前からコピーすると dst[0] に書いた時点で src の末尾を壊す → 後ろからコピーが必要
```

---

#### `my_memset`

```c
void *my_memset(void *dst, int c, size_t n);
```

| 項目 | 内容 |
|------|------|
| 概要 | `dst` の先頭 `n` バイトを `c` の下位 8 ビットで埋める |
| `c` | `int` で受け取るが `unsigned char` に変換して使う（標準に合わせた歴史的理由） |
| 戻り値 | `dst` |
| 計算量 | O(n) |

```c
unsigned char *d = (unsigned char *)dst;
unsigned char  v = (unsigned char)c;
while (n--) *d++ = v;
return dst;
```

---

#### `my_memcmp`

```c
int my_memcmp(const void *a, const void *b, size_t n);
```

| 項目 | 内容 |
|------|------|
| 概要 | 先頭 `n` バイトを辞書順で比較する |
| 戻り値 | 負: `a < b` / `0`: 等しい / 正: `a > b` |
| 計算量 | O(n)（最初の不一致で早期終了） |

```c
const unsigned char *pa = (const unsigned char *)a;
const unsigned char *pb = (const unsigned char *)b;
while (n--) {
    if (*pa != *pb) return (int)*pa - (int)*pb;
    pa++; pb++;
}
return 0;
```

---

### ユーティリティ層（`mem_` プレフィックス）

Vec / Str / Arena 等で実際に使う関数。`assert` による事前条件チェック付き。

---

#### `mem_copy`

```c
void mem_copy(void *dst, const void *src, size_t n);
```

| 項目 | 内容 |
|------|------|
| 概要 | `assert` 付きの `memcpy` ラッパー。重なり禁止 |
| 事前条件 | `dst != NULL`, `src != NULL`（assert で検証） |
| 計算量 | O(n) |

```c
assert(dst != NULL);
assert(src != NULL);
memcpy(dst, src, n);
```

---

#### `mem_move`

```c
void mem_move(void *dst, const void *src, size_t n);
```

| 項目 | 内容 |
|------|------|
| 概要 | `assert` 付きの `memmove` ラッパー。重なりあっても安全 |
| 事前条件 | `dst != NULL`, `src != NULL` |
| 計算量 | O(n) |

---

#### `mem_zero`

```c
void mem_zero(void *dst, size_t n);
```

| 項目 | 内容 |
|------|------|
| 概要 | `n` バイトをゼロクリアする。`memset(dst, 0, n)` の短縮形 |
| 事前条件 | `dst != NULL` |
| 計算量 | O(n) |

> Vec や Str の初期化・クリアで頻繁に使う。

---

#### `mem_eq`

```c
bool mem_eq(const void *a, const void *b, size_t n);
```

| 項目 | 内容 |
|------|------|
| 概要 | `n` バイトが等しければ `true` を返す |
| 事前条件 | `a != NULL`, `b != NULL` |
| 計算量 | O(n) |

```c
return memcmp(a, b, n) == 0;
```

---

#### `mem_swap`

```c
void mem_swap(void *a, void *b, size_t n);
```

| 項目 | 内容 |
|------|------|
| 概要 | `a` と `b` が指す `n` バイトを入れ替える |
| 事前条件 | `a != NULL`, `b != NULL`, `a` と `b` は重ならない |
| 計算量 | O(n) |

```c
// スタック上の固定バッファでチャンク単位に交換
#define MEM_SWAP_BUF 256
unsigned char tmp[MEM_SWAP_BUF];
unsigned char *pa = (unsigned char *)a;
unsigned char *pb = (unsigned char *)b;
while (n > 0) {
    size_t chunk = n < MEM_SWAP_BUF ? n : MEM_SWAP_BUF;
    memcpy(tmp, pa,  chunk);
    memcpy(pa,  pb,  chunk);
    memcpy(pb,  tmp, chunk);
    pa += chunk; pb += chunk; n -= chunk;
}
```

> `vec_swap_remove` や HashMap のバケット整理などで使う。

---

#### `mem_dup`

```c
void *mem_dup(const void *src, size_t n);
```

| 項目 | 内容 |
|------|------|
| 概要 | `src` の `n` バイトをヒープにコピーして返す |
| 戻り値 | 成功: 新しく確保したポインタ / 失敗: `NULL` |
| 計算量 | O(n) |

```c
void *dst = malloc(n);
if (!dst) return NULL;
memcpy(dst, src, n);
return dst;
```

> **呼び出し側が `free()` する責任を持つ。**
> Str の `str_init_from` 等の内部で使う。

---

## エラー処理方針

| エラーの種類 | 扱い方 |
|------------|--------|
| `NULL` ポインタ渡し | `assert` で即死（プログラミングエラー） |
| `n == 0` | 何もしないで正常終了（標準に合わせる） |
| `mem_dup` の OOM | `NULL` を返す |

> Tier 1 では `<assert.h>` の `assert()` を使う。
> 独自 assert マクロ（`docs/assert.md`）完成後に差し替える。

---

## `memcpy` vs `memmove` の使い分け

```
重なりが絶対にない場合    → memcpy  / mem_copy  （高速、最適化しやすい）
重なりがある可能性がある  → memmove / mem_move  （安全）
```

**実装中の判断基準:**

| 状況 | どちらを使うか |
|------|--------------|
| 異なるバッファ間のコピー（src と dst が別の malloc 領域） | `mem_copy` |
| 同じ配列内での要素シフト（`vec_insert` / `vec_remove` 等） | `mem_move` |
| 構造体メンバのコピー | `mem_copy` |
| `Str` 内での `str_insert` のシフト | `mem_move` |

---

## テストチェックリスト

### `my_memcpy`
- [ ] 通常のバイト列コピーが正しい
- [ ] `n == 0` で何もしない
- [ ] 戻り値が `dst` と等しい

### `my_memmove`
- [ ] 重なりなし: 通常のコピーと同じ結果
- [ ] `dst < src` で重なる: 正しくコピーされる
- [ ] `dst > src` で重なる: 正しくコピーされる（`memcpy` では失敗するケース）
- [ ] `dst == src`: 変化なし
- [ ] `n == 0` で何もしない

### `my_memset`
- [ ] `0x00` で埋められる
- [ ] `0xFF` で埋められる
- [ ] 一部だけ埋められる（`n < sizeof(buf)` の場合）
- [ ] `c` の下位 8 ビットのみ使われる（`c = 0x1FF` → `0xFF` で埋められる）

### `my_memcmp`
- [ ] 等しい場合は `0`
- [ ] 前の方が小さい場合は負
- [ ] 前の方が大きい場合は正
- [ ] `n == 0` は `0` を返す
- [ ] 最初の不一致バイトで判定される

### ユーティリティ層
- [ ] `mem_zero` 後にすべてのバイトが `0x00`
- [ ] `mem_eq` で等しいバイト列が `true`
- [ ] `mem_eq` で異なるバイト列が `false`
- [ ] `mem_swap` で 2 つの領域が入れ替わる
- [ ] `mem_swap` で `MEM_SWAP_BUF` より大きいサイズが正しく入れ替わる
- [ ] `mem_dup` でコピーした内容が同じ
- [ ] `mem_dup` の戻り値を `free()` できる

---

## 実装の壁と対策

### 壁 1: `char *` vs `unsigned char *`

バイト操作に `char *` を使うと、符号付き `char` の場合に比較や算術が意図しない動作をすることがある。

```c
// NG: signed char で比較すると 0x80 以上が負の値になる
char *d = (char *)dst;
return (int)*d - (int)*s;  // 0xFF - 0x00 が -1 になりうる

// OK: unsigned char を使う
const unsigned char *pa = (const unsigned char *)a;
const unsigned char *pb = (const unsigned char *)b;
return (int)*pa - (int)*pb;  // 必ず 0〜255 の差
```

### 壁 2: `memmove` の重なり判定で向きを間違える

「重なりがある → 後ろからコピー」は**間違い**。正しい判断は dst と src の位置関係による。

```
dst > src のとき後ろからコピー
dst <= src のとき前からコピー（重なりがあっても安全）
```

```c
// よくある間違い（重なりがあるかどうかで判断）
if (dst_overlaps_src) {
    // ← NG: dst < src で重なっている場合は前からコピーで OK

// 正しい判断（dst と src の位置で判断）
if ((unsigned char *)dst > (unsigned char *)src) {
    // 後ろからコピー
```

### 壁 3: ポインタ比較の UB

異なるオブジェクト間のポインタ比較（`<`, `>`）は厳密には UB。
`memmove` の実装では `uintptr_t` にキャストするか、標準ライブラリに任せることが多い。
自前実装では `unsigned char *` へのキャストで実用上問題ない。

```c
// 実用的な実装
if ((unsigned char *)dst > (unsigned char *)src) { ... }
```

### 壁 4: `mem_swap` でのバッファサイズ

要素が `MEM_SWAP_BUF`（256 バイト）より大きい場合にチャンク分割が必要。
ループでチャンクごとに処理することで任意サイズに対応する。

---

## 将来の拡張（Tier 2: Arena Allocator 完成後）

`mem_dup` はArena Allocator 対応版を追加する:

```c
// Arena から確保するバージョン（Tier 2 以降）
void *mem_dup_arena(const void *src, size_t n, Allocator *alloc);
```

これにより JSON パーサの文字列コピー等を Arena 上に確保できるようになる。

---

## Zig 移行時の対応

| Cでの苦しみ | Zigで解決される点 |
|------------|-----------------|
| `memcpy` vs `memmove` の使い分けを手動で判断 | `@memcpy` は重なりを静的に検出 |
| `unsigned char *` キャストの煩雑さ | スライス `[]u8` がバイト操作の標準手段 |
| `mem_dup` の解放責任が曖昧 | Allocator インターフェースで所有権が明確 |
| `restrict` の効果が実装依存 | Zig のポインタ属性（`allowzero`, `align`）で明示的に制御 |
