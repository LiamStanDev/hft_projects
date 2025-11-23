# 🚀 C++ 高性能項目快速入門與 HFT 技能樹

這是一個專為快速掌握 C++ 並應用於高頻交易（HFT）領域而設計的實戰學習計畫。計畫包含四個專案，涵蓋了從基礎構建到高性能網路、併發優化和數據處理的全方位技能。

---

## 項目總覽及技能對應表

| 項目編號 | 項目名稱 | HFT 相關性 | 核心目標 | 涵蓋關鍵技能 |
| :---: | :--- | :---: | :--- | :--- |
| **1** | 高性能行情數據解析器 | 高 | 處理低延遲 UDP Multicast 數據流 | CMake, spdlog, Linux Socket, Lock-free Queue, C++17 |
| **2** | 內存型訂單簿引擎 | 核心 | 實現低延遲 Limit Order Book | CMake/Google Test, Boost Intrusive, SIMD/AVX2, 低延遲測量 |
| **3** | TCP 市場數據代理伺服器 | 高 | 高效分發市場數據給多個客戶端 | Asio, Protocol Buffers, Linux epoll/io_uring, C++20 Coroutines |
| **4** | 簡易回測引擎與策略框架 | 高 | 實現高性能歷史數據回測與分析 | Arrow/Parquet, Memory-mapped files, SIMD, C++20 Ranges/Modules |

---

## ⚡ 項目 1: 高性能行情數據解析器（HFT 相關）

### 目標

解析 UDP multicast 市場數據（如 ITCH 協議），進行基本統計，並利用多線程和 Lock-free 機制最小化處理延遲。

### 核心技能點

| 技能類別 | 關鍵技術點 | 學習目標 |
| :--- | :--- | :--- |
| **構建/基礎** | **CMake** | 設定項目結構，管理 `spdlog`（日誌）和 `fmt`（格式化）等常用庫的依賴。 |
| **網路/系統** | **Linux Socket Programming (UDP Multicast)** | 學習加入 Multicast 組，設置套接字選項，優化網絡接收效率。 |
| **併發** | **Multi-threading (Producer-Consumer)** | 實現一個線程接收網絡數據（I/O 隔離），一個線程處理數據。 |
| **現代 C++ (C++17)** | `structured bindings`, `std::optional` | 簡化複雜數據結構的訪問；安全處理可能解析失敗的數據。 |
| **核心數據結構** | `std::vector`, `std::unordered_map` | 存儲訂單簿快照，並按 Symbol 快速查找。 |
| **進階併發** | **Lock-free queue** | 在線程間高效傳遞原始數據包或解析後的事件，以最小化延遲。 |

### 核心功能

* 接收模擬的市場數據 feed。
* 解析訂單簿更新事件（Add/Delete/Modify）。
* 計算 VWAP (Volume-Weighted Average Price) 和 Spread 等實時指標。

---

## 🧠 項目 2: 內存型訂單簿引擎（HFT 相關）

### 目標

實現一個低延遲、高效率的 Limit Order Book (LOB) 數據結構，能夠快速處理訂單事件。

### 核心技能點

| 技能類別 | 關鍵技術點 | 學習目標 |
| :--- | :--- | :--- |
| **測試** | **CMake + Google Test** | 為訂單簿的核心業務邏輯編寫單元測試，確保低延遲邏輯的正確性。 |
| **庫/數據結構** | **Boost (boost::intrusive containers)** | 使用侵入式容器實現高效的內部鏈表，**避免動態內存分配**帶來的延遲。 |
| **現代 C++ (C++17/20)** | `constexpr`, `std::span` | 利用編譯期計算提高性能；使用 `std::span` 安全處理連續內存區域。 |
| **核心數據結構** | **Custom Allocators**, 優先隊列替代方案 | 嘗試使用內存池優化 LOB 實體（如 `Order`、`PriceLevel`）的分配。 |
| **性能優化** | **SIMD (AVX2)** | 應用向量化指令集加速價格級別的批量更新或數據複製（例如 LOB 快照）。 |
| **併發** | **Thread-per-symbol 模型** | 為每個交易品種分配專屬線程處理事件，實現**無鎖/最小鎖競爭**。 |
| **核心功能** | **延遲測量 (p50, p99)** | 使用高精度計時器，準確量化訂單處理延遲分佈。 |

