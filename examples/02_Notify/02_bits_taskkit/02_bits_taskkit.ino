#include <ESP32TaskKit.h>
#include <ESP32SyncKit.h>

constexpr uint32_t kBitRxReady = 1 << 0;
constexpr uint32_t kBitTxDone = 1 << 1;

// en: Notify in bits mode (TaskKit tasks)
// ja: Notify ビットモード（TaskKit タスクで利用）
// en: preset to bits mode
// ja: ビットモードで初期化
ESP32SyncKit::Notify evt(ESP32SyncKit::Notify::Mode::Bits);
ESP32TaskKit::Task producer;
ESP32TaskKit::Task consumer;

void setup()
{
  Serial.begin(115200);

  // en: TaskKit producer (priority 2), runs every 200 ms in a two-phase state machine
  // ja: TaskKit 送信タスク（優先度2）。200 ms 周期の2フェーズ状態機械
  producer.startLoop(
      []
      {
        enum class Phase
        {
          SendRx,
          SendTx
        };
        static Phase phase = Phase::SendRx;
        static uint8_t cooldown = 0; // count 200ms ticks to space events

        if (cooldown > 0)
        {
          --cooldown;
          return true;
        }

        if (phase == Phase::SendRx)
        {
          if (!evt.setBits(kBitRxReady))
          {
            Serial.println("[Notify/bits] setBits RX failed");
          }
          phase = Phase::SendTx;
        }
        else
        {
          if (!evt.setBits(kBitTxDone))
          {
            Serial.println("[Notify/bits] setBits TX failed");
          }
          phase = Phase::SendRx;
          cooldown = 3; // wait another 600ms before the next RX-ready
        }
        return true;
      },
      ESP32TaskKit::TaskConfig{.name = "bits-setter", .priority = 2},
      200);

  // en: TaskKit consumer (priority 2), default 1 ms tick to wait bits cooperatively
  // ja: TaskKit 受信タスク（優先度2）。デフォルト 1 ms 周期でビット待ち
  consumer.startLoop(
      []
      {
        if (evt.waitBits(kBitRxReady | kBitTxDone, true, true))
        {
          Serial.printf("[Notify/bits] core=%d, RX ready & TX done\n", xPortGetCoreID());
        }
        return true;
      },
      ESP32TaskKit::TaskConfig{.name = "bits-waiter", .priority = 2});
}

void loop()
{
  delay(1);
}
