#include <ESP32SyncKit.h>
#include <ESP32TaskKit.h>

// en: TaskKit spams queue to force send failures; loop() watches a GPIO (default pin 0) and clears on falling edge.
// ja: TaskKit で大量送信して失敗を起こし、loop() で GPIO（デフォルト pin 0）の下降エッジを検知してキューをクリアする例

#ifndef CLEAR_PIN
#define CLEAR_PIN 0 // en: change for your board / ja: ボードに合わせて変更
#endif

constexpr uint32_t kQueueDepth = 4;
// en: Burst count to overflow the queue
// ja: キューを溢れさせるためのまとめ送り件数
constexpr int kSpamBatch = 2;
// en: Short period to force congestion
// ja: 短い周期で詰まりを発生させる
constexpr uint32_t kSpamPeriodMs = 200;

ESP32SyncKit::Queue<int> q(kQueueDepth);
ESP32TaskKit::Task spammer;
volatile bool gSendFailed = false;

void setup()
{
  Serial.begin(115200);

  pinMode(CLEAR_PIN, INPUT_PULLUP);

  // en: TaskKit task (default stack, priority 2, any core), runs every 200 ms and spams queue to force failures
  // ja: TaskKit タスク（デフォルトスタック、優先度2、コア指定なし）。200 ms 周期で大量送信してあふれを発生させる
  spammer.startLoop(
      []
      {
        static int value = 0;
        bool failed = false;
        for (int i = 0; i < kSpamBatch; ++i)
        {
          // en: trySend is non-blocking; returns false when the queue is full
          // ja: trySend はノンブロックで、満杯なら false を返す
          if (!q.trySend(value++))
          {
            failed = true;
            break;
          }
        }
        if (failed)
        {
          gSendFailed = true;
        }
        return true;
      },
      ESP32TaskKit::TaskConfig{.name = "spam", .priority = 2},
      kSpamPeriodMs);
}

void loop()
{
  Serial.println("[Queue/loop] start loop iteration =====================================");

  // en: Clear queue on GPIO falling edge (press button when you want to clear)
  // ja: GPIO 下降エッジでキューをクリア（クリアしたいタイミングでボタンを押す想定）
  static int lastLevel = HIGH;
  int level = digitalRead(CLEAR_PIN);
  if (lastLevel == HIGH && level == LOW)
  {
    if (q.clear())
    {
      Serial.println("[Queue/loop] cleared queue on GPIO LOW");
    }
  }
  lastLevel = level;

  if (gSendFailed)
  {
    Serial.println("[Queue/TaskKit spam] send failed (queue full)");
    gSendFailed = false;
  }

  int v = 0;
  while (q.tryReceive(v))
  {
    // en: Non-blocking receive; show core, data, and remaining count
    // ja: ノンブロック受信でコア番号、データ、残件数を表示
    Serial.printf("[Queue/loop] core=%d, got %d (remaining=%lu)\n",
                  xPortGetCoreID(), v, static_cast<unsigned long>(q.count()));
  }

  delay(500);
}
