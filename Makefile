.PHONY: all build release debug clean test install help fmt tidy pullall pullerrmsg coverage coverage-report benchmark docs runbeforecommit

# ANSI 颜色代码定义
COLOR_RESET   := \033[0m
COLOR_BOLD    := \033[1m
COLOR_RED     := \033[31m
COLOR_GREEN   := \033[32m
COLOR_YELLOW  := \033[33m
COLOR_BLUE    := \033[34m
COLOR_CYAN    := \033[36m

# 时间戳打印助手
define ts_msg
	@DATE_STR=$$(date '+%Y-%m-%d %H:%M:%S'); \
	printf "\n$(COLOR_CYAN)╭─────────────────────────────────────────╮\n$(COLOR_RESET)"; \
	printf "$(COLOR_CYAN)│$(COLOR_RESET) $(COLOR_BOLD)[%s]$(COLOR_RESET)\n" "$$DATE_STR"; \
	printf "$(COLOR_CYAN)│$(COLOR_RESET) $(COLOR_CYAN)▶ %s$(COLOR_RESET)\n" "$(1)"; \
	printf "$(COLOR_CYAN)╰─────────────────────────────────────────╯\n$(COLOR_RESET)"
endef

define ts_done
	@DATE_STR=$$(date '+%Y-%m-%d %H:%M:%S'); \
	printf "$(COLOR_GREEN)╭─────────────────────────────────────────╮\n$(COLOR_RESET)"; \
	printf "$(COLOR_GREEN)│$(COLOR_RESET) $(COLOR_BOLD)[%s]$(COLOR_RESET)\n" "$$DATE_STR"; \
	printf "$(COLOR_GREEN)│$(COLOR_RESET) $(COLOR_GREEN)✓ %s$(COLOR_RESET)\n" "$(1)"; \
	printf "$(COLOR_GREEN)╰─────────────────────────────────────────╯\n$(COLOR_RESET)"
endef

# 检测 CPU 核心数
ifeq ($(OS),Windows_NT)
    CMAKE_GENERATOR := -G "MinGW Makefiles"
    RM := cmd /c del /f /q
    RMDIR := cmd /c rmdir /s /q
    PATH_SEP := \\
    EXE_EXT := .exe
    NPROC := $(NUMBER_OF_PROCESSORS)
else
    CMAKE_GENERATOR :=
    RM := rm -f
    RMDIR := rm -rf
    PATH_SEP := /
    EXE_EXT :=
    NPROC := $(shell command -v nproc > /dev/null && nproc || sysctl -n hw.ncpu)
endif

# 默认目标
all: release

build: release

# Release 模式构建
release:
	$(call ts_msg,Building CZC Compiler (Release Mode))
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)╔═══════════════════════════════════════╗\n$(COLOR_RESET)"
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)║  CZC COMPILER - RELEASE BUILD        ║\n$(COLOR_RESET)"
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)╚═══════════════════════════════════════╝\n$(COLOR_RESET)"
	@printf "$(COLOR_CYAN)Configuration: $(COLOR_BOLD)Release (Optimized)$(COLOR_RESET)\n"
	@printf "$(COLOR_CYAN)CPU Cores: $(COLOR_BOLD)$(NPROC)$(COLOR_RESET)\n"
	@echo ""
	@cmake -B build $(CMAKE_GENERATOR) -DCMAKE_BUILD_TYPE=Release
	@cmake --build build --parallel $(NPROC)
	@echo ""
	@printf "$(COLOR_GREEN)$(COLOR_BOLD)╔═══════════════════════════════════════╗\n$(COLOR_RESET)"
	@printf "$(COLOR_GREEN)$(COLOR_BOLD)║  ✓ BUILD SUCCESSFUL                   ║\n$(COLOR_RESET)"
	@printf "$(COLOR_GREEN)$(COLOR_BOLD)╚═══════════════════════════════════════╝\n$(COLOR_RESET)"
	@echo ""
	@printf "$(COLOR_CYAN)$(COLOR_BOLD)Generated Executables:\n$(COLOR_RESET)"
	@printf "  $(COLOR_GREEN)▸$(COLOR_RESET) CLI Tool:         $(COLOR_BOLD).$(PATH_SEP)build$(PATH_SEP)czc-cli$(EXE_EXT)$(COLOR_RESET)\n"
	@printf "  $(COLOR_GREEN)▸$(COLOR_RESET) Test Suite:       $(COLOR_BOLD).$(PATH_SEP)build$(PATH_SEP)tests$(PATH_SEP)test_lexer$(EXE_EXT)$(COLOR_RESET)\n"
	@printf "  $(COLOR_GREEN)▸$(COLOR_RESET) Debug Operators:  $(COLOR_BOLD).$(PATH_SEP)build$(PATH_SEP)tests$(PATH_SEP)debug_operators$(EXE_EXT)$(COLOR_RESET)\n"
	$(call ts_done,Release Build Complete)
	@echo ""

# Debug 模式构建
debug:
	$(call ts_msg,Building CZC Compiler (Debug Mode))
	@printf "$(COLOR_YELLOW)$(COLOR_BOLD)╔═══════════════════════════════════════╗\n$(COLOR_RESET)"
	@printf "$(COLOR_YELLOW)$(COLOR_BOLD)║  CZC COMPILER - DEBUG BUILD          ║\n$(COLOR_RESET)"
	@printf "$(COLOR_YELLOW)$(COLOR_BOLD)╚═══════════════════════════════════════╝\n$(COLOR_RESET)"
	@printf "$(COLOR_CYAN)Configuration: $(COLOR_BOLD)Debug + Coverage$(COLOR_RESET)\n"
	@printf "$(COLOR_CYAN)CPU Cores: $(COLOR_BOLD)$(NPROC)$(COLOR_RESET)\n"
	@echo ""
	@cmake -B build $(CMAKE_GENERATOR) -DCMAKE_BUILD_TYPE=Debug
	@cmake --build build --parallel $(NPROC)
	@echo ""
	@printf "$(COLOR_GREEN)$(COLOR_BOLD)╔═══════════════════════════════════════╗\n$(COLOR_RESET)"
	@printf "$(COLOR_GREEN)$(COLOR_BOLD)║  ✓ DEBUG BUILD SUCCESSFUL             ║\n$(COLOR_RESET)"
	@printf "$(COLOR_GREEN)$(COLOR_BOLD)╚═══════════════════════════════════════╝\n$(COLOR_RESET)"
	$(call ts_done,Debug Build Complete)
	@echo ""

# 清理构建产物
clean:
	$(call ts_msg,Starting target 'clean')
	@printf "$(COLOR_CYAN)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"
	@printf "$(COLOR_CYAN)$(COLOR_BOLD)Cleaning CZC Project\n$(COLOR_RESET)"
	@printf "$(COLOR_CYAN)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"
	@cmake -E rm -rf build
	@printf "$(COLOR_GREEN)Build directory removed\n$(COLOR_RESET)"
	@cmake -E rm -f examples/*.tokens
	@printf "$(COLOR_GREEN)Build directory and .tokens files removed\n$(COLOR_RESET)"
	@cmake -E rm -rf examples/*.formatted
	@printf "$(COLOR_GREEN)Formatted example files removed\n$(COLOR_RESET)"
	@cmake -E rm -rf docs/html
	@printf "$(COLOR_GREEN)Auto generated documentation removed\n$(COLOR_RESET)"
	@echo ""
	@printf "$(COLOR_GREEN)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"
	@printf "$(COLOR_GREEN)$(COLOR_BOLD)Clean completed!\n$(COLOR_RESET)"
	@printf "$(COLOR_GREEN)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"
	$(call ts_done,Finished target 'clean')

# 运行测试
test: build
	$(call ts_msg,Starting target 'test')
	@printf "$(COLOR_CYAN)Running tests...\n$(COLOR_RESET)"
	@cd build && ctest --output-on-failure --parallel $(NPROC)
	$(call ts_done,Finished target 'test')

# 安装
install: build
	$(call ts_msg,Starting target 'install')
	@printf "$(COLOR_CYAN)Installing CZC...\n$(COLOR_RESET)"
	@cd build && cmake --install . --prefix /usr/local
	$(call ts_done,Finished target 'install')

# 重新构建
rebuild: clean build

# 运行示例
example: build
	$(call ts_msg,Starting target 'example')
	@printf "$(COLOR_CYAN)Tokenizing hello.zero...\n$(COLOR_RESET)"
	@./build/czc-cli tokenize examples/hello.zero
	@echo ""
	@printf "$(COLOR_GREEN)Output saved to examples/hello.zero.tokens\n$(COLOR_RESET)"
	$(call ts_done,Finished target 'example')

# 格式化代码
fmt:
	$(call ts_msg,Starting target 'fmt')
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)Formatting C/C++ Source Files\n$(COLOR_RESET)"
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"
	@if command -v clang-format >/dev/null 2>&1; then \
		printf "$(COLOR_CYAN)Formatting header files...\n$(COLOR_RESET)"; \
		find czc -type f \( -name "*.hpp" -o -name "*.h" \) -exec clang-format -i {} + ; \
		printf "$(COLOR_CYAN)Formatting source files...\n$(COLOR_RESET)"; \
		find src -type f \( -name "*.cpp" -o -name "*.c" \) -exec clang-format -i {} + ; \
		find cli -type f \( -name "*.cpp" -o -name "*.c" \) -exec clang-format -i {} + ; \
		find tests -type f \( -name "*.cpp" -o -name "*.c" \) -exec clang-format -i {} + ; \
		echo ""; \
		printf "$(COLOR_GREEN)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"; \
		printf "$(COLOR_GREEN)$(COLOR_BOLD)Formatting completed!\n$(COLOR_RESET)"; \
		printf "$(COLOR_GREEN)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"; \
	else \
		printf "$(COLOR_RED)$(COLOR_BOLD)Error: clang-format not found!\n$(COLOR_RESET)"; \
		printf "$(COLOR_YELLOW)Please install clang-format first.\n$(COLOR_RESET)"; \
		echo ""; \
		echo "Installation:"; \
		echo "  macOS:   brew install clang-format"; \
		echo "  Ubuntu:  sudo apt-get install clang-format"; \
		echo "  Fedora:  sudo dnf install clang-tools-extra"; \
		printf "$(COLOR_BLUE)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"; \
		exit 1; \
	fi
	$(call ts_done,Finished target 'fmt')

# 整理代码
tidy:
	$(call ts_msg,Starting target 'tidy')
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)Tidying C/C++ Source Files\n$(COLOR_RESET)"
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"
	@if command -v clang-tidy >/dev/null 2>&1; then \
		printf "$(COLOR_CYAN)Running clang-tidy...\n$(COLOR_RESET)"; \
		find src cli tests -type f \( -name "*.cpp" -o -name "*.c" \) -exec clang-tidy -p build {} + ; \
		echo ""; \
		printf "$(COLOR_GREEN)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"; \
		printf "$(COLOR_GREEN)$(COLOR_BOLD)Tidying completed!\n$(COLOR_RESET)"; \
		printf "$(COLOR_GREEN)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"; \
	else \
		printf "$(COLOR_RED)$(COLOR_BOLD)Error: clang-tidy not found!\n$(COLOR_RESET)"; \
		printf "$(COLOR_YELLOW)Please install clang-tidy first.\n$(COLOR_RESET)"; \
		echo ""; \
		echo "Installation:"; \
		echo "  macOS:   brew install llvm"; \
		echo "  Ubuntu:  sudo apt-get install clang-tidy"; \
		echo "  Fedora:  sudo dnf install clang-tools-extra"; \
		printf "$(COLOR_BLUE)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"; \
		exit 1; \
	fi
	$(call ts_done,Finished target 'tidy')

# 拉取所有子模块
pullall:
	$(call ts_msg,Starting target 'pullall')
	@printf "$(COLOR_CYAN)Pulling all submodules...\n$(COLOR_RESET)"
	@git submodule update --init --recursive
	@printf "$(COLOR_GREEN)All submodules pulled successfully!\n$(COLOR_RESET)"
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"
	$(call ts_done,Finished target 'pullall')

# 拉取错误信息
pullerrmsg:
	$(call ts_msg,Starting target 'pullerrmsg')
	@printf "$(COLOR_CYAN)Pulling error messages...\n$(COLOR_RESET)"
	@cd error-msg
	@git pull origin main
	@cd ..
	@printf "$(COLOR_GREEN)Error messages pulled successfully!\n$(COLOR_RESET)"
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"
	$(call ts_done,Finished target 'pullerrmsg')

# 帮助信息
help:
	$(call ts_msg,Starting target 'help')
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)CZ Compiler - Makefile Commands\n$(COLOR_RESET)"
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"
	@echo ""
	@echo "Available targets:"
	@echo "  make (all)           - Build the project in Release mode (default)"
	@echo "  make build           - Build the project in Release mode (alias)"
	@echo "  make release         - Build the project in Release mode"
	@echo "  make debug           - Build the project in Debug mode"
	@echo "  make clean           - Clean all build artifacts"
	@echo "  make test            - Build and run tests (parallel)"
	@echo "  make benchmark       - Build and run performance benchmarks"
	@echo "  make coverage        - Build with coverage and run tests"
	@echo "  make coverage-report - Generate HTML coverage report with percentage"
	@echo "  make runbeforecommit - Full quality check: clean, build, test (100%), coverage (≥80%), format"
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
	@echo "Note: All builds use $(NPROC) CPU cores for parallel compilation"
	@echo ""
	$(call ts_done,Finished target 'help')

# 性能基准测试
benchmark:
	$(call ts_msg,Starting target 'benchmark')
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)Building and Running Benchmarks\n$(COLOR_RESET)"
	@printf "$(COLOR_CYAN)Using $(NPROC) CPU cores\n$(COLOR_RESET)"
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"
	@cmake -B build $(CMAKE_GENERATOR) -DCMAKE_BUILD_TYPE=Release -DBUILD_BENCHMARKS=ON
	@cmake --build build --parallel $(NPROC)
	@echo ""
	@printf "$(COLOR_CYAN)Running lexer benchmarks...\n$(COLOR_RESET)"
	@./build/benchmarks/benchmark_lexer
	@echo ""
	@printf "$(COLOR_CYAN)Running parser benchmarks...\n$(COLOR_RESET)"
	@./build/benchmarks/benchmark_parser
	@echo ""
	@printf "$(COLOR_GREEN)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"
	@printf "$(COLOR_GREEN)$(COLOR_BOLD)Benchmark completed!\n$(COLOR_RESET)"
	@printf "$(COLOR_GREEN)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"
	$(call ts_done,Finished target 'benchmark')

# 生成文档
docs:
	$(call ts_msg,Starting target 'docs')
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)Generating Documentation\n$(COLOR_RESET)"
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"
	@if command -v doxygen >/dev/null 2>&1; then \
		printf "$(COLOR_CYAN)Running Doxygen...\n$(COLOR_RESET)"; \
		doxygen Doxyfile; \
		echo ""; \
		printf "$(COLOR_GREEN)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"; \
		printf "$(COLOR_GREEN)$(COLOR_BOLD)Documentation generated!\n$(COLOR_RESET)"; \
		printf "$(COLOR_CYAN)Open: docs/html/index.html\n$(COLOR_RESET)"; \
		printf "$(COLOR_GREEN)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"; \
	else \
		printf "$(COLOR_RED)$(COLOR_BOLD)Error: doxygen not found!\n$(COLOR_RESET)"; \
		printf "$(COLOR_YELLOW)Please install doxygen first.\n$(COLOR_RESET)"; \
		echo ""; \
		echo "Installation:"; \
		echo "  macOS:   brew install doxygen graphviz"; \
		echo "  Ubuntu:  sudo apt-get install doxygen graphviz"; \
		printf "$(COLOR_BLUE)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"; \
		exit 1; \
	fi
	$(call ts_done,Finished target 'docs')

# 代码覆盖率
coverage:
	$(call ts_msg,Starting target 'coverage')
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)Building with Code Coverage\n$(COLOR_RESET)"
	@printf "$(COLOR_CYAN)Using $(NPROC) CPU cores\n$(COLOR_RESET)"
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"
	@cmake -B build $(CMAKE_GENERATOR) -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON
	@cmake --build build --parallel $(NPROC)
	@echo ""
	@printf "$(COLOR_CYAN)Running tests with coverage...\n$(COLOR_RESET)"
	@cd build && ctest --output-on-failure --parallel $(NPROC)
	@echo ""
	@printf "$(COLOR_GREEN)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"
	@printf "$(COLOR_GREEN)$(COLOR_BOLD)Coverage build completed!\n$(COLOR_RESET)"
	@printf "$(COLOR_YELLOW)Run 'make coverage-report' to generate HTML report\n$(COLOR_RESET)"
	@printf "$(COLOR_GREEN)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"
	$(call ts_done,Finished target 'coverage')

# 生成覆盖率报告
coverage-report:
	$(call ts_msg,Starting target 'coverage-report')
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)Generating Coverage Report\n$(COLOR_RESET)"
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"
	@if command -v lcov >/dev/null 2>&1; then \
		printf "$(COLOR_CYAN)Capturing coverage data...\n$(COLOR_RESET)"; \
		lcov --capture --directory build/CMakeFiles/czc.dir --output-file build/coverage.info --ignore-errors inconsistent,unsupported 2>/dev/null; \
		printf "$(COLOR_CYAN)Filtering out system/external headers...\n$(COLOR_RESET)"; \
		lcov --remove build/coverage.info '/usr/*' '/Library/*' '*/_deps/*' --output-file build/coverage_temp.info --ignore-errors inconsistent,unsupported,empty 2>/dev/null; \
		printf "$(COLOR_CYAN)Filtering out virtual destructors...\n$(COLOR_RESET)"; \
		python3 filter_coverage.py build/coverage_temp.info build/coverage_filtered.info; \
		printf "$(COLOR_CYAN)Generating HTML report...\n$(COLOR_RESET)"; \
		genhtml build/coverage_filtered.info --output-directory build/coverage_html --ignore-errors inconsistent,unsupported,empty,category 2>/dev/null; \
		echo ""; \
		printf "$(COLOR_CYAN)Extracting coverage percentages...\n$(COLOR_RESET)"; \
		SUMMARY=$$(lcov --summary build/coverage_filtered.info --ignore-errors inconsistent,corrupt,count 2>&1); \
		LINE_COV=$$(echo "$$SUMMARY" | grep "lines" | grep -oE '[0-9]+\.[0-9]+%' | head -1 | sed 's/%//'); \
		FUNC_COV=$$(echo "$$SUMMARY" | grep "functions" | grep -oE '[0-9]+\.[0-9]+%' | head -1 | sed 's/%//'); \
		printf "$(COLOR_GREEN)$(COLOR_BOLD)Line coverage:     $$LINE_COV%%\n$(COLOR_RESET)"; \
		printf "$(COLOR_GREEN)$(COLOR_BOLD)Function coverage: $$FUNC_COV%%\n$(COLOR_RESET)"; \
		echo "$$LINE_COV" > build/coverage_line.txt; \
		echo "$$FUNC_COV" > build/coverage_func.txt; \
		echo ""; \
		printf "$(COLOR_GREEN)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"; \
		printf "$(COLOR_GREEN)$(COLOR_BOLD)Coverage report generated!\n$(COLOR_RESET)"; \
		printf "$(COLOR_CYAN)Open: build/coverage_html/index.html\n$(COLOR_RESET)"; \
		printf "$(COLOR_GREEN)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"; \
	else \
		printf "$(COLOR_RED)$(COLOR_BOLD)Error: lcov not found!\n$(COLOR_RESET)"; \
		printf "$(COLOR_YELLOW)Please install lcov first.\n$(COLOR_RESET)"; \
		echo ""; \
		echo "Installation:"; \
		echo "  macOS:   brew install lcov"; \
		echo "  Ubuntu:  sudo apt-get install lcov"; \
		printf "$(COLOR_BLUE)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"; \
		exit 1; \
	fi
	$(call ts_done,Finished target 'coverage-report')

# 提交前检查 - 确保代码质量
runbeforecommit:
	$(call ts_msg,Starting target 'runbeforecommit')
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)========================================\\n$(COLOR_RESET)"
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)Pre-Commit Quality Check\\n$(COLOR_RESET)"
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)========================================\\n$(COLOR_RESET)"
	@echo ""
	@printf "$(COLOR_CYAN)Step 1/5: Cleaning previous build...\\n$(COLOR_RESET)"
	@$(MAKE) clean
	@echo ""
	@printf "$(COLOR_CYAN)Step 2/5: Building project (Debug with coverage)...\\n$(COLOR_RESET)"
	@cmake -B build $(CMAKE_GENERATOR) -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON
	@cmake --build build --parallel $(NPROC)
	@echo ""
	@printf "$(COLOR_CYAN)Step 3/5: Running all tests...\\n$(COLOR_RESET)"
	@cd build && ctest --output-on-failure --parallel $(NPROC) || (printf "$(COLOR_RED)$(COLOR_BOLD)[FAIL]$(COLOR_RESET) Tests failed! Fix errors before committing.\\n" && exit 1)
	@echo ""
	@printf "$(COLOR_GREEN)$(COLOR_BOLD)[PASS]$(COLOR_RESET) All tests passed!\\n"
	@echo ""
	@printf "$(COLOR_CYAN)Step 4/5: Generating coverage report...\\n$(COLOR_RESET)"
	@if command -v lcov >/dev/null 2>&1; then \
		lcov --capture --directory build/CMakeFiles/czc.dir --output-file build/coverage.info --ignore-errors inconsistent,unsupported 2>/dev/null; \
		lcov --remove build/coverage.info '/usr/*' '/Library/*' '*/_deps/*' --output-file build/coverage_temp.info --ignore-errors inconsistent,unsupported,empty 2>/dev/null; \
		python3 ./filter_coverage.py build/coverage_temp.info build/coverage_filtered.info; \
		genhtml build/coverage_filtered.info --output-directory build/coverage_html --ignore-errors inconsistent,unsupported,empty,category 2>/dev/null; \
		SUMMARY=$$(lcov --summary build/coverage_filtered.info --ignore-errors inconsistent,corrupt,count 2>&1); \
		LINE_COV=$$(echo "$$SUMMARY" | grep "lines" | grep -oE '[0-9]+\.[0-9]+%' | head -1 | sed 's/%//'); \
		FUNC_COV=$$(echo "$$SUMMARY" | grep "functions" | grep -oE '[0-9]+\.[0-9]+%' | head -1 | sed 's/%//'); \
		if [ -z "$$LINE_COV" ]; then \
			printf "$(COLOR_YELLOW)[WARN]$(COLOR_RESET) Could not extract line coverage\\n"; \
			LINE_COV="0"; \
		fi; \
		if [ -z "$$FUNC_COV" ]; then \
			printf "$(COLOR_YELLOW)[WARN]$(COLOR_RESET) Could not extract function coverage\\n"; \
			FUNC_COV="0"; \
		fi; \
		printf "$(COLOR_CYAN)Line coverage:     $(COLOR_BOLD)$$LINE_COV%%$(COLOR_RESET)\\n"; \
		printf "$(COLOR_CYAN)Function coverage: $(COLOR_BOLD)$$FUNC_COV%%$(COLOR_RESET)\\n"; \
		printf "$(COLOR_CYAN)Required coverage: $(COLOR_BOLD)80%%$(COLOR_RESET) (both metrics)\\n"; \
		echo ""; \
		LINE_FAIL=0; \
		FUNC_FAIL=0; \
		if [ $$(awk "BEGIN {print ($$LINE_COV < 80)}") -eq 1 ]; then \
			LINE_FAIL=1; \
		fi; \
		if [ $$(awk "BEGIN {print ($$FUNC_COV < 80)}") -eq 1 ]; then \
			FUNC_FAIL=1; \
		fi; \
		if [ $$LINE_FAIL -eq 1 ] || [ $$FUNC_FAIL -eq 1 ]; then \
			printf "$(COLOR_RED)$(COLOR_BOLD)[FAIL]$(COLOR_RESET) Coverage is below 80%% threshold!\\n"; \
			if [ $$LINE_FAIL -eq 1 ]; then \
				printf "       Line coverage: $(COLOR_RED)$$LINE_COV%%$(COLOR_RESET) (need >= 80%%)\\n"; \
			fi; \
			if [ $$FUNC_FAIL -eq 1 ]; then \
				printf "       Function coverage: $(COLOR_RED)$$FUNC_COV%%$(COLOR_RESET) (need >= 80%%)\\n"; \
			fi; \
			printf "$(COLOR_YELLOW)Please add more tests before committing.$(COLOR_RESET)\\n"; \
			exit 1; \
		else \
			printf "$(COLOR_GREEN)$(COLOR_BOLD)[PASS]$(COLOR_RESET) Coverage check passed!\\n"; \
			printf "       Line coverage: $(COLOR_GREEN)$$LINE_COV%%$(COLOR_RESET)\\n"; \
			printf "       Function coverage: $(COLOR_GREEN)$$FUNC_COV%%$(COLOR_RESET)\\n"; \
		fi; \
	else \
		printf "$(COLOR_YELLOW)[WARN]$(COLOR_RESET) lcov not found, skipping coverage check\\n"; \
		printf "$(COLOR_YELLOW)Please install lcov to enable coverage verification:\\n$(COLOR_RESET)"; \
		echo "  macOS:   brew install lcov"; \
		echo "  Ubuntu:  sudo apt-get install lcov"; \
	fi
	@echo ""
	@printf "$(COLOR_CYAN)Step 5/5: Running code formatter...\\n$(COLOR_RESET)"
	@$(MAKE) fmt
	@echo ""
	@printf "$(COLOR_GREEN)$(COLOR_BOLD)========================================\\n$(COLOR_RESET)"
	@printf "$(COLOR_GREEN)$(COLOR_BOLD)Pre-Commit Check PASSED!\\n$(COLOR_RESET)"
	@printf "$(COLOR_GREEN)$(COLOR_BOLD)========================================\\n$(COLOR_RESET)"
	@echo ""
	@printf "$(COLOR_BOLD)Summary:\n$(COLOR_RESET)"
	@printf "  $(COLOR_GREEN)[PASS]$(COLOR_RESET) Build successful\n"
	@printf "  $(COLOR_GREEN)[PASS]$(COLOR_RESET) All tests passed (100%%)\n"
	@if command -v lcov >/dev/null 2>&1; then \
		if [ -f build/coverage_filtered.info ]; then \
			SUMMARY=$$(lcov --summary build/coverage_filtered.info --ignore-errors inconsistent,corrupt,count 2>&1); \
			LINE_COV=$$(echo "$$SUMMARY" | grep "lines" | grep -oE '[0-9]+\.[0-9]+%' | head -1); \
			FUNC_COV=$$(echo "$$SUMMARY" | grep "functions" | grep -oE '[0-9]+\.[0-9]+%' | head -1); \
			if [ ! -z "$$LINE_COV" ] && [ ! -z "$$FUNC_COV" ]; then \
				echo "  $(COLOR_GREEN)[PASS]$(COLOR_RESET) Coverage: Lines $$LINE_COV | Functions $$FUNC_COV (>= 80%)"; \
			fi; \
		fi; \
	fi
	@printf "  $(COLOR_GREEN)[PASS]$(COLOR_RESET) Code formatted\n"
	@echo ""
	@printf "$(COLOR_GREEN)$(COLOR_BOLD)You are ready to commit!$(COLOR_RESET)\n"
	@echo ""
	$(call ts_done,Finished target 'runbeforecommit')
