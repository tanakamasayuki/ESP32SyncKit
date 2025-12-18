#include <Arduino.h>
#include <ESP32SyncKit.h>

// en: One-shot semaphore between raw FreeRTOS tasks
// ja: 生 FreeRTOS タスク間のワンショットセマフォ
ESP32SyncKit::BinarySemaphore sem;

void signaler(void * /*pv*/)
{
  delay(500);
  if (!sem.give())
  {
    Serial.println("[BinarySemaphore/raw] give failed");
  }
  vTaskDelete(nullptr);
}

void waiter(void * /*pv*/)
{
  if (sem.take())
  {
    Serial.printf("[BinarySemaphore/raw] core=%d, got signal\n", xPortGetCoreID());
  }
  vTaskDelete(nullptr);
}

void setup()
{
  Serial.begin(115200);
  xTaskCreatePinnedToCore(signaler, "signaler", 4096, nullptr, 2, nullptr, 0);
  xTaskCreatePinnedToCore(waiter, "waiter", 4096, nullptr, 2, nullptr, 1);
}

void loop()
{
  delay(1);
}
