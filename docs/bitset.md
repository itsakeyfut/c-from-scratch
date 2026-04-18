# Bitset / Bitmap — 仕様・設計

## 概要

任意のビット数を動的に確保し、1ビット単位で読み書きするモジュール。
`uint64_t` をワード単位として、ビット演算の実用的な使い方を体験する。

**学習目標:**
- `word_index = bit / 64` / `bit_mask = 1ULL << (bit % 64)` によるビット位置計算
- 端数ビット（`n_bits % 64 != 0` のとき最後のワードに余分なビットが存在する）の扱い
- popcount（セットビット数）の実装（`__builtin_popcountll` vs 手動）
- 集合演算（AND / OR / XOR / NOT）を word 単位で一括処理する効率性

**依存関係:**
- `mem.h` のみ（`mem_zero` を使う）
- `<stdint.h>` / `<stddef.h>` / `<stdbool.h>`

**Tier 2 以降の予定:** Arena Allocator 完成後にアロケータパラメータを追加してリファクタリングする。

---

## ファイル構成

```
src/
  bitset.h   // 宣言
  bitset.c   // 実装
test/
  test_bitset.c
```

---

## データ構造

```c
typedef struct {
    uint64_t *words;    // ヒープ確保されたワード配列
    size_t    n_bits;   // 論理ビット数（ユーザーが指定した値）
    size_t    n_words;  // words の要素数 = (n_bits + 63) / 64
} Bitset;
```

**メモリレイアウト（n_bits = 70 の例）:**

```
n_words = 2  （70 / 64 = 1 余り 6 → 2ワード必要）

words[0]: bits  0 〜 63   （64ビット全部使用）
words[1]: bits 64 〜 69   （下位6ビットのみ有効、上位58ビットは常に0）
          [63........6][5....0]
           ←常に0→    ←有効→
```

**端数ビットマスク:**

```c
// 最後のワードで有効なビット数
size_t tail_bits = n_bits % 64;
// tail_bits == 0 のとき全64ビット有効（マスク不要）
// tail_bits != 0 のとき有効マスク: (1ULL << tail_bits) - 1
uint64_t tail_mask = tail_bits ? (1ULL << tail_bits) - 1 : ~0ULL;
```

---

## 定数・マクロ

```c
#define BS_WORD_BITS    64ULL
#define BS_WORD_IDX(i)  ((i) / BS_WORD_BITS)
#define BS_BIT_MASK(i)  (1ULL << ((i) % BS_WORD_BITS))
#define BS_NWORDS(n)    (((n) + BS_WORD_BITS - 1) / BS_WORD_BITS)
```

---

## API 仕様

### `bitset_init`

```c
int bitset_init(Bitset *b, size_t n_bits);
```

| 項目 | 内容 |
|------|------|
| 概要 | `n_bits` ビットの Bitset をゼロ初期化して作る |
| 事前条件 | `b != NULL`, `n_bits > 0` |
| 戻り値 | 成功: `0` / OOM: `-1` |
| 計算量 | O(n_words) |

```c
assert(b != NULL && n_bits > 0);
b->n_bits  = n_bits;
b->n_words = BS_NWORDS(n_bits);
b->words   = calloc(b->n_words, sizeof(uint64_t));
if (!b->words) return -1;
return 0;
```

---

### `bitset_free`

```c
void bitset_free(Bitset *b);
```

```c
assert(b != NULL);
free(b->words);
b->words = NULL;
b->n_bits = b->n_words = 0;
```

---

### `bitset_set`

```c
void bitset_set(Bitset *b, size_t bit);
```

| 項目 | 内容 |
|------|------|
| 概要 | ビット `bit` を 1 にする |
| 事前条件 | `b != NULL`, `bit < b->n_bits` |
| 計算量 | O(1) |

```c
assert(b != NULL && bit < b->n_bits);
b->words[BS_WORD_IDX(bit)] |= BS_BIT_MASK(bit);
```

---

### `bitset_clear`

```c
void bitset_clear(Bitset *b, size_t bit);
```

| 項目 | 内容 |
|------|------|
| 概要 | ビット `bit` を 0 にする |
| 計算量 | O(1) |

```c
assert(b != NULL && bit < b->n_bits);
b->words[BS_WORD_IDX(bit)] &= ~BS_BIT_MASK(bit);
```

---

### `bitset_toggle`

```c
void bitset_toggle(Bitset *b, size_t bit);
```

| 項目 | 内容 |
|------|------|
| 概要 | ビット `bit` を反転する |
| 計算量 | O(1) |

```c
assert(b != NULL && bit < b->n_bits);
b->words[BS_WORD_IDX(bit)] ^= BS_BIT_MASK(bit);
```

---

### `bitset_test`

```c
bool bitset_test(const Bitset *b, size_t bit);
```

| 項目 | 内容 |
|------|------|
| 概要 | ビット `bit` が 1 なら `true` |
| 計算量 | O(1) |

```c
assert(b != NULL && bit < b->n_bits);
return (b->words[BS_WORD_IDX(bit)] & BS_BIT_MASK(bit)) != 0;
```

---

