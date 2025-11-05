.PHONY: all build clean test install help fmt tidy

ifeq ($(OS),Windows_NT)
    CMAKE_GENERATOR := -G "MinGW Makefiles"
    RM := cmd /c del /f /q
    RMDIR := cmd /c rmdir /s /q
    PATH_SEP := \\
    EXE_EXT := .exe
else
    CMAKE_GENERATOR :=
    RM := rm -f
    RMDIR := rm -rf
    PATH_SEP := /
    EXE_EXT :=
endif

# 默认目标
all: build

# 配置并构建项目
build:
	@echo "==================================="
	@echo "Building CZC Compiler"
	@echo "==================================="
	@cmake -B build $(CMAKE_GENERATOR) -DCMAKE_BUILD_TYPE=Release
	@cmake --build build
	@echo ""
	@echo "==================================="
	@echo "Build completed successfully!"
	@echo "==================================="
	@echo ""
	@echo "Executables:"
	@echo "  - CLI tool:        .$(PATH_SEP)build$(PATH_SEP)czc-cli$(EXE_EXT)"
	@echo "  - Test suite:      .$(PATH_SEP)build$(PATH_SEP)tests$(PATH_SEP)test_lexer$(EXE_EXT)"
	@echo "  - Debug operators: .$(PATH_SEP)build$(PATH_SEP)tests$(PATH_SEP)debug_operators$(EXE_EXT)"
	@echo ""

# Debug 模式构建
debug:
	@echo "Building in Debug mode..."
	@cmake -B build $(CMAKE_GENERATOR) -DCMAKE_BUILD_TYPE=Debug
	@cmake --build build

# 清理构建产物
clean:
	@echo "==================================="
	@echo "Cleaning CZC Project"
	@echo "==================================="
	@cmake -E rm -rf build
	@echo "Build directory removed"
	@cmake -E rm -f examples/*.tokens
	@echo ".tokens files removed"
	@echo ""
	@echo "==================================="
	@echo "Clean completed!"
	@echo "==================================="

# 运行测试
test: build
	@echo "Running tests..."
	@cd build && ctest --output-on-failure

# 安装
install: build
	@echo "Installing CZC..."
	@cd build && cmake --install . --prefix /usr/local

# 重新构建
rebuild: clean build

# 运行示例
example: build
	@echo "Tokenizing hello.zero..."
	@./build/czc-cli tokenize examples/hello.zero
	@echo ""
	@echo "Output saved to examples/hello.zero.tokens"

# 格式化代码
fmt:
	@echo "==================================="
	@echo "Formatting C/C++ Source Files"
	@echo "==================================="
	@if command -v clang-format >/dev/null 2>&1; then \
		echo "Formatting header files..."; \
		find czc -type f \( -name "*.hpp" -o -name "*.h" \) -exec clang-format -i {} + ; \
		echo "Formatting source files..."; \
		find src -type f \( -name "*.cpp" -o -name "*.c" \) -exec clang-format -i {} + ; \
		find cli -type f \( -name "*.cpp" -o -name "*.c" \) -exec clang-format -i {} + ; \
		find tests -type f \( -name "*.cpp" -o -name "*.c" \) -exec clang-format -i {} + ; \
		echo ""; \
		echo "===================================";\
		echo "Formatting completed!";\
		echo "===================================";\
	else \
		echo "Error: clang-format not found!"; \
		echo "Please install clang-format first."; \
		echo ""; \
		echo "Installation:"; \
		echo "  macOS:   brew install clang-format"; \
		echo "  Ubuntu:  sudo apt-get install clang-format"; \
		echo "  Fedora:  sudo dnf install clang-tools-extra"; \
		echo "===================================";\
		exit 1; \
	fi

# 整理代码
tidy:
	@echo "==================================="
	@echo "Tidying C/C++ Source Files"
	@echo "==================================="
	@if command -v clang-tidy >/dev/null 2>&1; then \
		echo "Running clang-tidy..."; \
		find src cli tests -type f \( -name "*.cpp" -o -name "*.c" \) -exec clang-tidy -p build {} + ; \
		echo ""; \
		echo "===================================";\
		echo "Tidying completed!";\
		echo "===================================";\
	else \
		echo "Error: clang-tidy not found!"; \
		echo "Please install clang-tidy first."; \
		echo ""; \
		echo "Installation:"; \
		echo "  macOS:   brew install llvm"; \
		echo "  Ubuntu:  sudo apt-get install clang-tidy"; \
		echo "  Fedora:  sudo dnf install clang-tools-extra"; \
		echo "===================================";\
		exit 1; \
	fi

# 帮助信息
help:
	@echo "CZC Compiler - Makefile Commands"
	@echo "=================================="
	@echo ""
	@echo "Available targets:"
	@echo "  make (all)    - Build the project (default)"
	@echo "  make build    - Build the project in Release mode"
	@echo "  make debug    - Build the project in Debug mode"
	@echo "  make clean    - Clean all build artifacts"
	@echo "  make test     - Build and run tests"
	@echo "  make install  - Install the compiler"
	@echo "  make rebuild  - Clean and rebuild"
	@echo "  make example  - Run example tokenization"
	@echo "  make fmt      - Format C/C++ source files with clang-format"
	@echo "  make help     - Show this help message"
	@echo ""
