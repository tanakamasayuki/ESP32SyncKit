# ESP32AutoSync 仕様書

ESP32AutoSync は、ESP32（Arduino）向けの  
**FreeRTOS 同期プリミティブのヘッダオンリー C++ ラッパライブラリ**です。

FreeRTOS の Queue / TaskNotify / Semaphore / Mutex をベースに、  
- **型安全なテンプレート**、  
- **RAII**、  
- **ISR 自動判定**、  
- **統一されたブロック／非ブロック API**  
を提供することで、より高い可読性と安全性を実現します。

ESP32AutoSync は以下と**組み合わせて使うことができます**：

- 入門用タスクライブラリ：**ESP32AutoTask**
- 中級者向けタスク管理ライブラリ：**ESP32TaskKit**
- 生の FreeRTOS タスク（`xTaskCreatePinnedToCore` など）

> **タスクの作り方は何でもOK。  
> AutoSync は「タスク同士のつながり（Sync）」だけを担当するレイヤです。**

---

## 1. コンセプト

ESP32AutoSync の主な目的：

1. **FreeRTOS の Queue / Notify / Semaphore / Mutex を C++ で安全＆直感的に扱えるようにする**
2. **タスク・ISR 両方から安全に使える API にする**
3. **ESP32AutoTask / ESP32TaskKit / 生 FreeRTOS タスク など、どんなタスク構成でも同じ API で使えるようにする**
4. **良い習慣を自然に身につける設計** ISR では「通知・合図だけ」、タスクで「処理本体を行う」という FreeRTOS の基本思想を自然に守りやすい API になっています。

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

ユーザーはESP32AutoSync.hだよ読み込んで利用する方針。

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

### 5.3 BinarySemaphore / CountingSemaphore / Mutex
FreeRTOS ネイティブをシンプルにラップ。

---

## 6. ISR 対応

- ブロック禁止（自動で tryXXX と同挙動）
- portYIELD_FROM_ISR も内部処理

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

---
