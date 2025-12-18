#include <Arduino.h>
#include <ESP32SyncKit.h>

// en: Protect shared counter between raw FreeRTOS tasks
// ja: 生 FreeRTOS タスク間で共有カウンタを Mutex で保護
ESP32SyncKit::Mutex bufMutex;
int sharedCounter = 0;

void writer(void * /*pv*/)
{
  for (;;)
  {
    ESP32SyncKit::Mutex::LockGuard lock(bufMutex);
    if (lock.locked())
    {
      sharedCounter++;
      Serial.printf("[Mutex/raw] writer: %d\n", sharedCounter);
    }
    else
    {
      Serial.println("[Mutex/raw] writer lock failed");
    }
    delay(300);
  }
}

void reader(void * /*pv*/)
{
  for (;;)
  {
    ESP32SyncKit::Mutex::LockGuard lock(bufMutex);
    if (lock.locked())
    {
      Serial.printf("[Mutex/raw] reader: %d\n", sharedCounter);
    }
    else
    {
      Serial.println("[Mutex/raw] reader lock failed");
    }
    delay(500);
  }
}

void setup()
{
  Serial.begin(115200);
  xTaskCreatePinnedToCore(writer, "writer", 4096, nullptr, 2, nullptr, 0);
  xTaskCreatePinnedToCore(reader, "reader", 4096, nullptr, 2, nullptr, 1);
}

void loop()
{
  delay(1);
}
