# ESP32AutoSync 仕様書

ESP32AutoSync は、ESP32（Arduino）向けの  
**FreeRTOS 同期プリミティブのヘッダオンリー C++ ラッパライブラリ**です。

公式リポジトリ: https://github.com/tanakamasayuki/ESP32AutoSync

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
namespace ESP32AutoSync {
    // Queue<T>, Notify, BinarySemaphore, Mutex など
}
```

### 3.2 ファイル構成

```
ESP32AutoSync/
  src/
    ESP32AutoSync.h
    ESP32AutoSyncQueue.h
    ESP32AutoSyncNotify.h
    ESP32AutoSyncBinarySemaphore.h
    ESP32AutoSyncMutex.h
    detail/ESP32AutoSyncCommon.h
```

ユーザーは ESP32AutoSync.h を読み込んで利用する方針。

---

## 4. 共通仕様

### 4.1 WaitForever
```cpp
constexpr uint32_t WaitForever = portMAX_DELAY;
```
`timeoutMs = WaitForever` のとき `portMAX_DELAY` で待機。  
ISR では強制的にノンブロッキング動作。

### 4.2 ISR 自動判定  
内部で `xPortInIsrContext()` を用い、FromISR API を自動選択します。

### 4.3 API の統一性  
- `tryXXX()` … ノンブロック  
- `XXX(timeoutMs)` … ブロック（デフォルト無限）

tryXXX()はXXX(0)を呼び出す。

### 4.4 提供範囲とユースケース
- Queue<T>  
  - タスク間で型安全にデータを渡す。ISR→タスクでデータ移譲、タスク→タスクで処理分割。深さはコンストラクタ引数で指定し、リングバッファ代わりにも使える。
- Notify  
  - カウンタ用途: `notify()` / `take(timeoutMs)` で ISR→タスクのイベント回数を伝える（軽量な Counting Semaphore として）。  
  - ビット用途: `setBits(mask)` / `waitBits(mask, timeoutMs, clearOnExit)` で複数イベントを1本の通知で扱う（例: RX_READY / TX_DONE / ERROR のビット待ち）。  
  - 値書き込み系（上書き/未上書き）は初期リリースでは扱わない。
- BinarySemaphore  
  - 単発イベントの受け渡しや、タスク起動の合図に。ISR から `give` してタスク側が `take`。
- Mutex  
  - 共有リソースの排他（I2C/SPI/Serial、共有バッファ保護など）。ISR からは使わず、タスク間でのみ利用。

#### 使い分けの目安
- データを運びたい → Queue<T>（型付きで順序も保証）。  
- 単なる「来た/いくつ来た」を軽量に伝えたい → Notify（カウンタ）。  
- 複数種類のイベントをまとめて待ちたい → Notify（ビット）。  
- 一回だけの合図で十分・カウンタ不要 → BinarySemaphore。  
- 共有リソースの排他が目的 → Mutex（ISRでは使わない）。

### 4.5 エラーハンドリング
- 例外は使わず、戻り値はシンプルに `bool`（成功/失敗）で返す
- 失敗時は適切なログレベルで記録し、呼び出し側でリカバーする前提

### 4.6 時間とスケジューリング
- Arduino の `delay()` を基本にし、内部の待ち時間は 1 tick = 1 ms 固定設定のみを想定
- 他の tick 周波数や高精度時間 API は考慮しない

### 4.7 ロギング
- ESP-IDF の `ESP_LOGE/W/I/D/V` を常に利用可能にする（Arduino 環境でも有効）
- 目安: E=致命/操作失敗、W=リトライ・タイムアウト等、I=初期化や設定値、D/V=デバッグ用詳細
- ログの初期化やレベル設定は Arduino ボード定義側で行われる前提とし、ライブラリ側では触らない

### 4.8 設定方法
- 初期リリースはグローバル設定なし。各クラスのコンストラクタ/メソッド引数だけで使える構成とする
- マクロや外部設定ファイルは使わず、コード上で完結（将来必要なら小さな Config 構造体を追加検討）

### 4.9 非対象
- マルチコアのコア割り当てはタスク生成側で管理し、本ライブラリでは介入しない
- 電力管理やスリープ制御は考慮しない

---

## 5. クラス仕様

### 5.1 Queue<T>
テンプレートキュー（型安全・ISR自動判定）。

```cpp
Queue<T> q(depth);                   // 深さをコンストラクタで指定
q.trySend(value);                    // == send(value, 0)
q.send(value, timeoutMs = WaitForever);
q.tryReceive(out);                   // == receive(out, 0)
q.receive(out, timeoutMs = WaitForever);
```

- タスク上では `timeoutMs` に `WaitForever` で無限待ち、ISR では強制ノンブロック。  
- `send/receive` はタスク/ISR を自動判定し、`xQueueSend` / `xQueueSendFromISR` / `xQueueReceive` / `xQueueReceiveFromISR` を適切に選択。必要に応じて `portYIELD_FROM_ISR` も内部処理。  
- `T` はコピー/ムーブ可能な型を想定。サイズが大きい場合はポインタや小さな構造体を推奨。  
- 戻り値は `bool`（成功/タイムアウト/キュー満杯で false）。エラー時はログを出して呼び出し側でリカバーする前提。

### 5.2 Notify
タスク通知（カウンタ/ビット）。`notify()` で積み、`take()` で 1 件消費、`setBits()` / `waitBits()` でビット待ちができる。

```cpp
// カウンタ用途（軽量カウンティングセマフォ）
notify.notify();                         // give。ISR/Task 自動判定
notify.take(timeoutMs = WaitForever);    // 1 件消費（bool で成否）
notify.tryTake();                        // == take(0)