### `bitset_set_all`

```c
void bitset_set_all(Bitset *b);
```

| 項目 | 内容 |
|------|------|
| 概要 | 全ビットを 1 にする |
| 計算量 | O(n_words) |

```c
for (size_t i = 0; i < b->n_words; i++) b->words[i] = ~0ULL;
// 端数ビットを 0 に戻す（カウント等が壊れないように）
size_t tail = b->n_bits % 64;
if (tail) b->words[b->n_words - 1] = (1ULL << tail) - 1;
```

> **端数ビットの後始末が必要。** 「端数ビット」の節を参照。

---

### `bitset_clear_all`

```c
void bitset_clear_all(Bitset *b);
```

```c
mem_zero(b->words, b->n_words * sizeof(uint64_t));
```

---

### `bitset_count`

```c
size_t bitset_count(const Bitset *b);
```

| 項目 | 内容 |
|------|------|
| 概要 | セットされているビットの総数（popcount）を返す |
| 計算量 | O(n_words) |

```c
size_t cnt = 0;
for (size_t i = 0; i < b->n_words; i++) {
    cnt += (size_t)__builtin_popcountll(b->words[i]);
}
return cnt;
```

> `__builtin_popcountll` は GCC / Clang で利用可能。ポータブルにするなら
> `bitset_popcount64` を手動実装（後述）で差し替える。

---

### `bitset_find_first`

```c
size_t bitset_find_first(const Bitset *b);
```

| 項目 | 内容 |
|------|------|
| 概要 | 最初にセットされているビット番号を返す |
| 戻り値 | 見つかった場合はそのインデックス / 1ビットも立っていない場合は `SIZE_MAX` |
| 計算量 | O(n_words) |

```c
for (size_t i = 0; i < b->n_words; i++) {
    if (b->words[i] == 0) continue;
    // ワード内の最下位セットビット位置: __builtin_ctzll
    size_t bit = i * 64 + (size_t)__builtin_ctzll(b->words[i]);
    return bit < b->n_bits ? bit : SIZE_MAX;
}
return SIZE_MAX;
```

---

### 集合演算

```c
void bitset_and(Bitset *dst, const Bitset *a, const Bitset *b);
void bitset_or (Bitset *dst, const Bitset *a, const Bitset *b);
void bitset_xor(Bitset *dst, const Bitset *a, const Bitset *b);
void bitset_not(Bitset *b);
```

| 関数 | 概要 | 事前条件 |
|------|------|----------|
| `bitset_and` | `dst[i] = a[i] & b[i]` | 全て同じ `n_bits` |
| `bitset_or`  | `dst[i] = a[i] \| b[i]` | 全て同じ `n_bits` |
| `bitset_xor` | `dst[i] = a[i] ^ b[i]` | 全て同じ `n_bits` |
| `bitset_not` | `b[i] = ~b[i]`（in-place、端数マスク後処理） | — |

```c
// bitset_and の実装イメージ
assert(dst->n_bits == a->n_bits && a->n_bits == b->n_bits);
for (size_t i = 0; i < dst->n_words; i++)
    dst->words[i] = a->words[i] & b->words[i];

// bitset_not の実装イメージ（端数ビットを後でクリアする）
for (size_t i = 0; i < b->n_words; i++) b->words[i] = ~b->words[i];
size_t tail = b->n_bits % 64;
if (tail) b->words[b->n_words - 1] &= (1ULL << tail) - 1;
```

---

## 内部設計

### 手動 popcount（`__builtin_popcountll` が使えない場合）

```c
static uint64_t popcount64(uint64_t x) {
    x -= (x >> 1) & 0x5555555555555555ULL;
    x  = (x & 0x3333333333333333ULL) + ((x >> 2) & 0x3333333333333333ULL);
    x  = (x + (x >> 4)) & 0x0f0f0f0f0f0f0f0fULL;
    return (x * 0x0101010101010101ULL) >> 56;
}
```

ハミング重みを分割統治で計算する古典的アルゴリズム（Kernighan より高速）。

---

## エラー処理方針

| エラーの種類 | 扱い方 |
|------------|--------|
| `NULL` ポインタ渡し | `assert` で即死 |
| `bit >= n_bits` の範囲外アクセス | `assert` で即死（プログラミングエラー） |
| `bitset_init` の OOM | `-1` を返す |
| 集合演算でサイズ不一致 | `assert` で即死 |

> Tier 1 では `<assert.h>` の `assert()` を使う。
> 独自 assert マクロ（`docs/assert.md`）完成後に差し替える。

---

## 端数ビットの注意

`n_bits % 64 != 0` のとき、最後のワードに「有効でないビット」が存在する。
これを放置すると `bitset_count` や `bitset_not` が狂う。

```
n_bits = 70  →  words[1] の bit 6〜63 は常に 0 でなければならない
```

**後処理が必要な操作:**
- `bitset_set_all`: 全ワードを `~0ULL` にした後、最後のワードをマスクする
- `bitset_not`: 同様

