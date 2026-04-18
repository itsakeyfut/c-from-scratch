## なんでやねんノート
### dst, src って何？

dst = destination（行き先）、src = source（元）の略。

```c
memmove(
    dst,  // コピー先
    src,  // コピー元
    size, // バイト数
)
```

vec_insert での具体的な対応で表すと：

```c
memmove(
    (char *)v->data + (i + 1) * v->elem_size, // dst: index i+1 の先頭 → ここに書く
    (char *)v->data + i * v->elem_size,       // src: index i   の先頭 → ここから読む
    (v->len - 1) * v->elem_size               // size: 何バイト動かすか
)
```

読む場所 → src = data + i*(elem_size)
書く場所 → dst = data + (i+1)*(elem_size)

「src から size バイト読んで、dst に張り付ける」が memmove / memcpy のすべて。

### memmove/memcpy の計算式は何を表していますか？

「バイトアドレスの計算」を行っています。

なぜ data + i * elem_size か

```
アドレス:  0        4       8      12      16
          ↓        ↓       ↓       ↓       ↓ 
data → [  10  ][  20  ][  40  ][  50  ][      ]
       index0   index1  index2  index3
```

data はバッファの先頭アドレス（バイト単位）。
index=2 の要素に辿り着くには **先頭から 2要素分のバイト数** だけ進む必要がある。

```
index 2 のアドレス = data + 2 * 4 = data + 8
```

一般化すると：

```
index i のアドレス = data + i * elem_size
```

elem_size が 8 (double) なら：

```
アドレス:  0        8      16      24
          ↓        ↓       ↓       ↓       ↓ 
data → [  1.1  ][  2.2  ][  3.3  ]
         index0   index1  index2
```

index 2 = data + 2 * 8 = data + 16

#### なぜ (char *) が必要か

v->data は void * 型。 C では void * に整数を足すことができない（UB）。

```c
// NG: void* + int はエラーまたは UB
v->data + i * v->elem_size

// OK: char* は「1バイト単位で進む」ポインタなので加算が合法
(char *)v->data + i * v->elem_size
```

char * にキャストすることで「このポインタを1バイト刻みで動かす」と宣言している。

#### なぜバイト数が (v->len - i) * elem_size か
動かしたい要素の個数 x 1要素のバイト数 = 動かすバイト数。
[10, 20, 40, 50] に index=2 で挿入する場面（len=4, i=2）:

動かす対象： index 2 から末尾まで = 40, 50 の 2 要素
個数        = len - 1 = 4 - 2 = 2
バイト数    = 2 * 4   = 8
