// mylib.c — Implementation file corresponding to mylib.h
//
// Rules:
// 1. Always include the corresponding header first (checks self-consistency)
// 2. Mark functions not exposed to other files as static
// 3. Avoid global variables as much as possible

#include "mylib.h"   // include the corresponding header first
#include <stdio.h>
#include <math.h>    // sqrt

// =========================================================
// static (private) helper function
// Not declared in mylib.h -> cannot be called from outside this file
// =========================================================
static int clamp(int val, int min, int max) {
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

// =========================================================
// Public function implementations
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
