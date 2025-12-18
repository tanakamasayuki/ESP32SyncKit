#include <ESP32TaskKit.h>
#include <ESP32SyncKit.h>

// en: LockGuard with timeout; one task holds the mutex long, another times out and retries
// ja: LockGuard のタイムアウト例。1タスクが長時間保持し、もう1タスクはタイムアウトしつつ再試行

ESP32SyncKit::Mutex busMutex;
ESP32TaskKit::Task holderTask;
ESP32TaskKit::Task waiterTask;

// en: timings are slowed down so logs stay readable
// ja: ログを見やすくするため時間を長めに設定
constexpr uint32_t kHolderHoldMs = 1200;   // en: how long the holder keeps the mutex / ja: ホルダーが保持する時間
constexpr uint32_t kHolderLoopMs = 200;    // en: holder loop period / ja: ホルダーのループ周期
constexpr uint32_t kWaiterTimeoutMs = 250; // en: LockGuard timeout for waiter / ja: 待機側の LockGuard タイムアウト
constexpr uint32_t kWaiterWorkMs = 80;     // en: simulated work while holding / ja: 保持中に行う擬似作業時間
constexpr uint32_t kWaiterLoopMs = 600;    // en: waiter loop period / ja: 待機側のループ周期

void setup()
{
  Serial.begin(115200);

  // en: Holder task (priority 2) grabs the mutex and holds it for 800 ms repeatedly
  // ja: 長時間保持するタスク（優先度2）。ミューテックスを取得し 800 ms 保持を繰り返す
  holderTask.startLoop(
      []
      {
        static bool holding = false;
        static uint32_t releaseAtMs = 0;
        static uint32_t startMs = 0;

        if (!holding)
        {
          if (busMutex.lock())
          {
            holding = true;
            startMs = millis();
            releaseAtMs = startMs + kHolderHoldMs;
            Serial.printf("[Mutex/LockGuard] holder locked @ %lu ms (will hold %lu ms)\n",
                          static_cast<unsigned long>(startMs),
                          static_cast<unsigned long>(kHolderHoldMs));
          }
          else
          {
            Serial.println("[Mutex/LockGuard] holder lock failed");
          }
        }
        else if ((int32_t)(millis() - releaseAtMs) >= 0)
        {
          if (!busMutex.unlock())
          {
            Serial.println("[Mutex/LockGuard] holder unlock failed");
          }
          else
          {
            uint32_t now = millis();
            Serial.printf("[Mutex/LockGuard] holder unlocked @ %lu ms (held %lu ms)\n",
                          static_cast<unsigned long>(now),
                          static_cast<unsigned long>(now - startMs));
          }
          holding = false;
        }
        return true;
      },
      ESP32TaskKit::TaskConfig{.name = "holder", .priority = 2},
      kHolderLoopMs);

  // en: Waiter task (priority 2). Tries LockGuard with 120 ms timeout; shows success/failure.
  // ja: 待機タスク（優先度2）。LockGuard を 120 ms タイムアウトで試し、成功/失敗を表示
  waiterTask.startLoop(
      []
      {
        // en: scope for guard to ensure unlock before loop delay
        // ja: ループ遅延前に解放されるようガードのスコープを限定
        {
          ESP32SyncKit::Mutex::LockGuard guard(busMutex, kWaiterTimeoutMs);
          if (guard.locked())
          {
            Serial.printf("[Mutex/LockGuard] waiter got lock (timeout=%lu ms), doing %lu ms work\n",
                          static_cast<unsigned long>(kWaiterTimeoutMs),
                          static_cast<unsigned long>(kWaiterWorkMs));
            delay(kWaiterWorkMs); // en: simulate short work while holding / ja: 保持中の短い擬似作業
          }
          else
          {
            Serial.printf("[Mutex/LockGuard] waiter lock timeout (timeout=%lu ms)\n",
                          static_cast<unsigned long>(kWaiterTimeoutMs));
          }
        }
        return true;
      },
      ESP32TaskKit::TaskConfig{.name = "waiter", .priority = 2},
      kWaiterLoopMs);
}

void loop()
{
  delay(1);
}
