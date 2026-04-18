# 06 - 文字列 (Strings)

C言語の文字列に関する基本概念と操作をまとめたクイックレビュー用ドキュメントです。

---

## 目次

1. [文字列リテラルと char 配列の違い](#1-文字列リテラルと-char-配列の違い)
2. [strlen — 文字列の長さ](#2-strlen--文字列の長さ)
3. [strcmp — 文字列の比較](#3-strcmp--文字列の比較)
4. [strcpy / strncpy / snprintf — 文字列のコピー](#4-strcpy--strncpy--snprintf--文字列のコピー)
5. [strcat — 文字列の連結](#5-strcat--文字列の連結)
6. [strstr — 部分文字列の検索](#6-strstr--部分文字列の検索)
7. [ctype.h — 文字操作](#7-ctypeh--文字操作)
8. [ヒープ上に文字列を確保する](#8-ヒープ上に文字列を確保する)
9. [snprintf でフォーマット文字列を構築する](#9-snprintf-でフォーマット文字列を構築する)
10. [strlen はバイナリ安全でない](#10-strlen-はバイナリ安全でない)
11. [まとめ：文字列操作チートシート](#11-まとめ文字列操作チートシート)

---

## 1. 文字列リテラルと char 配列の違い

```c
// 1. char 配列: スタック上にコピーされる → 書き込み可能
char arr[] = "hello";
arr[0] = 'H';  // OK
printf("sizeof(arr): %zu\n", sizeof(arr));  // 6（'\0' を含む）

// 2. char ポインタ: 読み取り専用の文字列リテラルを指す → 書き込み禁止
const char *ptr = "world";
// ptr[0] = 'W';  // NG: 未定義動作（クラッシュする可能性）
```

### char 配列 vs const char * の比較

| | `char arr[] = "hello"` | `const char *ptr = "hello"` |
|--|------------------------|------------------------------|
| メモリ | スタック上にコピー | 読み取り専用セグメント（`.rodata`）を指す |
| 書き込み | 可能 | **不可（UB）** |
| `sizeof` | 7（`'\0'` 含む全バイト数） | 8（ポインタのサイズ） |
| 用途 | 書き換えが必要な文字列 | 定数文字列・関数引数の読み取り専用参照 |

### メモリイメージ

```
char arr[] = "hello":
  スタック: [ h ][ e ][ l ][ l ][ o ][ \0 ]
             ↑ arr が直接この領域を所有

const char *ptr = "world":
  スタック: [ ptr ──────────────────┐ ]
  .rodata:                          ↓
            [ w ][ o ][ r ][ l ][ d ][ \0 ]（読み取り専用）
```

### C言語の文字列の基本ルール

- C の文字列は **null 文字 `'\0'` で終端** される文字の配列
- `'\0'` は値が 0 の 1 バイト。文字列の終端を示す
- `strlen` は `'\0'` の手前までの長さを返す（`'\0'` 自体は含まない）

---

## 2. `strlen` — 文字列の長さ

```c
char s[] = "Hello, World!";
strlen(s)   // → 13（'\0' を含まない文字数）
sizeof(s)   // → 14（'\0' を含む全バイト数）
```

### `strlen` と `sizeof` の違い

| | `strlen(s)` | `sizeof(s)` |
|--|-------------|-------------|
| 返す値 | `'\0'` 手前までの文字数 | 配列全体のバイト数（`'\0'` 含む） |
| 計算 | O(n)（先頭から `'\0'` を探す） | O(1)（コンパイル時定数） |
| ポインタに対して | 文字列の長さを返す | **ポインタのサイズ（8）を返す** |

### 注意点

- `strlen` は **O(n)** なのでループ内で毎回呼ぶとパフォーマンスが悪化する
- `'\0'` がなければ範囲外を読み続けて未定義動作になる

```c
// 悪い例（ループごとに O(n) の strlen が走る）
for (int i = 0; i < strlen(s); i++) { ... }

// 良い例（長さを一度だけ計算）
size_t len = strlen(s);
for (size_t i = 0; i < len; i++) { ... }
```

---

## 3. `strcmp` — 文字列の比較

```c
const char *a = "apple";
const char *b = "apple";
const char *c = "banana";

a == b          // ポインタの比較（アドレスが同じかどうか）→ 文字列比較ではない
strcmp(a, b)    // → 0    （等しい）
strcmp(a, c)    // → 負の値 （a < c、辞書順で a が前）
strcmp(c, a)    // → 正の値 （c > a、辞書順で c が後）
```

### `==` で文字列を比較できない理由

`const char *` はアドレスを保持するポインタなので、`==` はアドレスの比較になる。  
同じ文字列リテラルでも実装によってアドレスが異なる場合がある。

```c
const char *p = "hello";
const char *q = "hello";
p == q;            // 実装依存（同じ .rodata を参照することもある）
strcmp(p, q) == 0; // 確実に文字列として比較できる
```

### `strcmp` の戻り値

| 戻り値 | 意味 |
|--------|------|
| `0` | 2つの文字列が等しい |
| 負の値 | 第1引数 < 第2引数（辞書順） |
| 正の値 | 第1引数 > 第2引数（辞書順） |

> **コツ:** `strcmp(a, b) == 0` で等値判定するのが最も明確な書き方。

---

## 4. `strcpy` / `strncpy` / `snprintf` — 文字列のコピー

```c
char dst[8];

// strcpy: dst のサイズを確認しない → 危険
strcpy(dst, "hello");    // OK（"hello" は 6バイトで dst[8] に収まる）
strcpy(dst, "toolongstring");  // NG: バッファオーバーフロー（UB）

// snprintf: サイズを指定して安全にコピー（推奨）
snprintf(dst, sizeof(dst), "%s", "toolongstring");
// → "toolong"（8バイト目を '\0' にして切り詰め）

// strncpy の罠: null 終端を保証しない
char dst2[5];
strncpy(dst2, "hello!", sizeof(dst2));  // 5文字コピーするが '\0' が入らない！
dst2[sizeof(dst2) - 1] = '\0';          // 手動で null 終端を設定する必要がある
```

### 各関数の比較

| 関数 | null 終端の保証 | サイズ指定 | 推奨度 |
|------|----------------|-----------|--------|
| `strcpy(dst, src)` | あり | **なし** | △（危険） |
| `strncpy(dst, src, n)` | **なし**（要注意） | あり | △（罠あり） |
| `snprintf(dst, n, "%s", src)` | あり | あり | ◎（推奨） |

### バッファオーバーフローの危険性

`strcpy` は `dst` のサイズを一切確認しないため、`src` が長すぎると隣接するメモリを上書きする。  
これは**バッファオーバーフロー**と呼ばれ、セキュリティ脆弱性（スタック破壊、任意コード実行）の原因になる。

```
dst[8]:
  before: [ ? ][ ? ][ ? ][ ? ][ ? ][ ? ][ ? ][ ? ] | 隣のデータ...
  after strcpy(dst, "toolongstring"):
          [ t ][ o ][ o ][ l ][ o ][ n ][ g ][ s ] [ t ][ r ][ i ][ n ][ g ][\0]
                                                      ↑ 範囲外を上書き！
```

---

## 5. `strcat` — 文字列の連結

```c
char buf[32] = "Hello";
strcat(buf, ", ");     // buf: "Hello, "
strcat(buf, "World!"); // buf: "Hello, World!"
```

### 重要なポイント

- `strcat` は `buf` の `'\0'` の位置を探してから追記する → O(n)
- `buf` に十分なサイズがないとバッファオーバーフローになる
- 複数回の `strcat` は効率が悪い（毎回先頭から `'\0'` を探す）

### 安全・効率的な代替手段

```c
// snprintf で一度にフォーマット（推奨）
snprintf(buf, sizeof(buf), "%s%s%s", "Hello", ", ", "World!");
```

---

## 6. `strstr` — 部分文字列の検索

```c
const char *haystack = "the quick brown fox";
const char *needle   = "brown";
char *found = strstr(haystack, needle);

if (found) {
    printf("offset: %td\n", found - haystack); // → 10
}
```

### 重要なポイント

- `strstr` は `needle` が最初に見つかった位置へのポインタを返す
- 見つからない場合は `NULL` を返す → **必ず `NULL` チェックをする**
- `found - haystack` でオフセット（何文字目か）が計算できる（ポインタ演算）

### 戻り値の使い方

```c
char *found = strstr(haystack, needle);

if (found) {
    // found 以降の文字列として使う
    printf("%s\n", found);        // "brown fox"

    // オフセットを計算する
    ptrdiff_t pos = found - haystack; // 10
}
```

---

## 7. `ctype.h` — 文字操作

```c
#include <ctype.h>

char word[] = "Hello World 123";
for (int i = 0; word[i] != '\0'; i++) {
    word[i] = (char)tolower((unsigned char)word[i]);
}
// → "hello world 123"

isdigit('5')  // 非0（真）
isspace(' ')  // 非0（真）
isalpha('a')  // 非0（真）
```

### 主な関数一覧

| 関数 | 説明 | 真になる文字の例 |
|------|------|-----------------|
| `isdigit(c)` | 数字かどうか | `'0'`〜`'9'` |
| `isalpha(c)` | アルファベットかどうか | `'a'`〜`'z'`、`'A'`〜`'Z'` |
| `isalnum(c)` | 英数字かどうか | `isdigit` または `isalpha` |
| `isspace(c)` | 空白文字かどうか | `' '`、`'\t'`、`'\n'` など |
| `isupper(c)` | 大文字かどうか | `'A'`〜`'Z'` |
| `islower(c)` | 小文字かどうか | `'a'`〜`'z'` |
| `toupper(c)` | 大文字に変換 | `'a'` → `'A'` |
| `tolower(c)` | 小文字に変換 | `'A'` → `'a'` |

### `(unsigned char)` キャストが必要な理由

```c
word[i] = (char)tolower((unsigned char)word[i]);
//                      ↑ なぜ必要？
```

`ctype.h` の関数は引数を `unsigned char` の範囲（0〜255）または `EOF`（-1）として受け取る仕様。  
`char` が `signed char` の環境では 128〜255 の文字が**負の値**として渡り、未定義動作になる。  
`(unsigned char)` にキャストすることで安全に使える。

---

## 8. ヒープ上に文字列を確保する

```c
const char *src = "dynamic string";
size_t len = strlen(src);
char *dup = malloc(len + 1);  // +1 は '\0' の分
if (dup) {
    memcpy(dup, src, len + 1);  // '\0' も含めてコピー
    printf("dup: %s\n", dup);
    free(dup);  // 使い終わったら解放
}
```

### なぜ `len + 1` バイト確保するのか

```
"dynamic string" の内部:
  [ d ][ y ][ n ][ a ][ m ][ i ][ c ][ ' '][ s ][ t ][ r ][ i ][ n ][ g ][ \0 ]
  ←────────── strlen = 14 ──────────────────────────────────────────→  ↑
                                                                      これも必要
```

`strlen` は `'\0'` を含まない長さを返すため、`'\0'` 分の 1 バイトを足す必要がある。

### `strdup` の代替実装

POSIX 環境には `strdup` があるが、C 標準（C11 まで）には含まれない。  
上記のパターンが `strdup` の手動実装に相当する。

```c
// POSIX 環境なら
char *dup = strdup(src); // malloc + strcpy を自動でやってくれる
// ...
free(dup);
```

---

## 9. `snprintf` でフォーマット文字列を構築する

```c
char result[64];
int  code = 404;
const char *msg = "Not Found";

snprintf(result, sizeof(result), "HTTP %d %s", code, msg);
printf("%s\n", result);  // → "HTTP 404 Not Found"
```

### `sprintf` と `snprintf` の違い

| 関数 | バッファ長の指定 | 推奨 |
|------|-----------------|------|
| `sprintf(buf, fmt, ...)` | **なし**（危険） | × |
| `snprintf(buf, n, fmt, ...)` | あり（安全） | ◎ |

`snprintf` は書き込む最大バイト数（`'\0'` 含む）を `n` で制限するため、バッファオーバーフローを防げる。

### 戻り値の活用

```c
int written = snprintf(result, sizeof(result), "HTTP %d %s", code, msg);
// 戻り値: 実際に書き込まれた文字数（'\0' 除く）
// written >= sizeof(result) なら切り詰めが発生している
if (written >= (int)sizeof(result)) {
    // 切り詰め発生 → バッファが足りなかった
}
```

---

## 10. `strlen` はバイナリ安全でない

```c
unsigned char binary[] = {0x61, 0x00, 0x62, 0x63};  // 'a', NUL, 'b', 'c'
strlen((char *)binary)  // → 1（最初の NUL で止まる）
sizeof(binary)          // → 4（実際のバイト数）
```

### なぜ問題になるか

`strlen` は `'\0'`（NUL バイト）で終端を判断するため、**途中に NUL が含まれるバイナリデータ**には使えない。

```
binary: [ 'a' | \0 | 'b' | 'c' ]
          ↑            ↑
     strlen はここで止まる  実際にはここまでデータがある
```

### バイナリデータを扱う場合のルール

- 長さは **`sizeof` で確定した値** または **別途管理する変数** で持つ
- 文字列関数（`strlen`、`strcpy` など）は使わず、`memcpy`・`memcmp`・`memmove` を使う

```c
// バイナリデータの操作
unsigned char buf1[4] = {0x61, 0x00, 0x62, 0x63};
unsigned char buf2[4];
memcpy(buf2, buf1, sizeof(buf1));  // NUL を含めて 4 バイトコピー
memcmp(buf1, buf2, sizeof(buf1));  // NUL を含めて 4 バイト比較
```

---

## 11. まとめ：文字列操作チートシート

```c
// --- 文字列の種類 ---
char arr[] = "hello";       // スタック上にコピー → 書き込み可
const char *ptr = "hello";  // 読み取り専用領域を指す → 書き込み禁止

// --- 長さ ---
strlen(s)           // '\0' を含まない文字数（O(n)）
sizeof(arr)         // '\0' を含む全バイト数（配列のみ・ポインタ不可）

// --- 比較（== は使わない）---
strcmp(a, b) == 0   // 等しい
strcmp(a, b)  < 0   // a < b（辞書順）
strcmp(a, b)  > 0   // a > b（辞書順）

// --- コピー（snprintf を優先）---
snprintf(dst, sizeof(dst), "%s", src);       // 安全（推奨）
strcpy(dst, src);                             // 危険（サイズ確認なし）
strncpy(dst, src, n); dst[n-1] = '\0';       // null 終端を手動で設定

// --- 連結 ---
strcat(dst, src);   // dst の末尾に src を追加（サイズ注意）
snprintf(buf, sizeof(buf), "%s%s", a, b);    // 安全な連結

// --- 検索 ---
char *p = strstr(haystack, needle);  // 部分文字列の位置（なければ NULL）
if (p) { ptrdiff_t offset = p - haystack; }

// --- 文字の判定・変換（unsigned char キャスト必須）---
tolower((unsigned char)c)   // 小文字化
toupper((unsigned char)c)   // 大文字化
isdigit((unsigned char)c)   // 数字かどうか
isspace((unsigned char)c)   // 空白かどうか

// --- ヒープ上の文字列 ---
char *dup = malloc(strlen(src) + 1);  // +1 は '\0' の分
memcpy(dup, src, strlen(src) + 1);
free(dup); dup = NULL;

// --- フォーマット文字列の構築 ---
snprintf(buf, sizeof(buf), "HTTP %d %s", code, msg);

// --- バイナリデータ（NUL 含む）→ mem 系関数を使う ---
memcpy(dst, src, n);    // n バイトコピー
memcmp(a, b, n);        // n バイト比較
memmove(dst, src, n);   // 重複領域でも安全なコピー
```
