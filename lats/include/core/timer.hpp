#pragma once

#include <cstdint>
#include <x86intrin.h>

namespace lats::core {

// Time Stamp Counter 計時器
class Timer {
public:
  static inline uint64_t now() {
    unsigned int aux; // 輔助變數用於接收處理器 ID
    return __rdtscp(&aux);
  }

  static uint64_t cycles_to_ns(uint64_t cycles);
  static void calibrate();

private:
  // 時脈頻率，尚未校準
  static double tsc_freq_ghz_;
};

class ScopedTimer {
public:
  ScopedTimer() : start_(Timer::now()) {}
  uint64_t elapsed_cycles() const { return Timer::now() - start_; }
  uint64_t elapsed_ns() const { return Timer::cycles_to_ns(elapsed_cycles()); }

private:
  uint64_t start_;
};

} // namespace lats::core
