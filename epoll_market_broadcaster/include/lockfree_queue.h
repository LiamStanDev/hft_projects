#pragma once
#include <array>
#include <atomic>
#include <cstddef>
#include <optional>

namespace emb {

template <typename T, std::size_t Capacity> class LockFreeQueue {
public:
  template <typename U> bool push(U &&item) {
    // 使用 relaxed 因為 pop 不會修改他，這樣就不會有同步開銷
    size_t current_tail = tail_.load(std::memory_order_relaxed);
    size_t next_tail = (current_tail + 1) % Capacity;

    // check full
    // 使用 acquire 是因為要確保讀取到 pop 之前修改的內容
    if (next_tail == head_.load(std::memory_order_acquire)) {
      return false;
    }

    buffer_[current_tail] = std::forward<U>(item);
    tail_.store(next_tail, std::memory_order_release);
    return true;
  }

  std::optional<T> pop() {
    // 使用 relaxed 因為 push 不會修改他，這樣就不會有同步開銷
    size_t current_head = head_.load(std::memory_order_relaxed);

    // check empty
    // 使用 acquire 是因為要確保讀取到 push 之前的修改內容
    if (current_head == tail_.load(std::memory_order_acquire)) {
      return std::nullopt;
    }

    T item = buffer_[current_head];
    head_.store((current_head + 1) % Capacity, std::memory_order_release);
    return item;
  }

private:
  std::array<T, Capacity> buffer_;
  // 要讓這兩個變數在不同 cache line (64 bytes)上，這樣可以讓兩個變數
  // cachce 獨立，不然就算只有其中一個修改另外一個也會導致 cache miss
  alignas(64) std::atomic<size_t> head_{0};
  alignas(64) std::atomic<size_t> tail_{0};
};
} // namespace emb
