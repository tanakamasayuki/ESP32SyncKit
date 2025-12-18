# ESP32SyncKit

English | [日本語](README.ja.md)

Header-only C++ wrappers for FreeRTOS sync primitives on ESP32 (Arduino). Designed to teach correct FreeRTOS usage through an Arduino-like API while staying safe for both tasks and ISRs.

## What is this?
- Header-only sync layer for Arduino-ESP32 (FreeRTOS).
- Safe from both tasks and ISRs; FromISR is picked automatically.
- Works with any task source: ESP32AutoTask, ESP32TaskKit, or raw FreeRTOS tasks.

## Highlights
- Type-safe queues, RAII helpers, unified blocking/non-blocking APIs.
- Common log tag: `ESP32SyncKit`. Add class markers like `[Queue]` if needed.
- WaitForever constant for “block forever” (forced non-block in ISR).

## Components
- Queue<T>: typed queue with ISR auto-detection.
- Notify: task notification wrapper (counter or bit mode per instance).
- BinarySemaphore: one-shot event handoff, ISR give supported.
- Mutex: priority-inheritance mutex (non-recursive), LockGuard included.

## How it fits with task libraries
- ESP32AutoTask: use weak hooks (`LoopCore0_*`, `LoopCore1_*`) and call ESP32SyncKit inside them.
- ESP32TaskKit: create tasks with ESP32TaskKit and use ESP32SyncKit for sync; clear separation between task mgmt and sync.
- Raw FreeRTOS: tasks created with `xTaskCreatePinnedToCore` can use ESP32SyncKit directly.

## Install
- Arduino IDE Library Manager: search “ESP32SyncKit”.
- Manual: download the release ZIP and place under `Arduino/libraries`.

## Quick usage (preview)
```cpp
#include <ESP32SyncKit.h>

ESP32SyncKit::Queue<int> q(4);

void IRAM_ATTR onGpio()
{
  q.trySend(42); // From ISR: non-blocking send
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

## Compatibility
- Target: Arduino-ESP32 (FreeRTOS tick = 1 ms). Dual-core aware; ISR-safe where applicable.
- Not intended for ESP8266 or non-FreeRTOS platforms.

## Documentation
- [Specification (EN)](SPEC.md)
- [仕様書 (JA)](SPEC.ja.md)

## Related libraries
- ESP32AutoTask: https://github.com/tanakamasayuki/ESP32AutoTask
- ESP32TaskKit: https://github.com/tanakamasayuki/ESP32TaskKit
- ESP32SyncKit: https://github.com/tanakamasayuki/ESP32SyncKit

## License
- MIT License (see LICENSE).
