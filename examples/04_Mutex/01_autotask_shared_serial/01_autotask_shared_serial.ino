#include <ESP32AutoTask.h>
#include <ESP32SyncKit.h>

// en: Shared Serial output protected by Mutex in AutoTask hooks
// ja: AutoTask フック間で共有するシリアル出力を Mutex で保護
ESP32SyncKit::Mutex serialMutex;

void setup()
{
  Serial.begin(115200);
  ESP32AutoTask::AutoTask.begin();
}

void printSafe(const char *msg)
{
  ESP32SyncKit::Mutex::LockGuard lock(serialMutex);
  if (lock.locked())
  {
    Serial.println(msg);
  }
  else
  {
    Serial.println("[Mutex/AutoTask] lock failed");
  }
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
