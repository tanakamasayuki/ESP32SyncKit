# ESP32AutoSync Specification

ESP32AutoSync is a **header-only C++ wrapper around FreeRTOS sync primitives for ESP32 (Arduino)**.

Official repository: https://github.com/tanakamasayuki/ESP32AutoSync

It is designed so Arduino-style modern C++ naturally guides you to correct FreeRTOS usage. Built on FreeRTOS Queue / TaskNotify / BinarySemaphore / Mutex with:
- **Type-safe templates**
- **RAII**
- **Automatic ISR detection**
- **Unified blocking / non-blocking APIs**

ESP32AutoSync works seamlessly with:
- Entry-level task library: **ESP32AutoTask** (https://github.com/tanakamasayuki/ESP32AutoTask)
- Intermediate task manager: **ESP32TaskKit** (https://github.com/tanakamasayuki/ESP32TaskKit)
- Raw FreeRTOS tasks (`xTaskCreatePinnedToCore`, etc.)

> **Create tasks however you like.  
> AutoSync is the layer that only handles “sync between tasks.”**

---

## 1. Concept

Goals of ESP32AutoSync:

1. Make FreeRTOS Queue / Notify / Semaphore / Mutex safe and intuitive in C++.
2. Keep Arduino-like ergonomics while encouraging correct FreeRTOS usage.
3. Provide APIs safe from both tasks and ISRs.
4. Offer one API surface regardless of task source (AutoTask / TaskKit / raw FreeRTOS).
5. Encourage good habits: ISRs “signal only,” tasks do the work.
6. Bridge from training wheels to raw FreeRTOS: works nicely with AutoTask weak hooks and TaskKit configs/coop-stop, so moving to raw FreeRTOS is easy.

---

## 2. Relationship to Other Libraries

### 2.1 With ESP32AutoTask
Weak-hook tasks (`LoopCore0_Low()`, etc.) created by AutoTask can call AutoSync Queue / Notify / Mutex directly.

### 2.2 With ESP32TaskKit
Tasks created by TaskKit use AutoSync naturally: TaskKit manages tasks, AutoSync handles sync.

### 2.3 With Raw FreeRTOS Tasks
Tasks made via `xTaskCreatePinnedToCore()` can also use AutoSync. Only core FreeRTOS APIs are required; works in Arduino.

### 2.4 Roles and Learning Path
- **AutoTask (training wheels):** `begin()` gives Low/Normal/High slots on core0/1; define weak hooks to run; undefined hooks exit immediately (zero overhead).
- **TaskKit (intermediate):** `TaskConfig` + `start/startLoop` to set stack/priority/core; `requestStop` for cooperative exit; lambdas/functors for C++ style.
- **AutoSync (this library):** Any task source; unified API for Queue/Notify/Semaphore/Mutex; examples teach “ISR signals, tasks do work.”
- Learning flow: start with AutoTask, learn sync with AutoSync, then design tasks flexibly with TaskKit.

---

## 3. Namespace and Layout

### 3.1 Namespace
```cpp
namespace ESP32AutoSync {
    // Queue<T>, Notify, BinarySemaphore, Mutex, etc.
}
```

### 3.2 Files
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
Users include ESP32AutoSync.h.

---

## 4. Common Specs

### 4.1 WaitForever
```cpp
constexpr uint32_t WaitForever = portMAX_DELAY;
```
`timeoutMs = WaitForever` waits with `portMAX_DELAY`. In ISRs it is forced to non-blocking.

### 4.2 ISR Auto-Detection
Internally uses `xPortInIsrContext()` and picks FromISR APIs automatically.

### 4.3 Unified APIs
- `tryXXX()` … non-blocking  
- `XXX(timeoutMs)` … blocking (default infinite)

`tryXXX()` calls `XXX(0)`.

### 4.4 Scope and Use Cases
- Queue<T>  
  - Type-safe data passing between tasks. Move data ISR→task or task→task. Depth is set in ctor; can be used like a ring buffer.
- Notify  
  - Counter: `notify()` / `take(timeoutMs)` to convey event counts from ISR→task (lightweight counting semaphore).  
  - Bits: `setBits(mask)` / `waitBits(mask, timeoutMs, clearOnExit)` to wait for multiple events in one notification (e.g., RX_READY / TX_DONE / ERROR).  
  - Value-write modes (overwrite/no-overwrite) are out of initial scope.
- BinarySemaphore  
  - One-shot event handoff or task start signal. ISR `give`, task `take`.
- Mutex  
  - Protect shared resources (I2C/SPI/Serial, shared buffers). Task-only; do not use from ISR.

#### Quick Guidance
- Need to carry data → Queue<T> (typed, ordered).
- Need lightweight “arrived/how many” → Notify (counter).
- Need to wait on multiple event types → Notify (bits).
- One-shot signal, counter not needed → BinarySemaphore.
- Need mutual exclusion → Mutex (task-only).

### 4.5 Error Handling
- No exceptions; return `bool` for success/failure.
- On failure, log at appropriate level; caller is expected to recover.

### 4.6 Time and Scheduling
- Assume Arduino `delay()` and tick = 1 ms.
- Other tick rates or high-precision time APIs are out of scope.

### 4.7 Logging
- Always use ESP-IDF `ESP_LOGE/W/I/D/V` (available in Arduino).
- E=critical/failure, W=retry/timeout, I=init/settings, D/V=debug details.
- Logging level init is handled by board definitions; library does not touch it.

### 4.8 Configuration
- No global settings initially. All via ctor/method args.
- No macros or external config files; pure code. (Small Config struct may be added later.)

### 4.9 Out of Scope
- Core affinity is decided by task creation, not by this library.
- Power management / sleep control is not considered.

---

## 5. Class Specs

### 5.0 Lifecycle & Ownership (common policy)
- RAII: ctor creates the FreeRTOS resource, dtor deletes it (except Notify, which has no RTOS object). On creation failure, keep handle null; methods return false and log.
- Copy is disallowed. Move is allowed; moved-from instances clear their handles.
- For global/static use, consider lazy creation on first use to avoid static-init ordering issues before FreeRTOS is fully up.

### 5.1 Queue<T>
Typed queue with ISR auto-detection.

```cpp
Queue<T> q(depth);                   // Depth set in ctor
q.trySend(value);                    // == send(value, 0)
q.send(value, timeoutMs = WaitForever);
q.tryReceive(out);                   // == receive(out, 0)
q.receive(out, timeoutMs = WaitForever);
```

- In tasks, `timeoutMs = WaitForever` blocks forever; in ISR it is forced non-blocking.  
- `send/receive` auto-select `xQueueSend` / `xQueueSendFromISR` / `xQueueReceive` / `xQueueReceiveFromISR`, with `portYIELD_FROM_ISR` handled inside when needed.  
- `T` should be copy/move-capable. For large payloads, pass pointers or small structs.  
- Returns `bool` (false on timeout/full). Failures log; caller recovers.

### 5.2 Notify
Task notification (counter/bits). Use `notify()` to accumulate, `take()` to consume one, `setBits()` / `waitBits()` for bit waits.

```cpp
// Counter mode (lightweight counting semaphore)
notify.notify();                         // give. Auto Task/ISR
notify.take(timeoutMs = WaitForever);    // consume one (bool)
notify.tryTake();                        // == take(0)

// Bit mode (EventGroup-like)
notify.setBits(mask);                    // OR accumulate. ISR-safe
notify.waitBits(mask,
                timeoutMs = WaitForever,
                clearOnExit = true,
                waitAll = false);        // any-of by default
notify.tryWaitBits(mask,
                   clearOnExit = true,
                   waitAll = false);     // == waitBits(mask, 0, clearOnExit, waitAll)
```

- Counter mode mirrors `ulTaskNotifyTake`: `notify()` counts up; `take()` consumes one; remaining counts stay for later `take()`.  
- Bit mode is based on `xTaskNotifyWait`: `waitAll=false` waits for any bit in mask, `true` waits for all; `clearOnExit=true` clears satisfied bits.  
- ISR `notify()` / `setBits()` auto-pick FromISR versions and call `portYIELD_FROM_ISR` as needed.  
- `timeoutMs = WaitForever` blocks forever in tasks, forced to 0 ms (non-blocking) in ISR. All return `bool` (success/timeout).
- Binding policy: start unbound. The first task that calls `take`/`waitBits` auto-binds as the receiver and remains fixed. If you want explicit binding, support ctor or `bindTo(handle)` / `bindToSelf()`, with no rebind allowed. Calling `notify`/`setBits` while unbound or calling `take`/`waitBits` from a non-receiver task returns false and logs a warning.
- Mode policy: each instance is either “counter” or “bits”. Either specify via ctor or auto-lock on the first API used (`take` family vs `waitBits` family). Calls from the other mode are rejected (false + log). Re-locking is not allowed.

### 5.3 BinarySemaphore
Thin wrapper of FreeRTOS binary semaphore. Use for one-shot events or start signals.

```cpp
binary.give();                          // Auto Task/ISR
binary.take(timeoutMs = WaitForever);   // bool
binary.tryTake();                       // == take(0)
```

- `give` auto-selects FromISR and runs `portYIELD_FROM_ISR` when needed.  
- `take` blocks with `WaitForever` in tasks; forced 0 ms in ISR. Returns success/timeout.
- Created via `xSemaphoreCreateBinary` (initial count 0). If creation fails, handle is null. Copy disallowed; move allowed. Lazy creation on first use is acceptable if needed.

### 5.4 Mutex
Wrapper of FreeRTOS standard mutex. Mutual exclusion for shared resources (task-only).

```cpp
mutex.lock(timeoutMs = WaitForever);    // bool
mutex.tryLock();                        // == lock(0)
mutex.unlock();                         // owner only
Mutex::LockGuard guard(mutex);          // RAII unlock on scope exit
```

- Uses priority-inheritance mutex. Do not use from ISR.  
- `lock` supports infinite wait; returns false on timeout.  
- `LockGuard` prevents leak/forget to unlock, even without exceptions.
- Created via `xSemaphoreCreateMutex` (non-recursive, priority inheritance). On failure, handle is null. Copy disallowed; move allowed. Lazy creation on first use is acceptable.
- LockGuard behavior: ctor calls `lock(timeoutMs)`, sets a “held” flag only on success. On failure, flag stays false (log a warning); dtor unlocks only when held to avoid double-unlock. Provide `locked()` (or similar) so callers can check acquisition. Default `timeoutMs` is `WaitForever` (block until acquired); if you need bounded wait, pass a shorter timeout and handle the failure explicitly.

---

## 6. ISR Behavior

- Blocking is forbidden in ISR (forced to tryXXX/0 ms).
- `portYIELD_FROM_ISR` handled internally where required.
- ISR should “signal only”; tasks do the actual work (examples follow this pattern).

---

## 7. Use Cases

### 7.1 ESP32AutoTask + ESP32AutoSync
ISR → ESP32AutoTask tasks for processing handoff.

### 7.2 ESP32TaskKit + ESP32AutoSync
Share queues among multiple ESP32TaskKit tasks.

### 7.3 Raw FreeRTOS + ESP32AutoSync
Add AutoSync as the sync layer for existing task sets.

### 7.4 Sample Comment Policy
- Samples include both English/Japanese.
- If placed at line start, separate by language (`// en: ...` then next line `// ja: ...`).
- If inline, keep in one line (`// en: ... / ja: ...`).

### 7.5 Sample Composition Policy
- ESP32AutoTask: task slots are limited; prefer “shared task structure” that runs multiple non-blocking functions in one task.
- ESP32TaskKit: tasks are cheap; split blocking work into dedicated tasks.
- Provide both non-blocking and blocking samples for each feature (Queue/Notify/BinarySemaphore/Mutex).

---

## 8. Design Policy

- Task creation/management is delegated to ESP32AutoTask / ESP32TaskKit / FreeRTOS.  
- ESP32AutoSync focuses solely on synchronization (simple & stable).  
- API is unified into tryXXX and XXX variants.  
- Only Queue is templated; keep others simple.  
- Return `bool`; no exceptions.  
- Assume tick = 1 ms, use `delay()`.  
- No core-affinity or sleep control inside this library (task side decides).

---
