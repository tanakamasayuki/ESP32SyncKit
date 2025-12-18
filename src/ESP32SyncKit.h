#pragma once

#include <Arduino.h>
#include <esp_log.h>
#include <utility>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

namespace ESP32SyncKit
{

  constexpr uint32_t WaitForever = portMAX_DELAY;
  inline constexpr const char *kLogTag = "ESP32SyncKit";

  template <class T>
  class Queue
  {
  public:
    explicit Queue(uint32_t depth)
        : handle_(nullptr)
    {
      if (depth == 0)
      {
        ESP_LOGE(kLogTag, "[Queue] create failed: depth must be > 0");
        return;
      }
      handle_ = xQueueCreate(depth, sizeof(T));
      if (!handle_)
      {
        ESP_LOGE(kLogTag, "[Queue] create failed: xQueueCreate");
      }
    }

    ~Queue()
    {
      if (handle_)
      {
        vQueueDelete(handle_);
        handle_ = nullptr;
      }
    }

    Queue(const Queue &) = delete;
    Queue &operator=(const Queue &) = delete;

    Queue(Queue &&other) noexcept : handle_(other.handle_)
    {
      other.handle_ = nullptr;
    }
    Queue &operator=(Queue &&other) noexcept
    {
      if (this != &other)
      {
        if (handle_)
        {
          vQueueDelete(handle_);
        }
        handle_ = other.handle_;
        other.handle_ = nullptr;
      }
      return *this;
    }

    bool trySend(const T &value) { return send(value, 0); }

    bool send(const T &value, uint32_t timeoutMs = WaitForever)
    {
      if (!handle_)
      {
        ESP_LOGE(kLogTag, "[Queue] send failed: handle null");
        return false;
      }

      const bool inIsr = xPortInIsrContext();
      TickType_t ticks = (inIsr || timeoutMs == 0) ? 0 : (timeoutMs == WaitForever ? portMAX_DELAY : pdMS_TO_TICKS(timeoutMs));
      const bool nonBlocking = (ticks == 0);

      if (inIsr)
      {
        BaseType_t taskWoken = pdFALSE;
        BaseType_t rc = xQueueSendFromISR(handle_, &value, &taskWoken);
        if (taskWoken == pdTRUE)
        {
          portYIELD_FROM_ISR();
        }
        if (rc != pdPASS)
        {
          ESP_LOGW(kLogTag, "[Queue] send ISR failed: rc=%ld", static_cast<long>(rc));
          return false;
        }
        return true;
      }
      else
      {
        BaseType_t rc = xQueueSend(handle_, &value, ticks);
        if (rc != pdPASS)
        {
          if (!nonBlocking)
          {
            ESP_LOGW(kLogTag, "[Queue] send timeout/full");
          }
          return false;
        }
        return true;
      }
    }

    bool trySendToFront(const T &value) { return sendToFront(value, 0); }

    bool sendToFront(const T &value, uint32_t timeoutMs = WaitForever)
    {
      if (!handle_)
      {
        ESP_LOGE(kLogTag, "[Queue] sendToFront failed: handle null");
        return false;
      }

      const bool inIsr = xPortInIsrContext();
      TickType_t ticks = (inIsr || timeoutMs == 0) ? 0 : (timeoutMs == WaitForever ? portMAX_DELAY : pdMS_TO_TICKS(timeoutMs));
      const bool nonBlocking = (ticks == 0);

      if (inIsr)
      {
        BaseType_t taskWoken = pdFALSE;
        BaseType_t rc = xQueueSendToFrontFromISR(handle_, &value, &taskWoken);
        if (taskWoken == pdTRUE)
        {
          portYIELD_FROM_ISR();
        }
        if (rc != pdPASS)
        {
          ESP_LOGW(kLogTag, "[Queue] sendToFront ISR failed: rc=%ld", static_cast<long>(rc));
          return false;
        }
        return true;
      }
      else
      {
        BaseType_t rc = xQueueSendToFront(handle_, &value, ticks);
        if (rc != pdPASS)
        {
          if (!nonBlocking)
          {
            ESP_LOGW(kLogTag, "[Queue] sendToFront timeout/full");
          }
          return false;
        }
        return true;
      }
    }

