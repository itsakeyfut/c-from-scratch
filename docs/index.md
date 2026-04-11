# C from Scratch — 自作インデックス

CでゼロからつくりZigへの移行を目指す。
各項目の仕様・設計・戦略は個別の `.md` に記載する。

---

## Tier 1: 基礎体力

| # | 項目 | ファイル | 壁 |
|---|------|----------|----|
| 1 | 動的配列 (Vector) | [vector.md](vector.md) | realloc, ポインタ無効化 |
| 2 | 可変長文字列 (String) | [string.md](string.md) | null終端 vs length, UTF-8 |
| 3 | メモリユーティリティ | [mem.md](mem.md) | memcpy vs memmove, 重なり |
| 4 | スタック / キュー / Ring Buffer | [queue.md](queue.md) | インデックス管理, オーバーフロー |
| 5 | 簡易printf | [printf.md](printf.md) | va_list, 型安全性ゼロ |
| 6 | Bitset / Bitmap | [bitset.md](bitset.md) | ビット演算, シフト, マスク |
| 7 | 独自assertとデバッグマクロ | [assert.md](assert.md) | `__FILE__`, `__LINE__`, NDEBUG |

---

## Tier 2: データ構造と設計力

| # | 項目 | ファイル | 壁 |
|---|------|----------|----|
| 8 | Arena Allocator | [arena.md](arena.md) | lifetime管理, 解放しない設計 |
| 9 | HashMap | [hashmap.md](hashmap.md) | ハッシュ関数, リサイズ, 衝突 |
| 10 | Binary Heap（優先度キュー） | [heap.md](heap.md) | 配列で木を表現, heapify |
| 11 | Resultパターン | [result.md](result.md) | error unionの模倣, 設計の不毛感 |
| 12 | setjmp / longjmp | [setjmp.md](setjmp.md) | 非局所脱出, スタック破壊の危険 |
| 13 | 参照カウント (RC) | [rc.md](rc.md) | 循環参照, weak参照, free戦略 |
| 14 | 汎用ソート | [sort.md](sort.md) | 関数ポインタ, 型消去の限界 |
| 15 | Linked List (optional) | [linked_list.md](linked_list.md) | ポインタ地獄 |

---

## Tier 3: パーサー地獄

| # | 項目 | ファイル | 壁 |
|---|------|----------|----|
| 16 | JSONパーサ | [json.md](json.md) | 再帰下降, エラー処理, 木構造のメモリ管理 |
| 17 | INI / CSVパーサ | [ini_csv.md](ini_csv.md) | 境界条件, フォーマットの曖昧さ |
| 18 | 文字列検索（KMP等） | [strSearch.md](strSearch.md) | 失敗関数の構築, O(n)保証 |
| 19 | 簡易言語パーサ (optional) | [lang.md](lang.md) | AST構築, 優先順位処理 |

---

## Tier 4: システム寄り

| # | 項目 | ファイル | 壁 |
|---|------|----------|----|
| 20 | ファイルI/Oラッパ | [fileio.md](fileio.md) | エラー処理, クロスプラットフォーム |
| 21 | シリアライザ / デシリアライザ | [serde.md](serde.md) | エンディアン, パディング |
| 22 | Base64 | [base64.md](base64.md) | ビット操作, パディング処理 |
| 23 | 乱数生成器（Xorshift等） | [rand.md](rand.md) | シード管理, 再現性, スレッド安全性 |
| 24 | TCPソケット基礎 | [tcp.md](tcp.md) | socket/bind/listen/accept, WinsockとPOSIXの差異 |
| 25 | Graph（隣接リスト） | [graph.md](graph.md) | Vector + HashMapの複合, BFS/DFS |
| 26 | スレッドプール | [thread.md](thread.md) | race condition, mutex地獄 |
| 27 | カスタムアロケータ (optional) | [allocator.md](allocator.md) | fragmentation, free list / slab |

---

## Tier 5: 余力

| # | 項目 | ファイル |
|---|------|----------|
| 28 | Trie | [trie.md](trie.md) |
| 29 | 簡易HTTPパーサ | [http.md](http.md) |
| 30 | ミニログライブラリ | [log.md](log.md) |
| 31 | テストフレームワーク | [test.md](test.md) |
| 32 | CLI引数パーサ | [args.md](args.md) |
| 33 | 簡易VM / バイトコード実行系 | [vm.md](vm.md) |

---

## Rust連携（C実装 → Rust理解が深まるもの）

既存のRust経験（FFmpegラッパー, Bevy, Axum/Leptos）を踏まえた項目。
Cで実装した後、Rustで再実装して比較する。

