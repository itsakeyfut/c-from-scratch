#include "str.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// ==== Internal Helpers ====
static int str_grow(Str *s, size_t min_cap) {
    size_t doubled = (s->cap == 0) ? STR_INITIAL_CAP : s->cap * 2;
    size_t new_cap = (min_cap > doubled) ? min_cap : doubled;
    char *tmp = realloc(s->data, new_cap);
    if (!tmp) return -1;
    s->data = tmp;
    s->cap = new_cap;
    return 0;
}

// ==== Public API ====
void str_init(Str *s) {
    assert(s != NULL);
    s->data = NULL;
    s->len  = 0;
    s->cap  = 0;
}

int str_init_from(Str *s, const char *cstr) {
    assert(s != NULL);
    assert(cstr != NULL);
    str_init(s);
    size_t len = strlen(cstr);
    if (str_grow(s, len + 1) != 0) return -1;
    memcpy(s->data, cstr, len);
    s->len = len;
    s->data[s->len] = '\0';
    return 0;
}

void str_free(Str *s) {
    assert(s != NULL);
    free(s->data);
    s->data = NULL;
    s->len = 0;
    s->cap = 0;
}

size_t str_len_bytes(const Str *s) {
    assert(s != NULL);
    return s->len;
}

size_t str_len_utf8(const Str *s) {
    assert(s != NULL);
    size_t count = 0;
    for (size_t i = 0; i < s->len; i++) {
        if (((unsigned char)s->data[i] & 0xC0) != 0x80) count++;
    }
    return count;
}

int str_append(Str *s, const char *src) {
    assert(s != NULL);
    assert(src != NULL);
    size_t src_len = strlen(src);
    if (str_grow(s, s->len + src_len + 1) != 0) return -1;
    memcpy(s->data + s->len, src, src_len);
    s->len += src_len;
    s->data[s->len] = '\0';
    return 0;
}

int str_append_str(Str *s, const Str *other) {
    assert(s != NULL);
    assert(other != NULL);
    size_t other_len = other->len; // grow 前に保存（自己追加対策）
    if (str_grow(s, s->len + other_len + 1) != 0) return -1;
    memcpy(s->data + s->len, other->data, other_len);
    s->len += other_len;
    s->data[s->len] = '\0';
    return 0;
}

int str_append_char(Str *s, char c) {
    assert(s != NULL);
    if (str_grow(s, s->len + 2) != 0) return -1; // +2: c の 1 バイト + '\0'
    s->data[s->len] = c;
    s->len++;
    s->data[s->len] = '\0';
    return 0;
}

int str_insert(Str *s, size_t byte_i, const char *src) {
    assert(s != NULL);
    assert(src != NULL);
    if (byte_i > s->len) return -1;
    size_t src_len = strlen(src);
    if (str_grow(s, s->len + src_len + 1) != 0) return -1;
    memmove(s->data + byte_i + src_len,
            s->data + byte_i,
            s->len - byte_i);
    memcpy(s->data + byte_i, src, src_len);
    s->len += src_len;
    s->data[s->len] = '\0';
    return 0;
}

void str_clear(Str *s) {
    assert(s != NULL);
    s->len = 0;
    if (s->data) s->data[0] = '\0';
}

int str_reserve(Str *s, size_t new_cap) {
    assert(s != NULL);
    if (new_cap <= s->cap) return 0;
    char *tmp = realloc(s->data, new_cap);
    if (!tmp) return -1;
    s->data = tmp;
    s->cap = new_cap;
    return 0;
}

bool str_eq(const Str *a, const Str *b) {
    assert(a != NULL);
    assert(b != NULL);
    if (a->len != b->len) return false;
    return memcmp(a->data, b->data, a->len) == 0;
}

int str_cmp(const Str *a, const Str *b) {
    assert(a != NULL);
    assert(b != NULL);
    size_t min_len = (a->len < b->len) ? a->len : b->len;
    int r = memcmp(a->data, b->data, min_len);
    if (r != 0) return r;  // 共通部分で差があればその結果を返す
    if (a->len < b->len) return -1;
    if (a->len > b->len) return 1;
    return 0;
}

ptrdiff_t str_find(const Str *s, const char *needle) {
    assert(s != NULL);
    assert(needle != NULL);
    size_t needle_len = strlen(needle);
    if (needle_len == 0) return 0;
    if (needle_len > s->len) return -1;
    for (size_t i = 0; i <= s->len - needle_len; i++) {
        if (memcmp(s->data + i, needle, needle_len) == 0)
            return (ptrdiff_t)i;
    }
    return -1;
}

