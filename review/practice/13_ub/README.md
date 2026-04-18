# 13 - 未定義動作と落とし穴 (Undefined Behavior & Pitfalls)

C言語の未定義動作（UB）と代表的な落とし穴をまとめたクイックレビュー用ドキュメントです。  
各パターンは「なぜ危険か」を理解することが目的で、実行可能なコードはコメントアウトしてあります。

---

## 目次

1. [未定義動作（UB）とは](#1-未定義動作ubとは)
2. [UB-1: 未初期化変数の読み取り](#2-ub-1-未初期化変数の読み取り)
3. [UB-2: NULL ポインタのデリファレンス](#3-ub-2-null-ポインタのデリファレンス)
4. [UB-3: Use-after-free](#4-ub-3-use-after-free)
5. [UB-4: 二重解放（double free）](#5-ub-4-二重解放double-free)
6. [UB-5: 配列の範囲外アクセス](#6-ub-5-配列の範囲外アクセス)
7. [UB-6: 符号付き整数オーバーフロー](#7-ub-6-符号付き整数オーバーフロー)
8. [UB-7: void* のポインタ演算](#8-ub-7-void-のポインタ演算)
9. [落とし穴: signed/unsigned の比較](#9-落とし穴-signedunsigned-の比較)
10. [落とし穴: strict aliasing 違反](#10-落とし穴-strict-aliasing-違反)
11. [サニタイザの活用](#11-サニタイザの活用)
12. [まとめ：UB 一覧チートシート](#12-まとめub-一覧チートシート)

---

## 1. 未定義動作（UB）とは

**C 標準が「何が起きるか保証しない」と定めている動作。**

> 「クラッシュする」「0 を返す」「正しく動く」のどれも起こりうる。  
> コンパイラは UB を踏んだコードを最適化で**削除・変形**することがある。

### UB が特に危険な理由

```
通常のバグ:   プログラムがクラッシュする → 発見しやすい
UB:          正しく動いているように見える → 本番環境でだけ壊れる
             コンパイラの最適化で全く別の動作になる
             セキュリティ脆弱性の根本原因になる
```

### UB の種類

| カテゴリ | 例 |
|----------|-----|
| メモリアクセス | NULL デリファレンス、範囲外アクセス、use-after-free |
| 算術 | 符号付き整数オーバーフロー、ゼロ除算 |
| 型 | strict aliasing 違反、不正な型キャスト |
| その他 | 未初期化変数の読み取り、`va_arg` の型ミスマッチ |

---

## 2. UB-1: 未初期化変数の読み取り

```c
int x = 0;       // OK: 初期化済み
int y;
printf("%d\n", y); // NG: ゴミ値 or UB
```

### なぜ危険か

ローカル変数は自動的に初期化されない。スタック上のゴミ値（前の関数呼び出しの残骸）が読まれる。

```
スタックの状態:
  前の関数が残したデータ: [0x00][0x4F][0x12][0x78]
  int y;  ← この領域はクリアされない
  printf("y = %d\n", y);  → y = 1249336 などのゴミ値
```

最適化によっては「初期化されていないことを知っている」コンパイラが  
`y` を使うコードごと削除・変形することもある。

### 対策

```c
// すべてのローカル変数は宣言と同時に初期化する
int x = 0;
int *p = NULL;
double d = 0.0;
char buf[64] = {0};
```

**検出:** `-fsanitize=memory`（MemorySanitizer）

---

## 3. UB-2: NULL ポインタのデリファレンス

```c
int *p = NULL;
*p = 42;    // NG: セグメンテーション違反（クラッシュ）
int x = *p; // NG: 同様
```

### なぜ危険か

`NULL`（アドレス 0）は通常 OS がマップしていないアドレス。  
アクセスするとカーネルがシグナル（SIGSEGV）を送ってプロセスを強制終了する。

クラッシュすれば発見できるが、  
まれに「NULL アドレス付近がマップされている環境」では動いてしまい、静かにデータを破壊する。

### 対策

```c
// ポインタを使う前に必ず NULL チェック
int *p = get_pointer();
if (p != NULL) {
    *p = 42;
}

// 関数の戻り値も必ずチェック
int *arr = malloc(n * sizeof(int));
if (!arr) {
    fprintf(stderr, "malloc failed\n");
    return -1;
}
```

---

## 4. UB-3: Use-after-free

```c
int *p = malloc(sizeof(int));
*p = 42;
free(p);

*p = 100;      // NG: 解放済みメモリへの書き込み（UB）
printf("%d", *p); // NG: 解放済みメモリからの読み取り（UB）
```

### なぜ危険か

`free` 後のメモリはヒープ管理システムに返却され、すぐに別の `malloc` に再利用される。

```
時系列:
  malloc() → p が指す領域を取得
  free(p)  → 領域がヒープに返却
  malloc() → 別の変数 q が同じ領域を取得（かもしれない）
  *p = 100 → q の内容を上書き！ → 予期しない動作
```

セキュリティ上は**use-after-free は攻撃者に悪用される**（Type Confusion 攻撃など）。

### 対策

```c
free(p);
p = NULL;  // free 後は必ず NULL を代入

// NULL チェックで誤使用を防ぐ
if (p != NULL) {
    *p = 100;  // p が NULL なら実行されない
}
```

**検出:** `-fsanitize=address`（AddressSanitizer）

---

## 5. UB-4: 二重解放（double free）

```c
int *p = malloc(sizeof(int));
free(p);
free(p);    // NG: 同じポインタを 2 回 free → クラッシュまたはヒープ破壊
```

### なぜ危険か

ヒープ管理構造（フリーリスト）が壊れる。  
次の `malloc` や `free` で予期しない動作が起きる。  
セキュリティ的にも悪用可能な脆弱性になる。

### 対策

```c
free(p);
p = NULL;   // NULL を代入しておけば 2 回目の free(NULL) は安全（何もしない）
free(p);    // OK: free(NULL) は安全（C 標準で保証）
```

---

## 6. UB-5: 配列の範囲外アクセス

```c
int arr[5] = {1, 2, 3, 4, 5};
arr[4];     // OK: 最後の有効インデックス
arr[5];     // NG: UB（隣接するメモリを読む）
arr[5] = 9; // NG: バッファオーバーフロー（他の変数を上書き）
```

### なぜ危険か

C は配列境界チェックを行わない。インデックスが範囲外でも「そのアドレス」を読み書きする。

```
スタック上のメモリ（arr の後ろに別の変数がある場合）:
  arr[0]=1  arr[1]=2  arr[2]=3  arr[3]=4  arr[4]=5 | other_var=99 ...
                                                      ↑
                                               arr[5] = 9 で上書き！
```

バッファオーバーフローは**スタック上のリターンアドレスを上書きする攻撃（Stack Smashing）**の根本原因。

### 対策

```c
// サイズを変数で管理し、必ずループ条件に使う
size_t len = sizeof(arr) / sizeof(arr[0]);
for (size_t i = 0; i < len; i++) {   // < で比較（<= は NG）
    arr[i] = (int)i;
}
```

**検出:** `-fsanitize=address`（AddressSanitizer）

---

## 7. UB-6: 符号付き整数オーバーフロー

```c
int max = INT_MAX;      // 2147483647
int overflow = max + 1; // NG: UB（符号付きオーバーフロー）

uint32_t u = UINT32_MAX;
uint32_t wrap = u + 1;  // OK: 0 にラップアラウンド（well-defined）
```

### なぜ符号付きは UB で、符号なしはよいのか

| 型 | オーバーフロー | 挙動 |
|----|--------------|------|
| `int`（符号付き） | UB | コンパイラが「絶対に起きない」と仮定して最適化する |
| `uint32_t`（符号なし） | well-defined | 2³² でラップアラウンド（C 標準で保証）|

### コンパイラが UB を「最適化」する例

```c
// コンパイラは signed オーバーフローは起きないと仮定できる
// そのため以下のループを無限ループに最適化することがある
for (int i = 0; i >= 0; i++) { ... }
// i が INT_MAX を越えることはない（UBだから）→ 条件が常に真と判断されることがある
```

### 対策

```c
// 加算前にオーバーフローをチェック
if (a <= INT_MAX - b) {
    int sum = a + b;  // 安全
}

// GCC/Clang の組み込み関数（C23以降や処理系拡張）
int result;
if (__builtin_add_overflow(a, b, &result)) {
    // オーバーフロー発生
}
```

**検出:** `-fsanitize=undefined`（UndefinedBehaviorSanitizer）

---

## 8. UB-7: `void*` のポインタ演算

```c
void *vp = arr;
vp + 1;        // NG: void* の演算は UB（型のサイズが不明なため）

// 正しいパターン: char* に変換してバイト単位で計算
char *cp = (char *)vp;
int  *ip = (int *)(cp + sizeof(int));  // 次の int 要素のアドレス
```

### なぜ `void*` で演算できないのか

ポインタ演算は「型のサイズ」を使う。`int *p; p + 1` は `p + sizeof(int)` バイト先を指す。  
`void` にはサイズがないため、コンパイラは移動量を計算できない。

```c
int  *ip; ip + 1;  // OK: sizeof(int) = 4 バイト先
double *dp; dp + 1; // OK: sizeof(double) = 8 バイト先
void *vp; vp + 1;  // NG: sizeof(void) = ? → 不明
```

> **GCC 拡張:** GCC は `void*` を `char*` と同様に扱う拡張があるが、C 標準外。  
> 移植性を保つために `char*` に明示キャストしてから演算する。

---

## 9. 落とし穴: `signed`/`unsigned` の比較

```c
int          i = -1;
unsigned int u = 1;

if (i < u) { ... }  // 危険！ -1 が UINT_MAX (4294967295) に変換される
```

### なぜこうなるのか

C の算術変換規則（暗黙の整数昇格）により、  
`int` と `unsigned int` を比較するとき `int` が `unsigned int` に変換される。

```
-1 を unsigned int に変換:
  -1 の補数表現: 0xFFFFFFFF
  unsigned int として解釈: 4294967295 = UINT_MAX

i < u  →  4294967295 < 1  →  false（期待: -1 < 1 → true）
```

### `size_t` との比較でよくやるミス

```c
size_t len = 5;
int    idx = -1;

if (idx < len)  { ... }  // NG: idx が size_t になり 18446744073709551615 に
if (idx < (int)len) { ... }  // OK: len を int にキャスト（len が INT_MAX 以下なら安全）
if (idx >= 0 && (size_t)idx < len) { ... }  // OK: 負数を先に除外
```

**検出:** `-Wall -Wsign-compare`（コンパイル警告）

---

## 10. 落とし穴: strict aliasing 違反

```c
uint32_t val = 0x12345678;

// NG: 異なる型のポインタで同じメモリにアクセスする（UB）
float *fp = (float *)&val;
printf("%f\n", *fp);  // コンパイラが最適化で壊す可能性がある

// OK: memcpy で型パニングを行う（C 標準で合法）
float f;
memcpy(&f, &val, sizeof(f));  // ビット表現をそのままコピー

// OK: union を使う（C99以降で合法）
union { uint32_t u; float f; } pun = { .u = val };
printf("%f\n", pun.f);
```

### strict aliasing 規則とは

**「異なる型のポインタは同じオブジェクトを指さない」というコンパイラの仮定。**

コンパイラはこの仮定を使って積極的な最適化（レジスタキャッシュなど）を行う。  
`int*` と `float*` が同じメモリを指すことはないと仮定するので、  
一方への書き込みが他方のキャッシュを更新しない最適化が行われることがある。

```c
// コンパイラの視点での最適化例:
int x = 10;
float *fp = (float *)&x;
*fp = 1.0f;            // x に書き込んだが...
printf("%d\n", x);     // コンパイラは「x は変わっていない」と判断して 10 を出力
                       // （*fp の書き込みは別のオブジェクトへの書き込みと見なす）
```

### 例外: `char*` はどんな型のエイリアスにもなれる

```c
// OK: char* / unsigned char* はどんな型のメモリも読み書きできる
unsigned char *bp = (unsigned char *)&val;
for (size_t i = 0; i < sizeof(val); i++) {
    printf("%02X ", bp[i]);  // OK: バイト単位のアクセスは常に合法
}
```

**検出:** `-fstrict-aliasing -Wstrict-aliasing`（コンパイル警告）、`-fsanitize=undefined`

---

## 11. サニタイザの活用

```bash
# AddressSanitizer: メモリ関連の UB を検出
gcc -fsanitize=address -g main.c -o main
./main

# UndefinedBehaviorSanitizer: 算術・型関連の UB を検出
gcc -fsanitize=undefined -g main.c -o main

# 両方同時に使う（推奨）
gcc -fsanitize=address,undefined -g main.c -o main

# MemorySanitizer: 未初期化変数の読み取りを検出（Clang のみ）
clang -fsanitize=memory -g main.c -o main
```

### 各サニタイザが検出できる UB

| サニタイザ | 検出できる UB |
|-----------|--------------|
| `-fsanitize=address` | use-after-free、範囲外アクセス、バッファオーバーフロー |
| `-fsanitize=undefined` | 符号付き整数オーバーフロー、NULL デリファレンス、不正キャスト |
| `-fsanitize=memory` | 未初期化変数の読み取り（Clang のみ）|

> **注意:** サニタイザはランタイムコストが大きい（2〜10x）。  
> **開発・テスト時のみ**有効にして、リリースビルドでは外す。

---

## 12. まとめ：UB 一覧チートシート

```c
// --- UB-1: 未初期化変数 ---
// int x; printf("%d", x);  // NG
int x = 0;                   // 必ず初期化する

// --- UB-2: NULL デリファレンス ---
// *p = 42;   // NG（p が NULL の場合）
if (p != NULL) { *p = 42; }  // 使う前に NULL チェック

// --- UB-3: Use-after-free ---
free(p);
p = NULL;                    // free 後は必ず NULL を代入
// *p = 100; は NULL チェックで防げる

// --- UB-4: 二重解放 ---
free(p); p = NULL;           // NULL を代入 → free(NULL) は安全（C 標準保証）

// --- UB-5: 範囲外アクセス ---
// arr[5]; // NG（arr[5] が範囲外の場合）
for (size_t i = 0; i < len; i++) { arr[i]; }  // インデックスを len 未満に保つ

// --- UB-6: 符号付き整数オーバーフロー ---
// INT_MAX + 1; // NG（UB）
uint32_t u = UINT32_MAX; u + 1;  // OK（符号なしはラップアラウンドが well-defined）
if (a <= INT_MAX - b) { a + b; } // 加算前にオーバーフローをチェック

// --- UB-7: void* 演算 ---
// vp + 1;  // NG
char *cp = (char *)vp; cp + sizeof(int);  // char* にキャストしてバイト演算

// --- 落とし穴: signed/unsigned 比較 ---
// if (i < u) → -1 が UINT_MAX に変換される
if (i < 0 || (size_t)i < len)   // 負数チェックを先に

// --- 落とし穴: strict aliasing ---
// float *fp = (float *)&int_val; *fp;  // NG
float f; memcpy(&f, &int_val, sizeof(f));  // OK: memcpy でビット変換
union { uint32_t u; float f; } pun = {.u = v}; pun.f;  // OK: union 経由

// --- 検出コマンド ---
// gcc -fsanitize=address,undefined -g ...  (開発時は常に有効にする)
```
