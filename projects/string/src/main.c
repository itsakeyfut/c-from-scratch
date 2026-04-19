#include "str.h"
#include <stdio.h>

// ==== Internal Helpers ====

static void print_str(const char *label, const Str *s) {
    printf("  %-24s \"%s\"  len=%zu cap=%zu\n",
           label, str_cstr(s), s->len, s->cap);
}

// ============================================================
// 1. 基本: init / init_from / free
// ============================================================
static void demo_init(void) {
    printf("=== 1. init / init_from / free ===\n");
    Str s;
    str_init(&s);
    print_str("str_init", &s);

    str_init_from(&s, "hello");
    print_str("init_from(\"hello\")", &s);

    str_free(&s);
    printf("  str_free → data=%s len=%zu cap=%zu\n",
           s.data ? s.data : "(null)", s.len, s.cap);
    printf("\n");
}

// ============================================================
// 2. 長さ: len_bytes / len_utf8
// ============================================================
static void demo_len(void) {
    printf("=== 2. len_bytes / len_utf8 ===\n");
    Str s;
    str_init_from(&s, "hello");
    printf("  \"hello\"         bytes=%zu  utf8=%zu\n",
           str_len_bytes(&s), str_len_utf8(&s));
    str_free(&s);

    str_init_from(&s, "あいう");          // 3文字 × 3バイト = 9バイト
    printf("  \"あいう\"        bytes=%zu  utf8=%zu\n",
           str_len_bytes(&s), str_len_utf8(&s));
    str_free(&s);
    printf("\n");
}

// ============================================================
// 3. 追記: append / append_str / append_char
// ============================================================
static void demo_append(void) {
    printf("=== 3. append / append_str / append_char ===\n");
    Str s;
    str_init(&s);

    str_append(&s, "foo");
    print_str("append(\"foo\")", &s);

    str_append(&s, "bar");
    print_str("append(\"bar\")", &s);

    Str other;
    str_init_from(&other, "baz");
    str_append_str(&s, &other);
    print_str("append_str(\"baz\")", &s);

    str_append_char(&s, '!');
    print_str("append_char('!')", &s);

    // 自己追記
    str_append_str(&s, &s);
    print_str("append_str(self)", &s);

    str_free(&s);
    str_free(&other);
    printf("\n");
}

// ============================================================
// 4. 挿入: insert
// ============================================================
static void demo_insert(void) {
    printf("=== 4. insert ===\n");
    Str s;
    str_init_from(&s, "helloworld");
    print_str("start", &s);

    str_insert(&s, 5, ", ");
    print_str("insert(5, \", \")", &s);

    str_insert(&s, 0, ">>> ");
    print_str("insert(0, \">>> \")", &s);

    str_free(&s);
    printf("\n");
}

// ============================================================
// 5. clear / reserve
// ============================================================
static void demo_clear_reserve(void) {
    printf("=== 5. clear / reserve ===\n");
    Str s;
    str_init_from(&s, "hello");
    print_str("before clear", &s);

    str_clear(&s);
    print_str("after clear", &s);

    str_append(&s, "reused");
    print_str("after reuse", &s);

    str_reserve(&s, 128);
    printf("  reserve(128)          cap=%zu\n", s.cap);

    str_free(&s);
    printf("\n");
}

// ============================================================
// 6. 比較: eq / cmp / find / starts_with / ends_with
// ============================================================
static void demo_compare(void) {
    printf("=== 6. eq / cmp / find / starts_with / ends_with ===\n");
    Str a, b;
    str_init_from(&a, "hello");
    str_init_from(&b, "hello");
    printf("  eq(\"hello\",\"hello\")   → %s\n", str_eq(&a, &b) ? "true" : "false");

    str_free(&b);
    str_init_from(&b, "world");
    printf("  eq(\"hello\",\"world\")   → %s\n", str_eq(&a, &b) ? "true" : "false");
    printf("  cmp(\"hello\",\"world\")  → %d\n", str_cmp(&a, &b));
    printf("  cmp(\"world\",\"hello\")  → %d\n", str_cmp(&b, &a));

    str_free(&a);
    str_init_from(&a, "hello, world");
    printf("  find(\"world\")         → %td\n", str_find(&a, "world"));
    printf("  find(\"xyz\")           → %td\n", str_find(&a, "xyz"));

    printf("  starts_with(\"hello\")  → %s\n", str_starts_with(&a, "hello") ? "true" : "false");
    printf("  starts_with(\"world\")  → %s\n", str_starts_with(&a, "world") ? "true" : "false");
    printf("  ends_with(\"world\")    → %s\n", str_ends_with(&a, "world") ? "true" : "false");
    printf("  ends_with(\"hello\")    → %s\n", str_ends_with(&a, "hello") ? "true" : "false");

    str_free(&a);
    str_free(&b);
    printf("\n");
}

// ============================================================
// 7. 変換: to_upper / to_lower / trim
// ============================================================
static void demo_transform(void) {
    printf("=== 7. to_upper / to_lower / trim ===\n");
    Str s;
    str_init_from(&s, "Hello, World!");
    print_str("start", &s);

    str_to_upper(&s);
    print_str("to_upper", &s);

    str_to_lower(&s);
    print_str("to_lower", &s);

    str_free(&s);
    str_init_from(&s, "  \t hello \n  ");
    print_str("before trim", &s);

    str_trim(&s);
    print_str("after trim", &s);

    str_free(&s);
    printf("\n");
}

// ============================================================
// 8. UTF-8: is_valid_utf8 / next_codepoint
// ============================================================
static void demo_utf8(void) {
    printf("=== 8. UTF-8 ===\n");
    Str s;

    str_init_from(&s, "hello");
    printf("  is_valid_utf8(\"hello\")  → %s\n", str_is_valid_utf8(&s) ? "true" : "false");
    str_free(&s);

    str_init_from(&s, "あいう");
    printf("  is_valid_utf8(\"あいう\") → %s\n", str_is_valid_utf8(&s) ? "true" : "false");
    str_free(&s);

    // 不正バイト列 (0xFF は UTF-8 不正)
    Str bad;
    str_init(&bad);
    str_append_char(&bad, (char)0xFF);
    printf("  is_valid_utf8(0xFF)     → %s\n", str_is_valid_utf8(&bad) ? "true" : "false");
    str_free(&bad);

    // next_codepoint
    const char *p = "A";
    uint32_t cp;
    int n = str_next_codepoint(p, &cp);
    printf("  next_cp(\"A\")            → U+%04X  bytes=%d\n", cp, n);

    p = "あ";  // E3 81 82
    n = str_next_codepoint(p, &cp);
    printf("  next_cp(\"あ\")           → U+%04X  bytes=%d\n", cp, n);

    printf("\n");
}

// ============================================================
// main
// ============================================================
int main(void) {
    demo_init();
    demo_len();
    demo_append();
    demo_insert();
    demo_clear_reserve();
    demo_compare();
    demo_transform();
    demo_utf8();
    return 0;
}
