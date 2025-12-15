#include <Arduino.h>
#include <ESP32AutoSync.h>

// en: Core0 -> Core1 queue (FIFO, depth=8)
// ja: コア0→コア1 の FIFO キュー（深さ8）
ESP32AutoSync::Queue<int> q(8);

void sender(void * /*pv*/)
{
  int counter = 0;
  for (;;)
  {
    if (!q.send(counter++, 1000))
    {
      Serial.println("[Queue/raw] send failed");
    }
    delay(200);
  }
}

void receiver(void * /*pv*/)
{
  int v = 0;
  for (;;)
  {
    if (q.receive(v))
    {
      Serial.printf("[Queue/raw] core=%d recv=%d\n", xPortGetCoreID(), v);
    }
  }
}

void setup()
{
  Serial.begin(115200);
  xTaskCreatePinnedToCore(sender, "sender", 4096, nullptr, 2, nullptr, 0);
  xTaskCreatePinnedToCore(receiver, "receiver", 4096, nullptr, 2, nullptr, 1);
}

void loop()
{
  delay(1);
}
