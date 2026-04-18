# 簡易 printf — 仕様・設計

## 概要

`printf` の内部を自前実装する。`va_list` の扱い・整数から文字列への変換・フォーマット文字列のパースという3つの本質を身につける。

**設計決定:**

| 決定事項 | 選択 | 理由 |
|----------|------|------|
| 出力先 | `Str *` バッファへ追記 | Str との連携練習 + テストが容易 |
| 対応指定子 | `%d %u %s %c %x %o %%` | va_list 理解に集中、幅/精度/フラグはなし |

**学習目標:**
- `va_list` / `va_start` / `va_arg` / `va_end` の正しい使い方
- `va_list` の型昇格ルール（`char` / `short` → `int`）
- 整数を文字列に変換するアルゴリズム（逆順バッファ方式）
- フォーマット文字列を1文字ずつパースするループ設計

**依存関係:**
- `str.h` / `str.c` （Tier 1 の String が必要）
- `<stdarg.h>`（標準ライブラリ）

---

## ファイル構成

```
src/
  my_printf.h    // 宣言
  my_printf.c    // 実装
test/
  test_printf.c  // テスト
```

---

## 対応フォーマット指定子

| 指定子 | 型 | 説明 |
|--------|-----|------|
| `%d`   | `int` | 符号付き 10 進数 |
| `%u`   | `unsigned int` | 符号なし 10 進数 |
| `%s`   | `const char *` | null 終端文字列 |
| `%c`   | `int` (`char` 昇格) | 1 文字 |
| `%x`   | `unsigned int` | 16 進数（小文字: `a`〜`f`） |
| `%o`   | `unsigned int` | 8 進数 |
| `%%`   | — | `%` リテラル |

> 幅・精度・フラグ（`-`, `0`, `+`, スペース）は対応しない。

---

## API 仕様

### `my_sprintf`

```c
int my_sprintf(Str *buf, const char *fmt, ...);
```

| 項目 | 内容 |
|------|------|
| 概要 | `fmt` を解析し、変換した結果を `buf` の末尾に追記する |
| 事前条件 | `buf != NULL`, `fmt != NULL` |
| 戻り値 | 追記したバイト数 / OOM 時 `-1` |
| 計算量 | O(len(fmt) + 各変換の出力長) |

```c
va_list args;
va_start(args, fmt);
int n = my_vsprintf(buf, fmt, args);
va_end(args);
return n;
```

---

### `my_vsprintf`

```c
int my_vsprintf(Str *buf, const char *fmt, va_list args);
```

| 項目 | 内容 |
|------|------|
| 概要 | `my_sprintf` の `va_list` バージョン。ログ関数等から呼ぶ用 |
| 事前条件 | `buf != NULL`, `fmt != NULL` |
| 戻り値 | 追記したバイト数 / OOM 時 `-1` |

```c
// 実装イメージ
int written = 0;
while (*fmt) {
    if (*fmt != '%') {
        // 通常文字
        if (str_push_char(buf, *fmt++) < 0) return -1;
        written++;
        continue;
    }
    fmt++;  // '%' を読み飛ばす
    switch (*fmt++) {
        case 'd': { /* int → 10進 */ ... break; }
        case 'u': { /* unsigned int → 10進 */ ... break; }
        case 's': { /* const char* → そのまま追記 */ ... break; }
        case 'c': { /* int (char) → 1文字 */ ... break; }
        case 'x': { /* unsigned int → 16進 */ ... break; }
        case 'o': { /* unsigned int → 8進 */ ... break; }
        case '%': { /* '%' リテラル */ ... break; }
        default:  { /* 未知の指定子: '%' + 文字をそのまま出力 */ ... break; }
    }
}
return written;
```

---

## 内部設計

### 整数→文字列変換（逆順バッファ方式）

整数変換の共通ヘルパー `fmt_uint` を `static` 関数として実装する。

