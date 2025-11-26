#pragma once

#include <algorithm>
#include <cstdint>
#include <vector>

namespace lats::core {

class LatencyStats {
public:
  void add_sample(uint64_t latency_ns) { samples_.push_back(latency_ns); }

  void compute() { std::sort(samples_.begin(), samples_.end()); }

  uint64_t min() const { return samples_.front(); }
  uint64_t max() const { return samples_.back(); }
  uint64_t percentile(double p) const;
  uint64_t p50() const { return percentile(0.50); }
  uint64_t p95() const { return percentile(0.95); }
  uint64_t p99() const { return percentile(0.99); }
  uint64_t p999() const { return percentile(0.999); }
  double mean() const;

private:
  std::vector<uint64_t> samples_;
};

} // namespace lats::core
