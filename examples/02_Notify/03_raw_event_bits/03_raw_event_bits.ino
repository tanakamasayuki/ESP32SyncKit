#include <Arduino.h>
#include <ESP32SyncKit.h>

constexpr uint32_t kBitSensor = 1 << 0;
constexpr uint32_t kBitTimeout = 1 << 1;

// en: Notify bits mode between raw FreeRTOS tasks
// ja: 生 FreeRTOS タスク間で Notify のビットモードを利用
ESP32SyncKit::Notify evt(ESP32SyncKit::Notify::Mode::Bits);

void producer(void * /*pv*/)
{
  for (;;)
  {
    if (!evt.setBits(kBitSensor))
    {
      Serial.println("[Notify/raw] setBits sensor failed");
    }
    delay(300);
    if (!evt.setBits(kBitTimeout))
    {
      Serial.println("[Notify/raw] setBits timeout failed");
    }
    delay(700);
  }
}

void consumer(void * /*pv*/)
{
  for (;;)
  {
    if (evt.waitBits(kBitSensor | kBitTimeout))
    {
      Serial.printf("[Notify/raw] core=%d, bits=0x%02lx\n",
                    xPortGetCoreID(), (unsigned long)(kBitSensor | kBitTimeout));
    }
  }
}

void setup()
{
  Serial.begin(115200);
  xTaskCreatePinnedToCore(producer, "setBits", 4096, nullptr, 2, nullptr, 0);
  xTaskCreatePinnedToCore(consumer, "waitBits", 4096, nullptr, 2, nullptr, 1);
}

void loop()
{
  delay(1);
}
