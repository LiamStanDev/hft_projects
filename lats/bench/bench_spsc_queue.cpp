#include "core/latency_stats.hpp"
#include "core/spsc_queue.hpp"
#include "core/timer.hpp"

namespace lats::core {

struct alignas(64) SmallMessage {
  uint64_t timestamp;
  uint64_t sequence;

  SmallMessage() = default;
  SmallMessage(uint64_t ts, uint64_t seq) : timestamp(ts), sequence(seq) {}
};
} // namespace lats::core