bool str_starts_with(const Str *s, const char *prefix) {
    assert(s != NULL);
    assert(prefix != NULL);
    size_t prefix_len = strlen(prefix);
    if (prefix_len == 0) return true;
    if (prefix_len > s->len) return false;
    return memcmp(s->data, prefix, prefix_len) == 0;
}

bool str_ends_with(const Str *s, const char *suffix) {
    assert(s != NULL);
    assert(suffix != NULL);
    size_t suffix_len = strlen(suffix);
    if (suffix_len == 0) return true;
    if (suffix_len > s->len) return false;
    return memcmp(s->data + s->len - suffix_len, suffix, suffix_len) == 0;
}

void str_to_upper(Str *s) {
    assert(s != NULL);
    for (size_t i = 0; i < s->len; i++) {
        s->data[i] = (char)toupper((unsigned char)s->data[i]);
    }
}

void str_to_lower(Str *s) {
    assert(s != NULL);
    for (size_t i = 0; i < s->len; i++) {
        s->data[i] = (char)tolower((unsigned char)s->data[i]);
    }
}

void str_trim(Str *s) {
    assert(s != NULL);
    // 末尾
    while (s->len > 0 && isspace((unsigned char)s->data[s->len - 1])) {
        s->len--;
    }
    s->data[s->len] = '\0';
    // 先頭
    size_t start = 0;
    while (start < s->len && isspace((unsigned char)s->data[start])) {
        start++;
    }
    memmove(s->data, s->data + start, s->len - start);
    s->len -= start;
    s->data[s->len] = '\0';
}

bool str_is_valid_utf8(const Str *s) {
    assert(s != NULL);
    for (size_t i = 0; i < s->len; i++) {
        unsigned char b = (unsigned char)s->data[i];
        int extra;  // 続く継続バイトの数

        if      ((b & 0x80) == 0x00) extra = 0;  // 0xxxxxxx -> ASCII
        else if ((b & 0xE0) == 0xC0) extra = 1;  // 110xxxxx -> 2バイト
        else if ((b & 0xF0) == 0xE0) extra = 2;  // 1110xxxx -> 3バイト
        else if ((b & 0xF8) == 0xF0) extra = 3;  // 11110xxx -> 4バイト
        else return false;                       // 10xxxxxx -> 先頭バイトとして無効

        for (int j = 0; j < extra; j++) {
            i++;
            if (i >= s->len) return false;  // 継続バイトが足りない
            if (((unsigned char)s->data[i] & 0xC0) != 0x80) return false;  // 継続バイトでない
        }
    }
    return true;
}

int str_next_codepoint(const char *p, uint32_t *out_cp) {
    assert(p != NULL);
    assert(out_cp != NULL);
    unsigned char b = (unsigned char)p[0];

    if ((b & 0x80) == 0x00) {
        *out_cp = b; // 1バイト（ASCII）
        return 1;
    } else if ((b & 0xE0) == 0xC0) {
        unsigned char b2 = (unsigned char)p[1];
        if ((b2 & 0xC0) != 0x80) return -1;  // 継続バイトでない
        *out_cp = ((uint32_t)(b & 0x1F) << 6) | (b2 & 0x3F);
        return 2;
    } else if ((b & 0xF0) == 0xE0) {
        unsigned char b2 = (unsigned char)p[1];
        unsigned char b3 = (unsigned char)p[2];
        if ((b2 & 0xC0) != 0x80 || (b3 & 0xC0) != 0x80) return -1;  // 継続バイトでない
        *out_cp = ((uint32_t)(b &0x0F) << 12) | ((uint32_t)(b2 & 0x3F) << 6) | (b3 & 0x3F);
        return 3;
    } else if ((b & 0xF8) == 0xF0) {
        unsigned char b2 = (unsigned char)p[1];
        unsigned char b3 = (unsigned char)p[2];
        unsigned char b4 = (unsigned char)p[3];
        if ((b2 & 0xC0) != 0x80 || (b3 & 0xC0) != 0x80 || (b4 &0xC0) != 0x80) return -1;  // 継続バイトでない
        *out_cp = ((uint32_t)(b &0x07) << 18) | ((uint32_t)(b2 & 0x3F) << 12) | ((uint32_t)(b3 & 0x3F) << 6) | (b4 & 0x3F);
        return 4;
    }
    return -1;  // 不正な先頭バイト
}