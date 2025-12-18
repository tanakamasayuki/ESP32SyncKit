#include <ESP32TaskKit.h>
#include <ESP32SyncKit.h>

// en: Queue between two TaskKit tasks (producer/consumer), depth=8
// ja: TaskKit の2タスク間で使うキュー（送受信）、深さ8
ESP32SyncKit::Queue<int> q(8);
ESP32TaskKit::Task producer;
ESP32TaskKit::Task consumer;

void setup()
{
  Serial.begin(115200);

  // en: TaskKit producer (default stack, priority 2, any core), runs every 500 ms
  // ja: TaskKit 送信タスク（デフォルトスタック、優先度2、コア指定なし）、500 ms 周期
  producer.startLoop(
      []
      {
        static int value = 0;
        // en: Blocking send with 1s timeout; logs if queue is full
        // ja: 1 秒タイムアウトのブロッキング送信。満杯ならログ
        if (!q.send(value++, 1000))
        {
          Serial.println("[Queue/TaskKit] send failed");
        }
        return true;
      },
      ESP32TaskKit::TaskConfig{.name = "producer"},
      500);

  // en: TaskKit consumer (priority 2, no core affinity), default 1 ms tick
  // ja: TaskKit 受信タスク（優先度2、コア指定なし）、デフォルト周期 1 ms
  consumer.startLoop(
      []
      {
        int v = 0;
        // en: Blocking receive; prints core id and value
        // ja: ブロッキング受信でコア番号と値を表示
        if (q.receive(v))
        {
          Serial.printf("[Queue/TaskKit] core=%d, received=%d\n", xPortGetCoreID(), v);
        }
        return true;
      },
      ESP32TaskKit::TaskConfig{.name = "consumer", .priority = 2, .core = tskNO_AFFINITY});
}

void loop()
{
  delay(1);
}
