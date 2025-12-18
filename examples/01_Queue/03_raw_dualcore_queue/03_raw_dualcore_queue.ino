#include <Arduino.h>
#include <ESP32SyncKit.h>

// en: Core0 -> Core1 queue (FIFO, depth=8)
// ja: コア0→コア1 の FIFO キュー（深さ8）
ESP32SyncKit::Queue<int> q(8);

void sender(void * /*pv*/)
{
  int counter = 0;
  for (;;)
  {
    // en: Blocking send with 1s timeout; retry by loop
    // ja: 1 秒タイムアウトのブロッキング送信。ループで再試行
    if (!q.send(counter++, 1000))
    {
      Serial.println("[Queue/raw] send failed");
    }
    delay(500);
  }
}

void receiver(void * /*pv*/)
{
  int v = 0;
  for (;;)
  {
    // en: Blocking receive; print the received value and core id
    // ja: ブロッキング受信で値とコア番号を表示
    if (q.receive(v))
    {
      Serial.printf("[Queue/raw] core=%d, received=%d\n", xPortGetCoreID(), v);
    }
  }
}

void setup()
{
  Serial.begin(115200);
  // en: Raw FreeRTOS tasks pinned to core0/core1 (priority 2, 4096 words stack)
  // ja: FreeRTOS タスクをコア0/1へ固定（優先度2、スタック4096ワード）
  xTaskCreatePinnedToCore(sender, "sender", 4096, nullptr, 2, nullptr, 0);
  xTaskCreatePinnedToCore(receiver, "receiver", 4096, nullptr, 2, nullptr, 1);
}

void loop()
{
  delay(1);
}
