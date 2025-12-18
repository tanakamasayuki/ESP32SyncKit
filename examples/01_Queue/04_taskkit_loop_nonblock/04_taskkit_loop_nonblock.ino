#include <ESP32SyncKit.h>
#include <ESP32TaskKit.h>

// en: TaskKit producer → main loop does non-blocking receive (tryReceive) and prints items.
// ja: TaskKit で送信し、loop() で tryReceive によるノンブロック受信と表示を行う例

constexpr uint32_t kQueueDepth = 8;
// en: Send once every 500 ms
// ja: 500 ms ごとに 1 件送信
constexpr uint32_t kSendIntervalMs = 500;

ESP32SyncKit::Queue<int> q(kQueueDepth);
ESP32TaskKit::Task producer;

void setup()
{
  Serial.begin(115200);

  // en: TaskKit task (default stack, priority 2, any core), runs every 500 ms and tries non-blocking send
  // ja: TaskKit タスク（デフォルトスタック、優先度2、コア指定なし）。500 ms 周期でノンブロック送信
  producer.startLoop(
      []
      {
        static int value = 0;
        // en: Producer tries non-blocking send; skip if queue is full
        // ja: 送信側はノンブロック送信のみ。満杯ならスキップ
        if (!q.send(value++, 0))
        {
          Serial.println("[Queue/TaskKit] send failed");
        }
        return true;
      },
      ESP32TaskKit::TaskConfig{.name = "producer", .priority = 2},
      kSendIntervalMs);
}

void loop()
{
  int v = 0;
  while (q.tryReceive(v))
  {
    // en: loop() side receives non-blocking and prints the item
    // ja: loop() 側はノンブロック受信でデータを表示
    Serial.printf("[Queue/loop] core=%d, received=%d\n", xPortGetCoreID(), v);
  }

  delay(1);
}
