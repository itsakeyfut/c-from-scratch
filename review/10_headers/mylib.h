// mylib.h — ヘッダファイルの書き方
//
// ルール:
// ① インクルードガードで二重インクルードを防ぐ
// ② 型定義・マクロ・関数プロトタイプのみ書く
// ③ 実装（関数の中身）はここに書かない
// ④ 他のヘッダが必要なら自分でインクルードする

#ifndef MYLIB_H   // ガード開始: このヘッダが未定義の場合のみコンパイル
#define MYLIB_H   // 定義して次回以降をスキップ

#include <stddef.h>   // size_t
#include <stdbool.h>  // bool

// =========================================================
// 定数
// =========================================================
#define MYLIB_VERSION  1
#define POINT_MAX      1000

// =========================================================
// 型定義
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
// 関数プロトタイプ（宣言のみ、実装は mylib.c）
// =========================================================
Point   point_new(int x, int y);
Point   point_add(Point a, Point b);
double  point_dist(Point a, Point b);
bool    point_eq(Point a, Point b);
void    point_print(const Point *p);

// static 関数はここに書かない（mylib.c 内部のみ有効）

#endif  // MYLIB_H — ガード終了