| # | 項目 | ファイル | 対応するRustの経験 | 壁 |
|---|------|----------|------------------|----|
| R1 | FFIブリッジ | [ffi.md](ffi.md) | FFmpegラッパー | ABI安定性, ポインタ所有権の明示, エラーコード設計 |
| R2 | 簡易ECS | [ecs.md](ecs.md) | Bevy | void*型消去, SparseSet, アライメント管理 |
| R3 | Lock-free MPSC queue | [mpsc.md](mpsc.md) | Bevy / Axum | メモリオーダリング, ABAハザード |
| R4 | 簡易イベントループ | [eventloop.md](eventloop.md) | Axum / Tokio | ノンブロッキングI/O, タスクキュー |
| R5 | Observer / Signalパターン | [signal.md](signal.md) | Leptos | 循環依存検出, リエントランシー |

---

## FFmpeg自作（コーデック・コンテナ・フィルタ）

将来ZigでFFmpegの代替を作るための基礎。
依存関係: Tier 1〜2の完了後に取り組む。

### 共通基盤

| # | 項目 | ファイル | 壁 |
|---|------|----------|----|
| F1 | ビットストリームリーダー/ライター | [bitstream.md](bitstream.md) | ビット操作, Exp-Golomb符号, エンディアン |
| F2 | 色空間変換 (YUV↔RGB) | [colorspace.md](colorspace.md) | 色行列, クランプ処理, 精度 |
| F3 | 数学ユーティリティ（固定小数点） | [fixedpoint.md](fixedpoint.md) | オーバーフロー設計, 精度とパフォーマンスの両立 |

### コンテナ解析

```
WAV（最小） → BMP → MP4/MOV boxパーサ
```

| # | 項目 | ファイル | 壁 |
|---|------|----------|----|
| F4 | WAVパーサ/ライター | [wav.md](wav.md) | RIFFチャンク構造, エンディアン |
| F5 | BMPパーサ/ライター | [bmp.md](bmp.md) | パディング, ピクセル形式の多様性 |
| F6 | MP4/MOV boxパーサ | [mp4.md](mp4.md) | 深いネスト, 64bit box, moov前後問題 |

### 圧縮・符号化

```
Huffman → DCT（整数版） → JPEGデコーダー
                        → DEFLATE → PNGデコーダー
```

| # | 項目 | ファイル | 壁 |
|---|------|----------|----|
| F7 | Huffman符号化/復号 | [huffman.md](huffman.md) | 木構築, 正準Huffman, テーブル設計 |
| F8 | DCT（整数版） | [dct.md](dct.md) | 量子化テーブル, 精度管理, ジグザグスキャン |
| F9 | JPEGデコーダー | [jpeg.md](jpeg.md) | MCU境界, クロマサンプリング, マーカー処理 |
| F10 | DEFLATE（簡易版） | [deflate.md](deflate.md) | LZ77 + Huffmanの組み合わせ, バックリファレンス |
| F11 | PNGデコーダー | [png.md](png.md) | フィルタ処理, チャンク検証, DEFLATEの組み込み |

### フィルタパイプライン（lavfi相当）

| # | 項目 | ファイル | 壁 |
|---|------|----------|----|
| F12 | フレームバッファ | [frame.md](frame.md) | refcounting設計, ピクセル形式のメタデータ |
| F13 | フィルタグラフ（DAGベース） | [filtergraph.md](filtergraph.md) | トポロジカルソート, push/pullモデル, バッファリング |
| F14 | 簡易フィルタ実装 | [filters.md](filters.md) | スケーリング, 輝度調整, クロップの境界処理 |

---

## Vulkan自作（ソフトウェアラスタライザー）

Vulkanのハードウェア部分はCで代替できないため、ソフトウェアラスタライザーとして実装する。
「Vulkanが何を抽象化しているか」を内側から理解するのが目的。

### 数学ライブラリ

| # | 項目 | ファイル | 壁 |
|---|------|----------|----|
| V1 | vec2/3/4, mat3/4 | [math_vec.md](math_vec.md) | `__attribute__((aligned))`, SIMDへの誘惑 |
| V2 | クォータニオン | [quat.md](quat.md) | gimbal lock, 正規化, slerpの精度 |
| V3 | プロジェクション行列 | [projection.md](projection.md) | NDC座標系, 深度範囲, left/right-hand |

### ソフトウェアラスタライザー

```
三角形塗りつぶし
→ デプスバッファ
→ テクスチャマッピング（パースペクティブ補正）
→ 頂点属性の補間
→ 簡易フラグメントシェーダー（関数ポインタ）
```

| # | 項目 | ファイル | 壁 |
|---|------|----------|----|
| V4 | 三角形ラスタライザー | [rasterizer.md](rasterizer.md) | バリセントリック座標, エッジ関数, タイルベース処理 |
| V5 | デプスバッファ | [depthbuffer.md](depthbuffer.md) | 精度（z-fighting）, デプステスト順序 |
| V6 | テクスチャマッピング | [texture.md](texture.md) | パースペクティブ補正(`w`除算のタイミング), ミップマップ |
| V7 | 頂点バッファ / インデックスバッファ | [vertexbuffer.md](vertexbuffer.md) | レイアウト設計, ストライド, `VkVertexInputAttributeDescription`との対比 |

### Vulkanの設計パターン（ハードウェア不要）

VulkanのAPIデザイン自体をCで模倣し「なぜこう設計されているか」を理解する。

