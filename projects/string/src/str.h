#ifndef STR_H
#define STR_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#define STR_INITIAL_CAP 16  // 最初の grow 時に確保するバイト数（'\0' 込み）

typedef struct {
    char   *data;  // data[len] == '\0' を常に保証。未確保時は NULL
    size_t len;    // '\0' を除いたバイト数
    size_t cap;    // data に確保済みのバイト数（'\0' の 1 バイトを含む）
} Str;

void str_init(Str *s);
int  str_init_from(Str *s, const char *cstr);
void str_free(Str *s);

#define str_cstr(s)  ((s)->data ? (s)->data : "")

size_t str_len_bytes(const Str *s);
size_t str_len_utf8(const Str *s);
int str_append(Str *s, const char *src);
int str_append_str(Str *s, const Str *other);
int str_append_char(Str *s, char c);
int str_insert(Str *s, size_t byte_i, const char *src);
void str_clear(Str *s);
int str_reserve(Str *s, size_t new_cap);
bool str_eq(const Str *a, const Str *b);
int str_cmp(const Str *a, const Str *b);
ptrdiff_t str_find(const Str *s, const char *needle);
bool str_starts_with(const Str *s, const char *prefix);
bool str_ends_with(const Str *s, const char *suffix);
void str_to_upper(Str *s);
void str_to_lower(Str *s);
void str_trim(Str *s);
bool str_is_valid_utf8(const Str *s);
int str_next_codepoint(const char *p, uint32_t *out_cp);

#endif // STR_H