---

## 🌐 項目 3: TCP 市場數據代理伺服器

### 目標

構建一個高性能、可處理多客戶端連接的市場數據分發系統。

### 核心技能點

| 技能類別 | 關鍵技術點 | 學習目標 |
| :--- | :--- | :--- |
| **構建/包管理** | **CMake + Conan/vcpkg** | 學習如何使用包管理器來引入和管理複雜的第三方庫（如 Asio, Protocol Buffers）。 |
| **網路編程** | **Asio (異步網路)** | 使用 Asio 庫實現非阻塞、事件驅動的 TCP Server 和 Client。 |
| **網路/系統** | **Linux epoll/io_uring** | 學習使用 Linux I/O 多路複用模型來高效管理大量並發連接。 |
| **併發** | **Thread Pool + Work Stealing** | 實現一個高效的線程池來處理網絡事件和業務邏輯。 |
| **現代 C++ (C++20)** | **Coroutines**, **Concepts** | 嘗試使用 C++20 協程簡化異步網路代碼；使用 Concepts 提升接口的表達性。 |
| **核心數據結構** | `std::shared_ptr`, `std::weak_ptr` | 安全地管理和跟蹤客戶端連接的生命週期。 |
| **核心功能** | **Protocol Buffers** | 使用 Protobuf 定義高效的跨語言/跨平台數據傳輸格式。 |
| **進階系統** | **零拷貝技術 (sendfile, splice)** | 探討和應用系統調用，在內核空間高效傳輸數據，減少 CPU 開銷。 |

---

## 📈 項目 4: 簡易回測引擎與策略框架

### 目標

實現一個可插拔策略、高效讀取大數據集並能進行並行計算的回測系統。

### 核心技能點

| 技能類別 | 關鍵技術點 | 學習目標 |
| :--- | :--- | :--- |
| **構建/發布** | **CMake + CPack** | 學習將編譯好的程序和資源打包成可分發的安裝包。 |
| **大數據 I/O** | **Arrow/Parquet**, **Memory-mapped files** | 使用 columnar 格式（如 Parquet）讀取歷史數據；利用 mmap **零延遲**訪問大文件。 |
| **算法/計算** | **TA-Lib**, **SIMD (向量化指標計算)** | 引入 TA-Lib 庫；嘗試用 SIMD 技術手動向量化常用技術指標（如 SMA, EMA）的計算。 |
| **現代 C++ (C++20)** | **Modules**, **Ranges** | 嘗試使用 C++ Modules 提升編譯速度和模塊化；用 C++ Ranges 簡化數據處理流程。 |
| **核心數據結構** | `std::algorithm`, **Execution Policies** | 應用 STL 算法，並利用 `std::execution::par` 並行執行策略加速數據處理。 |
| **併發** | **Parallel Backtesting** | 實現多個線程同時對多個 Symbols 或多個策略運行回測。 |
| **核心功能** | **事件驅動架構** | 設計清晰的策略接口（`virtual base class`），實現事件（Tick/Bar）驅動的回測流程。 |

---

## 💻 推薦開發環境

**作業系統:** 推薦 **Linux (Ubuntu/CentOS)**，因為 HFT 項目中的許多高性能網路和系統技術（如 epoll, io_uring, sendfile, Multicast）在 Linux 上更原生、更高效。
**編譯器:** GCC 11+ 或 Clang 14+（確保支援完整的 C++20 特性）。
**工具:** Git, GDB/LLDB (調試), Valgrind/Cachegrind (性能分析)。
