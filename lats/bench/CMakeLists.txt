find_package(benchmark REQUIRED)


set(LIB_SOURCES
    # ../src/network/udp_multicast_receiver.cpp
    # src/parser/message_parser.cpp
    # src/processing/order_book.cpp
    # src/processing/metric_calculator.cpp
)

add_executable(run_bench
    bench_spsc_queue.cpp
    ${LIB_SOURCES}
)

target_link_libraries(run_bench
    PRIVATE
    lats_core
    benchmark::benchmark
    benchmark::benchmark_main # 提供 main 函數
    pthread
)