**後処理が不要な操作:**
- `bitset_set` / `bitset_clear` / `bitset_toggle`: 1ビット操作なので範囲外に触れない
- `bitset_and` / `bitset_or` / `bitset_xor`: 端数ビットは入力が 0 のまま演算結果も 0

---

## テストチェックリスト

### 基本操作
- [ ] `bitset_set(b, 0)` 後 `bitset_test(b, 0)` が `true`
- [ ] `bitset_set(b, n_bits-1)` が最後のビットを正しくセットする
- [ ] `bitset_clear(b, bit)` 後 `bitset_test` が `false`
- [ ] `bitset_toggle` を2回呼ぶと元の値に戻る
- [ ] `bitset_test` はセットしていないビットで `false`

### 端数ビット（重要）
- [ ] `n_bits = 65`（端数1ビット）で `bitset_set_all` した後 `bitset_count` が `65`
- [ ] `n_bits = 65` で `bitset_not(全0)` した後 `bitset_count` が `65`（余分なビットが入っていない）
- [ ] `n_bits = 64`（端数なし）で `bitset_set_all` した後 `bitset_count` が `64`
- [ ] `n_bits = 128`（2ワード、端数なし）で問題なく動作する

### `bitset_count`
- [ ] 全0で `0`
- [ ] 全1で `n_bits`
- [ ] 特定のビットだけセットして正しい数を返す

### `bitset_find_first`
- [ ] 全0で `SIZE_MAX`
- [ ] bit 0 だけセットして `0`
- [ ] bit 63 だけセットして `63`
- [ ] bit 64 だけセットして `64`（2ワード目の先頭）
- [ ] 複数セットされている場合に最小のインデックスを返す

### 集合演算
- [ ] `AND`: `{0,1}` AND `{1,2}` = `{1}`
- [ ] `OR`:  `{0,1}` OR  `{1,2}` = `{0,1,2}`
- [ ] `XOR`: `{0,1}` XOR `{1,2}` = `{0,2}`
- [ ] `NOT`: 全0の NOT が全1（`bitset_count == n_bits`）になる（端数ビットが入らない）
- [ ] `NOT`: 全1の NOT が全0になる

### メモリ
- [ ] `bitset_free` 後のメモリリークがない（ASan）
- [ ] OOM 時に `bitset_init` が `-1` を返す

---

## 実装の壁と対策

### 壁 1: `1 << bit` で UB（32ビットシフト）

`bit = 40` のとき `1 << 40` は `int`（32ビット）のオーバーフローで UB。

```c
// NG
b->words[i] |= 1 << (bit % 64);   // bit >= 32 でオーバーフロー

// OK: 1ULL を使う
b->words[i] |= 1ULL << (bit % 64);
```

---

### 壁 2: `bitset_set_all` の端数ビット後始末忘れ

```c
// NG: 端数ビットが 1 になって count が狂う
for (size_t i = 0; i < b->n_words; i++) b->words[i] = ~0ULL;
// n_bits=70 のとき words[1] の bit 6〜63 も 1 になる → count = 128 になる

// OK: 端数を戻す
size_t tail = b->n_bits % 64;
if (tail) b->words[b->n_words - 1] = (1ULL << tail) - 1;
```

---

### 壁 3: `n_bits % 64 == 0` のときのマスク計算

`1ULL << 64` は UB（64ビットシフトは定義されない）。

```c
// NG
uint64_t mask = (1ULL << (n_bits % 64)) - 1;  // n_bits % 64 == 0 のとき 1ULL << 64 = UB

// OK: 端数がゼロかどうかで分岐
size_t tail = n_bits % 64;
uint64_t tail_mask = tail ? (1ULL << tail) - 1 : ~0ULL;
```

---

### 壁 4: `bitset_find_first` で `__builtin_ctzll(0)` を呼ぶ

`ctzll(0)` の結果は未定義。必ずワードが非ゼロのときだけ呼ぶ。

```c
// NG
for (...) {
    size_t bit = i * 64 + __builtin_ctzll(b->words[i]);  // words[i]==0 のとき UB
```

```c
// OK: ゼロワードをスキップ
if (b->words[i] == 0) continue;
size_t bit = i * 64 + (size_t)__builtin_ctzll(b->words[i]);
```

---

## 将来の拡張（Tier 2: Arena Allocator 完成後）

```c
// Arena から確保するバージョン
int bitset_init_arena(Bitset *b, size_t n_bits, Allocator *alloc);
```

### HashMap との連携

HashMap の削除フラグ（tombstone）を Bitset で管理すると、
キャッシュ効率が向上する（ポインタの NULL チェックよりビット操作が速い）。

---

## Zig 移行時の対応

| Cでの苦しみ | Zigで解決される点 |
|------------|-----------------|
| `1ULL << bit` の型を手動で管理 | `@as(u64, 1) << @intCast(bit)` で型安全 |
| 端数ビットを手動でマスク | `std.bit_set.DynamicBitSet` が内部で管理 |
| `__builtin_popcountll` が非標準 | `@popCount` が組み込み |
| `__builtin_ctzll` が非標準 | `@ctz` が組み込み |
| `calloc` + `free` の手動管理 | `std.bit_set.DynamicBitSet.init(allocator, n)` |
