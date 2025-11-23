#include "connect.h"

#include <cerrno>
#include <cstddef>
#include <cstring>
#include <optional>
#include <spdlog/spdlog.h>
#include <sys/socket.h>
#include <unistd.h>
#include <utility>

namespace emb {

Connection::Connection(int fd) : fd_{fd} {}

Connection::~Connection() {
  if (fd_ != -1) {
    ::close(fd_);
  }
}

Connection::Connection(Connection &&other) noexcept
    : fd_{std::exchange(other.fd_, -1)},
      read_buffer_(std::move(other.read_buffer_)) {}

Connection &Connection::operator=(Connection &&other) noexcept {
  if (this != &other) {
    if (fd_ != -1) {
      ::close(fd_);
    }

    fd_ = std::exchange(other.fd_, -1);
    read_buffer_ = std::move(other.read_buffer_);
  }

  return *this;
}

ssize_t Connection::read_to_buffer() {
  char buf[1024];
  ssize_t n = ::recv(fd_, buf, sizeof(buf), 0);

  if (n > 0) {
    read_buffer_.append(buf, n);
  }

  return n;
}

std::optional<std::string> Connection::get_message() {
  size_t pos = read_buffer_.find('\n');

  if (pos == std::string::npos) {
    return std::nullopt;
  }

  std::string message = read_buffer_.substr(0, pos);

  read_buffer_.erase(0, pos + 1);
  return message;
}

bool Connection::send_data(const std::string &data) {
  size_t total_send = 0;

  while (total_send < data.size()) {
    ssize_t n =
        ::send(fd_, data.data() + total_send, data.size() - total_send, 0);

    if (n == -1) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        spdlog::warn("Send buffer full for fd {}", fd_);
        return false;
      }
      spdlog::error("send() failed: {}", strerror(errno));
      return false;
    }

    total_send += n;
  }

  return true;
}

} // namespace emb
