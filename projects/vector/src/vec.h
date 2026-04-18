#ifndef VEC_H
#define VEC_H

#include <stddef.h>

#define VEC_INITIAL_CAP 8

typedef struct {
    void   *data;
    size_t  len;
    size_t  cap;
    size_t  elem_size;
} Vec;

void   vec_init       (Vec *v, size_t elem_size);
void   vec_free       (Vec *v);
int    vec_push       (Vec *v, const void *elem);
int    vec_pop        (Vec *v, void *out);
void  *vec_get        (const Vec *v, size_t i);
int    vec_set        (Vec *v, size_t i, const void *elem);
int    vec_reserve    (Vec *v, size_t new_cap);
void   vec_clear      (Vec *v);
size_t vec_len        (const Vec *v);
int    vec_insert     (Vec *v, size_t i, const void *elem);
int    vec_remove     (Vec *v, size_t i);
int    vec_swap_remove(Vec *v, size_t i);
void   vec_sort       (Vec *v, int (*cmp)(const void *, const void *));

#endif // VEC_H
