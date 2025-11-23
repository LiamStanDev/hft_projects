cmake_minimum_required(VERSION 3.16)
project(
  LATS
  VERSION 0.1.0
  LANGUAGES CXX)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compile options
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -fsanitize=address,undefined")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")

# Export compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Dependencies
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)
find_package(GTest REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/include)

# Submodules
add_subdirectory(src/core)
# add_subdirectory(src/feed)

# Testing

# enable_testing() add_subdirectory(tests)

# Becnmark

# add_subdirectory(bench)
