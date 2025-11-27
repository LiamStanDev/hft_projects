# Unit test
enable_testing()

find_package(GTest REQUIRED)

set(LIB_SOURCES
    # ../src/network/udp_multicast_receiver.cpp
    # src/parser/message_parser.cpp
    # src/processing/order_book.cpp
    # src/processing/metric_calculator.cpp
)

add_executable(run_tests
  core/test_spsc_queue.cpp
  ${LIB_SOURCES}
)

target_link_libraries(run_tests PRIVATE
    lats_core
    GTest::gtest
    GTest::gtest_main # 提供 main 函數
    pthread
)

include(GoogleTest)
gtest_discover_tests(run_tests)
