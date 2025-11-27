# 🚀 HFT 低延遲事件驅動交易核心系統 (LATS) - C++17 標準

## 📈 當前進度總覽

**最後更新：** 2025-11-27

| 階段 | 狀態 | 完成度 | 關鍵成果 |
|------|------|--------|---------|
| **階段一：數據採集與 I/O 隔離** | 🟡 進行中 | 80% | ✅ Lock-free Queue<br>✅ 性能測試 (15.6ns)<br>⚠️ UDP Multicast 待實現 |
| **階段二：LOB/Matching Engine** | ⚪ 未開始 | 0% | - |
| **階段三：策略與回測框架** | ⚪ 未開始 | 0% | - |
| **階段四：Gateway/Distribution** | ⚪ 未開始 | 0% | - |

### ✅ 已完成的核心組件

#### **Lock-free SPSC Queue** (`include/core/spsc_queue.hpp`)
- ✅ 單生產者單消費者無鎖隊列實現
- ✅ Cache line padding 防止 false sharing
- ✅ 使用 `std::atomic` 實現內存順序控制
- ✅ 支持任意可移動類型 (move semantics)

**性能指標：**
- 單次 Push/Pop 延遲：**15.6 納秒**
- 吞吐量：**1.39 億次操作/秒**
- 測試環境：6 核 12 線程 CPU @ 2.5GHz

**測試覆蓋：**
- ✅ 基本功能測試（Push/Pop/Empty）
- ✅ 邊界測試（隊列滿/空）
- ✅ FIFO 順序驗證
- ✅ Move Semantics 測試
- ✅ 併發安全性測試（Producer-Consumer）
- ✅ 壓力測試（100 萬次操作）

#### **高精度計時器** (`include/core/timer.hpp`)
- ✅ 基於 TSC (Time Stamp Counter) 的納秒級計時
- ✅ 自動校準 TSC 頻率
- ✅ 提供 RAII 風格的 `ScopedTimer`
- ✅ Cycles 到納秒的轉換

#### **延遲統計分析** (`include/core/latency_stats.hpp`)
- ✅ 支持百分位數計算 (P50/P95/P99/P999)
- ✅ 平均值、最小值、最大值統計
- ✅ 基於排序的精確百分位數

#### **性能基準測試框架** (`bench/bench_spsc_queue.cpp`)
- ✅ Google Benchmark 集成
- ✅ 單次操作延遲測試
- ✅ Producer-Consumer 端到端延遲測試
- ✅ 吞吐量測試
- ✅ 不同消息大小的性能影響分析

### 🚧 待完成的階段一任務

- ⚠️ **UDP Multicast 數據接收器** - 實現 Linux Socket 接收市場數據
- ⚠️ **I/O 線程隔離架構** - 使用 SPSC Queue 隔離網路 I/O
- ⚠️ **日誌系統集成** - 使用 spdlog 記錄精確時間戳

### 🎯 下一步計劃

**選項 A：完成階段一**
- 實現 UDP Multicast Receiver
- 集成 SPSC Queue 進行 I/O 隔離
- 完整的階段一驗收

**選項 B：進入階段二（推薦）**
- 實現 Limit Order Book (LOB) 引擎
- 學習 Boost Intrusive 容器
- 設計 Custom Memory Allocator

---

## 🌟 項目說明與核心目標

這個專案是一個以 **C++17** 為基礎，運行於 **Linux 環境**下的高性能、低延遲事件驅動交易系統骨幹（**L**ow **A**dded **T**rading **S**ystem）。本專案目標是在不依賴 C++20 特性的情況下，透過精湛的系統工程和現代 C++17 技巧，實現**納秒級別**的數據處理和策略信號生成。

### 📌 核心職能對應 (對應 JD 要求)

* **開發與優化交易系統：** 整個 LATS 系統就是一個可運行的交易骨幹。
* **低延遲、多執行緒：** 專案中大量使用 Lock-free、SIMD、epoll/io_uring 等技術進行系統性優化。
* **金融知識：** 實作 Limit Order Book (LOB) 和事件驅動回測框架，展現對交易流程的熟悉。

## 🏗️ 核心架構與技術棧

LATS 系統採用**管道（Pipeline）架構**，確保各模塊間的低耦合和高效通信。

### 核心模塊

| 模塊編號 | 模塊名稱 | 核心職責 | 關鍵技術與優化點 |
| :---: | :--- | :--- | :--- |
| **I** | Data Feed Handler | 接收、解析並隔離 I/O 延遲。 | **UDP Multicast, Linux Socket, Lock-free Queue, C++17** |
| **II** | LOB/Matching Engine | 核心業務邏輯，處理訂單簿更新。 | **Boost Intrusive, SIMD/AVX2, Custom Allocators, Google Test** |
| **III** | Strategy & Backtester | 策略開發、歷史數據分析與信號生成。 | **Arrow/Parquet, Memory-mapped files, Boost Algorithm/Lambda** |
| **IV** | Gateway/Distribution | 高效分發 LOB 快照，介接策略訂單。 | **Asio (Proactor 模式), Linux epoll, Protocol Buffers, C++17 `std::async`** |

### 推薦開發環境

* **作業系統：** Linux (Ubuntu/CentOS)
* **編譯器：** GCC 8+ / Clang 6+ (需完整支援 C++17)
* **構建工具：** CMake (含 Conan/vcpkg 進行依賴管理)

## 📋 實作步驟與階段驗收

