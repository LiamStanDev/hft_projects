add_library(lats_feed feed_handler.cpp)

target_link_libraries(
  lats_feed
  PUBLIC lats_core
  PRIVATE spdlog::spdlog)

add_executable(feed_receiver main.cpp)
target_link_libraries(feed_receiver lats_feed spdlog::spdlog)