// ビット用途（EventGroup 風）
notify.setBits(mask);                    // OR で積み上げ。ISR 可
notify.waitBits(mask,
                timeoutMs = WaitForever,
                clearOnExit = true,
                waitAll = false);        // any-of がデフォルト
notify.tryWaitBits(mask,
                   clearOnExit = true,
                   waitAll = false);     // == waitBits(mask, 0, clearOnExit, waitAll)
```

- カウンタは `ulTaskNotifyTake` 相当で、`notify()` の回数を蓄積し `take()` で 1 件ずつ消費。残りは後続 `take()` で順次処理する。  
- ビットは `xTaskNotifyWait` ベース。`waitAll=false` なら指定マスクのいずれかが立ったら復帰、`true` なら全ビットが揃うまで待つ。`clearOnExit=true` で満たしたビットをクリア。  
- ISR からの `notify()` / `setBits()` は自動で FromISR 版を選択し、必要に応じて `portYIELD_FROM_ISR` を内部で実行する。  
- `timeoutMs` に `WaitForever` を指定するとタスク上では無限待ち、ISR では強制ノンブロック（0ms）になる。戻り値は全て `bool`（成功/タイムアウト）。
- バインド方針: 初期状態は未バインド。最初に `take`/`waitBits` を呼んだタスクを受信者として自動バインドし、その後は固定。明示的に宛先を指定したい場合はコンストラクタや `bindTo(handle)` / `bindToSelf()` を用意し、一度バインドしたら再バインド不可。未バインドのまま `notify`/`setBits` が呼ばれた場合や、受信者と異なるタスクが `take`/`waitBits` を呼んだ場合は false を返しログで警告する。

### 5.3 BinarySemaphore
FreeRTOS バイナリセマフォの薄いラッパ。単発イベントや起動合図向け。

```cpp
binary.give();                          // ISR/Task 自動判定
binary.take(timeoutMs = WaitForever);   // bool で成否
binary.tryTake();                       // == take(0)
```

- `give` は FromISR を自動選択し、必要なら `portYIELD_FROM_ISR` を内部で実行。  
- `take` は `WaitForever` で無限待ち、ISR 上では強制 0ms（ノンブロック）。戻り値は成功/タイムアウトの bool。  

### 5.4 Mutex
FreeRTOS 標準ミューテックスのラッパ。共有リソースの排他用（タスク専用）。

```cpp
mutex.lock(timeoutMs = WaitForever);    // bool で成否
mutex.tryLock();                        // == lock(0)
mutex.unlock();                         // 所有者のみ
Mutex::LockGuard guard(mutex);          // RAII でスコープ解放時 unlock
```

- 優先度逆転防止付きの標準ミューテックスを利用。ISR からは使用不可。  
- `lock` は `WaitForever` で無限待ち、タイムアウト時は false。  
- `LockGuard` で取得漏れ/解放漏れを防ぎ、例外非使用環境でもスコープで確実に `unlock`。  

---

## 6. ISR 対応

- ブロック禁止（自動で tryXXX と同挙動）
- portYIELD_FROM_ISR も内部処理
- ISR では「送るだけ・通知するだけ」を推奨し、examples で正しい呼び方を提示する

---

## 7. ユースケース例

### 7.1 ESP32AutoTask + ESP32AutoSync
ISR → ESP32AutoTask タスクへ処理移譲。

### 7.2 ESP32TaskKit + ESP32AutoSync
複数 ESP32TaskKit タスクでキュー共有。

### 7.3 生 FreeRTOS + ESP32AutoSync
既存タスク群の同期レイヤとして追加可能。

### 7.4 サンプルコードのコメント方針
- サンプルは英語/日本語を併記する
- 行頭に書く場合は言語ごとに分ける（例: `// en: ...` の次行に `// ja: ...`）
- 行末に書く場合は 1 行で区切る（例: `// en: ... / ja: ...`）

### 7.5 サンプルの作り分け方針
- ESP32AutoTask: タスク枠が限られるため、1タスク内で複数ノンブロッキング関数を回す「共有タスク構造」を基本にする
- ESP32TaskKit: タスクを気軽に増やせるので、ブロッキング処理を専用タスクに分離する方式を基本にする
- 各機能（Queue/Notify/BinarySemaphore/Mutex）でノンブロックとブロックの双方のサンプルを用意する

---

## 8. 設計ポリシー

- タスク生成・管理は ESP32AutoTask / ESP32TaskKit / FreeRTOS に委譲  
- ESP32AutoSync は同期だけを担当（シンプル&安定）  
- API は tryXXX と XXX の2系統に統一  
- Queue の型だけテンプレートとし、その他はシンプルに保つ
- 戻り値は `bool` で返し、例外は使わない
- 1 tick = 1 ms 前提で設計し、delay() を基本にする
- マルチコアのコア割り当てやスリープ制御は扱わない（タスク側で指定）

---
