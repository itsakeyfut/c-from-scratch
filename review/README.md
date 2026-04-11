# C 構文レビュー

各ファイルを読んでビルド・実行して確認する。

## ビルドコマンド一覧

```sh
# 通常ビルド
clang -std=c11 -Wall -Wextra -o 01_types    01_types.c    && ./01_types
clang -std=c11 -Wall -Wextra -o 02_pointers 02_pointers.c && ./02_pointers
clang -std=c11 -Wall -Wextra -o 03_arrays   03_arrays.c   && ./03_arrays
clang -std=c11 -Wall -Wextra -o 04_structs  04_structs.c  && ./04_structs
clang -std=c11 -Wall -Wextra -o 05_funcs    05_functions.c && ./05_funcs
clang -std=c11 -Wall -Wextra -o 06_strings  06_strings.c  && ./06_strings
clang -std=c11 -Wall -Wextra -o 07_memory   07_memory.c   && ./07_memory
clang -std=c11 -Wall -Wextra -o 08_ptradv   08_pointers_adv.c && ./08_ptradv
clang -std=c11 -Wall -Wextra -o 09_pp       09_preprocessor.c && ./09_pp
clang -std=c11 -Wall -Wextra -o 11_bitops   11_bitops.c   && ./11_bitops
clang -std=c11 -Wall -Wextra -o 12_varargs  12_varargs.c  && ./12_varargs
clang -std=c11 -Wall -Wextra -o 13_ub       13_ub.c       && ./13_ub

# 10_headers のみ複数ファイルをまとめてビルド
cd 10_headers
clang -std=c11 -Wall -Wextra -o 10_headers main.c mylib.c && ./10_headers
cd ..

# UB 検出ビルド（開発中は常にこちらを推奨）
clang -std=c11 -Wall -Wextra -fsanitize=address,undefined -o 13_ub_san 13_ub.c && ./13_ub_san
```

## ファイル一覧

| ファイル | テーマ | 重要度 |
|---------|--------|--------|
| `01_types.c` | 基本型・固定幅型・bool・符号の罠 | ★★★ |
| `02_pointers.c` | ポインタ・`&`・`*`・`->` | ★★★ |
| `03_arrays.c` | 配列・decay・ポインタ演算 | ★★★ |
| `04_structs.c` | struct・typedef・enum・union | ★★★ |
| `05_functions.c` | 関数・値渡し・ポインタ渡し | ★★★ |
| `06_strings.c` | null終端文字列・string.h | ★★★ |
| `07_memory.c` | malloc/calloc/realloc/free | ★★★ |
| `08_pointers_adv.c` | void*・関数ポインタ・ポインタ演算 | ★★★ |
| `09_preprocessor.c` | マクロ・条件コンパイル | ★★☆ |
| `10_headers/` | .h/.c 分割・インクルードガード | ★★★ |
| `11_bitops.c` | ビット演算・マスク・エンディアン | ★★☆ |
| `12_varargs.c` | va_list・可変長引数 | ★★☆ |
| `13_ub.c` | 未定義動作・落とし穴 | ★★★ |

## Vector 実装を始める前に最低限確認すること

```
02_pointers → 07_memory → 03_arrays → 08_pointers_adv
```
