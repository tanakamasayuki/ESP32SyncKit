# ESP32AutoSync 仕様書

ESP32AutoSync は、ESP32（Arduino）向けの  
**FreeRTOS 同期プリミティブのヘッダオンリー C++ ラッパライブラリ**です。

Arduino らしいモダン C++ API から FreeRTOS の正しい使い方を自然に学べる設計で、  
FreeRTOS の Queue / TaskNotify / BinarySemaphore / Mutex をベースに  
- **型安全なテンプレート**、  
- **RAII**、  
- **ISR 自動判定**、  
- **統一されたブロック／非ブロック API**  
を提供することで、より高い可読性と安全性を実現します。

ESP32AutoSync は以下と**組み合わせて使うことができます**：

- 入門用タスクライブラリ：**ESP32AutoTask**（https://github.com/tanakamasayuki/ESP32AutoTask）
- 中級者向けタスク管理ライブラリ：**ESP32TaskKit**（https://github.com/tanakamasayuki/ESP32TaskKit）
- 生の FreeRTOS タスク（`xTaskCreatePinnedToCore` など）

> **タスクの作り方は何でもOK。  
> AutoSync は「タスク同士のつながり（Sync）」だけを担当するレイヤです。**

---

## 1. コンセプト

ESP32AutoSync の主な目的：

1. **FreeRTOS の Queue / Notify / Semaphore / Mutex を C++ で安全＆直感的に扱えるようにする**
2. **Arduino らしい書き味のまま FreeRTOS 的な正しい使い方を覚えられるようにする**
3. **タスク・ISR 両方から安全に使える API にする**
4. **ESP32AutoTask / ESP32TaskKit / 生 FreeRTOS タスク など、どんなタスク構成でも同じ API で使えるようにする**
5. **良い習慣を自然に身につける設計** ISR では「通知・合図だけ」、タスクで「処理本体を行う」という FreeRTOS の基本思想を自然に守りやすい API になっています。
6. **補助輪→中級→素の FreeRTOS への橋渡し** AutoTask の弱シンボルフックや TaskKit の Config/協調停止の設計と噛み合わせ、最終的に素の FreeRTOS API へ移行しやすい作りを目指します。

---

## 2. 他ライブラリとの関係

### 2.1 ESP32AutoTask との組み合わせ

AutoTask で作られる弱宣言タスク（`LoopCore0_Low()` など）から  
AutoSync の Queue / Notify / Mutex をそのまま利用できます。

### 2.2 ESP32TaskKit との組み合わせ

TaskKit で作ったタスクからも AutoSync を自然に利用できます。  
タスク管理は TaskKit、同期は AutoSync という明確な役割分担になります。

### 2.3 生の FreeRTOS タスクとの併用

`xTaskCreatePinnedToCore()` などで生成したタスクからも AutoSync を利用可能。  
FreeRTOS の基本APIのみへ依存しており、Arduinoで動作します。

### 2.4 ライブラリ間の役割分担と学習ステップ
- **AutoTask（補助輪）**: `begin()` を呼ぶだけでコア0/1の Low/Normal/High タスク枠が用意され、弱シンボルのフックを定義すれば動く。未定義フックは即終了しオーバーヘッドゼロ。
- **TaskKit（中級者向け）**: `TaskConfig` と `Task.start/startLoop` でスタック/優先度/コアを明示し、`requestStop` で協調終了する。ラムダや functor で C++ らしく書ける。
- **AutoSync（本ライブラリ）**: タスクは上記どれで作ってもよく、Queue/Notify/Semaphore/Mutex の正しい使い方を統一 API で提供する。ISR では「合図だけ」、タスクで処理本体という良い習慣を examples で身につけられる。
- 学習導線: AutoTask で動かしてから AutoSync で同期を学び、さらに TaskKit でタスク設計を柔軟にする、という段階的習得を想定。

---

## 3. 名前空間とファイル構成

### 3.1 名前空間

```cpp
namespace AutoSync {
    // Queue<T>, Notify, BinarySemaphore, Mutex など
}
```

