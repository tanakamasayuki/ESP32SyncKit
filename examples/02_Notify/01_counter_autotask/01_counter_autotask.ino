#include <ESP32AutoTask.h>
#include <ESP32SyncKit.h>

// en: Notify in counter mode (auto-lock on first use)
// ja: Notify カウンタモード（初回使用でモード確定）
// en: counter mode locks on first use
// ja: 初回使用でカウンタモードに固定
ESP32SyncKit::Notify n;

void setup()
{
  Serial.begin(115200);
  ESP32AutoTask::AutoTask.begin();
}

// en: Producer increments counter
// ja: 送信側がカウンタを増やす
void LoopCore0_Normal()
{
  // en: notify (auto-locks mode to counter)
  // ja: notify（カウンタモードに自動固定）
  if (!n.notify())
  {
    Serial.println("[Notify/counter] notify failed");
  }
  delay(1000);
}

// en: Consumer drains pending notifications every 2 seconds (non-blocking take)
// ja: 受信側は2秒ごとにたまった通知をノンブロックでまとめて消費
void LoopCore1_Normal()
{
  static uint32_t count = 0;
  uint32_t drained = 0;
  while (n.take(0)) // non-block: consume all queued notifications
  {
    ++drained;
    Serial.printf("[Notify/counter] core=%d, got event %u\n", xPortGetCoreID(), ++count);
  }
  // en: throttle checks to show batched output; roughly 2 notifications per loop
  // ja: 2秒ごとにチェックして、約2件まとめて出力される動作を確認
  delay(2000);
}

void loop()
{
  delay(1);
}
