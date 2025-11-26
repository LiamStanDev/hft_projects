#include "core/spsc_queue.hpp"

#include <gtest/gtest.h>

#include <thread>

namespace lats::core::test {
using namespace lats::core;

TEST(SPSCQueue, BasicPushPop) {
  SPSCQueue<int, 8> queue;

  EXPECT_TRUE(queue.empty());
  EXPECT_TRUE(queue.try_push(42));
  EXPECT_FALSE(queue.empty());

  auto result = queue.try_pop();
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), 42);
  EXPECT_TRUE(queue.empty());
}

TEST(SPSCQueueTest, QueueFull) {
  SPSCQueue<int, 4> queue;

  // Capacity = 4, 實際可用 3 個位置
  EXPECT_TRUE(queue.try_push(1));
  EXPECT_TRUE(queue.try_push(2));
  EXPECT_TRUE(queue.try_push(3));
  EXPECT_FALSE(queue.try_push(4)); // 應該失敗

  // 彈出一個後可以再推入
  auto result = queue.try_pop();
  EXPECT_TRUE(result.has_value());
  EXPECT_TRUE(queue.try_push(4));
}

TEST(SPSCQueueTest, QueueEmpty) {
  SPSCQueue<int, 8> queue;

  auto result = queue.try_pop();
  EXPECT_FALSE(result.has_value());
}

// 測試順序性
TEST(SPSCQueueTest, FIFOOrder) {
  SPSCQueue<int, 16> queue;

  for (int i = 0; i < 10; ++i) {
    EXPECT_TRUE(queue.try_push(i));
  }

  for (int i = 0; i < 10; ++i) {
    auto result = queue.try_pop();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), i);
  }
}

// 測試 Move Semantics
TEST(SPSCQueueTest, MoveSemantics) {
  struct NonCopyable {
    int value = 0;
    NonCopyable() = default;
    NonCopyable(int v) : value(v) {}
    NonCopyable(const NonCopyable &) = delete;
    NonCopyable &operator=(const NonCopyable &) = delete;
    NonCopyable(NonCopyable &&) = default;
    NonCopyable &operator=(NonCopyable &&) = default;
  };

  SPSCQueue<NonCopyable, 8> queue;

  NonCopyable obj(42);
  EXPECT_TRUE(queue.try_push(std::move(obj)));

  auto result = queue.try_pop();
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value().value, 42);
}

TEST(SPSCQueueTest, ConcurrentProducerConsumer) {
  constexpr size_t NUM_ITEMS = 1000000;
  SPSCQueue<uint64_t, 1024> queue;

  std::atomic<bool> producer_done{false};
  std::atomic<uint64_t> items_consumed{0};

  // Consumer 線程
  std::thread consumer([&]() {
    uint64_t expected = 0;
    while (items_consumed < NUM_ITEMS) {
      auto result = queue.try_pop();
      if (result.has_value()) {
        EXPECT_EQ(result.value(), expected);
        expected++;
        items_consumed++;
      }
    }
  });

  // Producer 線程
  std::thread producer([&]() {
    for (uint64_t i = 0; i < NUM_ITEMS; ++i) {
      while (!queue.try_push(i)) {
        // Spin until space available
        std::this_thread::yield();
      }
    }
    producer_done = true;
  });

  producer.join();
  consumer.join();

  EXPECT_EQ(items_consumed, NUM_ITEMS);
  EXPECT_TRUE(queue.empty());
}

// 壓力測試：快速 push/pop
TEST(SPSCQueueTest, StressTest) {
  constexpr size_t ITERATIONS = 10000;
  SPSCQueue<int, 64> queue;

  std::thread consumer([&]() {
    size_t consumed = 0;
    while (consumed < ITERATIONS) {
      if (queue.try_pop().has_value()) {
        consumed++;
      }
    }
  });

  std::thread producer([&]() {
    for (size_t i = 0; i < ITERATIONS; ++i) {
      while (!queue.try_push(static_cast<int>(i))) {
        std::this_thread::yield();
      }
    }
  });

  producer.join();
  consumer.join();
}
// 測試不同的 Capacity（驗證 power of 2）
TEST(SPSCQueueTest, DifferentCapacities) {
  SPSCQueue<int, 2> q2;
  SPSCQueue<int, 4> q4;
  SPSCQueue<int, 1024> q1024;

  EXPECT_TRUE(q2.empty());
  EXPECT_TRUE(q4.empty());
  EXPECT_TRUE(q1024.empty());
}
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

} // namespace lats::core::test
