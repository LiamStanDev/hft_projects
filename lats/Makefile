.PHONY: env-debug env-release build-debug build-release clean test bench
BUILD_DIR = build
SOURCE_DIR = .
TOOLCHAIN_FILE = conan_toolchain.cmake

# Debug target
env-debug:
	@conan install $(SOURCE_DIR) \
		--output-folder=$(BUILD_DIR) \
		--build=missing \
		-s build_type=Debug

build-debug: env-debug
	@cmake -S $(SOURCE_DIR) -B $(BUILD_DIR) \
		-G "Ninja" \
		-DCMAKE_TOOLCHAIN_FILE=$(BUILD_DIR)/$(TOOLCHAIN_FILE) \
		-DCMAKE_BUILD_TYPE=Debug
	@cmake --build $(BUILD_DIR)

# Release target
env-release:
	@conan install $(SOURCE_DIR) \
		--output-folder=$(BUILD_DIR) \
		--build=missing \
		-s build_type=Release

build-release: env-release
	@cmake -S $(SOURCE_DIR) -B $(BUILD_DIR) \
		-G "Ninja" \
		-DCMAKE_TOOLCHAIN_FILE=$(BUILD_DIR)/$(TOOLCHAIN_FILE) \
		-DCMAKE_BUILD_TYPE=Release
	@cmake --build $(BUILD_DIR)

# Default to debug
build: build-debug

clean:
	@rm -rf $(BUILD_DIR)

test: build
	@cd $(BUILD_DIR) && ctest --output-on-failure

bench: build-release
	@echo "Running benchmarks"
	@$(BUILD_DIR)/bench/run_bench --benchmark_format=console

