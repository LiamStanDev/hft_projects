#pragma once

#include "connect.h"
#include "lockfree_queue.h"

#include <atomic>
#include <cstdint>
#include <string>
#include <unordered_map>

namespace emb {

class EpollServer {
public:
  explicit EpollServer(uint16_t port);
  ~EpollServer();

  EpollServer(const EpollServer &) = delete;
  EpollServer &operator=(const EpollServer &) = delete;

  void run();
  void stop();

  bool enqueue_broadcast(const std::string &data);

private:
  int listen_fd_{-1};
  int epoll_fd_{-1};
  std::atomic<bool> running_{false};
  std::unordered_map<int, Connection> connections_;
  emb::LockFreeQueue<std::string, 1024> broadcast_queue;

  void create_listen_socket(uint16_t port);
  void create_epoll();
  void set_nonblocking(int fd);
  void add_to_epoll(int fd, uint32_t events);
  void remove_from_epoll(int fd);

  // 處理新的連接
  void handle_accept();
  // 處裡連線斷開
  void handle_close(int fd);
  // 處理可讀
  void handle_read(int fd);

  void broadcast(const std::string &data);
  void process_broadcast_queue();
};

} // namespace emb
