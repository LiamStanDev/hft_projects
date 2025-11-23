#pragma once

#include <optional>
#include <string>

namespace emb {

class Connection {
public:
  explicit Connection(int fd);
  ~Connection();

  Connection(const Connection &) = delete;
  Connection &operator=(const Connection &) = delete;

  Connection(Connection &&other) noexcept;
  Connection &operator=(Connection &&other) noexcept;

  int fd() const { return fd_; }

  // 從 socket 讀取數據到緩衝區
  ssize_t read_to_buffer();

  // 從緩衝區取得一條完整的消息
  std::optional<std::string> get_message();

  // 發送數據
  bool send_data(const std::string &data);

private:
  int fd_;
  std::string read_buffer_;
};

} // namespace emb
