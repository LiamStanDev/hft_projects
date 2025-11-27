#include "core/latency_stats.hpp"
#include "core/spsc_queue.hpp"
#include "core/timer.hpp"

#include <benchmark/benchmark.h>

#include <iostream>
#include <sched.h>
#include <thread>

namespace lats::core::bench {
using namespace lats::core;

struct alignas(64) SmallMessage {
  uint64_t timestamp;
  uint64_t sequence;

  SmallMessage() = default;
  SmallMessage(uint64_t ts, uint64_t seq) : timestamp(ts), sequence(seq) {}
};

struct alignas(64) MediumMessage {
  uint64_t timestamp;
  uint64_t sequence;
  int64_t price;
  int32_t quantity;
  uint32_t symbol_id;
  uint8_t side;
  uint8_t padding[32]; // 填充到 64 bytes
  //
  MediumMessage() = default;
  MediumMessage(uint64_t ts, uint64_t seq)
      : timestamp(ts), sequence(seq), price(0), quantity(0), symbol_id(0),
        side(0), padding{} {}
};

// ============================================================================
// Benchmark 1: 單次 Push/Pop 延遲
// ============================================================================
static void BM_SinglePushPop(benchmark::State &state) {
  SPSCQueue<SmallMessage, 1024> queue;
  uint64_t counter = 0;

  for (auto _ : state) {
    SmallMessage msg(Timer::now(), counter++);

    benchmark::DoNotOptimize(queue.try_push(std::move(msg)));
    auto result = queue.try_pop();
    benchmark::DoNotOptimize(result);
    benchmark::ClobberMemory(); // 防止編譯器重排優化
  }

  state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_SinglePushPop);

// ============================================================================
// Benchmark 2: Producer-Consumer 延遲（關鍵測試）
// ============================================================================
static void BM_ProducerConsumerLatency(benchmark::State &state) {
  constexpr size_t QUEUE_SIZE = 1024;
  constexpr size_t NUM_SAMPLES = 100000;

  for (auto _ : state) {
    SPSCQueue<SmallMessage, QUEUE_SIZE> queue;
    LatencyStats stats;

    std::atomic<bool> start{false};
    std::atomic<bool> done{false};
    std::atomic<uint64_t> samples_collected{0};

    // 校準 TSC 頻率（只需執行一次）
    static bool calibrated = false;
    if (!calibrated) {
      Timer::calibrate();
      calibrated = true;
    }

    // Consumer 線程
    std::thread consumer([&]() {
      // 等待開始信號
      while (!start.load(std::memory_order_acquire)) {
        std::this_thread::yield();
      }

      while (samples_collected.load(std::memory_order_relaxed) < NUM_SAMPLES) {
        auto result = queue.try_pop();
        if (result.has_value()) {
          uint64_t now = Timer::now();
          uint64_t latency_cycles = now - result->timestamp;
          uint64_t latency_ns = Timer::cycles_to_ns(latency_cycles);
          stats.add_sample(latency_ns);
          samples_collected.fetch_add(1, std::memory_order_relaxed);
        }
      }
      done.store(true, std::memory_order_release);
    });

    // Producer 線程 (主線程)
    start.store(true, std::memory_order_release);

    uint64_t sent = 0;
    while (sent < NUM_SAMPLES) {
      SmallMessage msg(Timer::now(), sent);
      if (queue.try_push(std::move(msg))) {
        sent++;
      } else {
        std::this_thread::yield();
      }
    }

    // 等待 consumer 完成
    while (!done.load(std::memory_order_acquire)) {
      std::this_thread::yield();
    }

    consumer.join();

    // 計算統計數據
    stats.compute();

    // 輸出詳細結果（只在第一次迭代時輸出）
    static bool first_run = true;
    if (first_run) {
      std::cout << "\n========================================\n";
      std::cout << "Producer-Consumer Latency Analysis\n";
      std::cout << "========================================\n";
      std::cout << "Samples:      " << NUM_SAMPLES << "\n";
      std::cout << "Min Latency:  " << stats.min() << " ns\n";
      std::cout << "Mean Latency: " << static_cast<uint64_t>(stats.mean())
                << " ns\n";
      std::cout << "P50 Latency:  " << stats.p50() << " ns\n";
      std::cout << "P95 Latency:  " << stats.p95() << " ns\n";
      std::cout << "P99 Latency:  " << stats.p99() << " ns\n";
      std::cout << "P999 Latency: " << stats.p999() << " ns\n";
      std::cout << "Max Latency:  " << stats.max() << " ns\n";
      std::cout << "========================================\n\n";
      first_run = false;
    }
  }
}
BENCHMARK(BM_ProducerConsumerLatency)
    ->Iterations(1)
    ->Unit(benchmark::kMillisecond);

// ============================================================================
// Benchmark 3: 吞吐量測試
// ============================================================================
static void BM_Throughput(benchmark::State &state) {
  for (auto _ : state) {
    SPSCQueue<SmallMessage, 2048> queue;
    std::atomic<bool> producer_done{false};
    std::atomic<uint64_t> items_consumed{0};

    const size_t num_items = 1000000; // 1M items for faster testing

    // Consumer 線程
    std::thread consumer([&]() {
      while (items_consumed.load(std::memory_order_relaxed) < num_items) {
        auto result = queue.try_pop();
        if (result.has_value()) {
          items_consumed.fetch_add(1, std::memory_order_relaxed);
          benchmark::DoNotOptimize(result);
        }
      }
    });

    // Producer 線程
    std::thread producer([&]() {
      for (uint64_t i = 0; i < num_items; ++i) {
        SmallMessage msg(Timer::now(), i);
        while (!queue.try_push(std::move(msg))) {
          std::this_thread::yield();
        }
      }
      producer_done.store(true, std::memory_order_release);
    });

    producer.join();
    consumer.join();

    state.SetItemsProcessed(num_items);
  }
}
BENCHMARK(BM_Throughput)->Unit(benchmark::kMillisecond);

// ============================================================================
// Benchmark 4: 不同消息大小的影響
// ============================================================================
template <typename T> static void BM_MessageSize(benchmark::State &state) {
  SPSCQueue<T, 1024> queue;
  uint64_t counter = 0;

  for (auto _ : state) {
    T msg(Timer::now(), counter++);
    benchmark::DoNotOptimize(queue.try_push(std::move(msg)));
    auto result = queue.try_pop();
    benchmark::DoNotOptimize(result);
  }

  state.SetLabel(std::to_string(sizeof(T)) + " bytes");
  state.SetItemsProcessed(state.iterations());
}
BENCHMARK_TEMPLATE(BM_MessageSize, SmallMessage);
BENCHMARK_TEMPLATE(BM_MessageSize, MediumMessage);

} // namespace lats::core::bench