```c
// n 進数でスタック上の一時バッファに書き込み、Str に追記する
static int fmt_uint(Str *buf, unsigned long long val, unsigned int base,
                    const char *digits) {
    // 例: val=255, base=16, digits="0123456789abcdef" → "ff"
    char tmp[66];  // 2進数最大64桁 + '\0'
    int  i = 0;
    if (val == 0) {
        tmp[i++] = '0';
    } else {
        while (val > 0) {
            tmp[i++] = digits[val % base];
            val /= base;
        }
    }
    // 逆順に追記
    int written = 0;
    for (int j = i - 1; j >= 0; j--) {
        if (str_push_char(buf, tmp[j]) < 0) return -1;
        written++;
    }
    return written;
}
```

`%d`（符号付き）は負の場合に `-` を先頭に追記してから `fmt_uint(abs(val))` を呼ぶ。

```c
// %d の実装
case 'd': {
    int val = va_arg(args, int);
    if (val < 0) {
        if (str_push_char(buf, '-') < 0) return -1;
        written++;
        // INT_MIN の場合は -(long long)val で正しく扱う
        n = fmt_uint(buf, (unsigned long long)(-(long long)val), 10, "0123456789");
    } else {
        n = fmt_uint(buf, (unsigned long long)val, 10, "0123456789");
    }
    if (n < 0) return -1;
    written += n;
    break;
}
```

---

### `%s` の実装

```c
case 's': {
    const char *s = va_arg(args, const char *);
    if (!s) s = "(null)";   // NULL 渡し時のフォールバック
    while (*s) {
        if (str_push_char(buf, *s++) < 0) return -1;
        written++;
    }
    break;
}
```

---

### va_list の型昇格ルール

`char` / `short` / `float` は可変長引数として渡すと自動昇格される。

| 渡した型 | `va_arg` で取り出す型 |
|----------|-----------------------|
| `char`   | `int` |
| `short`  | `int` |
| `float`  | `double` |
| `int`    | `int` |

`%c` で `va_arg(args, char)` と書くと **UB**。`int` で受け取ること。

```c
case 'c': {
    char c = (char)va_arg(args, int);   // int で受け取って char にキャスト
    if (str_push_char(buf, c) < 0) return -1;
    written++;
    break;
}
```

---

### INT_MIN の罠

`-INT_MIN` はオーバーフローする（`INT_MIN = -2147483648`、`INT_MAX = 2147483647`）。
`int` の絶対値を `unsigned long long` 経由で取る。

```c
// NG
unsigned int abs_val = (unsigned int)(-val);   // val == INT_MIN なら UB

// OK
unsigned long long abs_val = (unsigned long long)(-(long long)val);
```

---

## エラー処理方針

| エラーの種類 | 扱い方 |
|------------|--------|
| `buf == NULL` または `fmt == NULL` | `assert` で即死 |
| `str_push_char` の OOM | `-1` を返す（書きかけのまま） |
| `NULL` ポインタ渡し（`%s` に NULL） | `"(null)"` を出力して継続 |
| 未知の指定子 | `%` + 文字をそのまま出力（無視しない） |

> Tier 1 では `<assert.h>` の `assert()` を使う。
> 独自 assert マクロ（`docs/assert.md`）完成後に差し替える。

---

## テストチェックリスト

### `%d`
- [ ] 正の整数が正しく出力される（`42` → `"42"`）
- [ ] 負の整数が正しく出力される（`-42` → `"-42"`）
- [ ] `0` が `"0"` になる
- [ ] `INT_MAX`（`2147483647`）が正しく出力される
- [ ] `INT_MIN`（`-2147483648`）が正しく出力される（オーバーフローなし）

### `%u`
- [ ] `0` が `"0"` になる
- [ ] `UINT_MAX`（`4294967295`）が正しく出力される

### `%s`
- [ ] 通常の文字列が出力される
- [ ] 空文字列 `""` で何も追記されない
- [ ] `NULL` 渡しで `"(null)"` が出力される

### `%c`
- [ ] `'A'` が `"A"` になる
- [ ] `'\n'` が改行文字として追記される

### `%x`
- [ ] `255` → `"ff"`
- [ ] `0` → `"0"`
- [ ] `0xDEADBEEF` → `"deadbeef"`

