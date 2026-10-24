// en: Signal a worker with a binary semaphore
// ja: BinarySemaphoreで処理開始を知らせる
#include <ESP32AutoTask.h>
#include <ESP32SyncKit.h>

ESP32SyncKit::BinarySemaphore startSignal;

void setup()
{
  Serial.begin(115200);
  // en: Initialize this component after its prerequisites; inspect the returned status where provided.
  // ja: 前提となる機器を準備してから初期化し、結果を返すAPIでは成否を確認します。
  ESP32AutoTask::AutoTask.begin();
}

// en: Producer gives every 500 ms (if semaphore already given, give may fail)
// ja: 送信側は 500 ms ごとに give（セマフォ保持中は give 失敗する）
void LoopCore0_Normal()
{
  if (!startSignal.give())
  {
    // en: skip if already given; Binary semaphore caps at 1
    // ja: すでに保持されている場合はスキップ（バイナリは最大1）
  }
  delay(500);
}

// en: Consumer waits, prints on each take, then waits again
// ja: 受信側は take できたら出力し、再び待つ
void LoopCore1_Normal()
{
  if (startSignal.take())
  {
    Serial.printf("[BinarySemaphore] core=%d, millis=%lu take!\n", xPortGetCoreID(), static_cast<unsigned long>(millis()));
  }
}

void loop()
{
  delay(1);
}
