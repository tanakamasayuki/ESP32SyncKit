// en: Protect shared Serial output with a mutex
// ja: 共有するシリアル出力をMutexで保護する
#include <ESP32AutoTask.h>
#include <ESP32SyncKit.h>

// en: Shared Serial output protected by Mutex in AutoTask hooks
// ja: AutoTask フック間で共有するシリアル出力を Mutex で保護
ESP32SyncKit::Mutex serialMutex;

void setup()
{
  Serial.begin(115200);
  // en: Initialize this component after its prerequisites; inspect the returned status where provided.
  // ja: 前提となる機器を準備してから初期化し、結果を返すAPIでは成否を確認します。
  ESP32AutoTask::AutoTask.begin();
}

void printSafe(const char *msg)
{
  // en: Protect Serial output with Mutex. Lock is held until the end of this scope
  // ja: シリアル出力を Mutex で保護。このスコープが終わるまでロックを保持
  ESP32SyncKit::Mutex::LockGuard lock(serialMutex);
  Serial.println(msg);
}

void LoopCore0_Normal()
{
  printSafe("[Mutex/AutoTask] core0 says hello");
  delay(200);
}

void LoopCore1_Normal()
{
  printSafe("[Mutex/AutoTask] core1 says hi");
  delay(300);
}

void loop()
{
  delay(1);
}
