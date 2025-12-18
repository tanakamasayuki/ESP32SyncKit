#include <ESP32AutoSync.h>
#include <ESP32TaskKit.h>

// en: Mailbox-style queue (depth=1) using overwrite to keep the latest value only
// ja: 深さ1のキューを overwrite で最新値だけ保持するメールボックス例

ESP32AutoSync::Queue<int> mailbox(1); // depth 1 -> mailbox semantics
ESP32TaskKit::Task producer;
ESP32TaskKit::Task consumer;

constexpr uint32_t kProducePeriodMs = 30;
constexpr uint32_t kConsumePeriodMs = 200;

void setup()
{
  Serial.begin(115200);

  // en: Fast producer (priority 2). Overwrites the single-slot mailbox with the newest value.
  // ja: 高頻度の送信タスク（優先度2）。1スロットのメールボックスを最新値で上書き。
  producer.startLoop(
      []
      {
        static int value = 0;
        if (!mailbox.overwrite(value++))
        {
          Serial.println("[Queue/mailbox] overwrite failed");
        }
        return true;
      },
      ESP32TaskKit::TaskConfig{.name = "mailbox-producer", .priority = 2},
      kProducePeriodMs);

  // en: Slow consumer (priority 2). Polls every 200 ms and reads the freshest value non-blocking.
  // ja: 低頻度の受信タスク（優先度2）。200 ms ごとに最新値を非ブロックで取得。
  consumer.startLoop(
      []
      {
        int v = 0;
        if (mailbox.tryReceive(v))
        {
          Serial.printf("[Queue/mailbox] core=%d, latest=%d\n", xPortGetCoreID(), v);
        }
        return true;
      },
      ESP32TaskKit::TaskConfig{.name = "mailbox-consumer", .priority = 2},
      kConsumePeriodMs);
}

void loop()
{
  delay(1);
}
