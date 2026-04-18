#include "vec.h"
#include <stdio.h>
#include <string.h>

// ==== Internal Helpers ====

static void print_ints(const char *label, Vec *v) {
    printf("  %-20s [", label);
    for (size_t i = 0; i < vec_len(v); i++) {
        if (i) printf(", ");
        printf("%d", *(int *)vec_get(v, i));
    }
    printf("]  len=%zu cap=%zu\n", vec_len(v), v->cap);
}

static int cmp_int_asc(const void *a, const void *b) {
    return *(const int *)a - *(const int *)b;
}

typedef struct { char name[16]; int score; } Player;

static void print_players(const char *label, Vec *v) {
    printf("  %-20s [", label);
    for (size_t i = 0; i < vec_len(v); i++) {
        if (i) printf(", ");
        Player *p = vec_get(v, i);
        printf("%s:%d", p->name, p->score);
    }
    printf("]\n");
}

// ============================================================
// 1. 基本: push / get / pop
// ============================================================
static void demo_basics(void) {
    printf("=== 1. push / get / pop ===\n");
    Vec v;
    vec_init(&v, sizeof(int));

    int vals[] = {10, 20, 30, 40, 50};
    for (int i = 0; i < 5; i++) vec_push(&v, &vals[i]);
    print_ints("push x5", &v);

    int out;
    vec_pop(&v, &out);
    printf("  pop → %d\n", out);
    print_ints("after pop", &v);

    vec_pop(&v, NULL);   // 値を捨てて削除だけ
    print_ints("pop(NULL)", &v);

    vec_free(&v);
    printf("\n");
}

// ============================================================
// 2. 成長: cap の推移を観察する
// ============================================================
static void demo_grow(void) {
    printf("=== 2. 成長: cap の推移 ===\n");
    Vec v;
    vec_init(&v, sizeof(int));
    printf("  start         cap=%zu\n", v.cap);

    int x = 0;
    for (int i = 0; i < 20; i++) {
        size_t cap_before = v.cap;
        vec_push(&v, &x);
        if (v.cap != cap_before)
            printf("  push[%2d] grow  cap %zu → %zu\n", i + 1, cap_before, v.cap);
    }
    printf("  final         len=%zu cap=%zu\n", vec_len(&v), v.cap);
    vec_free(&v);
    printf("\n");
}

// ============================================================
// 3. reserve: 事前確保で realloc を抑制
// ============================================================
static void demo_reserve(void) {
    printf("=== 3. reserve ===\n");
    Vec v;
    vec_init(&v, sizeof(int));

    vec_reserve(&v, 100);
    printf("  reserve(100)  cap=%zu\n", v.cap);

    int x = 0;
    size_t realloc_count = 0;
    for (int i = 0; i < 100; i++) {
        size_t cap_before = v.cap;
        vec_push(&v, &x);
        if (v.cap != cap_before) realloc_count++;
    }
    printf("  100 pushes    len=%zu  realloc_count=%zu\n", vec_len(&v), realloc_count);
    vec_free(&v);
    printf("\n");
}

// ============================================================
// 4. insert / remove: 順序を保つ操作
// ============================================================
static void demo_insert_remove(void) {
    printf("=== 4. insert / remove ===\n");
    Vec v;
    vec_init(&v, sizeof(int));

    int init[] = {1, 2, 4, 5};
    for (int i = 0; i < 4; i++) vec_push(&v, &init[i]);
    print_ints("start", &v);

    int three = 3;
    vec_insert(&v, 2, &three);   // [1,2,4,5] → [1,2,3,4,5]
    print_ints("insert(2, 3)", &v);

    int zero = 0;
    vec_insert(&v, 0, &zero);    // 先頭挿入
    print_ints("insert(0, 0)", &v);

    vec_remove(&v, 0);           // 先頭削除
    print_ints("remove(0)", &v);

    vec_remove(&v, 3);           // 中間削除 [1,2,3,5] → [1,2,3,5]? 4 を消す
    print_ints("remove(3)", &v);

    vec_free(&v);
    printf("\n");
}

// ============================================================
// 5. swap_remove: O(1) 削除（順序不要なとき）
// ============================================================
static void demo_swap_remove(void) {
    printf("=== 5. swap_remove (順序不保証・O(1)) ===\n");
    Vec v;
    vec_init(&v, sizeof(int));

    int init[] = {10, 20, 30, 40, 50};
    for (int i = 0; i < 5; i++) vec_push(&v, &init[i]);
    print_ints("start", &v);

    vec_swap_remove(&v, 1);      // index=1(20) を削除 → 末尾(50)がその位置へ
    print_ints("swap_remove(1)", &v);

    vec_swap_remove(&v, 3);      // 末尾を削除 → ただの len--
    print_ints("swap_remove(3)", &v);

    vec_free(&v);
    printf("\n");
}

// ============================================================
// 6. sort: qsort ラッパー
// ============================================================
static void demo_sort(void) {
    printf("=== 6. sort ===\n");
    Vec v;
    vec_init(&v, sizeof(int));

    int init[] = {42, 7, 99, 1, 55, 23};
    for (int i = 0; i < 6; i++) vec_push(&v, &init[i]);
    print_ints("before", &v);

    vec_sort(&v, cmp_int_asc);
    print_ints("after", &v);

    vec_free(&v);
    printf("\n");
}

// ============================================================
// 7. 構造体の格納: sizeof(Player) で動く汎用性
// ============================================================
static void demo_struct(void) {
    printf("=== 7. 構造体 (Player) ===\n");
    Vec v;
    vec_init(&v, sizeof(Player));

    Player players[] = {
        {"Alice",  95},
        {"Bob",    72},
        {"Carol", 108},
        {"Dave",   60},
    };
    for (int i = 0; i < 4; i++) vec_push(&v, &players[i]);
    print_players("start", &v);

    // Dave (index=3) を swap_remove で削除
    vec_swap_remove(&v, 3);
    print_players("rm Dave", &v);

    // Carol (index=2) のスコアを上書き
    Player carol_updated = {"Carol", 120};
    vec_set(&v, 2, &carol_updated);
    print_players("upd Carol", &v);

    vec_free(&v);
    printf("\n");
}

// ============================================================
// 8. clear: バッファを保持したままリセット
// ============================================================
static void demo_clear(void) {
    printf("=== 8. clear ===\n");
    Vec v;
    vec_init(&v, sizeof(int));

    int x = 0;
    for (int i = 0; i < 16; i++) vec_push(&v, &x);
    printf("  before clear  len=%zu cap=%zu\n", vec_len(&v), v.cap);

    vec_clear(&v);
    printf("  after  clear  len=%zu cap=%zu  (バッファは保持)\n", vec_len(&v), v.cap);

    // 再利用: realloc なしで再び push できる
    for (int i = 1; i <= 5; i++) vec_push(&v, &i);
    print_ints("re-push x5", &v);

    vec_free(&v);
    printf("\n");
}

// ============================================================
// main
// ============================================================
int main(void) {
    demo_basics();
    demo_grow();
    demo_reserve();
    demo_insert_remove();
    demo_swap_remove();
    demo_sort();
    demo_struct();
    demo_clear();
    return 0;
}
