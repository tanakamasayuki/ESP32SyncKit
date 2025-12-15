# ESP32AutoSync

[English](README.md) | 日本語

ESP32（Arduino）向けの FreeRTOS 同期プリミティブをラップするヘッダオンリー C++ ライブラリ。Arduino らしい書き味のまま、タスク/ISR 両方から安全に使える設計です。

## これは何？
- Arduino-ESP32（FreeRTOS）向け同期レイヤ。
- ISR 自動判定で FromISR を自動選択。
- タスク生成元を選ばない（ESP32AutoTask / ESP32TaskKit / 生 FreeRTOS）。

## 特長
- Queue の型安全テンプレート、RAII ヘルパ、ブロック/ノンブロックの統一 API。
- ログタグは共通で `ESP32AutoSync`（必要に応じて `[Queue]` などを付与）。
- WaitForever 定数で「無限待ち」を表現（ISR では強制ノンブロック）。

## コンポーネント
- Queue<T>: 型安全キュー、ISR 自動判定付き。
- Notify: タスク通知ラッパ（インスタンスごとにカウンタモード/ビットモード固定）。
- BinarySemaphore: 単発イベント用。ISR give 対応。
- Mutex: 標準ミューテックス（優先度継承・非再帰）。LockGuard 付き。

## インストール
- Arduino IDE ライブラリマネージャで「ESP32AutoSync」を検索。
- 手動: リリース ZIP をダウンロードし、`Arduino/libraries` に配置。

## 簡単な使い方（プレビュー）
```cpp
#include <ESP32AutoSync.h>

ESP32AutoSync::Queue<int> q(4);

void IRAM_ATTR onGpio()
{
  q.trySend(42); // ISR からの非ブロッキング送信
}

void setup() {
  Serial.begin(115200);
}

void loop() {
  int v;
  if (q.receive(v, ESP32AutoSync::WaitForever)) {
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

## 関連ライブラリ
- ESP32AutoTask: https://github.com/tanakamasayuki/ESP32AutoTask
- ESP32TaskKit: https://github.com/tanakamasayuki/ESP32TaskKit

## ライセンス
- MIT License（LICENSE を参照）。
