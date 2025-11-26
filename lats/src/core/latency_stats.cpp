#include <core/latency_stats.hpp>
#include <numeric>
#include <stdexcept>

namespace lats::core {

uint64_t LatencyStats::percentile(double p) const {
  if (samples_.empty()) {
    throw std::runtime_error("No samples available");
  }

  if (p < 0.0 || p > 1.0) {
    throw std::invalid_argument("Percentile must be between 0.0 and 1.0");
  }

  size_t n = samples_.size();
  double idx = p * (n - 1); // 0 + (n - 1 - 0) * p
  size_t lower = static_cast<size_t>(idx);
  size_t upper = lower + 1;

  if (upper >= n) {
    return samples_.back();
  }

  double fraction = idx - lower;
  return static_cast<uint64_t>(samples_[lower] * (1.0 - fraction) +
                               samples_[upper] * fraction);
}

double LatencyStats::mean() const {
  if (samples_.empty()) {
    throw std::runtime_error("No samples available");
  }

  uint64_t sum = std::accumulate(samples_.begin(), samples_.end(), 0ULL);
  return static_cast<double>(sum) / samples_.size();
}

} // namespace lats::core
