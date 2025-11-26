#include "core/timer.hpp"

#include <chrono>
#include <cstdint>
#include <thread>

namespace lats::core {

double Timer::tsc_freq_ghz_ = 0.0;

void Timer::calibrate() {
  constexpr int calibration_ms = 100; // 使用 100 ms 來校準
  auto start_time = std::chrono::high_resolution_clock::now();
  uint64_t start_tsc = now();

  std::this_thread::sleep_for(std::chrono::milliseconds(calibration_ms));
  uint64_t end_tsc = now();
  auto end_time = std::chrono::high_resolution_clock::now();

  auto duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
                         end_time - start_time)
                         .count();

  uint64_t tsc_diff = end_tsc - start_tsc;
  tsc_freq_ghz_ = static_cast<double>(tsc_diff) / duration_ns;
}

uint64_t Timer::cycles_to_ns(uint64_t cycles) {
  if (tsc_freq_ghz_ == 0.0) {
    calibrate();
  }

  return static_cast<uint64_t>(cycles / tsc_freq_ghz_);
}
} // namespace lats::core
