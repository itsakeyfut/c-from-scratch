# String (可変長文字列) — 仕様・設計

## 概要

ヒープ上に確保した可変長文字列。
常に null 終端を保証しつつ `len` でバイト数を管理するハイブリッド設計。
バイト操作をメインとし、UTF-8 のバリデーション・コードポイント操作をサブセットとして提供する。

**学習目標:**
- `null 終端 vs length 管理` の設計トレードオフを理解する
- UTF-8 のバイト構造と「バイト数 ≠ 文字数」の壁を体験する
- Vec と共通の grow/realloc パターンを再適用する

**依存関係:** Vector (#1) の実装後に取り組む。grow の仕組みは Str でも同じ。
**Tier 2 以降の予定:** Arena Allocator 完成後にアロケータパラメータを追加してリファクタリングする。

---

## ファイル構成

```
src/
  str.h        // 型定義・関数宣言・マクロ
  str.c        // 実装
test/
  test_str.c   // テスト
```

---

## データ構造

```c
typedef struct {
    char   *data;  // data[len] == '\0' を常に保証。未確保時は NULL
    size_t  len;   // '\0' を除いたバイト数
    size_t  cap;   // data に確保済みのバイト数（'\0' の 1 バイトを含む）
} Str;
```

`cap` の定義:

```
cap = 実際に格納できる文字のバイト数 + 1 ('\0' の分)
cap == 0  → data は NULL（未確保）
cap == 1  → '\0' のみ格納（空文字列、len == 0）
cap == 16 → 最大 15 バイトの文字列を格納可能
```

**メモリレイアウト:**

```
data → [ 'H' ][ 'e' ][ 'l' ][ 'l' ][ 'o' ][ '\0' ][     ][     ]
        |<------- len = 5 ------->|               |
        |<------------ cap = 8 ----------------------->|
```

---

## 定数

```c
#define STR_INITIAL_CAP 16  // 最初の grow 時に確保するバイト数（'\0' 込み）
```

---

## API 仕様

### ライフサイクル

#### `str_init`

```c
void str_init(Str *s);
```

| 項目 | 内容 |
|------|------|
| 概要 | 空の文字列で初期化する。メモリ確保は行わない（遅延確保） |
| 戻り値 | なし |
| 計算量 | O(1) |

```c
s->data = NULL;
s->len  = 0;
s->cap  = 0;
```

---

#### `str_init_from`

```c
int str_init_from(Str *s, const char *cstr);
```

| 項目 | 内容 |
|------|------|
| 概要 | C 文字列を内容として初期化する |
| `cstr` | NULL であってはならない |
| 戻り値 | 成功: `0` / OOM: `-1` |
| 計算量 | O(n) |

```
len = strlen(cstr)
必要な cap = len + 1
grow して data にコピー
data[len] = '\0'
```

---

#### `str_free`

```c
void str_free(Str *s);
```

| 項目 | 内容 |
|------|------|
| 概要 | 内部バッファを解放し Str をゼロ初期化する |
| 事後条件 | `s->data == NULL`, `s->len == 0`, `s->cap == 0` |
| 計算量 | O(1) |

---

### 読み取り

#### `str_cstr`

```c
#define str_cstr(s)  ((s)->data ? (s)->data : "")
```

| 項目 | 内容 |
|------|------|
| 概要 | null 終端の C 文字列ポインタを返す |
| 戻り値 | `data` が `NULL` のとき `""` （空文字列リテラル）を返す |

> **注意:** 返したポインタは append/insert/reserve の呼び出し後に無効になる可能性がある。

---

#### `str_len_bytes`

```c
size_t str_len_bytes(const Str *s);
```

| 項目 | 内容 |
|------|------|
| 概要 | バイト数を返す（`s->len` と同値） |
| 計算量 | O(1) |

---

#### `str_len_utf8`

```c
size_t str_len_utf8(const Str *s);
```

| 項目 | 内容 |
|------|------|
| 概要 | UTF-8 コードポイント数を返す。不正な UTF-8 の場合は未定義 |
| 計算量 | O(n)（先頭バイトを走査して計算） |

```
count = 0
p = data の先頭から末尾まで走査
  先頭バイトが継続バイト（10xxxxxx）でなければ count++
return count
```

---

### 変更操作

#### `str_append`

```c
int str_append(Str *s, const char *src);
```

| 項目 | 内容 |
|------|------|
| 概要 | C 文字列を末尾に追加する |
| `src` | NULL であってはならない |
| 戻り値 | 成功: `0` / OOM: `-1` |
| 計算量 | 償却 O(n) |

```
src_len = strlen(src)
必要な cap = len + src_len + 1 を確保（grow）
memcpy(data + len, src, src_len)
len += src_len
data[len] = '\0'     ← 必ず最後に設定
```

---

#### `str_append_str`

```c
int str_append_str(Str *s, const Str *other);
```

| 項目 | 内容 |
|------|------|
| 概要 | Str を末尾に追加する |
| 戻り値 | 成功: `0` / OOM: `-1` |
| 計算量 | 償却 O(n) |

> **注意:** `s == other` の自己追加（`str_append_str(&s, &s)`）を正しく処理すること。
> grow 後に `other->data` が無効になる場合があるため、先に長さを保存しておく。

---

#### `str_append_char`

```c
int str_append_char(Str *s, char c);
```

| 項目 | 内容 |
|------|------|
| 概要 | 1 バイト追加する |
| 戻り値 | 成功: `0` / OOM: `-1` |
| 計算量 | 償却 O(1) |

---

#### `str_insert`

```c
int str_insert(Str *s, size_t byte_i, const char *src);
```

| 項目 | 内容 |
|------|------|
| 概要 | バイトオフセット `byte_i` の位置に挿入する。`byte_i` 以降を後ろにシフト |
| `byte_i` | 0 〜 `len` の範囲。`len` の場合は末尾追加と同じ |
| 戻り値 | 成功: `0` / `byte_i > len`: `-1` / OOM: `-1` |
| 計算量 | O(n) |

```
grow して容量を確保
memmove で [byte_i .. len] を src_len バイト後ろにずらす
src をコピー
len += src_len
data[len] = '\0'
```

> **注意:** `byte_i` は「バイトオフセット」である。マルチバイト文字の中間を指している場合、不正な UTF-8 になる。呼び出し側の責任とする。

---

#### `str_clear`

```c
void str_clear(Str *s);
```

| 項目 | 内容 |
|------|------|
| 概要 | `len` を 0 にリセットし `data[0] = '\0'` を設定する。メモリは解放しない |
| 計算量 | O(1) |

---

#### `str_reserve`

```c
int str_reserve(Str *s, size_t new_cap);
```

| 項目 | 内容 |
|------|------|
| 概要 | `new_cap` バイト分の容量を確保する（'\0' 込み） |
| 戻り値 | 成功: `0` / OOM: `-1` |
| 計算量 | O(n) |

- `new_cap <= s->cap` の場合は何もしない
- `len` は変更しない
- 既存の内容と null 終端は保持される

---

### 検索・比較

#### `str_eq`

```c
bool str_eq(const Str *a, const Str *b);
```

| 項目 | 内容 |
|------|------|
| 概要 | バイト列として等値比較する |
| 計算量 | O(n) |

```
if a->len != b->len → false
memcmp(a->data, b->data, a->len) == 0
```

---

#### `str_cmp`

```c
int str_cmp(const Str *a, const Str *b);
```

| 項目 | 内容 |
|------|------|
| 概要 | 辞書順比較（バイト値による） |
| 戻り値 | 負: a < b / 0: 等しい / 正: a > b |
| 計算量 | O(n) |

---

#### `str_find`

```c
ptrdiff_t str_find(const Str *s, const char *needle);
```

| 項目 | 内容 |
|------|------|
| 概要 | `needle` が最初に現れるバイトオフセットを返す |
| 戻り値 | 見つかった場合: オフセット（`>= 0`） / 見つからない: `-1` |
| 計算量 | O(n × m)（ナイーブ実装） |

> 文字列検索アルゴリズム（KMP 等）は Tier 3 の #18 で実装する。ここではナイーブで良い。

---

#### `str_starts_with`

```c
bool str_starts_with(const Str *s, const char *prefix);
```

---

#### `str_ends_with`

```c
bool str_ends_with(const Str *s, const char *suffix);
```

---

### 変換・トリム

#### `str_to_upper` / `str_to_lower`

```c
void str_to_upper(Str *s);
void str_to_lower(Str *s);
```

| 項目 | 内容 |
|------|------|
| 概要 | ASCII 文字のみ大文字/小文字変換する（インプレース） |
| 注意 | UTF-8 のマルチバイト文字は変換しない（バイト値が `0x7F` 以下のみ操作） |
| 計算量 | O(n) |

---

#### `str_trim`

```c
void str_trim(Str *s);
```

| 項目 | 内容 |
|------|------|
| 概要 | 先頭と末尾の ASCII 空白文字（` `, `\t`, `\n`, `\r`）を除去する（インプレース） |
| 計算量 | O(n) |

```
末尾のトリム: data[len-1] が空白の間 len-- して data[len] = '\0'
先頭のトリム: 空白でない最初の位置を start として
             memmove(data, data + start, len - start + 1)
             len -= start
```

---

### UTF-8 サブセット

#### `str_is_valid_utf8`

```c
bool str_is_valid_utf8(const Str *s);
```

| 項目 | 内容 |
|------|------|
| 概要 | バイト列が有効な UTF-8 エンコーディングかを検証する |
| 計算量 | O(n) |

```
先頭バイトのビットパターンから期待するバイト数を決定:
  0xxxxxxx → 1バイト
  110xxxxx → 2バイト（継続バイト 1 個）
  1110xxxx → 3バイト（継続バイト 2 個）
  11110xxx → 4バイト（継続バイト 3 個）
  10xxxxxx → 先頭バイトとして無効

継続バイトはすべて 10xxxxxx のパターンであることを確認
オーバーロング・サロゲート・U+10FFFF超過も弾く
```

---

#### `str_next_codepoint`

```c
int str_next_codepoint(const char *p, uint32_t *out_cp);
```

| 項目 | 内容 |
|------|------|
| 概要 | ポインタ `p` が指す位置から 1 コードポイントをデコードする |
| `out_cp` | デコードされたコードポイントの格納先。NULL の場合は長さ計算のみ |
| 戻り値 | 消費したバイト数（1〜4） / 不正なバイト列: `-1` |
| 計算量 | O(1) |

```c
// 使用例: 文字列全体をイテレート
const char *p = str_cstr(&s);
const char *end = p + str_len_bytes(&s);
while (p < end) {
    uint32_t cp;
    int n = str_next_codepoint(p, &cp);
    if (n < 0) { /* 不正なUTF-8 */ break; }
    // cp でコードポイントを処理
    p += n;
}
```

---

## 内部設計

### `str_grow`（非公開）

```c
static int str_grow(Str *s, size_t min_cap);
```

```
new_cap = max(min_cap, s->cap == 0 ? STR_INITIAL_CAP : s->cap * 2)
tmp = realloc(s->data, new_cap)
if tmp == NULL → return -1  （s->data は変更しない）
s->data = tmp
s->cap  = new_cap
return 0
```

`str_append` から呼ぶ例:

```c
size_t need = s->len + src_len + 1;  // +1 は '\0'
if (need > s->cap) {
    if (str_grow(s, need) != 0) return -1;
}
```

### null 終端の維持ルール

変更操作の実装では以下を徹底する:

```c
// NG: data[len] への書き込みを忘れやすい
memcpy(s->data + s->len, src, src_len);
s->len += src_len;
// ← '\0' を設定し忘れた！

// OK: 必ず最後に設定する
memcpy(s->data + s->len, src, src_len);
s->len += src_len;
s->data[s->len] = '\0';  // ← 必須
```

### UTF-8 先頭バイトの判定

```c
// 先頭バイト b から期待するバイト数を返す
static int utf8_seq_len(unsigned char b) {
    if ((b & 0x80) == 0x00) return 1;  // 0xxxxxxx
    if ((b & 0xE0) == 0xC0) return 2;  // 110xxxxx
    if ((b & 0xF0) == 0xE0) return 3;  // 1110xxxx
    if ((b & 0xF8) == 0xF0) return 4;  // 11110xxx
    return -1;  // 先頭バイトとして無効
}

// 継続バイトの確認
static bool is_continuation(unsigned char b) {
    return (b & 0xC0) == 0x80;  // 10xxxxxx
}
```

---

## エラー処理方針

| エラーの種類 | 扱い方 |
|------------|--------|
| プログラミングエラー（`s == NULL`, `byte_i > len` 等） | `assert` で即死 |
| 実行時エラー（OOM） | `-1` を返す。Str の状態は変更しない |

> Tier 1 では `<assert.h>` の `assert()` を使う。
> 独自 assert マクロ（`docs/assert.md`）完成後に差し替える。

---

## インデックスの単位について

**この実装のすべての位置引数はバイトオフセットである。**

| 関数 | インデックスの単位 |
|------|----------------|
| `str_insert(s, i, src)` | バイト |
| `str_find(s, needle)` | 戻り値もバイト |
| `str_slice(s, start, end, out)` | バイト |

UTF-8 コードポイント単位で操作したい場合は、`str_next_codepoint` でバイトオフセットを事前に計算してから渡す。

```c
// 「3 文字目」のバイトオフセットを求める
const char *p = str_cstr(&s);
for (int i = 0; i < 3; i++) {
    int n = str_next_codepoint(p, NULL);
    if (n < 0) break;
    p += n;
}
size_t byte_offset = p - str_cstr(&s);
```

---

## テストチェックリスト

### ライフサイクル
- [ ] `str_init` → `data == NULL`, `len == 0`, `cap == 0`
- [ ] `str_init_from("hello")` → `len == 5`, `data[5] == '\0'`
- [ ] `str_init_from("")` → `len == 0`, `data[0] == '\0'`
- [ ] `str_free` 後に `data == NULL`

### null 終端の保証
- [ ] 任意の変更操作後に `data[len] == '\0'`
- [ ] `str_cstr(&s)` を `printf("%s", ...)` に渡せる
- [ ] `data == NULL` のとき `str_cstr` が `""` を返す

### 容量・成長
- [ ] `STR_INITIAL_CAP` バイト以上の文字を append しても正しく動作する
- [ ] cap が 0 → 16 → 32 → 64 と推移する
- [ ] `str_reserve(s, 100)` 後に 99 バイトの文字列を append してもリアロケートしない

### 変更操作
- [ ] `str_append` で文字列が末尾に追加される
- [ ] `str_append_str(&s, &s)`（自己追加）が正しく動作する
- [ ] `str_insert(s, 0, "prefix")` で先頭挿入
- [ ] `str_insert(s, len, "suffix")` が `str_append` と同じ結果
- [ ] `str_clear` 後に `len == 0` かつ `data[0] == '\0'`
- [ ] `str_trim` で前後の空白が除去される

### 検索・比較
- [ ] `str_eq`: 同じ内容が等しい
- [ ] `str_eq`: 長さが異なれば false
- [ ] `str_find`: 見つかる場合に正しいバイトオフセット
- [ ] `str_find`: 見つからない場合は `-1`
- [ ] `str_starts_with` / `str_ends_with` の正常系・異常系

### UTF-8
- [ ] `str_is_valid_utf8`: ASCII 文字列は有効
- [ ] `str_is_valid_utf8`: マルチバイト文字を含む有効な UTF-8 文字列
- [ ] `str_is_valid_utf8`: 不正なバイト列（中断されたシーケンス等）で `false`
- [ ] `str_len_utf8("hello") == 5`
- [ ] `str_len_utf8("こんにちは") == 5`（バイト数は 15）
- [ ] `str_next_codepoint`: ASCII 1 文字を正しくデコード
- [ ] `str_next_codepoint`: 2/3/4 バイト文字を正しくデコード
- [ ] `str_next_codepoint`: 不正バイトで `-1`

---

## 実装の壁と対策

### 壁 1: `data[len] = '\0'` の書き忘れ

変更操作のあとに null 終端の設定を忘れると、`printf` 等で未定義動作になる。
テストで「全変更操作後に `data[len] == '\0'`」を必ずアサートする。

```c
// デバッグ用マクロ（開発中に有効化）
#define STR_CHECK_NUL(s)  assert((s)->data == NULL || (s)->data[(s)->len] == '\0')
```

### 壁 2: `cap` の単位混乱（'\0' 込みか否か）

`cap` は **'\0' を含む総バイト数** と定義した。
「格納できる文字は `cap - 1` バイトまで」を常に意識する。

```c
// 容量が必要かの判定（必ず +1 する）
size_t need = s->len + src_len + 1;   // +1 は '\0'
if (need > s->cap) str_grow(s, need);
```

### 壁 3: `str_append_str` の自己追加

`str_append_str(&s, &s)` では grow が発生すると `other->data` が解放される。

```c
// NG: grow 後に other->data が無効になる
int str_append_str(Str *s, const Str *other) {
    return str_append(s, other->data);  // grow で other->data が死ぬ
}

// OK: grow 前に必要な情報をスタック上にコピー
int str_append_str(Str *s, const Str *other) {
    size_t other_len = other->len;
    // other->data が s->data と同じ場合を検出して先にコピーするか、
    // 一時バッファを作るか、grow を先に済ませてから参照する
}
```

### 壁 4: UTF-8 のバイト境界を無視した操作

`str_insert(s, 2, "x")` が「2 文字目の前」ではなく「2 バイト目の前」に挿入することを忘れると、マルチバイト文字を分断した不正な UTF-8 を生成する。

対策: バイトオフセットを受け取る API であることを明記し、コードポイント単位で操作するヘルパーは呼び出し側が `str_next_codepoint` で計算する設計とする（上記「インデックスの単位について」参照）。

### 壁 5: `realloc` の罠

Vec と同様。必ず一時変数を使う。

```c
// NG
s->data = realloc(s->data, new_cap);

// OK
char *tmp = realloc(s->data, new_cap);
if (!tmp) return -1;
s->data = tmp;
s->cap  = new_cap;
```

---

## 将来の拡張（Tier 2: Arena Allocator 完成後）

```c
// 変更後のシグネチャ
void str_init(Str *s, Allocator *alloc);
int  str_init_from(Str *s, const char *cstr, Allocator *alloc);
```

変更点:
- `Str` に `Allocator *alloc` フィールドを追加
- 内部の `malloc` / `realloc` / `free` をアロケータ経由に置き換え
- JSON パーサのキー・値文字列を Arena 上に確保できるようになる

---

## Zig 移行時の対応

| Cでの苦しみ | Zigで解決される点 |
|------------|-----------------|
| `data[len] == '\0'` の手動管理 | `[]u8`（スライス）は length-based で null 終端不要 |
| `str_len_bytes` と `str_len_utf8` の使い分け | `std.unicode.utf8CountCodepoints` 等が標準提供 |
| `void *` / `char *` キャストの煩雑さ | スライスが型安全で長さを持つ |
| `cap` の '\0' 込み計算 | `ArrayList(u8)` で管理が自動化 |