### 3.2 ファイル構成

```
ESP32AutoSync/
  src/
    ESP32AutoSync.h
    AutoSyncQueue.h
    AutoSyncNotify.h
    AutoSyncSemaphore.h
    AutoSyncMutex.h
    detail/AutoSyncCommon.h
```

ユーザーは ESP32AutoSync.h を読み込んで利用する方針。

---

## 4. 共通仕様

### 4.1 WaitForever
```cpp
constexpr uint32_t WaitForever = UINT32_MAX;
```
`timeoutMs = WaitForever` のとき `portMAX_DELAY` で待機。  
ISR では強制的にノンブロッキング動作。

### 4.2 ISR 自動判定  
内部で `xPortInIsrContext()` を用い、FromISR API を自動選択します。

### 4.3 API の統一性  
- `tryXXX()` … ノンブロック  
- `XXX(timeoutMs)` … ブロック（デフォルト無限）

tryXXX()はXXX(0)を呼び出す。

### 4.4 提供範囲（初期）
- Queue<T>
- Notify
- BinarySemaphore
- Mutex

### 4.5 エラーハンドリング
- 例外は使わず、戻り値（bool やステータス enum）で結果を返す
- 失敗時は適切なログレベルで記録し、呼び出し側でリカバーする前提

### 4.6 時間とスケジューリング
- Arduino の `delay()` を基本にし、内部の待ち時間は 1 tick = 1 ms 固定設定のみを想定
- 他の tick 周波数や高精度時間 API は考慮しない

### 4.7 ロギング
- ESP-IDF の `ESP_LOGE/W/I/D/V` を常に利用可能にする（Arduino 環境でも有効）
- 目安: E=致命/操作失敗、W=リトライ・タイムアウト等、I=初期化や設定値、D/V=デバッグ用詳細

### 4.8 設定方法
- 初期リリースはグローバル設定なし。各クラスのコンストラクタ/メソッド引数だけで使える構成とする
- マクロや外部設定ファイルは使わず、コード上で完結（将来必要なら小さな Config 構造体を追加検討）

### 4.9 非対象
- マルチコアのコア割り当てはタスク生成側で管理し、本ライブラリでは介入しない
- 電力管理やスリープ制御は考慮しない

---

## 5. クラス仕様（要約）

### 5.1 Queue<T>
テンプレートキュー（型安全・ISR対応）。

```cpp
Queue<T>::trySend(value);
Queue<T>::send(value, timeoutMs);
Queue<T>::tryReceive(out);
Queue<T>::receive(out, timeoutMs);
```

### 5.2 Notify
タスク通知（カウンタ/ビット）。

```cpp
notify.notify();             // ISR/Task 自動判定
notify.take(timeoutMs);      // ulTaskNotifyTake
notify.wait(bits,...);       // xTaskNotifyWait
```

### 5.3 BinarySemaphore / Mutex
FreeRTOS ネイティブをシンプルにラップ（初期リリースはこの2種）。

---

## 6. ISR 対応

- ブロック禁止（自動で tryXXX と同挙動）
- portYIELD_FROM_ISR も内部処理
- ISR では「送るだけ・通知するだけ」を推奨し、examples で正しい呼び方を提示する

---

## 7. ユースケース例

### 7.1 AutoTask + AutoSync
ISR → AutoTask タスクへ処理移譲。

### 7.2 TaskKit + AutoSync
複数 TaskKit タスクでキュー共有。

### 7.3 生 FreeRTOS + AutoSync
既存タスク群の同期レイヤとして追加可能。

---

## 8. 設計ポリシー

- タスク生成・管理は AutoTask / TaskKit / FreeRTOS に委譲  
- AutoSync は同期だけを担当（シンプル&安定）  
- API は tryXXX と XXX の2系統に統一  
- Queue の型だけテンプレートとし、その他はシンプルに保つ
- 戻り値ベースでエラーを返し、例外は使わない
- 1 tick = 1 ms 前提で設計し、delay() を基本にする
- マルチコアのコア割り当てやスリープ制御は扱わない（タスク側で指定）

---
