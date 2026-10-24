# 02_taskkit_producer_consumer

> 日本語版: [README.ja.md](README.ja.md)

Connect a producer and consumer through a queue.

## Prepare and run

1. Check the [example guide](../../../docs/GUIDE_EXAMPLES.md) for setup and hardware. Use the root README for supported boards and dependencies.
2. Open [02_taskkit_producer_consumer.ino](02_taskkit_producer_consumer.ino) and select the actual board/port. Prepare the dependencies used by these includes:

   `<ESP32TaskKit.h>`, `<ESP32SyncKit.h>`

3. Upload and open the Serial monitor at 115200 baud. Check startup messages and input conditions in the source.

## Follow the sketch

These APIs are listed in their first source order. Registering a callback does not execute its body at that point.

1. `producer.startLoop(...)` — Start a repeating callback; true continues and false finishes. Check creation success and keep referenced context alive.
2. `q.send(...)` — Submit the supplied data and check the API-specific result; acceptance is different from later processing.
3. `q.receive(...)` — Consume output only after a successful receive; a timeout does not produce a new valid item.

## API usage and precautions

Choose a primitive for the meaning of your data: Queue retains copied values, Notify targets a task, BinarySemaphore retains at most one pending signal, and Mutex protects shared access. This wrapper expresses timeouts in milliseconds; zero means no waiting and WaitForever can block the caller indefinitely. ISR calls must remain bounded and must not print or perform blocking work.

A queue copies the item bytes, not an object graph behind pointers; use fixed, trivially copyable payloads and retain pointed-to storage separately. Notify fixes its mode on use and needs the intended receiving task bound before producers rely on it. Mutex ownership belongs to the task that acquired it; use `LockGuard::locked()` before touching protected data, and keep critical sections short.

## Inputs and observable output

These are selected output formats from the source. printf specifiers are replaced by runtime values; inspect the surrounding conditions rather than assuming these lines always occur in this order.

```text
[Queue/TaskKit] core=%d, received=%d
```

## Verify and troubleshoot

Connect a producer and consumer through a queue and verify the result on its output. First use the supplied input, then change one configuration or data source at a time.

| Symptom | First check |
|---|---|
| No output | Board/dependencies, initialization results, output destination and Serial baud |
| Starts but stops making progress | Wait conditions, buffer space, peer/input readiness and service calls |
| Unexpected values | Lengths/units, format, data lifetime, shared state and partial results |

See the [guide](../../../docs/GUIDE_EXAMPLES.md) for shared principles and the [example index](../../README.md) for the next sketch.