整個專案分為四個主要階段，每個階段都需通過嚴格的驗收與測試。

### 階段一：數據採集與 I/O 隔離 (Data Feed Handler)

| 實作內容 | 核心學習內容 | 驗收與測試標準 |
| :--- | :--- | :--- |
| **功能實作** | 1. 設置 CMake 專案結構，引入 `spdlog`, `fmt`。2. 實現 UDP Multicast 數據接收。3. 實作一個高性能 **Lock-free Queue**。4. 隔離 I/O 線程，將原始數據包放入 Lock-free Queue。 | **單元測試：** Lock-free Queue 的併發安全性測試。**性能測試：** 驗證 I/O 線程與處理線程間的數據傳輸延遲 P99 需 **< 100ns**。**日誌系統：** 必須能精準記錄接收與傳輸時間戳。 |
| **關鍵技術** | Linux Socket (Multicast), 多線緒 (Producer-Consumer), **Lock-free Queue** 設計, C++17 `structured bindings`, `std::optional`。 |

### 階段二：核心訂單簿引擎 (LOB/Matching Engine)

| 實作內容 | 核心學習內容 | 驗收與測試標準 |
| :--- | :--- | :--- |
| **功能實作** | 1. 實現 LOB 核心數據結構，使用 **Boost Intrusive** 避免堆內存分配。2. 應用 **Custom Allocators** 管理訂單實體內存。3. 實作 Add/Delete/Modify 訂單邏輯。4. 導入高精度延遲測量工具。 | **單元測試：** 100% 覆蓋 LOB 的核心邏輯，確保邊緣條件（如：撤銷不存在的訂單）的健壯性。**性能測試：** 測量單一訂單處理的 P50/P99 延遲，目標 P99 需 **< 1us**。**優化驗證：** 使用 Cachegrind 驗證內存分配和快取命中率。 |
| **關鍵技術** | **Boost Intrusive** 應用, 內存池/自定義分配器, **SIMD/AVX2** 向量化計算, 高精度計時器, `std::variant` (用於事件類型)。 |

### 階段三：策略與數據分析框架 (Strategy & Backtester)

| 實作內容 | 核心學習內容 | 驗收與測試標準 |
| :--- | :--- | :--- |
| **功能實作** | 1. 實現事件驅動架構 (Tick/Bar 事件)。2. 導入 **Arrow/Parquet** 庫讀取歷史數據。3. 使用 **Memory-mapped files (mmap)** 實現零延遲歷史數據訪問。4. 實作簡易策略介面，並使用 **Boost Algorithm** 實現指標計算。 | **準確性測試：** 實時模式與回測模式的指標計算結果必須一致。**I/O 性能：** 測量歷史數據集從磁盤讀取到內存的速度，驗證 mmap 的性能優勢。**模塊化：** 策略必須能以即插即用 (Plug-and-Play) 的方式加載。 |
| **關鍵技術** | **Memory-mapped files (mmap)**, Arrow/Parquet 數據 I/O, **Boost Algorithm & Lambda** (替代 C++20 Ranges), 策略虛擬基礎類設計。 |

### 階段四：高性能服務網關 (Gateway/Distribution Server)

| 實作內容 | 核心學習內容 | 驗收與測試標準 |
| :--- | :--- | :--- |
| **功能實作** | 1. 使用 **Protocol Buffers** 定義數據傳輸格式。2. 構建基於 **Asio (非阻塞 I/O)** 的異步 TCP Server，採用 **Proactor** 設計模式。3. 使用 **Linux epoll** 管理大量併發連線。4. 實作 LOB 快照的高效分發。5. 使用 **零拷貝技術** (`sendfile`) 優化數據傳輸。 | **網路壓力測試：** 伺服器必須能在處理 **100+** 併發連線時，保持數據分發延遲穩定。**系統優化：** 驗證 `sendfile` 的應用，減少 CPU 開銷。**協議兼容性：** 客戶端必須能正確解析 Protobuf 消息。 |
| **關鍵技術** | **Asio (Proactor 模式)**, **Linux epoll** I/O 多路複用, **Protocol Buffers**, 零拷貝技術 (`sendfile`), C++17 `std::async` 或線程池。 |

## 📚 核心學習內容與技術樹 (C++17 聚焦)

透過完成 LATS 專案，您將紮實掌握以下 HFT 領域的關鍵 C++ 工程技能，並證明您能夠在穩定的 C++17 環境中實現卓越性能：

| 技術類別 | 關鍵技能點 | 核心應用場景 |
| :--- | :--- | :--- |
| **C++ 語言特性** | C++17 Structured Bindings, `std::optional`, `std::variant`, `std::string_view` | 簡化複雜數據訪問、安全處理可能失敗的解析、優化字符串操作。 |
| **併發與延遲優化** | Lock-free Queue (SPSC/SPMC), Thread-per-Symbol, Cache Line Padding | I/O 線程與處理線程間通信、最小化鎖競爭。 |
| **系統與 I/O** | Linux epoll, UDP Multicast, Memory-mapped files (mmap), 零拷貝 | 高效能網路處理、零延遲文件 I/O。 |
| **數據結構/算法** | Boost Intrusive Containers, Custom Allocators, SIMD (AVX2), Boost Algorithm | LOB 引擎，避免堆內存分配、向量化計算、數據流程處理。 |
| **工程與測試** | CMake, spdlog, Google Test, 高精度延遲測量 (TSC, p50/p99) | 構建管理、精準日誌、單元測試、性能量化。 |
