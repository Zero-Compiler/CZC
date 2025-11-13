.PHONY: all build clean test install help fmt tidy pullall pullerrmsg coverage coverage-report benchmark docs

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
	@echo "Build directory and .tokens files removed"
	@cmake -E rm -rf examples/*.formatted
	@echo "Formatted example files removed"
	@cmake -E rm -rf docs/html
	@echo "Auto generated documentation removed"
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

# 拉取所有子模块
pullall:
	@echo "Pulling all submodules..."
	@git submodule update --init --recursive
	@echo "All submodules pulled successfully!"
	@echo "==================================="

# 拉取错误信息
pullerrmsg:
	@echo "Pulling error messages..."
	@cd error-msg
	@git pull origin main
	@cd ..
	@echo "Error messages pulled successfully!"
	@echo "==================================="

# 帮助信息
help:
	@echo "CZC Compiler - Makefile Commands"
	@echo "=================================="
	@echo ""
	@echo "Available targets:"
	@echo "  make (all)           - Build the project (default)"
	@echo "  make build           - Build the project in Release mode"
	@echo "  make debug           - Build the project in Debug mode"
	@echo "  make clean           - Clean all build artifacts"
	@echo "  make test            - Build and run tests"
	@echo "  make benchmark       - Build and run performance benchmarks"
	@echo "  make coverage        - Build with coverage and run tests"
	@echo "  make coverage-report - Generate HTML coverage report"
	@echo "  make docs            - Generate API documentation with Doxygen"
	@echo "  make install         - Install the compiler"
	@echo "  make rebuild         - Clean and rebuild"
	@echo "  make example         - Run example tokenization"
	@echo "  make fmt             - Format C/C++ source files with clang-format"
	@echo "  make tidy            - Run clang-tidy on source files"
	@echo "  make pullall         - Pull all git submodules"
	@echo "  make pullerrmsg      - Pull error message submodule"
	@echo "  make help            - Show this help message"
	@echo ""

# 性能基准测试
benchmark:
	@echo "==================================="
	@echo "Building and Running Benchmarks"
	@echo "==================================="
	@cmake -B build $(CMAKE_GENERATOR) -DCMAKE_BUILD_TYPE=Release -DBUILD_BENCHMARKS=ON
	@cmake --build build
	@echo ""
	@echo "Running lexer benchmarks..."
	@./build/benchmarks/benchmark_lexer
	@echo ""
	@echo "Running parser benchmarks..."
	@./build/benchmarks/benchmark_parser
	@echo ""
	@echo "==================================="
	@echo "Benchmark completed!"
	@echo "==================================="

# 生成文档
docs:
	@echo "==================================="
	@echo "Generating Documentation"
	@echo "==================================="
	@if command -v doxygen >/dev/null 2>&1; then \
		echo "Running Doxygen..."; \
		doxygen Doxyfile; \
		echo ""; \
		echo "===================================";\
		echo "Documentation generated!";\
		echo "Open: docs/html/index.html";\
		echo "===================================";\
	else \
		echo "Error: doxygen not found!"; \
		echo "Please install doxygen first."; \
		echo ""; \
		echo "Installation:"; \
		echo "  macOS:   brew install doxygen graphviz"; \
		echo "  Ubuntu:  sudo apt-get install doxygen graphviz"; \
		echo "===================================";\
		exit 1; \
	fi

# 代码覆盖率
coverage:
	@echo "==================================="
	@echo "Building with Code Coverage"
	@echo "==================================="
	@cmake -B build $(CMAKE_GENERATOR) -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON
	@cmake --build build
	@echo ""
	@echo "Running tests with coverage..."
	@cd build && ctest --output-on-failure
	@echo ""
	@echo "==================================="
	@echo "Coverage build completed!"
	@echo "Run 'make coverage-report' to generate HTML report"
	@echo "==================================="

# 生成覆盖率报告
coverage-report:
	@echo "==================================="
	@echo "Generating Coverage Report"
	@echo "==================================="
	@if command -v lcov >/dev/null 2>&1; then \
		echo "Capturing coverage data..."; \
		lcov --capture --directory build/CMakeFiles/czc.dir --output-file build/coverage.info --ignore-errors inconsistent,unsupported; \
		echo "Filtering out system/external headers..."; \
		lcov --remove build/coverage.info '/usr/*' '/Library/*' '*/_deps/*' --output-file build/coverage_filtered.info --ignore-errors inconsistent,unsupported,empty; \
		echo "Generating HTML report..."; \
		genhtml build/coverage_filtered.info --output-directory build/coverage_html --ignore-errors inconsistent,unsupported,empty,category; \
		echo ""; \
		echo "===================================";\
		echo "Coverage report generated!";\
		echo "Open: build/coverage_html/index.html";\
		echo "===================================";\
	else \
		echo "Error: lcov not found!"; \
		echo "Please install lcov first."; \
		echo ""; \
		echo "Installation:"; \
		echo "  macOS:   brew install lcov"; \
		echo "  Ubuntu:  sudo apt-get install lcov"; \
		echo "===================================";\
		exit 1; \
	fi
