# スタック / キュー / Ring Buffer — 仕様・設計

## 概要

Stack と Queue（Ring Buffer）の2つを実装するモジュール。

**設計方針:**

| 構造 | 実装戦略 | 理由 |
|------|----------|------|
| Stack | Vec の薄いラッパー | push/pop は vec_push/vec_pop そのもの |
| Queue | 固定サイズ Ring Buffer | FIFO の本質（循環インデックス）を理解する |

**学習目標:**
- Ring Buffer の循環インデックス管理（`% cap` によるラップアラウンド）
- `head == tail` の曖昧性問題と `len` フィールドによる解決
- Stack と Queue の計算量プロファイルの違いを体感する

**依存関係:**
- Stack は `vec.h` / `vec.c` に依存する（Tier 1 の Vector が必要）
- Queue は `mem.h` のみ依存（`mem_copy` を使う）

---

## ファイル構成

```
src/
  stack.h      // Stack 宣言
  stack.c      // Stack 実装
  queue.h      // Queue 宣言
  queue.c      // Queue 実装
test/
  test_stack.c
  test_queue.c
```

---

## Stack

### データ構造

```c
typedef struct {
    Vec base;
} Stack;
```

Vec をそのままラップする。`Stack` という名前を持つことでコードの意図が明確になる。

---

### API 仕様

#### `stack_init`

```c
void stack_init(Stack *s, size_t elem_size);
```

| 項目 | 内容 |
|------|------|
| 概要 | Stack を初期化する |
| 事前条件 | `s != NULL`, `elem_size > 0` |

```c
assert(s != NULL);
vec_init(&s->base, elem_size);
```

---

#### `stack_free`

```c
void stack_free(Stack *s);
```

| 項目 | 内容 |
|------|------|
| 概要 | Stack を解放する |

```c
vec_free(&s->base);
```

---

#### `stack_push`

```c
int stack_push(Stack *s, const void *elem);
```

| 項目 | 内容 |
|------|------|
| 概要 | 要素をスタックトップに積む |
| 戻り値 | 成功: `0` / OOM: `-1` |
| 計算量 | 償却 O(1) |

```c
return vec_push(&s->base, elem);
```

---

#### `stack_pop`

```c
int stack_pop(Stack *s, void *out);
```

| 項目 | 内容 |
|------|------|
| 概要 | スタックトップを取り出して `out` に書く |
| 戻り値 | 成功: `0` / 空: `-1` |
| 計算量 | O(1) |

```c
if (vec_len(&s->base) == 0) return -1;
return vec_pop(&s->base, out);
```

---

#### `stack_peek`

```c
int stack_peek(const Stack *s, void *out);
```

| 項目 | 内容 |
|------|------|
| 概要 | スタックトップを取り出さずに読む |
| 戻り値 | 成功: `0` / 空: `-1` |
| 計算量 | O(1) |

```c
size_t len = vec_len(&s->base);
if (len == 0) return -1;
mem_copy(out, vec_get(&s->base, len - 1), s->base.elem_size);
return 0;
```

---

#### `stack_is_empty`

```c
bool stack_is_empty(const Stack *s);
```

```c
return vec_len(&s->base) == 0;
```

---

#### `stack_len`

```c
size_t stack_len(const Stack *s);
```

```c
return vec_len(&s->base);
```

---

## Queue（Ring Buffer）

### 設計決定

**固定サイズ** — `cap` は `queue_init` 時に確定し、以後変わらない。
満杯のときの `enqueue` は `-1` を返す（リサイズしない）。

---

### データ構造

```c
typedef struct {
    void   *data;       // heap 上の固定バッファ
    size_t  head;       // 次に dequeue する位置
    size_t  tail;       // 次に enqueue する位置
    size_t  len;        // 現在の要素数
    size_t  cap;        // 確保済みのスロット数（変わらない）
    size_t  elem_size;
} Queue;
```

#### なぜ `len` フィールドが必要か

`head == tail` の状態だけでは空と満杯を区別できない。

```
空:   head == tail, len == 0
満杯: head == tail, len == cap
```

