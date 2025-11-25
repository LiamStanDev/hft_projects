cmake_minimum_required(VERSION 3.16)
project(LATS VERSION 0.1.0 LANGUAGES CXX)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compile options
add_compile_options(-Wall -Wextra -Wpedantic)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_options(-g -O0 -fsanitize=address,undefined)
  add_link_options(-fsanitize=address,undefined)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  add_compile_options(-O3 -march=native -DNDEBUG)
endif()

# Export compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Dependencies
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/include)

# Submodules
add_subdirectory(src/core)
# add_subdirectory(src/feed)

# Testing
add_subdirectory(tests)

# Becnmark
# add_subdirectory(bench)
