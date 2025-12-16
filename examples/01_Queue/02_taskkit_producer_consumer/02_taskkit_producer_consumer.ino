#include <ESP32TaskKit.h>
#include <ESP32AutoSync.h>

// en: Queue between two TaskKit tasks (producer/consumer), depth=8
// ja: TaskKit の2タスク間で使うキュー（送受信）、深さ8
ESP32AutoSync::Queue<int> q(8);
ESP32TaskKit::Task producer;
ESP32TaskKit::Task consumer;

void setup()
{
  Serial.begin(115200);

  producer.startLoop(
      []
      {
        static int value = 0;
        if (!q.send(value++, 1000))
        {
          Serial.println("[Queue/TaskKit] send failed");
        }
        return true;
      },
      ESP32TaskKit::TaskConfig{.name = "producer"},
      500);

  consumer.startLoop(
      []
      {
        int v = 0;
        if (q.receive(v))
        {
          Serial.printf("[Queue/TaskKit] core=%d, received=%d\n", xPortGetCoreID(), v);
        }
        return true;
      },
      ESP32TaskKit::TaskConfig{.name = "consumer", .priority = 2, .core = tskNO_AFFINITY});
}

void loop()
{
  delay(1);
}
