// mylib.c — ヘッダに対応する実装ファイル
//
// ルール:
// ① 必ず対応するヘッダを最初にインクルードする（自己整合性のチェック）
// ② 外部に公開しない関数は static をつける
// ③ グローバル変数は極力使わない

#include "mylib.h"   // 対応ヘッダを先頭で
#include <stdio.h>
#include <math.h>    // sqrt

// =========================================================
// static（非公開）ヘルパー関数
// mylib.h には書かない → このファイル外から呼べない
// =========================================================
static int clamp(int val, int min, int max) {
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

// =========================================================
// 公開関数の実装
// =========================================================
Point point_new(int x, int y) {
    return (Point){ clamp(x, -POINT_MAX, POINT_MAX),
                    clamp(y, -POINT_MAX, POINT_MAX) };
}

Point point_add(Point a, Point b) {
    return point_new(a.x + b.x, a.y + b.y);
}

double point_dist(Point a, Point b) {
    double dx = (double)(a.x - b.x);
    double dy = (double)(a.y - b.y);
    return sqrt(dx * dx + dy * dy);
}

bool point_eq(Point a, Point b) {
    return a.x == b.x && a.y == b.y;
}

void point_print(const Point *p) {
    printf("(%d, %d)", p->x, p->y);
}
