cmake_minimum_required(VERSION 3.16)
project(epoll_market_broadcaster LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dependencies
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
FetchContent_MakeAvailable(googletest)
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)

# Compile options
add_compile_options(-Wall -Wextra -Wpedantic)

# Release
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")

include_directories(${PROJECT_SOURCE_DIR}/include)

# Sources
set(SOURCES
    src/main.cpp
    src/server.cpp
    src/connection.cpp
)

# Main program
add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE
    pthread
    fmt::fmt
    spdlog::spdlog
)

# # Test client
# add_executable(test_client
#     client/test_client.cpp
# )
#
# target_link_libraries(test_client PRIVATE pthread)
#
# # Testing
# enable_testing()
#
# set(LIB_SOURCES
#     # src/network/udp_multicast_receiver.cpp
#     # src/parser/message_parser.cpp
#     # src/processing/order_book.cpp
#     # src/processing/metric_calculator.cpp
# )
#
# add_executable(run_tests
#   # tests/test_order_book.cpp
#   # tests/test_lockfree_queue.cpp
#   # tests/test_metric_calculator.cpp
#   ${LIB_SOURCES}
# )
#
# target_link_libraries(run_tests
#   GTest::gtest
#   GTest::gtest_main
# )
#
# include(GoogleTest)
# gtest_discover_tests(run_tests)