`len` を持つことで O(1) に判定できる。

---

### メモリレイアウト（cap=6 の例）

```
初期状態（空）:
 index: [0][1][2][3][4][5]
          ^
          head = tail = 0,  len = 0

3回 enqueue 後:
 index: [A][B][C][ ][ ][ ]
          ^        ^
          head=0   tail=3,  len = 3

2回 dequeue 後:
 index: [ ][ ][C][ ][ ][ ]
                ^  ^
                head=2, tail=3,  len = 1

wrap-around（tail が 5 → 0 へ）:
 index: [F][ ][C][D][E][ ]
          ^    ^
          tail=1, head=2,  len = 4
         (tail が head を追い越す前に満杯になる)
```

---

### API 仕様

#### `queue_init`

```c
int queue_init(Queue *q, size_t elem_size, size_t cap);
```

| 項目 | 内容 |
|------|------|
| 概要 | 固定サイズ `cap` の Queue を初期化する |
| 事前条件 | `q != NULL`, `elem_size > 0`, `cap > 0` |
| 戻り値 | 成功: `0` / OOM: `-1` |

```c
assert(q != NULL && elem_size > 0 && cap > 0);
q->data = malloc(elem_size * cap);
if (!q->data) return -1;
q->head = q->tail = q->len = 0;
q->cap       = cap;
q->elem_size = elem_size;
return 0;
```

---

#### `queue_free`

```c
void queue_free(Queue *q);
```

```c
assert(q != NULL);
free(q->data);
q->data = NULL;
q->head = q->tail = q->len = q->cap = 0;
```

---

#### `queue_enqueue`

```c
int queue_enqueue(Queue *q, const void *elem);
```

| 項目 | 内容 |
|------|------|
| 概要 | キューの末尾に要素を追加する |
| 戻り値 | 成功: `0` / 満杯: `-1` |
| 計算量 | O(1) |

```c
if (q->len == q->cap) return -1;   // 満杯
unsigned char *dst = (unsigned char *)q->data + q->tail * q->elem_size;
mem_copy(dst, elem, q->elem_size);
q->tail = (q->tail + 1) % q->cap;
q->len++;
return 0;
```

---

#### `queue_dequeue`

```c
int queue_dequeue(Queue *q, void *out);
```

| 項目 | 内容 |
|------|------|
| 概要 | キューの先頭から要素を取り出す |
| 戻り値 | 成功: `0` / 空: `-1` |
| 計算量 | O(1) |

```c
if (q->len == 0) return -1;        // 空
unsigned char *src = (unsigned char *)q->data + q->head * q->elem_size;
mem_copy(out, src, q->elem_size);
q->head = (q->head + 1) % q->cap;
q->len--;
return 0;
```

---

#### `queue_peek`

```c
int queue_peek(const Queue *q, void *out);
```

| 項目 | 内容 |
|------|------|
| 概要 | 先頭要素を取り出さずに読む |
| 戻り値 | 成功: `0` / 空: `-1` |
| 計算量 | O(1) |

```c
if (q->len == 0) return -1;
const unsigned char *src = (const unsigned char *)q->data + q->head * q->elem_size;
mem_copy(out, src, q->elem_size);
return 0;
```

---

#### `queue_is_empty` / `queue_is_full`

```c
bool queue_is_empty(const Queue *q);
bool queue_is_full(const Queue *q);
```

```c
bool queue_is_empty(const Queue *q) { return q->len == 0; }
bool queue_is_full (const Queue *q) { return q->len == q->cap; }
```

---

#### `queue_len` / `queue_cap`

```c
size_t queue_len(const Queue *q);
size_t queue_cap(const Queue *q);
```

---

## エラー処理方針

| エラーの種類 | 扱い方 |
|------------|--------|
| `NULL` ポインタ渡し | `assert` で即死 |
| 空のキューへの dequeue | `-1` を返す（実行時エラー） |
| 満杯のキューへの enqueue | `-1` を返す（固定サイズの仕様） |
| `queue_init` の OOM | `-1` を返す |

