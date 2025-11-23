# Epoll 行情廣播伺服器 (Epoll Market Broadcaster)

## 項目簡介

從零使用 Linux epoll 系統調用實現一個高性能 TCP Server，接收模擬的市場行情數據並廣播給所有連接的客戶端。**不依賴任何網路庫 (如 Asio)**，直接學習底層 I/O 多路複用機制。

---

## 項目架構

```
epoll_market_broadcaster/
├── CMakeLists.txt              # CMake 配置
├── README.md
├── include/
│   ├── server.hpp              # Epoll Server 核心類
│   └── connection.hpp          # 連接管理封裝
├── src/
│   ├── main.cpp                # 入口 + 行情生產者
│   ├── server.cpp              # Epoll 核心邏輯
│   └── connection.cpp          # 連接管理實現
└── client/
    └── test_client.cpp         # 測試客戶端
```

---

## 核心學習目標

| 優先級 | 技能 | 具體內容 | HFT 相關性 |
|:---:|:---|:---|:---|
| ⭐⭐⭐ | **epoll 核心 API** | `epoll_create1`, `epoll_ctl`, `epoll_wait` | Linux 高性能 I/O 多路複用的核心 |
| ⭐⭐⭐ | **Non-blocking I/O** | `fcntl(O_NONBLOCK)`, 處理 `EAGAIN` | HFT 系統絕不能阻塞 |
| ⭐⭐⭐ | **Edge-Triggered 模式** | ET vs LT 的差異與正確使用 | ET 是高性能場景的標準選擇 |
| ⭐⭐ | **TCP Socket 編程** | `socket`, `bind`, `listen`, `accept`, `send`, `recv` | 網路編程基礎 |
| ⭐⭐ | **緩衝區管理** | 處理粘包/拆包、部分讀寫 | 實際網路編程必備 |
| ⭐⭐ | **RAII 封裝** | 用 class 封裝 fd，自動關閉 | 現代 C++ 資源管理 |

---

## 實現模塊詳解

### 1. server.hpp - Epoll Server 核心類

```cpp
class EpollServer {
public:
    EpollServer(int port);
    ~EpollServer();
    
    void run();                                    // 主事件循環
    void broadcast(const std::string& data);       // 廣播給所有客戶端
    void stop();                                   // 停止伺服器
    
private:
    void handle_accept();                          // 處理新連接
    void handle_read(int fd);                      // 處理客戶端數據 (ET 模式)
    void handle_close(int fd);                     // 關閉連接
    void set_nonblocking(int fd);                  // 設置非阻塞
    void add_to_epoll(int fd, uint32_t events);    // 添加到 epoll
    
    int listen_fd_;
    int epoll_fd_;
    bool running_;
    std::unordered_map<int, Connection> connections_;
};
```

### 2. connection.hpp - 連接封裝

```cpp
class Connection {
public:
    explicit Connection(int fd);
    ~Connection();
    
    // 禁止拷貝，允許移動
    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;
    Connection(Connection&& other) noexcept;
    Connection& operator=(Connection&& other) noexcept;
    
    int fd() const { return fd_; }
    
    // 讀取數據到緩衝區
    ssize_t read_to_buffer();
    
    // 從緩衝區獲取完整消息
    std::optional<std::string> get_message();
    
    // 發送數據
    bool send_data(const std::string& data);

private:
    int fd_;
    std::string read_buffer_;
};
```

---

### 3. main.cpp - 主程序架構

```cpp
#include <thread>
#include <atomic>
#include "server.hpp"

int main() {
    EpollServer server(8888);
    std::atomic<bool> running{true};
    
    // 行情生產者線程 - 模擬市場數據
    std::thread producer([&]() {
        int tick = 0;
        while (running.load()) {
            // 模擬行情: SYMBOL|PRICE|VOLUME
            std::string quote = "AAPL|" + std::to_string(150.0 + tick % 10) 
                              + "|" + std::to_string(100 * (tick % 5 + 1)) + "\n";
            
            server.broadcast(quote);
            
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            tick++;
        }
    });
    
    // 主線程運行 epoll 事件循環
    server.run();
    
    running.store(false);
    producer.join();
    
    return 0;
}
```

---

## 關鍵代碼實現指南

### epoll 事件循環 (server.cpp)

```cpp
void EpollServer::run() {
    constexpr int MAX_EVENTS = 64;
    struct epoll_event events[MAX_EVENTS];
    
    while (running_) {
        int n = epoll_wait(epoll_fd_, events, MAX_EVENTS, 100);  // 100ms timeout
        
        if (n == -1) {
            if (errno == EINTR) continue;  // 被信號中斷
            perror("epoll_wait");
            break;
        }
        
        for (int i = 0; i < n; i++) {
            int fd = events[i].data.fd;
            uint32_t ev = events[i].events;
            
            if (fd == listen_fd_) {
                // 新連接
                handle_accept();
            } else if (ev & (EPOLLERR | EPOLLHUP)) {
                // 錯誤或掛起
                handle_close(fd);
            } else if (ev & EPOLLIN) {
                // 可讀 (Edge-Triggered)
                handle_read(fd);
            }
        }
    }
}
```

