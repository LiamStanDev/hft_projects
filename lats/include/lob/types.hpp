#pragma once

#include <cstdint>

namespace lats::lob {

/// 訂單方向
enum class Side : uint8_t { Buy = 0, Sell = 1 };

using OrderID = uint64_t;
using Price = int64_t;      // 定點數
using Quantity = uint32_t;  // 數量
using TimeStamp = uint64_t; // 納秒時間戳

/// 價格精度常數
constexpr int64_t PRICE_SCALE = 10'000; // 精度到 0.0001

inline Price to_fixed_price(double price) {
  return static_cast<Price>(price * PRICE_SCALE);
}

} // namespace lats::lob
