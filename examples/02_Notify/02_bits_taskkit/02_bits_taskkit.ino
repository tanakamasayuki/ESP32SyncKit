#include <ESP32TaskKit.h>
#include <ESP32AutoSync.h>

constexpr uint32_t kBitRxReady = 1 << 0;
constexpr uint32_t kBitTxDone = 1 << 1;

// en: Notify in bits mode (TaskKit tasks)
// ja: Notify ビットモード（TaskKit タスクで利用）
// en: preset to bits mode
// ja: ビットモードで初期化
ESP32AutoSync::Notify evt(ESP32AutoSync::Notify::Mode::Bits);
ESP32TaskKit::Task producer;
ESP32TaskKit::Task consumer;

void setup()
{
  Serial.begin(115200);

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

  consumer.startLoop(
      []
      {
        if (evt.waitBits(kBitRxReady | kBitTxDone, true, true))
        {
          Serial.println("[Notify/bits] RX ready & TX done");
        }
        return true;
      },
      ESP32TaskKit::TaskConfig{.name = "bits-waiter", .priority = 2});
}

void loop()
{
  delay(1);
}
