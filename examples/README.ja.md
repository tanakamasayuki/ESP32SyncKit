# ESP32SyncKit サンプル

> English: [README.md](README.md)

## 実行ガイドと個別README

最初に[サンプル入門ガイド](../docs/GUIDE_EXAMPLES.ja.md)を読み、目的に合う例へ進んでください。

最初の候補は[01_Queue/01_basic_autotask](01_Queue/01_basic_autotask/README.ja.md)です。必要な機器を確認し、対象が異なる場合は下の一覧から選び直してください。

| 例 | 学ぶこと |
|---|---|
| [01_Queue/01_basic_autotask](01_Queue/01_basic_autotask/README.ja.md) | AutoTaskフック間で値のコピーを渡す |
| [01_Queue/02_taskkit_producer_consumer](01_Queue/02_taskkit_producer_consumer/README.ja.md) | Queueで生成側と処理側をつなぐ |
| [01_Queue/03_raw_dualcore_queue](01_Queue/03_raw_dualcore_queue/README.ja.md) | FreeRTOSの生タスクからQueueを使う |
| [01_Queue/04_taskkit_loop_nonblock](01_Queue/04_taskkit_loop_nonblock/README.ja.md) | Arduino loopを止めずにQueueを確認する |
| [01_Queue/05_irq_gpio_send](01_Queue/05_irq_gpio_send/README.ja.md) | GPIO割り込みをQueueへ渡してタスクで処理する |
| [01_Queue/06_taskkit_send_fail_clear](01_Queue/06_taskkit_send_fail_clear/README.ja.md) | Queue満杯による拒否とclearの動作を確認する |
| [01_Queue/07_send_to_front](01_Queue/07_send_to_front/README.ja.md) | Queueの先頭へ優先する要素を入れる |
| [01_Queue/08_mailbox_overwrite](01_Queue/08_mailbox_overwrite/README.ja.md) | 1要素のMailboxで最新の値だけを保持する |
| [01_Queue/09_taskkit_struct_payload](01_Queue/09_taskkit_struct_payload/README.ja.md) | 固定長の構造体をQueueで受け渡す |
| [02_Notify/01_counter_autotask](02_Notify/01_counter_autotask/README.ja.md) | フック間の通知をカウンタとして受け取る |
| [02_Notify/02_bits_taskkit](02_Notify/02_bits_taskkit/README.ja.md) | 選択したイベントbitの通知を待つ |
| [02_Notify/03_raw_event_bits](02_Notify/03_raw_event_bits/README.ja.md) | FreeRTOSの生タスクでイベントbitを通知する |
| [02_Notify/04_taskkit_takeall_loop](02_Notify/04_taskkit_takeall_loop/README.ja.md) | 保留中の通知回数をloopでまとめて受け取る |
| [03_BinarySemaphore/01_autotask_start_signal](03_BinarySemaphore/01_autotask_start_signal/README.ja.md) | BinarySemaphoreで処理開始を知らせる |
| [03_BinarySemaphore/02_taskkit_button_isr](03_BinarySemaphore/02_taskkit_button_isr/README.ja.md) | ボタン割り込みをSemaphoreでタスクへ渡す |
| [03_BinarySemaphore/03_raw_one_shot](03_BinarySemaphore/03_raw_one_shot/README.ja.md) | 生タスク間で1回の合図を受け渡す |
| [04_Mutex/01_autotask_shared_serial](04_Mutex/01_autotask_shared_serial/README.ja.md) | 共有するシリアル出力をMutexで保護する |
| [04_Mutex/02_taskkit_sensor_bus](04_Mutex/02_taskkit_sensor_bus/README.ja.md) | 2つのタスクが1つのバスLockを使う様子を見る |
| [04_Mutex/03_raw_buffer_guard](04_Mutex/03_raw_buffer_guard/README.ja.md) | 共有カウンタをスコープ付きLockで保護する |
| [04_Mutex/04_taskkit_lockguard_timeout](04_Mutex/04_taskkit_lockguard_timeout/README.ja.md) | Mutexの待ち時間を制限して失敗にも対応する |
