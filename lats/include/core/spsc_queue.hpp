#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <optional>
#include <type_traits>
namespace lats::core {

inline constexpr size_t CACHE_LINE_SIZE = 64;

template <typename T, size_t Capacity> class SPSCQueue {
  // 從二進制來看 2 的冪次只有一個位數為 1 若他減去 1 在與一定為 0
  static_assert((Capacity & (Capacity - 1)) == 0 && Capacity > 0,
                "Capacity must be power of 2");

  static_assert(std::is_default_constructible_v<T>,
                "T must be DefaultConstructible for zero-overhead");

public:
  SPSCQueue() : head_(0), tail_(0) {};

  // Disable copy and move
  SPSCQueue(const SPSCQueue &) = delete;
  SPSCQueue &operator=(const SPSCQueue &) = delete;

  template <typename U> bool try_push(U &&item) {
    const size_t head = head_.load(std::memory_order_relaxed);
    const size_t next_head = (head + 1) & (Capacity - 1);

    if (next_head == tail_.load(std::memory_order_acquire)) {
      return false;
    }

    buffer_[head] = std::forward<U>(item);
    head_.store(next_head, std::memory_order_release);
    return true;
  }

  std::optional<T> try_pop() {
    const size_t tail = tail_.load(std::memory_order_relaxed);

    if (tail == head_.load(std::memory_order_acquire)) {
      return std::nullopt;
    }

    T item = std::move(buffer_[tail]);
    tail_.store((tail + 1) & (Capacity - 1), std::memory_order_release);
    return item;
  }

  bool empty() const {
    return head_.load(std::memory_order_acquire) ==
           tail_.load(std::memory_order_acquire);
  }

private:
  // Buffer
  std::array<T, Capacity> buffer_;

  // Cache line padding to prevent false sharing
  alignas(CACHE_LINE_SIZE) std::atomic<size_t> head_;
  alignas(CACHE_LINE_SIZE) std::atomic<size_t> tail_;
};
} // namespace lats::core
