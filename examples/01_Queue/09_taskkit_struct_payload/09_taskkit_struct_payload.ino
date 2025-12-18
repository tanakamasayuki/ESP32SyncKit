#include <ESP32TaskKit.h>
#include <ESP32SyncKit.h>

// en: TaskKit producer/consumer that pass a struct via Queue (depth=6)
// ja: TaskKit の送受信タスクで構造体をキュー経由で受け渡し（深さ6）

// en: Plain-old-data payload to enqueue
// ja: キューに載せるシンプルな POD 構造体
struct SensorPacket
{
  uint32_t seq;
  float temperatureC;
  uint32_t timestampMs;
};

constexpr uint32_t kQueueDepth = 6;
constexpr uint32_t kProducePeriodMs = 400;

ESP32SyncKit::Queue<SensorPacket> q(kQueueDepth);
ESP32TaskKit::Task producer;
ESP32TaskKit::Task consumer;

void setup()
{
  Serial.begin(115200);

  // en: Producer (priority 2) sends a SensorPacket every 400 ms with 1 s timeout
  // ja: 送信タスク（優先度2）。400 ms ごとに SensorPacket を 1 秒タイムアウトで送信
  producer.startLoop(
      []
      {
        static uint32_t seq = 0;
        SensorPacket pkt{
            .seq = seq++,
            .temperatureC = 24.5f + (seq % 20) * 0.1f, // en/ja: dummy value
            .timestampMs = millis(),
        };
        if (!q.send(pkt, 1000))
        {
          Serial.println("[Queue/struct] send failed");
        }
        return true;
      },
      ESP32TaskKit::TaskConfig{.name = "producer", .priority = 2},
      kProducePeriodMs);

  // en: Consumer (priority 2, any core). Blocks on receive and prints struct fields.
  // ja: 受信タスク（優先度2, コア指定なし）。ブロッキング受信して構造体の内容を表示。
  consumer.startLoop(
      []
      {
        SensorPacket pkt{};
        if (q.receive(pkt))
        {
          Serial.printf("[Queue/struct] core=%d, seq=%lu, temp=%.1f C, ts=%lu ms\n",
                        xPortGetCoreID(),
                        static_cast<unsigned long>(pkt.seq),
                        static_cast<double>(pkt.temperatureC),
                        static_cast<unsigned long>(pkt.timestampMs));
        }
        return true;
      },
      ESP32TaskKit::TaskConfig{.name = "consumer", .priority = 2, .core = tskNO_AFFINITY});
}

void loop()
{
  delay(1);
}