| # | 項目 | ファイル | Vulkanとの対応 | 壁 |
|---|------|----------|---------------|----|
| V8 | コマンドバッファ | [cmdbuffer.md](cmdbuffer.md) | `vkCmdDraw`等 | 記録と実行の分離設計 |
| V9 | デスクリプタ/バインディング | [descriptor.md](descriptor.md) | `VkDescriptorSet` | リソースバインディングの抽象化 |
| V10 | レンダーパス依存グラフ | [renderpass.md](renderpass.md) | `VkRenderPass`, バリア | 依存追跡, 自動バリア挿入 |
| V11 | メモリサブアロケータ（GPU向け） | [gpumem.md](gpumem.md) | VMA相当 | ヒープ種別の選択, 断片化 |

### 簡易シェーダーVM

SPIR-Vの設計を理解するための簡易実装。

| # | 項目 | ファイル | 壁 |
|---|------|----------|----|
| V12 | レジスタベースVM（float/vec型） | [shadervm.md](shadervm.md) | 型システム（float/int/vec）, 命令ディスパッチ |
| V13 | 頂点シェーダー相当 | [vertshader.md](vertshader.md) | 座標変換パイプライン, uniform管理 |
| V14 | フラグメントシェーダー相当 | [fragshader.md](fragshader.md) | テクスチャサンプリング, ブレンディング |

---

## 推奨進行ルート

```
Vector → String → mem → Bitset → assert マクロ
→ Stack/Queue → printf
→ Arena Allocator → HashMap → Binary Heap
→ Result → setjmp/longjmp → RC
→ 汎用ソート → JSONパーサ → 文字列検索
→ File I/O → Serde → TCPソケット → Thread
```

- **Arena AllocatorをHashMapの前に置く**: HashMapをArena上で実装することでアロケータを渡す設計がZigのインターフェースに直結する
- **assertマクロを序盤に**: 以降の実装でデバッグコストが大幅に下がる
- **setjmp/longjmpはResultの直後**: 「過去のエラー処理」→「Resultパターン」→「Zigのerror union」という歴史的な流れで体感する
- **JSONパーサは早め**: 総合力（パーサ + メモリ + エラー処理）が一気に上がる

---

## ビルド管理

Makefileは各Tierと並行して育てていく。
詳細: [makefile.md](makefile.md)

---

## Zig移行の橋渡し対応表

| Cで作るもの | Zigで対応するもの |
|------------|-----------------|
| Arena Allocator | `std.heap.ArenaAllocator` + Allocatorインターフェース |
| Resultパターン | `!T` (error union), `try`, `catch` |
| setjmp / longjmp | `try` / `errdefer` の設計思想の対極 |
| 参照カウント (RC) | Zigに標準RCなし — 自前実装領域 |
| 汎用ソート (void* + 関数ポインタ) | `anytype` + `comptime` |
| 簡易printf (va_list) | `std.fmt.format` (comptime安全) |
| Bitset / Bitmap | `packed struct`, `u1`〜`u128` |
| TCPソケット | `std.net` |
| スレッドプール | `std.Thread` |
| 独自assertマクロ | `@panic`, `@src()`, `std.debug.assert` |
| 色空間変換 / DCT | `@Vector` でポータブルSIMD |
| フィルタグラフ (void*型消去) | `anytype` + `comptime` で型安全なグラフ |
| シェーダーVM命令ディスパッチ | `comptime` でインライン展開 |
| フレームバッファのRC | Allocatorインターフェースで差し替え可能 |
| vec/mat（Cの`__attribute__`） | `@Vector`, `packed struct` |

---

## Rust再実装ロードマップ

Cで実装した後、Rustで再実装して比較する。

| Cで作るもの | Rustで再実装するもの | 得られる理解 |
|------------|-------------------|------------|
| Arena Allocator | `Allocator` trait の実装 | unsafe境界の設計 |
| HashMap | `RawTable`相当をunsafeで | `unsafe` + 型安全ラッパーの分離 |
| FFIブリッジ | 手書きバインディング → `bindgen`と比較 | FFmpegラッパーの設計判断が明確になる |
| 簡易ECS | Bevyのソースと見比べる | `Query<(&Transform, &Velocity)>`の仕組み |
| 簡易イベントループ | `Future` / `Poll` トレイトとの対比 | Tokioが何をしているかが分かる |
| Lock-free MPSC | `crossbeam`のソースと見比べる | `Ordering`の選択基準が直感的になる |
| Observer / Signal | Leptosの`Signal`/`Memo`/`Effect`と対比 | リアクティビティの設計判断が分かる |
| JPEGデコーダー / DEFLATE | `std.compress`との比較 | 標準ライブラリの設計判断が分かる |
| ソフトウェアラスタライザー | `wgpu-rs` / `ash`との対比 | Vulkanの抽象化レイヤーの意味が分かる |
| フィルタグラフ (DAG) | `petgraph`クレートとの対比 | グラフライブラリの設計判断が分かる |
