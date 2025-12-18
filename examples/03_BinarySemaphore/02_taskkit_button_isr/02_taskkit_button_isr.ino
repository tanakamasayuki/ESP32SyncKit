#include <ESP32TaskKit.h>
#include <ESP32SyncKit.h>

// en: adjust to your board
// ja: ボードに合わせてピン番号を設定
constexpr int kButtonPin = 0;

// en: Button ISR gives semaphore; TaskKit task takes it
// ja: ボタン ISR が give し、TaskKit タスクが take
ESP32SyncKit::BinarySemaphore buttonSem;
ESP32TaskKit::Task handlerTask;

void IRAM_ATTR onButton()
{
  // en: ISR-safe give
  // ja: ISR から安全に give
  if (!buttonSem.give())
  {
    // ISR なのでシリアルは避け、最小限の処理のみ
  }
}

void setup()
{
  Serial.begin(115200);
  pinMode(kButtonPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(kButtonPin), onButton, FALLING);

  // en: TaskKit handler (priority 2), default 1 ms tick to poll semaphore
  // ja: TaskKit ハンドラ（優先度2）。デフォルト 1 ms 周期でセマフォをポーリング
  handlerTask.startLoop(
      []
      {
        if (buttonSem.take())
        {
          Serial.printf("[BinarySemaphore] core=%d, button pressed\n", xPortGetCoreID());
        }
        return true;
      },
      ESP32TaskKit::TaskConfig{.name = "button", .priority = 2});
}

void loop()
{
  delay(1);
}
