// mylib.h — How to write a header file
//
// Rules:
// 1. Use include guards to prevent double inclusion
// 2. Only put type definitions, macros, and function prototypes here
// 3. Do not write implementations (function bodies) here
// 4. Include any other headers this header depends on

#ifndef MYLIB_H   // guard start: only compile if this header has not been included yet
#define MYLIB_H   // define it so subsequent inclusions are skipped

#include <stddef.h>   // size_t
#include <stdbool.h>  // bool

// =========================================================
// Constants
// =========================================================
#define MYLIB_VERSION  1
#define POINT_MAX      1000

// =========================================================
// Type definitions
// =========================================================
typedef struct {
    int x;
    int y;
} Point;

typedef enum {
    MYLIB_OK    =  0,
    MYLIB_ERR   = -1,
    MYLIB_OOM   = -2,
} MyLibResult;

// =========================================================
// Function prototypes (declarations only; implementations are in mylib.c)
// =========================================================
Point   point_new(int x, int y);
Point   point_add(Point a, Point b);
double  point_dist(Point a, Point b);
bool    point_eq(Point a, Point b);
void    point_print(const Point *p);

// Do not declare static functions here (they are only visible within mylib.c)

#endif  // MYLIB_H — guard end
