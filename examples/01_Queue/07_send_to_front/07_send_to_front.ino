#include <ESP32AutoSync.h>
#include <ESP32TaskKit.h>

// en: Demonstrate sendToFront by inserting urgent items at the front of the queue
// ja: sendToFront で緊急データをキュー先頭に追加する例

constexpr uint32_t kQueueDepth = 8;
constexpr uint32_t kNormalPeriodMs = 300;
constexpr uint32_t kUrgentInterval = 5;

ESP32AutoSync::Queue<int> q(kQueueDepth);
ESP32TaskKit::Task producer;
ESP32TaskKit::Task consumer;

void setup()
{
  Serial.begin(115200);

  // en: Producer (priority 2). Sends normal items every 300 ms; every 5th send is urgent and goes to the front.
  // ja: 送信タスク（優先度2）。300 ms ごとに通常送信、5回に1回は先頭へ緊急送信。
  producer.startLoop(
      []
      {
        static int value = 0;
        static uint32_t tick = 0;
        ++tick;

        const bool urgent = (tick % kUrgentInterval == 0);
        int payload = urgent ? -1000 - static_cast<int>(tick) : value++;

        if (urgent)
        {
          // en: Push urgent item to the front so it is received next
          // ja: 緊急データを先頭に積んで次に受信されるようにする
          if (!q.sendToFront(payload, 0))
          {
            Serial.println("[Queue/front] sendToFront failed");
          }
        }
        else
        {
          if (!q.send(payload, 0))
          {
            Serial.println("[Queue/front] send failed");
          }
        }
        return true;
      },
      ESP32TaskKit::TaskConfig{.name = "front-producer", .priority = 2},
      kNormalPeriodMs);

  // en: Consumer (priority 2). Receives blocking and shows the core to observe front insertion.
  // ja: 受信タスク（優先度2）。ブロッキング受信でコア番号を表示し、先頭追加の効果を確認。
  consumer.startLoop(
      []
      {
        int v = 0;
        if (q.receive(v))
        {
          Serial.printf("[Queue/front] core=%d, received=%d\n", xPortGetCoreID(), v);
        }
        return true;
      },
      ESP32TaskKit::TaskConfig{.name = "front-consumer", .priority = 2});
}

void loop()
{
  delay(1);
}