    bool overwrite(const T &value)
    {
      if (!handle_)
      {
        ESP_LOGE(kLogTag, "[Queue] overwrite failed: handle null");
        return false;
      }

      const bool inIsr = xPortInIsrContext();
      if (inIsr)
      {
        BaseType_t taskWoken = pdFALSE;
        BaseType_t rc = xQueueOverwriteFromISR(handle_, &value, &taskWoken);
        if (taskWoken == pdTRUE)
        {
          portYIELD_FROM_ISR();
        }
        if (rc != pdPASS)
        {
          ESP_LOGW(kLogTag, "[Queue] overwrite ISR failed: rc=%ld", static_cast<long>(rc));
          return false;
        }
        return true;
      }
      else
      {
        BaseType_t rc = xQueueOverwrite(handle_, &value);
        if (rc != pdPASS)
        {
          ESP_LOGW(kLogTag, "[Queue] overwrite failed");
          return false;
        }
        return true;
      }
    }

    bool tryReceive(T &out) { return receive(out, 0); }

    bool receive(T &out, uint32_t timeoutMs = WaitForever)
    {
      if (!handle_)
      {
        ESP_LOGE(kLogTag, "[Queue] receive failed: handle null");
        return false;
      }

      const bool inIsr = xPortInIsrContext();
      TickType_t ticks = (inIsr || timeoutMs == 0) ? 0 : (timeoutMs == WaitForever ? portMAX_DELAY : pdMS_TO_TICKS(timeoutMs));
      const bool nonBlocking = (ticks == 0);

      if (inIsr)
      {
        BaseType_t taskWoken = pdFALSE;
        BaseType_t rc = xQueueReceiveFromISR(handle_, &out, &taskWoken);
        if (taskWoken == pdTRUE)
        {
          portYIELD_FROM_ISR();
        }
        if (rc != pdPASS)
        {
          return false;
        }
        return true;
      }
      else
      {
        BaseType_t rc = xQueueReceive(handle_, &out, ticks);
        if (rc != pdPASS)
        {
          if (!nonBlocking)
          {
            ESP_LOGW(kLogTag, "[Queue] receive timeout");
          }
          return false;
        }
        return true;
      }
    }

    uint32_t count() const
    {
      if (!handle_)
      {
        ESP_LOGE(kLogTag, "[Queue] count failed: handle null");
        return 0;
      }
      return xPortInIsrContext() ? uxQueueMessagesWaitingFromISR(handle_) : uxQueueMessagesWaiting(handle_);
    }

    bool clear()
    {
      if (!handle_)
      {
        ESP_LOGE(kLogTag, "[Queue] clear failed: handle null");
        return false;
      }
      if (xPortInIsrContext())
      {
        ESP_LOGW(kLogTag, "[Queue] clear not allowed in ISR");
        return false;
      }
      BaseType_t rc = xQueueReset(handle_);
      if (rc != pdPASS)
      {
        ESP_LOGW(kLogTag, "[Queue] clear failed");
        return false;
      }
      return true;
    }

  private:
    QueueHandle_t handle_;
  };

  class Notify
  {
  public:
    enum class Mode
    {
      Unknown,
      Counter,
      Bits
    };

    Notify() = default;
    explicit Notify(Mode mode) : mode_(mode), modeLocked_(mode != Mode::Unknown) {}
    explicit Notify(TaskHandle_t handle) : target_(handle) {}
    Notify(TaskHandle_t handle, Mode mode) : target_(handle), mode_(mode), modeLocked_(mode != Mode::Unknown) {}

    Notify(const Notify &) = delete;
    Notify &operator=(const Notify &) = delete;

    Notify(Notify &&other) noexcept
        : target_(other.target_), mode_(other.mode_), modeLocked_(other.modeLocked_)
    {
      other.target_ = nullptr;
      other.mode_ = Mode::Unknown;
      other.modeLocked_ = false;
    }
    Notify &operator=(Notify &&other) noexcept
    {
      if (this != &other)
      {
        target_ = other.target_;
        mode_ = other.mode_;
        modeLocked_ = other.modeLocked_;

        other.target_ = nullptr;
        other.mode_ = Mode::Unknown;
        other.modeLocked_ = false;
      }
      return *this;
    }

    bool bindTo(TaskHandle_t handle)
    {
      if (target_)
      {
        ESP_LOGW(kLogTag, "[Notify] bind failed: already bound");
        return false;
      }
      if (!handle)
      {
        ESP_LOGE(kLogTag, "[Notify] bind failed: null handle");
        return false;
      }
      target_ = handle;
      return true;
    }

    bool bindToSelf()
    {
      return bindTo(xTaskGetCurrentTaskHandle());
    }

