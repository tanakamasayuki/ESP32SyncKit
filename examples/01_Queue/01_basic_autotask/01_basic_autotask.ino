#include <ESP32AutoTask.h>
#include <ESP32SyncKit.h>

// en: Queue for int messages between AutoTask hooks on different cores (depth=8)
// ja: AutoTask フック間で int をやり取りするキュー（深さ8）
ESP32SyncKit::Queue<int> q(8);

void setup()
{
  Serial.begin(115200);
  // en: starts weak-hook tasks on core0/core1
  // ja: コア0/1の弱シンボルタスクを起動
  ESP32AutoTask::AutoTask.begin();
}

// en: Producer (core0 normal)
// ja: 送信側（コア0, Normal）
void LoopCore0_Normal()
{
  static int counter = 0;
  // en: Blocking send with default timeout; logs on failure
  // ja: デフォルトタイムアウトでブロッキング送信。失敗時はログ
  if (!q.send(counter++))
  {
    Serial.println("[Queue] send failed");
  }
  delay(500); // en: throttle send rate / ja: 送信間隔を確保
}

// en: Consumer (core1 normal)
// ja: 受信側（コア1, Normal）
void LoopCore1_Normal()
{
  int value = 0;
  // en: Blocking receive; prints the received value
  // ja: ブロッキング受信で値を取得し、表示
  if (q.receive(value))
  {
    Serial.printf("[Queue] core=%d, received=%d\n", xPortGetCoreID(), value);
  }
}

void loop()
{
  delay(1);
}
