# From Scratch In C

## Build

```sh
clang -o main.exe main.c && ./main.exe
```

### オプション

| フラグ | 内容 |
|--------|------|
| `-Wall -Wextra` | 警告を有効化 |
| `-std=c11` | C11 標準を指定 |
| `-g` | デバッグ情報を付与（gdb / lldb 用） |
| `-fsanitize=address,undefined` | AddressSanitizer + UBSan を有効化 |

開発中の推奨コマンド:

```sh
clang -std=c11 -Wall -Wextra -g -fsanitize=address,undefined -o main.exe main.c && ./main.exe
```
