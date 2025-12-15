#include <ESP32TaskKit.h>
#include <ESP32AutoSync.h>

using namespace ESP32AutoSync;
using namespace TaskKit;

// en: adjust to your board
// ja: ボードに合わせてピン番号を設定
constexpr int kButtonPin = 0;

// en: Button ISR gives semaphore; TaskKit task takes it
// ja: ボタン ISR が give し、TaskKit タスクが take
BinarySemaphore buttonSem;
Task handlerTask;

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

  handlerTask.startLoop(
      [] {
        if (buttonSem.take())
        {
          Serial.println("[BinarySemaphore] button pressed");
        }
        return true;
      },
      TaskConfig{.name = "button", .priority = 2});
}

void loop()
{
  delay(1);
}
