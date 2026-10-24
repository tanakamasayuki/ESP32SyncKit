# ESP32SyncKit examples

> 日本語版: [README.ja.md](README.ja.md)

## Workflow and individual READMEs

Start with the [example workflow](../docs/GUIDE_EXAMPLES.md), then choose the relevant sketch.

A useful starting point is [01_Queue/01_basic_autotask](01_Queue/01_basic_autotask/README.md). Check its required hardware and choose another entry below if it does not match your setup.

| Example | Learn |
|---|---|
| [01_Queue/01_basic_autotask](01_Queue/01_basic_autotask/README.md) | Pass copied values between AutoTask hooks |
| [01_Queue/02_taskkit_producer_consumer](01_Queue/02_taskkit_producer_consumer/README.md) | Connect a producer and consumer through a queue |
| [01_Queue/03_raw_dualcore_queue](01_Queue/03_raw_dualcore_queue/README.md) | Use a queue from raw FreeRTOS tasks |
| [01_Queue/04_taskkit_loop_nonblock](01_Queue/04_taskkit_loop_nonblock/README.md) | Poll queued values without blocking Arduino loop |
| [01_Queue/05_irq_gpio_send](01_Queue/05_irq_gpio_send/README.md) | Queue a GPIO interrupt signal for task processing |
| [01_Queue/06_taskkit_send_fail_clear](01_Queue/06_taskkit_send_fail_clear/README.md) | Observe queue-full rejection and clearing |
| [01_Queue/07_send_to_front](01_Queue/07_send_to_front/README.md) | Insert an item at the front of a queue |
| [01_Queue/08_mailbox_overwrite](01_Queue/08_mailbox_overwrite/README.md) | Keep only the newest value in a one-slot mailbox |
| [01_Queue/09_taskkit_struct_payload](01_Queue/09_taskkit_struct_payload/README.md) | Transfer a fixed-size structure through a queue |
| [02_Notify/01_counter_autotask](02_Notify/01_counter_autotask/README.md) | Count task notifications between hooks |
| [02_Notify/02_bits_taskkit](02_Notify/02_bits_taskkit/README.md) | Wait for selected notification event bits |
| [02_Notify/03_raw_event_bits](02_Notify/03_raw_event_bits/README.md) | Use event-bit notifications from raw tasks |
| [02_Notify/04_taskkit_takeall_loop](02_Notify/04_taskkit_takeall_loop/README.md) | Take all pending notification counts in loop |
| [03_BinarySemaphore/01_autotask_start_signal](03_BinarySemaphore/01_autotask_start_signal/README.md) | Signal a worker with a binary semaphore |
| [03_BinarySemaphore/02_taskkit_button_isr](03_BinarySemaphore/02_taskkit_button_isr/README.md) | Defer a button interrupt with a semaphore |
| [03_BinarySemaphore/03_raw_one_shot](03_BinarySemaphore/03_raw_one_shot/README.md) | Send a one-shot signal between raw tasks |
| [04_Mutex/01_autotask_shared_serial](04_Mutex/01_autotask_shared_serial/README.md) | Protect shared Serial output with a mutex |
| [04_Mutex/02_taskkit_sensor_bus](04_Mutex/02_taskkit_sensor_bus/README.md) | Observe two tasks competing for one bus lock |
| [04_Mutex/03_raw_buffer_guard](04_Mutex/03_raw_buffer_guard/README.md) | Guard a shared counter with scoped locking |
| [04_Mutex/04_taskkit_lockguard_timeout](04_Mutex/04_taskkit_lockguard_timeout/README.md) | Handle a bounded wait for a mutex |
