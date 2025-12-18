#include <ESP32AutoTask.h>
#include <ESP32SyncKit.h>

ESP32SyncKit::BinarySemaphore startSignal;

void setup()
{
  Serial.begin(115200);
  ESP32AutoTask::AutoTask.begin();
}

// en: Producer sends one-shot start signal
// ja: 送信側が一度だけ起動合図を送る
void LoopCore0_Normal()
{
  static bool sent = false;
  if (!sent)
  {
    if (!startSignal.give())
    {
      Serial.println("[BinarySemaphore] give failed");
    }
    sent = true;
  }
}

// en: Consumer waits for start
// ja: 受信側が開始合図を待つ
void LoopCore1_Normal()
{
  if (startSignal.take())
  {
    Serial.printf("[BinarySemaphore] core=%d, start!\n", xPortGetCoreID());
    // en: run once, then idle
    // ja: 1回動かしたら待機
    for (;;)
    {
      delay(1000);
    }
  }
}

void loop()
{
  delay(1);
}
