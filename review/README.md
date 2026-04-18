# C 構文レビュー

各ディレクトリの `main.c` を読んでビルド・実行して確認する。  
各ディレクトリに `README.md` があり、セクションごとの詳細な解説とチートシートを記載している。

## ビルドコマンド一覧

```sh
# 通常ビルド（各ディレクトリに移動して実行）
cd 01_types      && clang -std=c11 -Wall -Wextra -o out main.c             && ./out && cd ..
cd 02_pointers   && clang -std=c11 -Wall -Wextra -o out main.c             && ./out && cd ..
cd 03_arrays     && clang -std=c11 -Wall -Wextra -o out main.c             && ./out && cd ..
cd 04_structs    && clang -std=c11 -Wall -Wextra -o out main.c             && ./out && cd ..
cd 05_functions  && clang -std=c11 -Wall -Wextra -o out main.c             && ./out && cd ..
cd 06_strings    && clang -std=c11 -Wall -Wextra -o out main.c             && ./out && cd ..
cd 07_memory     && clang -std=c11 -Wall -Wextra -o out main.c             && ./out && cd ..
cd 08_pointers_adv && clang -std=c11 -Wall -Wextra -o out main.c           && ./out && cd ..
cd 09_preprocessor && clang -std=c11 -Wall -Wextra -o out main.c           && ./out && cd ..
cd 10_headers    && clang -std=c11 -Wall -Wextra -o out main.c mylib.c     && ./out && cd ..
cd 11_bitops     && clang -std=c11 -Wall -Wextra -o out main.c             && ./out && cd ..
cd 12_varargs    && clang -std=c11 -Wall -Wextra -o out main.c             && ./out && cd ..
cd 13_ub         && clang -std=c11 -Wall -Wextra -o out main.c             && ./out && cd ..

# UB 検出ビルド（開発中は常にこちらを推奨）
cd 13_ub && clang -std=c11 -Wall -Wextra -fsanitize=address,undefined -o out main.c && ./out && cd ..
```

## ディレクトリ一覧

| ディレクトリ | テーマ | 重要度 |
|-------------|--------|--------|
| `01_types/` | 基本型・固定幅型・bool・符号の罠 | ★★★ |
| `02_pointers/` | ポインタ・`&`・`*`・`->` | ★★★ |
| `03_arrays/` | 配列・decay・ポインタ演算 | ★★★ |
| `04_structs/` | struct・typedef・enum・union | ★★★ |
| `05_functions/` | 関数・値渡し・ポインタ渡し | ★★★ |
| `06_strings/` | null終端文字列・string.h | ★★★ |
| `07_memory/` | malloc/calloc/realloc/free | ★★★ |
| `08_pointers_adv/` | void*・関数ポインタ・ポインタ演算 | ★★★ |
| `09_preprocessor/` | マクロ・条件コンパイル | ★★☆ |
| `10_headers/` | .h/.c 分割・インクルードガード | ★★★ |
| `11_bitops/` | ビット演算・マスク・エンディアン | ★★☆ |
| `12_varargs/` | va_list・可変長引数 | ★★☆ |
| `13_ub/` | 未定義動作・落とし穴 | ★★★ |

## Vector 実装を始める前に最低限確認すること

```
02_pointers → 07_memory → 03_arrays → 08_pointers_adv
```
