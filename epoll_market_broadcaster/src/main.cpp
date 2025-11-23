#include "server.h"

#include <atomic>
#include <csignal>
#include <cstdint>
#include <exception>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>

emb::EpollServer *g_server = nullptr;

void signal_handler(int sig) {
  spdlog::info("Received signal {}, shutting down...", sig);
  if (g_server) {
    g_server->stop();
  }
}

int main(int argc, char *argv[]) {
  spdlog::set_level(spdlog::level::debug);

  uint16_t port = 8888;
  if (argc > 1) {
    port = static_cast<uint16_t>(std::atoi(argv[1]));
  }

  try {
    emb::EpollServer server(port);
    g_server = &server;

    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    std::atomic<bool> running{true};

    std::thread producer([&server, &running]() {
      int tick = 0;

      const char *symbols[] = {"AAPL", "GOOG", "TSLA", "MSFT", "AMZN"};
      const int num_symbols = 5;

      while (running.load(std::memory_order_relaxed)) {
        int idx = tick % num_symbols;
        double price = 100.0 + (idx * 50) + (tick % 10);
        int volume = 100 * ((tick % 5) + 1);

        // 格式: SYMBOL|PRICE|VOLUME\n
        std::string quote = std::string(symbols[idx]) + "|" +
                            std::to_string(price) + "|" +
                            std::to_string(volume) + "\n";

        if (!server.enqueue_broadcast(quote)) {
          spdlog::warn("Broadcast queue full, dropping message");
        }

        spdlog::debug("Producing: {}", quote.substr(0, quote.size() - 1));

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        tick++;
      }

      spdlog::info("Producer thread stopped");
    });

    spdlog::info("Starting event loop...");
    server.run();

    running.store(false, std::memory_order_relaxed);
    producer.join();

    spdlog::info("Server shutdown complete");

  } catch (const std::exception &e) {
    spdlog::error("Fatal error: {}", e.what());
    return 1;
  }

  return 0;
}
