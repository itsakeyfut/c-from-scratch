# 独自 assert とデバッグマクロ — 仕様・設計

## 概要

標準 `assert()` より詳細なメッセージを出す独自マクロ群を実装する。
Tier 1 の他モジュール（Vec / Str / Queue 等）が `<assert.h>` を使っている箇所を
このモジュール完成後にすべて差し替える。

**設計決定:**

| 決定事項 | 選択 |
|----------|------|
| 失敗ハンドラ | `abort()` 固定（予測可能・シンプル） |
| 含めるマクロ | `ASSERT` / `PANIC` / `UNREACHABLE` / `DEBUG_LOG` |

**学習目標:**
- `__FILE__` / `__LINE__` / `__func__` などのコンパイラ組み込みマクロの使い方
- `#cond`（文字列化演算子）と `do { ... } while (0)` パターン
- `NDEBUG` フラグによる条件コンパイルと副作用の罠
- `__builtin_unreachable()` のコンパイラ最適化への影響

**依存関係:** なし（標準ライブラリのみ: `<stdio.h>` / `<stdlib.h>`）

> このモジュール完成後、Tier 1 の全 `.c` ファイルの `#include <assert.h>` を
> `#include "dbg.h"` に置き換え、`assert(...)` を `ASSERT(...)` に差し替える。

---

## ファイル構成

```
src/
  dbg.h    // 宣言 + マクロ定義（実装は全てヘッダ内マクロで完結）
           // dbg.c は不要
test/
  test_dbg.c
```

> ヘッダのみで完結する（`.c` ファイルなし）。

---

## マクロ一覧

| マクロ | NDEBUG 時 | 用途 |
|--------|-----------|------|
| `ASSERT(cond)` | 無効化 | 事前条件・不変条件のチェック |
| `PANIC(msg)` | **有効のまま** | 到達してはいけない場所、回復不能エラー |
| `UNREACHABLE()` | `__builtin_unreachable()` | コンパイラへの到達不能ヒント |
| `DEBUG_LOG(fmt, ...)` | 無効化 | デバッグビルド時のみのログ出力 |

---

## API 仕様

### `ASSERT`

```c
#define ASSERT(cond)  /* ... */
```

| 項目 | 内容 |
|------|------|
| 概要 | `cond` が偽のとき、ファイル・行・関数名付きのメッセージを stderr に出力して `abort()` |
| NDEBUG | 定義されている場合は完全に無効化（`((void)0)` になる） |
| 副作用 | `cond` に副作用のある式を渡してはならない（NDEBUG で消えるため） |

```c
#ifdef NDEBUG
#  define ASSERT(cond)  ((void)0)
#else
#  define ASSERT(cond) \
    do { \
        if (!(cond)) { \
            fprintf(stderr, \
                "ASSERT failed: %s\n" \
                "  at %s:%d in %s()\n", \
                #cond, __FILE__, __LINE__, __func__); \
            abort(); \
        } \
    } while (0)
#endif
```

**出力例:**
```
ASSERT failed: ptr != NULL
  at src/vec.c:42 in vec_push()
```

---

### `PANIC`

```c
#define PANIC(msg)  /* ... */
```

| 項目 | 内容 |
|------|------|
| 概要 | メッセージを stderr に出力して `abort()`。条件チェックなし |
| NDEBUG | **影響を受けない**（常に有効） |
| 用途 | 回復不能エラー、`switch` の `default` など絶対に来てはいけない分岐 |

```c
#define PANIC(msg) \
    do { \
        fprintf(stderr, \
            "PANIC: %s\n" \
            "  at %s:%d in %s()\n", \
            (msg), __FILE__, __LINE__, __func__); \
        abort(); \
    } while (0)
```

**使用例:**
```c
switch (state) {
    case STATE_IDLE:  handle_idle();  break;
    case STATE_RUN:   handle_run();   break;
    default: PANIC("unknown state");
}
```

---

### `UNREACHABLE`

```c
#define UNREACHABLE()  /* ... */
```

| 項目 | 内容 |
|------|------|
| 概要 | この行に到達しないことを表明する |
| NDEBUG なし | `PANIC("unreachable")` と同じ動作 |
| NDEBUG あり | `__builtin_unreachable()` に展開 → コンパイラが最適化ヒントとして使う |
| 危険性 | NDEBUG で実際に到達したとき UB。ASSERT は消えるが UNREACHABLE は別の意味で危険 |

