#include <ESP32SyncKit.h>

// en: ISR sends to queue on GPIO interrupt (default pin 0). loop() polls non-blocking and prints.
// ja: GPIO 割り込み（デフォルト pin 0）でキューへ送信し、loop() でノンブロック受信して表示する例

#ifndef BUTTON_PIN
#define BUTTON_PIN 0 // en: change for your board / ja: ボードに合わせて変更
#endif

// en: Queue depth 8; ISR pushes into it
// ja: 深さ 8 のキューに ISR から積む
constexpr uint32_t kQueueDepth = 8;

ESP32SyncKit::Queue<int> q(kQueueDepth);
volatile bool gSendFailed = false;

void IRAM_ATTR onButton()
{
  static int counter = 0;
  // en: ISR context, keep it minimal; no Serial here.
  // ja: ISR 内なので処理は最小限。Serial は使わない
  if (!q.trySend(counter++))
  {
    // en: Avoid logging in ISR; set a flag instead
    // ja: ISR ではログを避け、フラグだけセット
    gSendFailed = true;
  }
}

void setup()
{
  Serial.begin(115200);

  // en: Pull-up the button input and trigger ISR on falling edge
  // ja: ボタン入力をプルアップし、FALLING で割り込み送信
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), onButton, FALLING);
}

void loop()
{
  // en: Report any ISR send failure on the task side
  // ja: ISR 送信の失敗をタスク側で表示
  if (gSendFailed)
  {
    Serial.println("[Queue/ISR] send failed (queue full)");
    gSendFailed = false;
  }

  int v = 0;
  while (q.tryReceive(v))
  {
    // en: Non-blocking receive; print core, data, and remaining count
    // ja: ノンブロック受信でコア番号、データ、残件数を表示
    Serial.printf("[Queue/loop] core=%d, got %d (remaining=%lu)\n",
                  xPortGetCoreID(), v, static_cast<unsigned long>(q.count()));
  }

  delay(1);
}