    bool notify()
    {
      if (!lockMode(Mode::Counter))
      {
        return false;
      }
      if (!ensureBoundForSend())
      {
        return false;
      }

      const bool inIsr = xPortInIsrContext();
      if (inIsr)
      {
        BaseType_t taskWoken = pdFALSE;
        BaseType_t rc = xTaskNotifyFromISR(target_, 0, eIncrement, &taskWoken);
        if (taskWoken == pdTRUE)
        {
          portYIELD_FROM_ISR();
        }
        if (rc != pdPASS)
        {
          ESP_LOGW(kLogTag, "[Notify] notify ISR failed: rc=%ld", static_cast<long>(rc));
          return false;
        }
        return true;
      }
      else
      {
        BaseType_t rc = xTaskNotifyGive(target_);
        if (rc != pdPASS)
        {
          ESP_LOGW(kLogTag, "[Notify] notify failed: rc=%ld", static_cast<long>(rc));
          return false;
        }
        return true;
      }
    }

    bool take(uint32_t timeoutMs = WaitForever)
    {
      if (!lockMode(Mode::Counter))
      {
        return false;
      }
      if (!ensureBoundForReceive())
      {
        return false;
      }
      if (xPortInIsrContext())
      {
        // FreeRTOS does not support ulTaskNotifyTake from ISR; return immediately (non-block)
        return false;
      }

      TickType_t ticks = (timeoutMs == 0) ? 0 : (timeoutMs == WaitForever ? portMAX_DELAY : pdMS_TO_TICKS(timeoutMs));
      uint32_t count = ulTaskNotifyTake(pdFALSE, ticks);
      return count > 0;
    }

    bool tryTake() { return take(0); }

    // en: Take all pending notifications at once, returning the count (clears to 0)
    // ja: 溜まった通知をすべて取得し、件数を返す（カウンタを0にクリア）
    uint32_t takeAll(uint32_t timeoutMs = WaitForever)
    {
      if (!lockMode(Mode::Counter))
      {
        return 0;
      }
      if (!ensureBoundForReceive())
      {
        return 0;
      }
      if (xPortInIsrContext())
      {
        // FreeRTOS does not support ulTaskNotifyTake from ISR; return immediately (non-block)
        return 0;
      }

      TickType_t ticks = (timeoutMs == 0) ? 0 : (timeoutMs == WaitForever ? portMAX_DELAY : pdMS_TO_TICKS(timeoutMs));
      return ulTaskNotifyTake(pdTRUE, ticks);
    }

    bool setBits(uint32_t mask)
    {
      if (!lockMode(Mode::Bits))
      {
        return false;
      }
      if (!ensureBoundForSend())
      {
        return false;
      }

      const bool inIsr = xPortInIsrContext();
      if (inIsr)
      {
        BaseType_t taskWoken = pdFALSE;
        BaseType_t rc = xTaskNotifyFromISR(target_, mask, eSetBits, &taskWoken);
        if (taskWoken == pdTRUE)
        {
          portYIELD_FROM_ISR();
        }
        if (rc != pdPASS)
        {
          ESP_LOGW(kLogTag, "[Notify] setBits ISR failed: rc=%ld", static_cast<long>(rc));
          return false;
        }
        return true;
      }
      else
      {
        BaseType_t rc = xTaskNotify(target_, mask, eSetBits);
        if (rc != pdPASS)
        {
          ESP_LOGW(kLogTag, "[Notify] setBits failed: rc=%ld", static_cast<long>(rc));
          return false;
        }
        return true;
      }
    }

