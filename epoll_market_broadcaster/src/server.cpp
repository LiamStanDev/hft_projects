#include "server.h"

#include <atomic>
#include <spdlog/spdlog.h>

#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

namespace emb {

EpollServer::EpollServer(uint16_t port) {
  create_listen_socket(port);
  create_epoll();

  add_to_epoll(listen_fd_, EPOLLIN);

  spdlog::info("Server listening on port {}", port);
}

void EpollServer::create_listen_socket(uint16_t port) {
  listen_fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
  if (listen_fd_ < 0) {
    throw std::runtime_error("socket() failed: " +
                             std::string(strerror(errno)));
  }

  int reuse = 1;
  if (::setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &reuse,
                   sizeof(reuse)) == -1) {
    throw std::runtime_error("setsockopt() failed: " +
                             std::string(strerror(errno)));
  }

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(port);

  if (::bind(listen_fd_, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) ==
      -1) {
    throw std::runtime_error("bind() failed: " + std::string(strerror(errno)));
  }

  if (::listen(listen_fd_, SOMAXCONN) == -1) {
    throw std::runtime_error("listen() failed: " +
                             std::string(strerror(errno)));
  }

  set_nonblocking(listen_fd_);
}

EpollServer::~EpollServer() {
  connections_.clear();

  if (epoll_fd_ != -1) {
    ::close(epoll_fd_);
  }
  if (listen_fd_ != -1) {
    ::close(listen_fd_);
  }
}

void EpollServer::set_nonblocking(int fd) {
  int flags = ::fcntl(fd, F_GETFL, 0); // get all flags
  if (flags == -1) {
    throw std::runtime_error("fcntl F_GETFL failed");
  }
  if (::fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
    throw std::runtime_error("fcntl F_SETFL failed");
  }
}

void EpollServer::create_epoll() {
  epoll_fd_ = epoll_create1(0);
  if (epoll_fd_ == -1) {
    throw std::runtime_error("epoll_create1() failed");
  }
}

void EpollServer::add_to_epoll(int fd, uint32_t events) {
  epoll_event ev{};
  ev.events = events | EPOLLET; // Edege-Triggered
  ev.data.fd = fd;

  if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev) == -1) {
    throw std::runtime_error("epoll_ctl ADD failed");
  }
}

void EpollServer::remove_from_epoll(int fd) {
  epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr);
}

void EpollServer::run() {
  constexpr int MAX_EVENTS = 64;
  epoll_event events[MAX_EVENTS]{};

  running_.store(true, std::memory_order_release);

  while (running_.load(std::memory_order_relaxed)) {
    int n = ::epoll_wait(epoll_fd_, events, MAX_EVENTS, 10);

    if (n == -1) {
      if (errno == EINTR) { // signal interupt
        continue;
      }
      throw std::runtime_error("epoll_wait failed");
    }

    for (int i = 0; i < n; ++i) {
      int fd = events[i].data.fd;
      uint32_t ev = events[i].events;

      if (fd == listen_fd_) {
        handle_accept();
      } else if (ev & (EPOLLERR | EPOLLHUP)) { // 錯誤或者對方中斷連線
        handle_close(fd);
      } else if (ev & EPOLLIN) {
        handle_read(fd);
      }
    }

    process_broadcast_queue();
  }
}

void EpollServer::handle_accept() {
  while (true) { // ET 下必需處裡完成，因為不會再次通知
    sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);

    int client_fd = ::accept(
        listen_fd_, reinterpret_cast<sockaddr *>(&client_addr), &client_len);

    if (client_fd == -1) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        break; // 讀完了
      }

      spdlog::error("accept() failed: {}", strerror(errno));
      break;
    }

    set_nonblocking(client_fd);
    add_to_epoll(client_fd, EPOLLIN);

    connections_.emplace(client_fd, Connection(client_fd));

    char ip_str[INET_ADDRSTRLEN];
    ::inet_ntop(AF_INET, &client_addr.sin_addr, ip_str, sizeof(ip_str));
    spdlog::info("New connection: fd={} from {}:{}, total={}", client_fd,
                 ip_str, ntohs(client_addr.sin_port), connections_.size());
  }
}

void EpollServer::handle_read(int fd) {
  auto it = connections_.find(fd);
  if (it == connections_.end()) {
    spdlog::warn("Unknown fd {} in handle_read", fd);
    return;
  }

  Connection &conn = it->second;

  while (true) {
    ssize_t n = conn.read_to_buffer();

    if (n == -1) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        break; // 讀完了
      }
      spdlog::error("recv() failed: {}", strerror(errno));
      handle_close(fd);
      return;
    } else if (n == 0) {
      // 正常關閉
      handle_close(fd);
      return;
    }
  }

  while (auto msg = conn.get_message()) {
    spdlog::debug("Received from fd {}: {}", fd, *msg);

    // Echo
    // conn.send_data(*msg + "\n");
  }
}

void EpollServer::handle_close(int fd) {
  spdlog::info("Connection closed: fd={}, remaining={}", fd,
               connections_.size() - 1); // -1 因為還沒 erase
  remove_from_epoll(fd);
  connections_.erase(fd);
  remove_from_epoll(fd);
  connections_.erase(fd);
}

void EpollServer::stop() { running_.store(false, std::memory_order_relaxed); }

void EpollServer::broadcast(const std::string &data) {
  for (auto &[fd, conn] : connections_) {
    if (!conn.send_data(data)) {
      spdlog::warn("Failed to send to fd {}", fd);
    }
  }
}

bool EpollServer::enqueue_broadcast(const std::string &data) {
  return broadcast_queue.push(data);
}

void EpollServer::process_broadcast_queue() {
  while (auto msg = broadcast_queue.pop()) {
    broadcast(*msg);
  }
}

} // namespace emb
