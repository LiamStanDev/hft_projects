#include "lob/types.hpp"
#include <boost/intrusive/list.hpp>

namespace lats::lob {

struct Order {
  OrderID order_id;
  Price price;
  Quantity quantity;
  Side side;
  TimeStamp timestamp;
};

} // namespace lats::lob