```c
#ifdef NDEBUG
#  define UNREACHABLE() __builtin_unreachable()
#else
#  define UNREACHABLE() \
    do { \
        fprintf(stderr, \
            "UNREACHABLE reached\n" \
            "  at %s:%d in %s()\n", \
            __FILE__, __LINE__, __func__); \
        abort(); \
    } while (0)
#endif
```

> **注意:** `__builtin_unreachable()` は GCC / Clang の拡張。MSVC では `__assume(0)`。
> Windows + MSVC ターゲットの場合は下記に差し替える:
> ```c
> #ifdef _MSC_VER
> #  define UNREACHABLE_IMPL()  __assume(0)
> #else
> #  define UNREACHABLE_IMPL()  __builtin_unreachable()
> #endif
> ```

---

### `DEBUG_LOG`

```c
#define DEBUG_LOG(fmt, ...)  /* ... */
```

| 項目 | 内容 |
|------|------|
| 概要 | `DEBUG` が定義されているときだけ stderr にログを出力する |
| 有効条件 | `NDEBUG` ではなく `DEBUG` マクロで制御（独立したスイッチ） |
| フォーマット | `[DBG file:line] メッセージ` 形式 |

```c
#ifdef DEBUG
#  define DEBUG_LOG(fmt, ...) \
    fprintf(stderr, "[DBG %s:%d] " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#  define DEBUG_LOG(fmt, ...)  ((void)0)
#endif
```

**使用例:**
```c
DEBUG_LOG("vec_push: len=%zu cap=%zu", v->len, v->cap);
// ビルド: clang -DDEBUG ... で有効化
// 出力:   [DBG src/vec.c:55] vec_push: len=3 cap=8
```

> `##__VA_ARGS__` は引数なし（`DEBUG_LOG("hello")`）のとき末尾カンマを消す GCC/Clang 拡張。
> C23 では `__VA_OPT__(,)` が標準化されたが、C11 では `##` を使う。

---

## NDEBUG と DEBUG の関係

```
ビルド種別        | NDEBUG  | DEBUG  | ASSERT  | PANIC  | UNREACHABLE   | DEBUG_LOG
------------------|---------|--------|---------|--------|---------------|----------
開発（デフォルト）| 未定義  | 未定義 | 有効    | 有効   | abort()       | 無効
デバッグ詳細      | 未定義  | 定義   | 有効    | 有効   | abort()       | 有効
リリース          | 定義    | 未定義 | 無効    | 有効   | unreachable() | 無効
```

ビルドコマンド例:
```sh
# 開発（デフォルト）
clang -std=c11 -Wall -Wextra -g -fsanitize=address,undefined -o out main.c

# デバッグ詳細
clang -std=c11 -Wall -Wextra -g -DDEBUG -fsanitize=address,undefined -o out main.c

# リリース
clang -std=c11 -O2 -DNDEBUG -o out main.c
```

---

## `do { } while (0)` パターン

複数文のマクロを安全に展開するための慣用句。

```c
// NG: if/else に挟まれると壊れる
#define ASSERT_BAD(cond) if (!(cond)) { fprintf(...); abort(); }

if (x)
    ASSERT_BAD(y);   // → if (x) if (!(y)) { ... };
else
    do_something();  // ← この else は ASSERT_BAD の if に紐付く!

// OK: do-while でブロック化
#define ASSERT(cond) do { ... } while (0)

if (x)
    ASSERT(y);   // → if (x) do { ... } while (0);
else
    do_something();  // 正しく x の else に紐付く
```

---

## ASSERT の副作用の罠

```c
// NG: NDEBUG で foo() が消える → プログラムの動作が変わる
ASSERT(foo() != NULL);

// OK: 先に変数に代入する
void *result = foo();
ASSERT(result != NULL);
use(result);
```

---

## テストチェックリスト

> `abort()` を呼ぶマクロは通常テストで呼べない。
> 代わりに「正常系（assert が発火しないケース）」と「コンパイル・展開の正しさ」を検証する。

