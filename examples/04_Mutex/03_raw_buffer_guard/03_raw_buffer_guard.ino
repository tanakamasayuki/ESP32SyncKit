// en: Guard a shared counter with scoped locking
// ja: 共有カウンタをスコープ付きLockで保護する
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
    {
      ESP32SyncKit::Mutex::LockGuard lock(bufMutex);
      // en: Check scoped-lock acquisition before accessing protected data.
      // ja: 保護した値へ触る前に、スコープ付きLockの取得を確認します。
      if (lock.locked())
      {
        sharedCounter++;
        Serial.printf("[Mutex/raw] writer: %d\n", sharedCounter);
      }
      else
      {
        Serial.println("[Mutex/raw] writer lock failed");
      }
    }
    // en: lock is released here (end of scope) before sleeping
    // ja: スコープ終端でロックを解放してからスリープ
    delay(300);
  }
}

void reader(void * /*pv*/)
{
  for (;;)
  {
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
    }
    // en: lock is released here (end of scope) before sleeping
    // ja: スコープ終端でロックを解放してからスリープ
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
