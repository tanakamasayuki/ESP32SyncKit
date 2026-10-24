// en: Observe two tasks competing for one bus lock
// ja: 2つのタスクが1つのバスLockを使う様子を見る
#include <ESP32TaskKit.h>
#include <ESP32SyncKit.h>

// en: Protect shared bus (I2C/SPI) across TaskKit tasks with Mutex
// ja: TaskKit タスク間で共有バスを Mutex で保護
ESP32SyncKit::Mutex busMutex;
ESP32TaskKit::Task sensorTask;
ESP32TaskKit::Task loggerTask;
constexpr uint32_t kSensorHoldMs = 250;
constexpr uint32_t kLoggerHoldMs = 500;
constexpr uint32_t kMutexLoopIntervalMs = 50;

void setup()
{
  Serial.begin(115200);

  // en: TaskKit sensor task (priority 2), runs every 50 ms to lock/unlock after ~250 ms hold
  // ja: TaskKit センサタスク（優先度2）。50 ms 周期で約 250 ms 保持しロック/アンロック
  sensorTask.startLoop(
      []
      {
        static bool holding = false;
        static uint32_t releaseAtMs = 0;
        static uint32_t startMs = 0;

        if (!holding)
        {
          // en: Acquire exclusive ownership before shared access; respect this wrapper’s timeout unit.
          // ja: 共有領域を使う前に排他権を取得し、このラッパーの待ち時間単位に合わせます。
          if (busMutex.lock())
          {
            holding = true;
            startMs = millis();
            releaseAtMs = startMs + kSensorHoldMs;
            Serial.printf("[Mutex/TaskKit] sensor locked (hold %lu ms) @ %lu ms\n",
                          static_cast<unsigned long>(kSensorHoldMs),
                          static_cast<unsigned long>(startMs));
            // en: explicit lock/unlock pattern (no RAII) to show timing
            // ja: タイミングを示すため、RAII ではなく明示 lock/unlock
          }
          else
          {
            Serial.println("[Mutex/TaskKit] lock failed");
          }
        }
        else if ((int32_t)(millis() - releaseAtMs) >= 0)
        {
          if (!busMutex.unlock())
          {
            Serial.println("[Mutex/TaskKit] unlock failed");
          }
          holding = false;
          uint32_t now = millis();
          Serial.printf("[Mutex/TaskKit] sensor unlocked @ %lu ms (held %lu ms)\n",
                        static_cast<unsigned long>(now),
                        static_cast<unsigned long>(now - startMs));
          // en: explicit unlock after simulated work
          // ja: 擬似作業後に明示的に unlock
        }
        return true;
      },
      ESP32TaskKit::TaskConfig{.name = "sensor", .priority = 2},
      kMutexLoopIntervalMs);

  // en: TaskKit logger task (priority 2), runs every 50 ms to lock/unlock after ~500 ms hold
  // ja: TaskKit ロガータスク（優先度2）。50 ms 周期で約 500 ms 保持しロック/アンロック
  loggerTask.startLoop(
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
            releaseAtMs = startMs + kLoggerHoldMs;
            Serial.printf("[Mutex/TaskKit] logger locked (hold %lu ms) @ %lu ms\n",
                          static_cast<unsigned long>(kLoggerHoldMs),
                          static_cast<unsigned long>(startMs));
            // en: explicit lock/unlock pattern (no RAII) to show timing
            // ja: タイミングを示すため、RAII ではなく明示 lock/unlock
          }
          else
          {
            Serial.println("[Mutex/TaskKit] lock failed");
          }
        }
        else if ((int32_t)(millis() - releaseAtMs) >= 0)
        {
          if (!busMutex.unlock())
          {
            Serial.println("[Mutex/TaskKit] unlock failed");
          }
          holding = false;
          uint32_t now = millis();
          Serial.printf("[Mutex/TaskKit] logger unlocked @ %lu ms (held %lu ms)\n",
                        static_cast<unsigned long>(now),
                        static_cast<unsigned long>(now - startMs));
          // en: explicit unlock after simulated work
          // ja: 擬似作業後に明示的に unlock
        }
        return true;
      },
      ESP32TaskKit::TaskConfig{.name = "logger", .priority = 2},
      kMutexLoopIntervalMs);
}

void loop()
{
  delay(1);
}