    bool waitBits(uint32_t mask, uint32_t timeoutMs = WaitForever, bool clearOnExit = true, bool waitAll = false)
    {
      if (!lockMode(Mode::Bits))
      {
        return false;
      }
      if (!ensureBoundForReceive())
      {
        return false;
      }
      if (xPortInIsrContext())
      {
        // FreeRTOS does not support xTaskNotifyWait from ISR; return immediately (non-block)
        return false;
      }

      TickType_t totalTicks = (timeoutMs == 0) ? 0 : (timeoutMs == WaitForever ? portMAX_DELAY : pdMS_TO_TICKS(timeoutMs));
      const bool infinite = (totalTicks == portMAX_DELAY);
      TickType_t start = xTaskGetTickCount();
      TickType_t remaining = totalTicks;
      uint32_t accumulated = 0;

      while (true)
      {
        uint32_t value = 0;
        BaseType_t rc = xTaskNotifyWait(
            0,
            clearOnExit ? mask : 0,
            &value,
            remaining);
        if (rc != pdPASS)
        {
          return false; // en: timeout or failure / ja: タイムアウトまたは失敗
        }

        if (!waitAll)
        {
          return (value & mask) != 0;
        }

        accumulated |= value;
        if ((accumulated & mask) == mask)
        {
          if (clearOnExit)
          {
            uint32_t dummy;
            (void)xTaskNotifyWait(0, mask, &dummy, 0); // en: clear matched bits / ja: 満たしたビットをクリア
          }
          return true;
        }

        if (!infinite)
        {
          TickType_t now = xTaskGetTickCount();
          TickType_t elapsed = now - start;
          if (elapsed >= totalTicks)
          {
            return false;
          }
          remaining = totalTicks - elapsed;
        }
        else
        {
          remaining = portMAX_DELAY;
        }
      }
    }

    bool tryWaitBits(uint32_t mask, bool clearOnExit = true, bool waitAll = false)
    {
      return waitBits(mask, 0, clearOnExit, waitAll);
    }

    // Convenience overload to use defaults for timeout while specifying flags
    bool waitBits(uint32_t mask, bool clearOnExit, bool waitAll)
    {
      return waitBits(mask, WaitForever, clearOnExit, waitAll);
    }

  private:
    bool lockMode(Mode desired)
    {
      if (!modeLocked_ || mode_ == Mode::Unknown)
      {
        mode_ = desired;
        modeLocked_ = true;
        return true;
      }
      if (mode_ != desired)
      {
        ESP_LOGW(kLogTag, "[Notify] mode conflict");
        return false;
      }
      return true;
    }

    bool ensureBoundForSend()
    {
      if (!target_)
      {
        ESP_LOGW(kLogTag, "[Notify] send failed: not bound");
        return false;
      }
      return true;
    }

    bool ensureBoundForReceive()
    {
      if (!target_)
      {
        if (xPortInIsrContext())
        {
          ESP_LOGW(kLogTag, "[Notify] receive failed: not bound (ISR)");
          return false;
        }
        // en: auto-bind to current task / ja: 自タスクに自動バインド
        target_ = xTaskGetCurrentTaskHandle();
      }
      if (target_ != xTaskGetCurrentTaskHandle())
      {
        ESP_LOGW(kLogTag, "[Notify] receive failed: called from non-bound task");
        return false;
      }
      return true;
    }

    TaskHandle_t target_ = nullptr;
    Mode mode_ = Mode::Unknown;
    bool modeLocked_ = false;
  };

  class BinarySemaphore
  {
  public:
    BinarySemaphore()
        : handle_(xSemaphoreCreateBinary())
    {
      if (!handle_)
      {
        ESP_LOGE(kLogTag, "[BinarySemaphore] create failed");
      }
    }

    ~BinarySemaphore()
    {
      if (handle_)
      {
        vSemaphoreDelete(handle_);
        handle_ = nullptr;
      }
    }

    BinarySemaphore(const BinarySemaphore &) = delete;
    BinarySemaphore &operator=(const BinarySemaphore &) = delete;

    BinarySemaphore(BinarySemaphore &&other) noexcept : handle_(other.handle_)
    {
      other.handle_ = nullptr;
    }
    BinarySemaphore &operator=(BinarySemaphore &&other) noexcept
    {
      if (this != &other)
      {
        if (handle_)
        {
          vSemaphoreDelete(handle_);
        }
        handle_ = other.handle_;
        other.handle_ = nullptr;
      }
      return *this;
    }

    bool give()
    {
      if (!handle_)
      {
        ESP_LOGE(kLogTag, "[BinarySemaphore] give failed: handle null");
        return false;
      }
      const bool inIsr = xPortInIsrContext();
      if (inIsr)
      {
        BaseType_t taskWoken = pdFALSE;
        BaseType_t rc = xSemaphoreGiveFromISR(handle_, &taskWoken);
        if (taskWoken == pdTRUE)
        {
          portYIELD_FROM_ISR();
        }
        if (rc != pdPASS)
        {
          ESP_LOGW(kLogTag, "[BinarySemaphore] give ISR failed: rc=%ld", static_cast<long>(rc));
          return false;
        }
        return true;
      }
      else
      {
        BaseType_t rc = xSemaphoreGive(handle_);
        if (rc != pdPASS)
        {
          ESP_LOGW(kLogTag, "[BinarySemaphore] give failed: rc=%ld", static_cast<long>(rc));
          return false;
        }
        return true;
      }
    }

