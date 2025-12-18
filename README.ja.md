# ESP32SyncKit

[English](README.md) | 日本語

ESP32（Arduino）向けの FreeRTOS 同期プリミティブをラップするヘッダオンリー C++ ライブラリ。Arduino らしい書き味のまま、タスク/ISR 両方から安全に使える設計です。

## これは何？
- Arduino-ESP32（FreeRTOS）向け同期レイヤ。
- ISR 自動判定で FromISR を自動選択。
- タスク生成元を選ばない（ESP32AutoTask / ESP32TaskKit / 生 FreeRTOS）。

## 特長
- Queue の型安全テンプレート、RAII ヘルパ、ブロック/ノンブロックの統一 API。
- ログタグは共通で `ESP32SyncKit`（必要に応じて `[Queue]` などを付与）。
- WaitForever 定数で「無限待ち」を表現（ISR では強制ノンブロック）。

## コンポーネント
- Queue<T>: 型安全キュー、ISR 自動判定付き。
- Notify: タスク通知ラッパ（インスタンスごとにカウンタモード/ビットモード固定）。
- BinarySemaphore: 単発イベント用。ISR give 対応。
- Mutex: 標準ミューテックス（優先度継承・非再帰）。LockGuard 付き。

## タスクライブラリとの組み合わせ
- ESP32AutoTask: 弱シンボルフック（`LoopCore0_*`, `LoopCore1_*`）の中で ESP32SyncKit を利用。
- ESP32TaskKit: ESP32TaskKit でタスクを作り、同期は ESP32SyncKit に任せる（タスク管理と同期を分離）。
- 生 FreeRTOS: `xTaskCreatePinnedToCore` などで作ったタスクから直接 ESP32SyncKit を利用。

## インストール
- Arduino IDE ライブラリマネージャで「ESP32SyncKit」を検索。
- 手動: リリース ZIP をダウンロードし、`Arduino/libraries` に配置。

## 簡単な使い方（プレビュー）
```cpp
#include <ESP32SyncKit.h>

ESP32SyncKit::Queue<int> q(4);

void IRAM_ATTR onGpio()
{
  q.trySend(42); // ISR からの非ブロッキング送信
}

void setup() {
  Serial.begin(115200);
}

void loop() {
  int v;
  if (q.receive(v, ESP32SyncKit::WaitForever)) {
    Serial.println(v);
  }
}
```

## 対応環境
- 対象: Arduino-ESP32（FreeRTOS tick = 1 ms 前提）。デュアルコア対応、ISR セーフ（該当 API）。
- ESP8266 や非 FreeRTOS 環境は対象外。

## ドキュメント
- [Specification (EN)](SPEC.md)
- [仕様書 (JA)](SPEC.ja.md)

## 兄弟ライブラリと使い分け
- [ESP32AutoTask](https://github.com/tanakamasayuki/ESP32AutoTask): `LoopCore0_Low` や `LoopCore1_High` などの弱シンボル関数を定義すると自動でタスク化。手軽なスケッチや少数の単純タスク向けで、大量のタスクや細かい調整には不向き。
- [ESP32TaskKit](https://github.com/tanakamasayuki/ESP32TaskKit): 優先度/コア/スタック/ライフサイクルを設定できる C++ タスク管理キット。生 FreeRTOS を書かずに、ほとんどのプロジェクトで求められるタスク設定を扱える。
- [ESP32SyncKit](https://github.com/tanakamasayuki/ESP32SyncKit)（本ライブラリ）: キュー/通知/セマフォ/ミューテックスのラッパでタスク生成手段に依存しない。AutoTask や TaskKit（あるいは生 FreeRTOS）と組み合わせて、安全にデータや通知をやり取りするために使う。

## ライセンス
- MIT License（LICENSE を参照）。