> Tier 1 では `<assert.h>` の `assert()` を使う。
> 独自 assert マクロ（`docs/assert.md`）完成後に差し替える。

---

## テストチェックリスト

### Stack

- [ ] `stack_push` → `stack_pop` で同じ値が戻る（LIFO 順）
- [ ] 複数回 push した後 pop すると逆順に出てくる
- [ ] 空の stack に `stack_pop` すると `-1` が返る
- [ ] 空の stack に `stack_peek` すると `-1` が返る
- [ ] `stack_peek` は要素を消費しない（len が変わらない）
- [ ] `stack_is_empty` が正しい
- [ ] `stack_free` 後のメモリリークがない（valgrind / ASan）

### Queue（Ring Buffer）

- [ ] `enqueue` → `dequeue` で同じ値が戻る（FIFO 順）
- [ ] 複数回 enqueue した後 dequeue すると正順に出てくる
- [ ] 空の queue に `dequeue` すると `-1` が返る
- [ ] 空の queue に `peek` すると `-1` が返る
- [ ] `peek` は要素を消費しない
- [ ] 満杯の queue に `enqueue` すると `-1` が返る
- [ ] wrap-around: `tail` が `cap` を超えて `0` に戻っても正しく動く
- [ ] wrap-around: `head` が `cap` を超えて `0` に戻っても正しく動く
- [ ] dequeue → enqueue を繰り返し、インデックスが正しく循環する
- [ ] `len == cap` 全スロット埋まった後に dequeue できる
- [ ] `queue_is_empty` / `queue_is_full` が全状態で正しい
- [ ] `queue_free` 後のメモリリークがない

---

## 実装の壁と対策

### 壁 1: 空と満杯の区別

`len` フィールドを持たず `head == tail` だけで判定しようとすると詰まる。

```c
// NG: head == tail が空か満杯か分からない
bool is_empty(Queue *q) { return q->head == q->tail; }  // 満杯も true になる

// OK: len で判定
bool queue_is_empty(const Queue *q) { return q->len == 0; }
bool queue_is_full (const Queue *q) { return q->len == q->cap; }
```

`len` を持てば `head`/`tail` の解釈が常に一意になる。

---

### 壁 2: `% cap` のタイミングを間違える

インデックス更新は**書き込み・読み込みの後**に行う。

```c
// NG: tail をインクリメントしてからアクセスすると 1 ずれる
q->tail = (q->tail + 1) % q->cap;
dst = (unsigned char *)q->data + q->tail * q->elem_size;  // ← ずれている

// OK: 現在の tail にアクセスしてからインクリメント
dst = (unsigned char *)q->data + q->tail * q->elem_size;
q->tail = (q->tail + 1) % q->cap;
```

---

### 壁 3: バイトオフセット計算に `void *` を使う

`void *` への加算は C の UB。必ず `unsigned char *` にキャストする。

```c
// NG
void *dst = q->data + q->tail * q->elem_size;  // void* arithmetic は UB

// OK
unsigned char *dst = (unsigned char *)q->data + q->tail * q->elem_size;
```

---

### 壁 4: `queue_init` の cap=0

`% 0` は除算ゼロ。`assert(cap > 0)` で防ぐ。

---

## 将来の拡張（Tier 2: Arena Allocator 完成後）

現在の `queue_init` は `malloc` で確保している。Arena 完成後:

```c
// Arena から確保するバージョン（Tier 2 以降）
int queue_init_arena(Queue *q, size_t elem_size, size_t cap, Allocator *alloc);
```

---

## Zig 移行時の対応

| Cでの苦しみ | Zigで解決される点 |
|------------|-----------------|
| `len` フィールドで空/満杯を手動管理 | `std.fifo.LinearFifo` が状態を型安全に管理 |
| `void *` + `elem_size` の型消去 | `comptime T` でジェネリックに型安全 |
| `% cap` のラップアラウンドを手動計算 | `std.fifo.LinearFifo` が内部で処理 |
| Stack が Vec の手動ラッパー | `std.ArrayList` に push/pop が揃っている |