    bool take(uint32_t timeoutMs = WaitForever)
    {
      if (!handle_)
      {
        ESP_LOGE(kLogTag, "[BinarySemaphore] take failed: handle null");
        return false;
      }
      const bool inIsr = xPortInIsrContext();
      if (inIsr)
      {
        ESP_LOGW(kLogTag, "[BinarySemaphore] take called in ISR (non-block only, not recommended)");
        BaseType_t rc = xSemaphoreTakeFromISR(handle_, nullptr);
        return rc == pdPASS;
      }

      TickType_t ticks = (timeoutMs == 0) ? 0 : (timeoutMs == WaitForever ? portMAX_DELAY : pdMS_TO_TICKS(timeoutMs));
      BaseType_t rc = xSemaphoreTake(handle_, ticks);
      return rc == pdPASS;
    }

    bool tryTake() { return take(0); }

  private:
    SemaphoreHandle_t handle_;
  };

  class Mutex
  {
  public:
    Mutex()
        : handle_(xSemaphoreCreateMutex())
    {
      if (!handle_)
      {
        ESP_LOGE(kLogTag, "[Mutex] create failed");
      }
    }

    ~Mutex()
    {
      if (handle_)
      {
        vSemaphoreDelete(handle_);
        handle_ = nullptr;
      }
    }

    Mutex(const Mutex &) = delete;
    Mutex &operator=(const Mutex &) = delete;

    Mutex(Mutex &&other) noexcept : handle_(other.handle_)
    {
      other.handle_ = nullptr;
    }
    Mutex &operator=(Mutex &&other) noexcept
    {
      if (this != &other)
      {
        if (handle_)
        {
          vSemaphoreDelete(handle_);
        }
        handle_ = other.handle_;
        other.handle_ = nullptr;
      }
      return *this;
    }

    bool lock(uint32_t timeoutMs = WaitForever)
    {
      if (!handle_)
      {
        ESP_LOGE(kLogTag, "[Mutex] lock failed: handle null");
        return false;
      }
      if (xPortInIsrContext())
      {
        ESP_LOGE(kLogTag, "[Mutex] lock called in ISR");
        return false;
      }

      TickType_t ticks = (timeoutMs == 0) ? 0 : (timeoutMs == WaitForever ? portMAX_DELAY : pdMS_TO_TICKS(timeoutMs));
      BaseType_t rc = xSemaphoreTake(handle_, ticks);
      if (rc != pdPASS)
      {
        ESP_LOGW(kLogTag, "[Mutex] lock timeout");
        return false;
      }
      return true;
    }

    bool tryLock() { return lock(0); }

    bool unlock()
    {
      if (!handle_)
      {
        ESP_LOGE(kLogTag, "[Mutex] unlock failed: handle null");
        return false;
      }
      BaseType_t rc = xSemaphoreGive(handle_);
      if (rc != pdPASS)
      {
        ESP_LOGW(kLogTag, "[Mutex] unlock failed");
        return false;
      }
      return true;
    }

    class LockGuard
    {
    public:
      explicit LockGuard(Mutex &m, uint32_t timeoutMs = WaitForever)
          : mutex_(&m), locked_(m.lock(timeoutMs))
      {
        if (!locked_)
        {
          ESP_LOGW(kLogTag, "[Mutex] LockGuard lock failed");
        }
      }

      ~LockGuard()
      {
        if (locked_ && mutex_)
        {
          mutex_->unlock();
        }
      }

      LockGuard(const LockGuard &) = delete;
      LockGuard &operator=(const LockGuard &) = delete;

      LockGuard(LockGuard &&other) noexcept
          : mutex_(other.mutex_), locked_(other.locked_)
      {
        other.mutex_ = nullptr;
        other.locked_ = false;
      }

      LockGuard &operator=(LockGuard &&other) noexcept
      {
        if (this != &other)
        {
          if (locked_ && mutex_)
          {
            mutex_->unlock();
          }
          mutex_ = other.mutex_;
          locked_ = other.locked_;
          other.mutex_ = nullptr;
          other.locked_ = false;
        }
        return *this;
      }

      bool locked() const { return locked_; }

    private:
      Mutex *mutex_;
      bool locked_;
    };

  private:
    SemaphoreHandle_t handle_;
  };

} // namespace ESP32SyncKit
