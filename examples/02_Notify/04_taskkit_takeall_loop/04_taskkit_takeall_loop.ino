// en: Take all pending notification counts in loop
// ja: 保留中の通知回数をloopでまとめて受け取る
#include <ESP32TaskKit.h>
#include <ESP32SyncKit.h>

// en: TaskKit producer sends notify bursts; loop() drains them with takeAll()
// ja: TaskKit 送信タスクで通知をバースト送信し、loop() で takeAll() によりまとめて受信

// en: auto-locks to counter mode on first use
// ja: 初回利用でカウンタモードに自動固定
ESP32SyncKit::Notify n;
ESP32TaskKit::Task producer;

void setup()
{
  Serial.begin(115200);

  // en: Producer (priority 2). Every 400 ms, send a burst of 3 notifications.
  // ja: 送信タスク（優先度2）。400 ms ごとに通知を3件まとめて送信。
  producer.startLoop(
      []
      {
        for (int i = 0; i < 3; ++i)
        {
          if (!n.notify())
          {
            Serial.println("[Notify/takeAll] notify failed");
          }
        }
        return true;
      },
      ESP32TaskKit::TaskConfig{.name = "takeall-producer", .priority = 2},
      400);
}

void loop()
{
  static uint32_t total = 0;
  // en: block up to 1s to gather a batch; takeAll returns the drained count
  // ja: 最大1秒まで待ってまとめ取り。takeAll は取り出した件数を返す
  uint32_t got = n.takeAll(1000);
  if (got > 0)
  {
    total += got;
    uint32_t nowMs = millis();
    Serial.printf("[Notify/takeAll] got batch=%lu (total=%lu) @ %lu ms\n",
                  static_cast<unsigned long>(got),
                  static_cast<unsigned long>(total),
                  static_cast<unsigned long>(nowMs));
  }
}