### Edge-Triggered 讀取 (重點!)

```cpp
void EpollServer::handle_read(int fd) {
    auto it = connections_.find(fd);
    if (it == connections_.end()) return;
    
    Connection& conn = it->second;
    
    // ET 模式必須循環讀取直到 EAGAIN
    while (true) {
        ssize_t n = conn.read_to_buffer();
        
        if (n == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // 數據讀完了
                break;
            }
            // 真正的錯誤
            handle_close(fd);
            return;
        } else if (n == 0) {
            // 客戶端關閉連接
            handle_close(fd);
            return;
        }
    }
    
    // 處理接收到的消息
    while (auto msg = conn.get_message()) {
        // 處理客戶端發來的消息 (例如訂閱請求)
        printf("Received from fd %d: %s\n", fd, msg->c_str());
    }
}
```

### 設置非阻塞 Socket

```cpp
void EpollServer::set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl F_GETFL");
        return;
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl F_SETFL");
    }
}
```

### 添加到 epoll (Edge-Triggered)

```cpp
void EpollServer::add_to_epoll(int fd, uint32_t events) {
    struct epoll_event ev;
    ev.events = events | EPOLLET;  // Edge-Triggered!
    ev.data.fd = fd;
    
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev) == -1) {
        perror("epoll_ctl ADD");
    }
}
```

---

## 學習重點

### 1. epoll 三個核心 API

| API | 功能 | 示例 |
|:---|:---|:---|
| `epoll_create1(0)` | 創建 epoll 實例 | `int epoll_fd = epoll_create1(0);` |
| `epoll_ctl()` | 添加/修改/刪除 fd | `epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev);` |
| `epoll_wait()` | 等待事件發生 | `int n = epoll_wait(epoll_fd, events, MAX_EVENTS, timeout);` |

### 2. Edge-Triggered vs Level-Triggered

| 模式 | 觸發條件 | 使用要點 |
|:---|:---|:---|
| **LT (默認)** | fd 可讀/可寫時持續觸發 | 簡單但效率較低 |
| **ET** | fd 狀態變化時觸發一次 | 必須一次讀完所有數據，否則會丟失事件 |

### 3. 常見錯誤處理

| errno | 含義 | 處理方式 |
|:---|:---|:---|
| `EAGAIN` / `EWOULDBLOCK` | 無數據可讀/緩衝區滿 | 正常，停止讀/寫 |
| `EINTR` | 被信號中斷 | 重試操作 |
| `ECONNRESET` | 連接被對方重置 | 關閉連接 |

---

## 分階段實現計畫

### Phase 1: 基礎 Echo Server (建議時間: 3-4 天)

**目標**: 實現能接受連接、收發數據的基本框架

- [ ] 創建 TCP listen socket
- [ ] 創建 epoll 實例，監聽 listen socket
- [ ] 實現 `handle_accept()` 接受新連接
- [ ] 實現基本的 `handle_read()` (Level-Triggered)
- [ ] 實現 `handle_close()` 清理連接

**驗證**: 使用 `telnet localhost 8888` 測試 echo 功能

### Phase 2: Edge-Triggered 模式 (建議時間: 2-3 天)

**目標**: 正確實現 ET 模式的讀取

- [ ] 設置 `EPOLLET` 標誌
- [ ] 修改 `handle_read()` 循環讀取直到 `EAGAIN`
- [ ] 測試大數據量傳輸

**驗證**: 發送大量數據確保不丟失

### Phase 3: 行情廣播功能 (建議時間: 2-3 天)

**目標**: 完整的行情廣播系統

- [ ] 實現 `broadcast()` 向所有客戶端發送數據
- [ ] 創建行情生產者線程
- [ ] 處理發送緩衝區滿的情況

**驗證**: 多個客戶端同時接收行情

### Phase 4: 優化與擴展 (選做)

- [ ] 實現訂閱機制 (客戶端只接收特定 Symbol)
- [ ] 添加延遲測量 (從生產到發送的時間)
- [ ] 嘗試 `EPOLLOUT` 處理寫緩衝區

---

## 構建與運行

```bash
# 構建
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)

# 運行伺服器
./epoll_server

# 另一個終端運行測試客戶端
./test_client

# 或使用 telnet 測試
telnet localhost 8888
```

---

## 測試客戶端使用

```bash
# 基本連接測試
./test_client

# 多客戶端測試 (開多個終端)
./test_client &
./test_client &
./test_client &
```

---

## 預期成果

完成本項目後，你將能夠：

1. 獨立編寫基於 epoll 的事件驅動網路程序
2. 理解 Edge-Triggered 和 Level-Triggered 的差異
3. 正確處理非阻塞 I/O 的各種邊界情況
4. 理解 Asio/libuv 等庫在底層做了什麼
5. 具備閱讀和修改 HFT 系統網路層代碼的能力

---

## 參考資源

- `man 7 epoll` - epoll 概述
- `man 2 epoll_wait` - epoll_wait 詳細說明
- `man 2 socket` - socket 系統調用
- The Linux Programming Interface - Chapter 63 (Alternative I/O Models)