### コンパイル検証
- [ ] `ASSERT(1)` がコンパイルエラーなし
- [ ] `ASSERT(0)` がコンパイルエラーなし（発火は実行時）
- [ ] `PANIC("msg")` がコンパイルエラーなし
- [ ] `UNREACHABLE()` がコンパイルエラーなし
- [ ] `DEBUG_LOG("x=%d", 42)` がコンパイルエラーなし
- [ ] `DEBUG_LOG("hello")` （引数なし）がコンパイルエラーなし

### ASSERT 正常系
- [ ] `ASSERT(1 == 1)` は何もしない
- [ ] `ASSERT(ptr != NULL)` は `ptr` が非 NULL のとき何もしない

### NDEBUG 検証
- [ ] `-DNDEBUG` でビルドすると `ASSERT(0)` が発火せず通過する
- [ ] `-DNDEBUG` でも `PANIC("msg")` は発火する（abort する）

### DEBUG_LOG 検証
- [ ] `-DDEBUG` なしのビルドで `DEBUG_LOG` が何も出力しない
- [ ] `-DDEBUG` ありのビルドで `DEBUG_LOG("x=%d", 42)` が stderr に `[DBG ...]` を出力する

### 展開形の正しさ
- [ ] `if (cond) ASSERT(x); else stmt;` が正しくコンパイルされる（dangling else なし）
- [ ] `if (cond) PANIC("msg"); else stmt;` が正しくコンパイルされる

---

## 実装の壁と対策

### 壁 1: セミコロンを付けると `do-while` の後に余分な `;` が出る

```c
ASSERT(x);
// 展開後: do { ... } while (0);
// → 問題なし。do-while の後の ; は空文として合法
```

`do { } while (0)` は末尾の `;` を自然に吸収するために設計されている。

---

### 壁 2: `if-else` と `;` だけのマクロの衝突

```c
// NG: 単純な { } で括るパターン
#define ASSERT_BAD(cond) { if (!(cond)) abort(); }

if (ok) ASSERT_BAD(x);
else    do_thing();   // コンパイルエラー: else の前に ; がない
```

`do { } while (0)` で必ず括ること。

---

### 壁 3: `##__VA_ARGS__` が C 標準外

`DEBUG_LOG("hello")` のように可変引数が空のとき、標準 C11 では末尾カンマが残って
コンパイルエラーになる場合がある。`##__VA_ARGS__` は GCC/Clang 拡張で解決するが
厳密な C11 準拠では使えない。

```c
// C11 厳密準拠で空の可変引数を扱う回避策:
// 引数を必ず 1 つ以上要求するか、C23 の __VA_OPT__ を使う
```

clang の `-std=c11` では `##__VA_ARGS__` が動作するため、このプロジェクトでは使用する。

---

### 壁 4: `__func__` は C99 以降のみ

`__func__` はC99から導入された。C89では使えない。
このプロジェクトは C11 なので問題ない。

---

## 将来の拡張

### ASSERT_MSG（メッセージ付き assert）

```c
#define ASSERT_MSG(cond, msg) \
    do { \
        if (!(cond)) { \
            fprintf(stderr, "ASSERT failed: %s\n  msg: %s\n  at %s:%d in %s()\n", \
                    #cond, (msg), __FILE__, __LINE__, __func__); \
            abort(); \
        } \
    } while (0)

// 使用例
ASSERT_MSG(index < len, "index out of bounds");
```

### ログレベル付き DEBUG_LOG

Tier 4 のロギングシステムに移行後は `DEBUG_LOG` を廃止し、
レベル（INFO / WARN / ERROR）付きの `LOG(level, fmt, ...)` に統一する。

---

## Zig 移行時の対応

| Cでの苦しみ | Zigで解決される点 |
|------------|-----------------|
| マクロで `__FILE__` / `__LINE__` を手動埋め込み | `@src()` で呼び出し元の情報を型安全に取得 |
| `do { } while (0)` パターンが必要 | `comptime` ブロックで関数として書ける |
| NDEBUG 依存のマクロが副作用の罠を生む | `std.debug.assert` はリリースビルドでも安全なオーバーライドが可能 |
| `PANIC` と `UNREACHABLE` が別マクロ | `@panic` / `unreachable` が言語組み込み |
