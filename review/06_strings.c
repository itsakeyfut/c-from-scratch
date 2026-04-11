// 06_strings.c — null終端文字列
// ビルド: clang -std=c11 -Wall -Wextra -o 06_strings 06_strings.c && ./06_strings

// Windows の MSVC ランタイムは strcpy/strcat 等を非推奨扱いにする
// このマクロで警告を抑止する（意図的に使うため）
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>  // strlen, strcpy, strcat, strcmp, strncpy, strstr
#include <stdlib.h>  // malloc, free
#include <ctype.h>   // toupper, tolower, isdigit, isspace

int main(void) {

    // =========================================================
    // 文字列リテラルと char 配列の違い
    // =========================================================
    printf("=== 文字列の種類 ===\n");

    // ① char 配列: スタック上にコピーされる → 書き込み可能
    char arr[] = "hello";
    arr[0] = 'H';
    printf("char arr[]:  %s\n", arr);  // Hello
    printf("sizeof(arr): %zu（null含む）\n", sizeof(arr));  // 6

    // ② char ポインタ: 読み取り専用の文字列リテラルを指す → 書き込み禁止
    const char *ptr = "world";
    printf("const char*: %s\n", ptr);
    // ptr[0] = 'W';  // NG: 未定義動作（クラッシュする可能性）

    // =========================================================
    // strlen — O(n) であることに注意
    // =========================================================
    printf("\n=== strlen ===\n");
    char s[] = "Hello, World!";
    printf("strlen(\"%s\") = %zu\n", s, strlen(s));  // 13（'\0'は含まない）
    printf("sizeof(\"%s\") = %zu\n", s, sizeof(s));  // 14（'\0'含む）

    // =========================================================
    // strcmp — == では文字列を比較できない
    // =========================================================
    printf("\n=== strcmp ===\n");
    const char *a = "apple";
    const char *b = "apple";
    const char *c = "banana";
    printf("a == b（ポインタ比較）: %d（アドレスを比較）\n", a == b);
    printf("strcmp(a, b) = %d（0 = 等しい）\n", strcmp(a, b));
    printf("strcmp(a, c) = %d（負 = a < c）\n", strcmp(a, c));
    printf("strcmp(c, a) = %d（正 = c > a）\n", strcmp(c, a));

    // =========================================================
    // strcpy / strncpy — バッファオーバーフローに注意
    // =========================================================
    printf("\n=== strcpy / snprintf ===\n");
    char dst[8];

    // strcpy: dst のサイズを確認しない → 危険
    strcpy(dst, "hello");
    printf("strcpy:   %s\n", dst);

    // strcpy(dst, "toolongstring");  // ← バッファオーバーフロー（UB）

    // snprintf: サイズを指定して安全にコピー
    snprintf(dst, sizeof(dst), "%s", "toolongstring");
    printf("snprintf: %s（切り詰め）\n", dst);  // "toolong"

    // strncpy の罠: null終端を保証しない
    char dst2[5];
    strncpy(dst2, "hello!", sizeof(dst2));
    dst2[sizeof(dst2) - 1] = '\0';  // 手動で null 終端を設定する
    printf("strncpy:  %s\n", dst2);

    // =========================================================
    // strcat / strncat — 末尾に追加
    // =========================================================
    printf("\n=== strcat ===\n");
    char buf[32] = "Hello";
    strcat(buf, ", ");
    strcat(buf, "World!");
    printf("strcat: %s\n", buf);

    // =========================================================
    // strstr — 部分文字列検索
    // =========================================================
    printf("\n=== strstr ===\n");
    const char *haystack = "the quick brown fox";
    const char *needle   = "brown";
    char *found = strstr(haystack, needle);
    if (found) {
        printf("found '%s' at offset %td\n", needle, found - haystack);
    }

    // =========================================================
    // 文字操作（ctype.h）
    // =========================================================
    printf("\n=== ctype.h ===\n");
    char word[] = "Hello World 123";
    for (int i = 0; word[i] != '\0'; i++) {
        word[i] = (char)tolower((unsigned char)word[i]);  // unsigned char キャストが必要
    }
    printf("tolower: %s\n", word);

    printf("isdigit('5'): %d\n", isdigit('5'));   // 非0
    printf("isspace(' '): %d\n", isspace(' '));   // 非0
    printf("isalpha('a'): %d\n", isalpha('a'));   // 非0

    // =========================================================
    // ヒープ上に文字列を確保する
    // =========================================================
    printf("\n=== ヒープ上の文字列 ===\n");
    const char *src = "dynamic string";
    size_t len = strlen(src);
    char *dup = malloc(len + 1);  // +1 は '\0'
    if (dup) {
        memcpy(dup, src, len + 1);
        printf("dup: %s\n", dup);
        free(dup);
    }

    // =========================================================
    // sprintf / snprintf でフォーマット文字列を構築
    // =========================================================
    printf("\n=== snprintf でフォーマット ===\n");
    char result[64];
    int  code  = 404;
    const char *msg = "Not Found";
    snprintf(result, sizeof(result), "HTTP %d %s", code, msg);
    printf("%s\n", result);

    // =========================================================
    // 文字列をバイト列として扱う（null 文字が含まれる場合）
    // =========================================================
    printf("\n=== バイナリ安全でない strlen ===\n");
    // null が途中にあると strlen は最初の null で止まる
    unsigned char binary[] = {0x61, 0x00, 0x62, 0x63};  // 'a', NUL, 'b', 'c'
    printf("strlen: %zu (NUL で止まる)\n", strlen((char *)binary));  // 1
    printf("実際のバイト数: %zu\n", sizeof(binary));                  // 4
    // → バイナリデータは長さを別途管理する必要がある

    return 0;
}
