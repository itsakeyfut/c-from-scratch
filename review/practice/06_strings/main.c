// String in C

// MSVC runtime on Windows deprecates functions like strcpy/strcat
// This macro suppresses the warnings (intentionally using them)
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>  // strlen, strcpy, strcat, strcmp, strncpy, strstr
#include <stdlib.h>  // malloc, free
#include <ctype.h>   // toupper, tolower, isdigit, isspace

int main(void) {
    // =========================================================
    // Differences between string literals and char arrays
    // =========================================================
    printf("=== Types of Strings ===\n");

    // 1. char array: copied onto the stack → writable
    char arr[] = "hello";
    arr[0] = 'H';
    printf("char arr[]:  %s\n", arr);  // Hello
    printf("sizeof(arr): %zu (including null)\n", sizeof(arr));  // 6

    // 2. char pointer: points to a read-only string literal -> not writable
    const char *ptr = "world";
    printf("const char*: %s\n", ptr);
    // ptr[0] = 'W';  // NG: undefined behavior (may crash)

    // =========================================================
    // strlen — Note that this is O(n)
    // =========================================================
    printf("\n=== strlen ===\n");
    char s[] = "Hello, World!";
    printf("strlen(\"%s\") = %zu\n", s, strlen(s));  // 13 (excluding '\0')
    printf("sizeof(\"%s\") = %zu\n", s, sizeof(s));  // 14 (including '\0')

    // =========================================================
    // strcmp — Cannot compare strings with ==
    // =========================================================
    printf("\n=== strcmp ===\n");
    const char *a = "apple";
    const char *b = "apple";
    const char *c = "banana";
    printf("a == b (pointer comparison): %d (compares addresses)\n", a == b);
    printf("strcmp(a, b) = %d (0 = equal)\n", strcmp(a, b));
    printf("strcmp(a, c) = %d (negative = a < c)\n", strcmp(a, c));
    printf("strcmp(c, a) = %d (positive = c > a)\n", strcmp(c, a));

    // =========================================================
    // strcpy / strncpy — Watch out for buffer overflow
    // =========================================================
    printf("\n=== strcpy / snprintf ===\n");
    char dst[8];

    // strcpy: does not check the size of dst → dangerous
    strcpy(dst, "hello");
    printf("strcpy:   %s\n", dst);

    // strcpy(dst, "toolongstring");  // ← buffer overflow (UB)

    // snprintf: safely copies with size limit
    snprintf(dst, sizeof(dst), "%s", "toolongstring");
    printf("snprintf: %s (truncated)\n", dst);  // "toolong"

    // strncpy pitfall: does not guarantee null termination
    char dst2[5];
    strncpy(dst2, "hello!", sizeof(dst2));
    dst2[sizeof(dst2) - 1] = '\0';  // manually set null terminator
    printf("strncpy:  %s\n", dst2);

    // =========================================================
    // strcat / strncat — Append to the end
    // =========================================================
    printf("\n=== strcat ===\n");
    char buf[32] = "Hello";
    strcat(buf, ", ");
    strcat(buf, "World!");
    printf("strcat: %s\n", buf);

    // =========================================================
    // strstr — Substring search
    // =========================================================
    printf("\n=== strstr ===\n");
    const char *haystack = "the quick brown fox";
    const char *needle   = "brown";
    char *found = strstr(haystack, needle);
    if (found) {
        printf("found '%s' at offset %td\n", needle, found - haystack);
    }

    // =========================================================
    // Character operations (ctype.h)
    // =========================================================
    printf("\n=== ctype.h ===\n");
    char word[] = "Hello World 123";
    for (int i = 0; word[i] != '\0'; i++) {
        word[i] = (char)tolower((unsigned char)word[i]);  // unsigned char cast is required
    }
    printf("tolower: %s\n", word);

    printf("isdigit('5'): %d\n", isdigit('5'));   // non-zero
    printf("isspace(' '): %d\n", isspace(' '));   // non-zero
    printf("isalpha('a'): %d\n", isalpha('a'));   // non-zero

    // =========================================================
    // Allocate a string on the heap
    // =========================================================
    printf("\n=== String on the Heap ===\n");
    const char *src = "dynamic string";
    size_t len = strlen(src);
    char *dup = malloc(len + 1);  // +1 for '\0'
    if (dup) {
        memcpy(dup, src, len + 1);
        printf("dup: %s\n", dup);
        free(dup);
    }

    // =========================================================
    // Build a formatted string with sprintf / snprintf
    // =========================================================
    printf("\n=== Formatting with snprintf ===\n");
    char result[64];
    int  code  = 404;
    const char *msg = "Not Found";
    snprintf(result, sizeof(result), "HTTP %d %s", code, msg);
    printf("%s\n", result);

    // =========================================================
    // Treating strings as byte sequences (when null bytes are present)
    // =========================================================
    printf("\n=== strlen is not binary-safe ===\n");
    // strlen stops at the first null byte, even if there is more data after it
    unsigned char binary[] = {0x61, 0x00, 0x62, 0x63};  // 'a', NUL, 'b', 'c'
    printf("strlen: %zu (stops at NUL)\n", strlen((char *)binary));  // 1
    printf("actual byte count: %zu\n", sizeof(binary));               // 4
    // → binary data requires tracking length separately

    return 0;
}
