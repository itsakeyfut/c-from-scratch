// Header file splitting demo

#include <stdio.h>
#include "mylib.h" // double quotes for relative path (<> is for system headers)

int main(void) {
    printf("=== Header Splitting Demo ===\n");

    // Use the types and functions defined in mylib.h
    Point p1 = point_new(3, 4);
    Point p2 = point_new(6, 8);

    printf("p1 = "); point_print(&p1); printf("\n");
    printf("p2 = "); point_print(&p2); printf("\n");

    Point sum = point_add(p1, p2);
    printf("p1 + p2 = "); point_print(&sum); printf("\n");

    double dist = point_dist(p1, p2);
    printf("dist(p1, p2) = %.4f\n", dist);

    printf("p1 == p1: %d\n", point_eq(p1, p1));
    printf("p1 == p2: %d\n", point_eq(p1, p2));

    printf("MYLIB_VERSION = %d\n", MYLIB_VERSION);

    // =========================================================
    // How include guards work
    // =========================================================
    // Including mylib.h a second time does not cause an error.
    // #ifndef MYLIB_H skips the second inclusion.
    #include "mylib.h"  // 2nd time: nothing happens (the guard prevents it)

    printf("\ndone\n");
    return 0;
}
