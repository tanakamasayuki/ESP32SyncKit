# 06_taskkit_send_fail_clear

> 日本語版: [README.ja.md](README.ja.md)

Observe queue-full rejection and clearing.

## Prepare and run

1. Check the [example guide](../../../docs/GUIDE_EXAMPLES.md) for setup and hardware. Use the root README for supported boards and dependencies.
2. Open [06_taskkit_send_fail_clear.ino](06_taskkit_send_fail_clear.ino) and select the actual board/port. Prepare the dependencies used by these includes:

   `<ESP32SyncKit.h>`, `<ESP32TaskKit.h>`

3. Upload and open the Serial monitor at 115200 baud. Check startup messages and input conditions in the source.

## Follow the sketch

These APIs are listed in their first source order. Registering a callback does not execute its body at that point.

1. `spammer.startLoop(...)` — Start a repeating callback; true continues and false finishes. Check creation success and keep referenced context alive.
2. `q.trySend(...)` — Attempt a send without waiting; define what to do if the queue is full.
3. `q.clear(...)` — Discard or reset the selected content intentionally; pending data may be lost.
4. `q.tryReceive(...)` — Poll without waiting and use output only when an item was returned.

Review these application constants before adapting the sketch: `kQueueDepth`, `kSpamBatch`, `kSpamPeriodMs`. Match pins, channels and capacities to your application.

## API usage and precautions

Choose a primitive for the meaning of your data: Queue retains copied values, Notify targets a task, BinarySemaphore retains at most one pending signal, and Mutex protects shared access. This wrapper expresses timeouts in milliseconds; zero means no waiting and WaitForever can block the caller indefinitely. ISR calls must remain bounded and must not print or perform blocking work.

A queue copies the item bytes, not an object graph behind pointers; use fixed, trivially copyable payloads and retain pointed-to storage separately. Notify fixes its mode on use and needs the intended receiving task bound before producers rely on it. Mutex ownership belongs to the task that acquired it; use `LockGuard::locked()` before touching protected data, and keep critical sections short.

The producer deliberately fills the queue. clear() discards pending items; it is a demonstration of a loss policy, not a way to recover those messages.

## Inputs and observable output

These are selected output formats from the source. printf specifiers are replaced by runtime values; inspect the surrounding conditions rather than assuming these lines always occur in this order.

```text
[Queue/loop] start loop iteration =====================================
[Queue/loop] cleared queue on GPIO LOW
[Queue/loop] core=%d, got %d (remaining=%lu)
```

## Verify and troubleshoot

Observe queue-full rejection and clearing and verify the result on its output. First use the supplied input, then change one configuration or data source at a time.

| Symptom | First check |
|---|---|
| No output | Board/dependencies, initialization results, output destination and Serial baud |
| Starts but stops making progress | Wait conditions, buffer space, peer/input readiness and service calls |
| Unexpected values | Lengths/units, format, data lifetime, shared state and partial results |

See the [guide](../../../docs/GUIDE_EXAMPLES.md) for shared principles and the [example index](../../README.md) for the next sketch.