### `%o`
- [ ] `8` → `"10"`
- [ ] `255` → `"377"`

### `%%`
- [ ] `"100%%"` フォーマットで `"100%"` が出力される

### 複合
- [ ] `my_sprintf(&buf, "x=%d, s=%s", 42, "hello")` → `"x=42, s=hello"`
- [ ] 複数回呼び出して Str の末尾に正しく追記される
- [ ] OOM 時に `-1` が返り、buf は部分的に書かれた状態（メモリ破壊なし）
- [ ] `my_vsprintf` を `my_sprintf` 経由で呼んで同じ結果になる

---

## 実装の壁と対策

### 壁 1: va_arg の型昇格を知らない

```c
// NG: char は int に昇格されるので直接 char で取れない
char c = va_arg(args, char);   // UB（サイズが一致しない）

// OK: int で受け取ってキャスト
char c = (char)va_arg(args, int);
```

同様に `float` は `double` で取り出す。

---

### 壁 2: va_end を忘れる / 呼ぶ場所を間違える

`va_start` と `va_end` は必ずペアで、**同じ関数スコープ内**で呼ぶ。
`my_vsprintf` に `va_list` を渡す場合は呼び出し元が `va_start` / `va_end` を担当する。

```c
// NG: va_list を渡した後に va_end を呼ばない
va_start(args, fmt);
my_vsprintf(buf, fmt, args);
// va_end 忘れ → リソースリーク（スタック破壊の可能性）

// OK
va_start(args, fmt);
my_vsprintf(buf, fmt, args);
va_end(args);
```

---

### 壁 3: 逆順バッファで桁がずれる

逆順に一時バッファへ書き、**後ろから前へ**追記する必要がある。

```c
// NG: 書いた順（逆順）に追記してしまう
for (int j = 0; j < i; j++) str_push_char(buf, tmp[j]);  // "ff" が "ff" じゃなく "ff" の逆 = OK に見えるが...
// 例: 123 → tmp = {'3','2','1'}, これをそのまま追記すると "321"

// OK: 逆順に格納したものを末尾から取り出す
for (int j = i - 1; j >= 0; j--) str_push_char(buf, tmp[j]);  // "123"
```

---

### 壁 4: `%` の次が `\0` のとき

フォーマット文字列が `"value: %"` で終わる場合、`*fmt++` の後が `'\0'`。
`switch` の `default` で `%` だけ追記して終了すれば安全。

```c
fmt++;  // '%' を読み飛ばす
if (*fmt == '\0') {
    str_push_char(buf, '%');
    break;   // ループ終了
}
switch (*fmt++) { ... }
```

---

### 壁 5: INT_MIN の符号反転がオーバーフロー

「壁と対策」の「INT_MIN の罠」セクション参照。`(unsigned long long)(-(long long)val)` が正解。

---

## 将来の拡張

### 幅・精度・フラグの追加（任意）

現実装は指定子のみ対応。後から幅を追加するには
`fmt_uint` にパラメータを渡すか、別ヘルパーを用意する。

### Tier 2: ログシステムとの連携

Arena Allocator と組み合わせて `log_printf(Arena *a, ...)` を作ると、
Arena のリセットだけでログバッファを全解放できる。

### `my_vsprintf` を使ったラッパー群

```c
// 将来追加できるラッパー例
void my_printf(const char *fmt, ...);          // stdout へ
void my_eprintf(const char *fmt, ...);         // stderr へ
Str  my_sprintf_new(const char *fmt, ...);     // 新しい Str を返す（呼び出し側が free）
```

---

## Zig 移行時の対応

| Cでの苦しみ | Zigで解決される点 |
|------------|-----------------|
| `va_arg` の型昇格を手動で把握する必要がある | `anytype` + `comptime` でコンパイル時に型チェック |
| `%s` に NULL を渡すと実行時クラッシュ | Zig の optional `?[]u8` で NULL 不能を型レベルで表現 |
| INT_MIN の符号反転 UB を手動回避 | `@abs` が符号なし整数で安全に返す |
| Str バッファへの追記を手動管理 | `std.fmt.format` + Writer インターフェース |
