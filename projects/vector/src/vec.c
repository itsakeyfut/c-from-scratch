#include "vec.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

// ==== Internal Helpers ====

// OOM 時に v->data を壊さないよう一時変数を経由する。
// 直接 v->data = realloc(v->data, ...) と書くと NULL 代入でリークする。
static int vec_grow(Vec *v) {
    size_t new_cap = (v->cap == 0) ? VEC_INITIAL_CAP : v->cap * 2;
    void  *tmp     = realloc(v->data, new_cap * v->elem_size);
    if (!tmp) return -1;
    v->data = tmp;
    v->cap  = new_cap;
    return 0;
}

// ==== Public API ====

void vec_init(Vec *v, size_t elem_size) {
    assert(v != NULL);
    assert(elem_size > 0);
    v->data      = NULL;
    v->len       = 0;
    v->cap       = 0;
    v->elem_size = elem_size;
}

void vec_free(Vec *v) {
    assert(v != NULL);
    free(v->data);
    v->data = NULL;
    v->len  = 0;
    v->cap  = 0;
}

int vec_push(Vec *v, const void *elem) {
    assert(v != NULL);
    assert(elem != NULL);
    if (v->len == v->cap && vec_grow(v) != 0) return -1;
    memcpy((char *)v->data + v->len * v->elem_size, elem, v->elem_size);
    v->len++;
    return 0;
}

int vec_pop(Vec *v, void *out) {
    assert(v != NULL);
    if (v->len == 0) return -1;
    v->len--;
    if (out) memcpy(out, (char *)v->data + v->len * v->elem_size, v->elem_size);
    return 0;
}

// void * への加算は UB なので char * にキャストしてオフセット計算する。
void *vec_get(const Vec *v, size_t i) {
    assert(v != NULL);
    if (i >= v->len) return NULL;
    return (char *)v->data + i * v->elem_size;
}

int vec_set(Vec *v, size_t i, const void *elem) {
    assert(v != NULL);
    assert(elem != NULL);
    if (i >= v->len) return -1;
    memcpy((char *)v->data + i * v->elem_size, elem, v->elem_size);
    return 0;
}

int vec_reserve(Vec *v, size_t new_cap) {
    assert(v != NULL);
    if (new_cap <= v->cap) return 0;
    void *tmp = realloc(v->data, new_cap * v->elem_size);
    if (!tmp) return -1;
    v->data = tmp;
    v->cap  = new_cap;
    return 0;
}

void vec_clear(Vec *v) {
    assert(v != NULL);
    v->len = 0;
}

size_t vec_len(const Vec *v) {
    assert(v != NULL);
    return v->len;
}

int vec_insert(Vec *v, size_t i, const void *elem) {
    assert(v != NULL);
    assert(elem != NULL);
    if (i > v->len) return -1;
    if (v->len == v->cap && vec_grow(v) != 0) return -1;
    // [i..len-1] を右に 1 スロットずらす。コピー元と先が重なるので memmove。
    // i == len のとき size=0 なので memmove は何もしない（末尾追記と同じ）。
    memmove((char *)v->data + (i + 1) * v->elem_size,
            (char *)v->data + i       * v->elem_size,
            (v->len - i) * v->elem_size);
    memcpy((char *)v->data + i * v->elem_size, elem, v->elem_size);
    v->len++;
    return 0;
}

int vec_remove(Vec *v, size_t i) {
    assert(v != NULL);
    if (i >= v->len) return -1;
    // [i+1..len-1] を左に 1 スロットずらす。i が末尾なら size=0 で no-op。
    memmove((char *)v->data + i       * v->elem_size,
            (char *)v->data + (i + 1) * v->elem_size,
            (v->len - i - 1) * v->elem_size);
    v->len--;
    return 0;
}

int vec_swap_remove(Vec *v, size_t i) {
    assert(v != NULL);
    if (i >= v->len) return -1;
    v->len--;
    // len-- 後に i == v->len なら i は元から末尾だった → コピー不要。
    if (i != v->len)
        memcpy((char *)v->data + i      * v->elem_size,
               (char *)v->data + v->len * v->elem_size,
               v->elem_size);
    return 0;
}

void vec_sort(Vec *v, int (*cmp)(const void *, const void *)) {
    assert(v != NULL);
    assert(cmp != NULL);
    qsort(v->data, v->len, v->elem_size, cmp);
}
