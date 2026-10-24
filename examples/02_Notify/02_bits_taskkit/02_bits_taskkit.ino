// en: Wait for selected notification event bits
// ja: 選択したイベントbitの通知を待つ
#include <ESP32TaskKit.h>
#include <ESP32SyncKit.h>

constexpr uint32_t kBitRxReady = 1 << 0; // en: RX ready flag / ja: RX ready 用ビット
constexpr uint32_t kBitTxDone = 1 << 1;  // en: TX done flag / ja: TX done 用ビット
// en: unused/noisy bit (should be ignored)
// ja: 未使用・ノイズ用ビット（無視される想定）
constexpr uint32_t kBitNoise = 1 << 2;

// en: Notify in bits mode (TaskKit tasks)
// ja: Notify ビットモード（TaskKit タスクで利用）
ESP32SyncKit::Notify evt;
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
        // en: count 200 ms ticks to space events
        // ja: 200 ms 単位で間引き
        static uint8_t cooldown = 0;

        if (cooldown > 0)
        {
          --cooldown;
          return true;
        }

        if (phase == Phase::SendRx)
        {
          // en: publish RX-ready first
          // ja: まず RX ready をセット
          if (!evt.setBits(kBitRxReady))
          {
            Serial.println("[Notify/bits] setBits RX failed");
          }
          phase = Phase::SendTx;
        }
        else
        {
          // en: then publish TX-done, leaving both bits set for the consumer
          // ja: 次に TX done をセットし、両ビットが揃った状態にする
          if (!evt.setBits(kBitTxDone))
          {
            Serial.println("[Notify/bits] setBits TX failed");
          }
          // en: also set an unused bit to demonstrate that waitBits ignores unmasked bits
          // ja: マスク外ビットは waitBits で無視されることを示すため、未使用ビットも立てる
          (void)evt.setBits(kBitNoise);
          phase = Phase::SendRx;
          // en: wait another 600 ms before the next RX-ready
          // ja: 次の RX ready まで 600 ms 空ける
          cooldown = 3;
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
        // en: remember previous reception time
        // ja: 前回受信時刻を保持
        static uint32_t lastMs = 0;
        // en: wait for both bits (RX+TX). Only masked bits matter; extra bits are ignored. Clear on exit.
        // ja: RX+TX の2ビットが揃うまで待機。マスクしたビットだけを評価し、他のビットは無視。復帰時にクリア。
        if (evt.waitBits(kBitRxReady | kBitTxDone, true, true))
        {
          uint32_t nowMs = millis();
          uint32_t deltaMs = (lastMs == 0) ? 0 : (nowMs - lastMs);
          lastMs = nowMs;
          Serial.printf("[Notify/bits] core=%d, RX ready & TX done @ %lu ms (delta=%lu ms)\n",
                        xPortGetCoreID(),
                        static_cast<unsigned long>(nowMs),
                        static_cast<unsigned long>(deltaMs));
        }
        return true;
      },
      ESP32TaskKit::TaskConfig{.name = "bits-waiter", .priority = 2});
}

void loop()
{
  delay(1);
}
