# ============================================================================
# C++20 Project Makefile Template
# ============================================================================
# Compiler:    Clang
# Build:       CMake
# Package Mgr: vcpkg
# Testing:     Google Test
# Docs:        Doxygen
# Format:      clang-format
# Linting:     clang-tidy
# ============================================================================

.PHONY: all build release debug clean test install help fmt tidy docs \
        coverage coverage-report benchmark rebuild runbeforecommit \
        vcpkg-install analyze analyze-clang-tidy analyze-cppcheck analyze-full \
        check-deps run stats info

# ============================================================================
# ANSI Color Codes
# ============================================================================
COLOR_RESET   := \033[0m
COLOR_BOLD    := \033[1m
COLOR_RED     := \033[31m
COLOR_GREEN   := \033[32m
COLOR_YELLOW  := \033[33m
COLOR_BLUE    := \033[34m
COLOR_CYAN    := \033[36m

# ============================================================================
# Project Configuration (Customize these for your project)
# ============================================================================
PROJECT_NAME     := czc
PROJECT_VERSION  := 0.0.1
BUILD_DIR        := build
SRC_DIRS         := src
INCLUDE_DIRS     := include
TEST_DIRS        := tests
BENCHMARK_DIRS   := benchmarks
DOCS_DIR         := docs

# Executable names
MAIN_EXECUTABLE  := $(PROJECT_NAME)
TEST_EXECUTABLE  := test_$(PROJECT_NAME)

# Full paths to executables (relative to BUILD_DIR)
MAIN_EXECUTABLE_PATH  := $(BUILD_DIR)/$(MAIN_EXECUTABLE)
TEST_EXECUTABLE_PATH  := $(BUILD_DIR)/tests/$(TEST_EXECUTABLE)

# ============================================================================
# vcpkg Configuration
# ============================================================================
# Set VCPKG_ROOT environment variable or modify this path
VCPKG_ROOT       ?= $(HOME)/vcpkg
VCPKG_TOOLCHAIN  := $(VCPKG_ROOT)/scripts/buildsystems/vcpkg.cmake

# ============================================================================
# Compiler Configuration
# ============================================================================
CC               := clang
CXX              := clang++
CMAKE            := cmake
CTEST            := ctest

# C++ Standard
CXX_STANDARD     := 20

# Coverage threshold (percentage)
COVERAGE_THRESHOLD := 80

# Parallel build jobs (0 = auto-detect CPU cores)
PARALLEL_JOBS    ?= 0

# Optimization level for Release builds: O0, O1, O2, O3, Os, Oz, Ofast
OPTIMIZATION     ?= O3

# Enable Link-Time Optimization (LTO) for Release builds: ON or OFF
ENABLE_LTO       ?= OFF

# Enable Native CPU optimizations (march=native): ON or OFF
NATIVE_ARCH      ?= OFF

# ============================================================================
# Timestamp Message Helpers
# ============================================================================
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

# ============================================================================
# Box Drawing Helpers (Fixed Width: 60 chars inner content)
# ============================================================================
BOX_WIDTH := 60

# Print box top with title: $(call box_top,Title)
define box_top
	@printf "$(COLOR_CYAN)┌─ %s $(COLOR_CYAN)" "$(1)"; \
	TITLE_LEN=$$(printf "%s" "$(1)" | wc -c | tr -d ' '); \
	PADDING=$$(($(BOX_WIDTH) - TITLE_LEN - 1)); \
	printf "%*s" "$$PADDING" "" | tr ' ' '─'; \
	printf "┐\n$(COLOR_RESET)"
endef

# Print box bottom
define box_bottom
	@printf "$(COLOR_CYAN)└"; \
	printf "%*s" "$$(($(BOX_WIDTH) + 2))" "" | tr ' ' '─'; \
	printf "┘\n$(COLOR_RESET)"
endef

# Print box row with label and value: $(call box_row,Label,Value)
define box_row
	@printf "$(COLOR_CYAN)│$(COLOR_RESET)  %-14s $(COLOR_BOLD)%-43s$(COLOR_RESET) $(COLOR_CYAN)│\n$(COLOR_RESET)" "$(1)" "$(2)"
endef

# Print box row with status indicator: $(call box_row_status,Label,Status,Value)
# Status: ok, warn, err, info
define box_row_status
	@case "$(2)" in \
		ok)   STATUS="$(COLOR_GREEN)[OK]$(COLOR_RESET)" ;; \
		warn) STATUS="$(COLOR_YELLOW)[--]$(COLOR_RESET)" ;; \
		err)  STATUS="$(COLOR_RED)[!!]$(COLOR_RESET)" ;; \
		info) STATUS="$(COLOR_YELLOW)[!]$(COLOR_RESET)" ;; \
		*)    STATUS="    " ;; \
	esac; \
	printf "$(COLOR_CYAN)│$(COLOR_RESET)  %-14s $$STATUS $(COLOR_BOLD)%-38s$(COLOR_RESET) $(COLOR_CYAN)│\n$(COLOR_RESET)" "$(1)" "$(3)"
endef

# ============================================================================
# Platform Detection (macOS / Linux only)
# ============================================================================
CMAKE_GENERATOR :=
RM              := rm -f
RMDIR           := rm -rf
PATH_SEP        := /
EXE_EXT         :=
CPU_CORES       := $(shell command -v nproc > /dev/null 2>&1 && nproc || sysctl -n hw.ncpu 2>/dev/null || echo 4)
# Use PARALLEL_JOBS if set, otherwise use all CPU cores
ifeq ($(PARALLEL_JOBS),0)
	NPROC := $(CPU_CORES)
else
	NPROC := $(PARALLEL_JOBS)
endif
UNAME_S         := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	OPEN_CMD    := open
	# macOS uses BSD sed which requires backup extension with -i
	SED_INPLACE := sed -i ''
else
	OPEN_CMD    := xdg-open
	# GNU sed doesn't require backup extension
	SED_INPLACE := sed -i
endif

# ============================================================================
# CMake Common Options
# ============================================================================
CMAKE_COMMON_OPTS := \
	-DCMAKE_C_COMPILER=$(CC) \
	-DCMAKE_CXX_COMPILER=$(CXX) \
	-DCMAKE_CXX_STANDARD=$(CXX_STANDARD) \
	-DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Add vcpkg toolchain if available
ifneq ($(wildcard $(VCPKG_TOOLCHAIN)),)
	CMAKE_COMMON_OPTS += -DCMAKE_TOOLCHAIN_FILE=$(VCPKG_TOOLCHAIN)
endif

# ============================================================================
# Optimization Flags
# ============================================================================
# Build optimization flags for Release mode
RELEASE_CXX_FLAGS := -$(OPTIMIZATION)

ifeq ($(NATIVE_ARCH),ON)
	RELEASE_CXX_FLAGS += -march=native
endif

ifeq ($(ENABLE_LTO),ON)
	RELEASE_CXX_FLAGS += -flto
	CMAKE_COMMON_OPTS += -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON
endif

# ============================================================================
# Dependency Check
# ============================================================================
check-deps:
	$(call ts_msg,Checking Dependencies)
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)Checking Required Tools\n$(COLOR_RESET)"
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"
	@MISSING=0; \
	for cmd in cmake clang clang++ ctest; do \
		if command -v $$cmd >/dev/null 2>&1; then \
			VER=$$($$cmd --version 2>&1 | head -1); \
			printf "  $(COLOR_GREEN)[OK]$(COLOR_RESET) $$cmd: $$VER\n"; \
		else \
			printf "  $(COLOR_RED)[MISSING]$(COLOR_RESET) $$cmd\n"; \
			MISSING=1; \
		fi; \
	done; \
	echo ""; \
	printf "$(COLOR_CYAN)Optional Tools:\n$(COLOR_RESET)"; \
	for cmd in clang-format clang-tidy cppcheck doxygen lcov cloc; do \
		if command -v $$cmd >/dev/null 2>&1; then \
			VER=$$($$cmd --version 2>&1 | head -1); \
			printf "  $(COLOR_GREEN)[OK]$(COLOR_RESET) $$cmd: $$VER\n"; \
		else \
			printf "  $(COLOR_YELLOW)[MISSING]$(COLOR_RESET) $$cmd (optional)\n"; \
		fi; \
	done; \
	echo ""; \
	if command -v vcpkg >/dev/null 2>&1; then \
		VCPKG_VER=$$(vcpkg --version 2>&1 | head -1); \
		printf "  $(COLOR_GREEN)[OK]$(COLOR_RESET) vcpkg: $$VCPKG_VER\n"; \
		if [ -d "$(VCPKG_ROOT)" ]; then \
			printf "       VCPKG_ROOT: $(VCPKG_ROOT)\n"; \
		else \
			printf "  $(COLOR_YELLOW)[WARN]$(COLOR_RESET) VCPKG_ROOT not set or invalid ($(VCPKG_ROOT))\n"; \
			printf "       Consider setting: export VCPKG_ROOT=\$$(dirname \$$(dirname \$$(which vcpkg)))\n"; \
		fi; \
	elif [ -d "$(VCPKG_ROOT)" ]; then \
		printf "  $(COLOR_GREEN)[OK]$(COLOR_RESET) vcpkg: $(VCPKG_ROOT)\n"; \
	else \
		printf "  $(COLOR_YELLOW)[MISSING]$(COLOR_RESET) vcpkg (optional)\n"; \
	fi; \
	echo ""; \
	if [ $$MISSING -eq 1 ]; then \
		printf "$(COLOR_RED)$(COLOR_BOLD)Some required tools are missing!$(COLOR_RESET)\n"; \
		printf "$(COLOR_YELLOW)Install missing tools before building.$(COLOR_RESET)\n"; \
		exit 1; \
	else \
		printf "$(COLOR_GREEN)$(COLOR_BOLD)All required dependencies are installed!$(COLOR_RESET)\n"; \
	fi
	$(call ts_done,Dependency Check Complete)

# ============================================================================
# Directory Validation Helper
# ============================================================================
define check_dir
	@if [ ! -d "$(1)" ]; then \
		printf "$(COLOR_YELLOW)[WARN]$(COLOR_RESET) Directory '$(1)' does not exist, skipping...\n"; \
	fi
endef

# ============================================================================
# Default Target
# ============================================================================
all: release

build: release

# ============================================================================
# Release Build
# ============================================================================
release:
	$(call ts_msg,Building $(PROJECT_NAME) (Release Mode))
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)╔═══════════════════════════════════════════════════╗\n$(COLOR_RESET)"
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)║    $(PROJECT_NAME) - RELEASE BUILD                 \n$(COLOR_RESET)"
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)╚═══════════════════════════════════════════════════╝\n$(COLOR_RESET)"
	@printf "$(COLOR_CYAN)Configuration: $(COLOR_BOLD)Release (Optimized)$(COLOR_RESET)\n"
	@printf "$(COLOR_CYAN)C++ Standard:  $(COLOR_BOLD)C++$(CXX_STANDARD)$(COLOR_RESET)\n"
	@printf "$(COLOR_CYAN)Compiler:      $(COLOR_BOLD)$(CXX)$(COLOR_RESET)\n"
	@printf "$(COLOR_CYAN)Optimization:  $(COLOR_BOLD)-$(OPTIMIZATION)$(COLOR_RESET)"
	@if [ "$(ENABLE_LTO)" = "ON" ]; then printf " $(COLOR_BOLD)+LTO$(COLOR_RESET)"; fi
	@if [ "$(NATIVE_ARCH)" = "ON" ]; then printf " $(COLOR_BOLD)+native$(COLOR_RESET)"; fi
	@printf "\n"
	@printf "$(COLOR_CYAN)Parallel Jobs: $(COLOR_BOLD)$(NPROC)$(COLOR_RESET) (of $(CPU_CORES) cores)\n"
	@echo ""
	@$(CMAKE) -B $(BUILD_DIR) $(CMAKE_GENERATOR) $(CMAKE_COMMON_OPTS) \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_CXX_FLAGS_RELEASE="$(RELEASE_CXX_FLAGS)"
	@$(CMAKE) --build $(BUILD_DIR) --parallel $(NPROC)
	@echo ""
	@printf "$(COLOR_GREEN)$(COLOR_BOLD)╔═══════════════════════════════════════════════════╗\n$(COLOR_RESET)"
	@printf "$(COLOR_GREEN)$(COLOR_BOLD)║  ✓ BUILD SUCCESSFUL                               ║\n$(COLOR_RESET)"
	@printf "$(COLOR_GREEN)$(COLOR_BOLD)╚═══════════════════════════════════════════════════╝\n$(COLOR_RESET)"
	@echo ""
	@printf "$(COLOR_CYAN)$(COLOR_BOLD)Generated Executables:\n$(COLOR_RESET)"
	@printf "  $(COLOR_GREEN)▸$(COLOR_RESET) Main:  $(COLOR_BOLD)./$(MAIN_EXECUTABLE_PATH)$(COLOR_RESET)\n"
	@if [ -f "$(TEST_EXECUTABLE_PATH)" ]; then \
		printf "  $(COLOR_GREEN)▸$(COLOR_RESET) Tests: $(COLOR_BOLD)./$(TEST_EXECUTABLE_PATH)$(COLOR_RESET)\n"; \
	fi
	$(call ts_done,Release Build Complete)
	@echo ""

# ============================================================================
# Debug Build
# ============================================================================
debug:
	$(call ts_msg,Building $(PROJECT_NAME) (Debug Mode))
	@printf "$(COLOR_YELLOW)$(COLOR_BOLD)╔═══════════════════════════════════════════════════╗\n$(COLOR_RESET)"
	@printf "$(COLOR_YELLOW)$(COLOR_BOLD)║    $(PROJECT_NAME) - DEBUG BUILD                   \n$(COLOR_RESET)"
	@printf "$(COLOR_YELLOW)$(COLOR_BOLD)╚═══════════════════════════════════════════════════╝\n$(COLOR_RESET)"
	@printf "$(COLOR_CYAN)Configuration: $(COLOR_BOLD)Debug + Symbols$(COLOR_RESET)\n"
	@printf "$(COLOR_CYAN)C++ Standard:  $(COLOR_BOLD)C++$(CXX_STANDARD)$(COLOR_RESET)\n"
	@printf "$(COLOR_CYAN)Compiler:      $(COLOR_BOLD)$(CXX)$(COLOR_RESET)\n"
	@printf "$(COLOR_CYAN)Parallel Jobs: $(COLOR_BOLD)$(NPROC)$(COLOR_RESET) (of $(CPU_CORES) cores)\n"
	@echo ""
	@$(CMAKE) -B $(BUILD_DIR) $(CMAKE_GENERATOR) $(CMAKE_COMMON_OPTS) \
		-DCMAKE_BUILD_TYPE=Debug
	@$(CMAKE) --build $(BUILD_DIR) --parallel $(NPROC)
	@echo ""
	@printf "$(COLOR_GREEN)$(COLOR_BOLD)╔═══════════════════════════════════════════════════╗\n$(COLOR_RESET)"
	@printf "$(COLOR_GREEN)$(COLOR_BOLD)║  ✓ DEBUG BUILD SUCCESSFUL                         ║\n$(COLOR_RESET)"
	@printf "$(COLOR_GREEN)$(COLOR_BOLD)╚═══════════════════════════════════════════════════╝\n$(COLOR_RESET)"
	$(call ts_done,Debug Build Complete)
	@echo ""

# ============================================================================
# Clean Build Artifacts
# ============================================================================
clean:
	$(call ts_msg,Cleaning Build Artifacts)
	@printf "$(COLOR_CYAN)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"
	@printf "$(COLOR_CYAN)$(COLOR_BOLD)Cleaning $(PROJECT_NAME) Project\n$(COLOR_RESET)"
	@printf "$(COLOR_CYAN)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"
	@$(CMAKE) -E rm -rf $(BUILD_DIR)
	@printf "$(COLOR_GREEN)Build directory removed\n$(COLOR_RESET)"
	@$(CMAKE) -E rm -rf $(DOCS_DIR)/html
	@printf "$(COLOR_GREEN)Documentation removed\n$(COLOR_RESET)"
	@$(CMAKE) -E rm -f compile_commands.json
	@printf "$(COLOR_GREEN)Compile commands removed\n$(COLOR_RESET)"
	@echo ""
	@printf "$(COLOR_GREEN)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"
	@printf "$(COLOR_GREEN)$(COLOR_BOLD)Clean completed!\n$(COLOR_RESET)"
	@printf "$(COLOR_GREEN)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"
	$(call ts_done,Clean Complete)

# ============================================================================
# Rebuild (Clean + Build)
# ============================================================================
rebuild: clean build

# ============================================================================
# Run Tests
# ============================================================================
test: build
	$(call ts_msg,Running Tests)
	@printf "$(COLOR_CYAN)Running Google Tests...\n$(COLOR_RESET)"
	@cd $(BUILD_DIR) && $(CTEST) --output-on-failure
	$(call ts_done,Tests Complete)

# ============================================================================
# Install
# ============================================================================
install: build
	$(call ts_msg,Installing $(PROJECT_NAME))
	@printf "$(COLOR_CYAN)Installing to /usr/local...\n$(COLOR_RESET)"
	@cd $(BUILD_DIR) && $(CMAKE) --install . --prefix /usr/local
	$(call ts_done,Installation Complete)

# ============================================================================
# vcpkg Dependency Installation
# ============================================================================
vcpkg-install:
	$(call ts_msg,Installing vcpkg Dependencies)
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)Installing Dependencies via vcpkg\n$(COLOR_RESET)"
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"
	@if [ -f vcpkg.json ]; then \
		printf "$(COLOR_CYAN)Found vcpkg.json manifest...\n$(COLOR_RESET)"; \
		if [ -d "$(VCPKG_ROOT)" ]; then \
			cd $(VCPKG_ROOT) && ./vcpkg install --x-manifest-root=$(CURDIR); \
			printf "$(COLOR_GREEN)Dependencies installed successfully!\n$(COLOR_RESET)"; \
		else \
			printf "$(COLOR_RED)$(COLOR_BOLD)Error: VCPKG_ROOT not found!$(COLOR_RESET)\n"; \
			printf "$(COLOR_YELLOW)Please set VCPKG_ROOT environment variable or install vcpkg:\n$(COLOR_RESET)"; \
			echo "  git clone https://github.com/Microsoft/vcpkg.git"; \
			echo "  cd vcpkg && ./bootstrap-vcpkg.sh"; \
			echo "  export VCPKG_ROOT=\$$PWD"; \
			exit 1; \
		fi; \
	else \
		printf "$(COLOR_YELLOW)No vcpkg.json found. Creating template...\n$(COLOR_RESET)"; \
		VERSION=$$(grep -E 'project\([^\)]*VERSION[[:space:]]+[0-9]+\.[0-9]+\.[0-9]+' CMakeLists.txt | sed -E 's/.*VERSION[[:space:]]+([0-9]+\.[0-9]+\.[0-9]+).*/\1/'); \
		printf "$(COLOR_GREEN)Created vcpkg.json template. Edit and run again.\n$(COLOR_RESET)"; \
	fi
	$(call ts_done,vcpkg Install Complete)

# ============================================================================
# Code Formatting (clang-format)
# ============================================================================
fmt:
	$(call ts_msg,Formatting Source Code)
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)Formatting C/C++ Source Files\n$(COLOR_RESET)"
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"
	@if ! command -v clang-format >/dev/null 2>&1; then \
		printf "$(COLOR_RED)$(COLOR_BOLD)Error: clang-format not found!\n$(COLOR_RESET)"; \
		printf "$(COLOR_YELLOW)Please install clang-format first.\n$(COLOR_RESET)"; \
		echo ""; \
		echo "Installation:"; \
		echo "  macOS:   brew install clang-format"; \
		echo "  Ubuntu:  sudo apt-get install clang-format"; \
		echo "  Fedora:  sudo dnf install clang-tools-extra"; \
		exit 1; \
	fi
	@FORMATTED=0; \
	if [ -d "$(INCLUDE_DIRS)" ]; then \
		printf "$(COLOR_CYAN)Formatting header files in $(INCLUDE_DIRS)...\n$(COLOR_RESET)"; \
		COUNT=$$(find $(INCLUDE_DIRS) -type f \( -name "*.hpp" -o -name "*.h" \) 2>/dev/null | wc -l | tr -d ' '); \
		if [ "$$COUNT" -gt 0 ]; then \
			find $(INCLUDE_DIRS) -type f \( -name "*.hpp" -o -name "*.h" \) -exec clang-format -i {} +; \
			FORMATTED=$$((FORMATTED + COUNT)); \
			printf "  Formatted $$COUNT header file(s)\n"; \
		else \
			printf "  No header files found\n"; \
		fi; \
	else \
		printf "$(COLOR_YELLOW)[SKIP]$(COLOR_RESET) Directory '$(INCLUDE_DIRS)' not found\n"; \
	fi; \
	if [ -d "$(SRC_DIRS)" ]; then \
		printf "$(COLOR_CYAN)Formatting source files in $(SRC_DIRS)...\n$(COLOR_RESET)"; \
		COUNT=$$(find $(SRC_DIRS) -type f \( -name "*.cpp" -o -name "*.cc" -o -name "*.c" \) 2>/dev/null | wc -l | tr -d ' '); \
		if [ "$$COUNT" -gt 0 ]; then \
			find $(SRC_DIRS) -type f \( -name "*.cpp" -o -name "*.cc" -o -name "*.c" \) -exec clang-format -i {} +; \
			FORMATTED=$$((FORMATTED + COUNT)); \
			printf "  Formatted $$COUNT source file(s)\n"; \
		else \
			printf "  No source files found\n"; \
		fi; \
	else \
		printf "$(COLOR_YELLOW)[SKIP]$(COLOR_RESET) Directory '$(SRC_DIRS)' not found\n"; \
	fi; \
	if [ -d "$(TEST_DIRS)" ]; then \
		printf "$(COLOR_CYAN)Formatting test files in $(TEST_DIRS)...\n$(COLOR_RESET)"; \
		COUNT=$$(find $(TEST_DIRS) -type f \( -name "*.cpp" -o -name "*.cc" -o -name "*.c" \) 2>/dev/null | wc -l | tr -d ' '); \
		if [ "$$COUNT" -gt 0 ]; then \
			find $(TEST_DIRS) -type f \( -name "*.cpp" -o -name "*.cc" -o -name "*.c" \) -exec clang-format -i {} +; \
			FORMATTED=$$((FORMATTED + COUNT)); \
			printf "  Formatted $$COUNT test file(s)\n"; \
		else \
			printf "  No test files found\n"; \
		fi; \
	else \
		printf "$(COLOR_YELLOW)[SKIP]$(COLOR_RESET) Directory '$(TEST_DIRS)' not found\n"; \
	fi; \
	echo ""; \
	printf "$(COLOR_GREEN)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"; \
	printf "$(COLOR_GREEN)$(COLOR_BOLD)Formatted $$FORMATTED file(s) total\n$(COLOR_RESET)"; \
	printf "$(COLOR_GREEN)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"
	$(call ts_done,Formatting Complete)

# ============================================================================
# Code Linting (clang-tidy)
# ============================================================================
tidy:
	$(call ts_msg,Running clang-tidy)
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)Running Static Analysis\n$(COLOR_RESET)"
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"
	@if ! command -v clang-tidy >/dev/null 2>&1; then \
		printf "$(COLOR_RED)$(COLOR_BOLD)Error: clang-tidy not found!\n$(COLOR_RESET)"; \
		printf "$(COLOR_YELLOW)Please install clang-tidy first.\n$(COLOR_RESET)"; \
		echo ""; \
		echo "Installation:"; \
		echo "  macOS:   brew install llvm"; \
		echo "  Ubuntu:  sudo apt-get install clang-tidy"; \
		echo "  Fedora:  sudo dnf install clang-tools-extra"; \
		exit 1; \
	fi
	@if [ ! -f $(BUILD_DIR)/compile_commands.json ]; then \
		printf "$(COLOR_YELLOW)compile_commands.json not found.\n$(COLOR_RESET)"; \
		printf "$(COLOR_YELLOW)Run 'make build' first to generate it.\n$(COLOR_RESET)"; \
		exit 1; \
	fi
	@printf "$(COLOR_CYAN)Running clang-tidy...\n$(COLOR_RESET)"; \
	FILES=""; \
	if [ -d "$(SRC_DIRS)" ]; then \
		FILES="$$FILES $$(find $(SRC_DIRS) -type f \( -name '*.cpp' -o -name '*.cc' -o -name '*.c' \) 2>/dev/null)"; \
	fi; \
	if [ -d "$(TEST_DIRS)" ]; then \
		FILES="$$FILES $$(find $(TEST_DIRS) -type f \( -name '*.cpp' -o -name '*.cc' -o -name '*.c' \) 2>/dev/null)"; \
	fi; \
	if [ -n "$$FILES" ]; then \
		echo $$FILES | xargs clang-tidy -p $(BUILD_DIR); \
	else \
		printf "$(COLOR_YELLOW)No source files found to analyze.\n$(COLOR_RESET)"; \
	fi; \
	echo ""; \
	printf "$(COLOR_GREEN)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"; \
	printf "$(COLOR_GREEN)$(COLOR_BOLD)Static analysis completed!\n$(COLOR_RESET)"; \
	printf "$(COLOR_GREEN)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"
	$(call ts_done,Static Analysis Complete)

# ============================================================================
# Static Analysis (clang-tidy only)
# ============================================================================
analyze-clang-tidy: build
	$(call ts_msg,Running clang-tidy Analysis)
	@if ! command -v clang-tidy >/dev/null 2>&1; then \
		printf "$(COLOR_RED)clang-tidy not found!\n$(COLOR_RESET)"; \
		exit 1; \
	fi
	@if [ ! -d "$(SRC_DIRS)" ]; then \
		printf "$(COLOR_RED)Source directory '$(SRC_DIRS)' not found!\n$(COLOR_RESET)"; \
		exit 1; \
	fi
	@mkdir -p $(BUILD_DIR)
	@printf "$(COLOR_CYAN)Running clang-tidy analysis...\n$(COLOR_RESET)"
	@find $(SRC_DIRS) -type f \( -name "*.cpp" -o -name "*.cc" \) \
		-exec clang-tidy -p $(BUILD_DIR) --checks='*,-llvm*,-fuchsia*' {} + 2>&1 | tee $(BUILD_DIR)/clang-tidy-report.txt
	@printf "$(COLOR_GREEN)Report saved to $(BUILD_DIR)/clang-tidy-report.txt\n$(COLOR_RESET)"
	$(call ts_done,clang-tidy Analysis Complete)

# ============================================================================
# Static Analysis (cppcheck)
# ============================================================================
analyze-cppcheck:
	$(call ts_msg,Running cppcheck Analysis)
	@if ! command -v cppcheck >/dev/null 2>&1; then \
		printf "$(COLOR_RED)cppcheck not found!\n$(COLOR_RESET)"; \
		printf "$(COLOR_YELLOW)Install with: brew install cppcheck (macOS) or apt install cppcheck (Ubuntu)\n$(COLOR_RESET)"; \
		exit 1; \
	fi
	@if [ ! -d "$(SRC_DIRS)" ]; then \
		printf "$(COLOR_RED)Source directory '$(SRC_DIRS)' not found!\n$(COLOR_RESET)"; \
		exit 1; \
	fi
	@mkdir -p $(BUILD_DIR)
	@printf "$(COLOR_CYAN)Running cppcheck analysis...\n$(COLOR_RESET)"
	@INCLUDE_FLAG=""; \
	if [ -d "$(INCLUDE_DIRS)" ]; then \
		INCLUDE_FLAG="-I$(INCLUDE_DIRS)"; \
	fi; \
	cppcheck --enable=all --std=c++$(CXX_STANDARD) --suppress=missingIncludeSystem \
		$$INCLUDE_FLAG $(SRC_DIRS) 2>&1 | tee $(BUILD_DIR)/cppcheck-report.txt
	@printf "$(COLOR_GREEN)Report saved to $(BUILD_DIR)/cppcheck-report.txt\n$(COLOR_RESET)"
	$(call ts_done,cppcheck Analysis Complete)

# ============================================================================
# Full Static Analysis
# ============================================================================
analyze-full: build analyze-clang-tidy analyze-cppcheck
	$(call ts_msg,Full Static Analysis Complete)

analyze: analyze-clang-tidy

# ============================================================================
# Documentation (Doxygen)
# ============================================================================
docs:
	$(call ts_msg,Generating Documentation)
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)Generating API Documentation\n$(COLOR_RESET)"
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"
	@if command -v doxygen >/dev/null 2>&1; then \
		if [ -f Doxyfile ]; then \
			printf "$(COLOR_CYAN)Running Doxygen...\n$(COLOR_RESET)"; \
			doxygen Doxyfile; \
			echo ""; \
			printf "$(COLOR_GREEN)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"; \
			printf "$(COLOR_GREEN)$(COLOR_BOLD)Documentation generated!\n$(COLOR_RESET)"; \
			printf "$(COLOR_CYAN)Open: $(DOCS_DIR)/html/index.html\n$(COLOR_RESET)"; \
			printf "$(COLOR_GREEN)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"; \
		else \
			printf "$(COLOR_YELLOW)Doxyfile not found. Creating default...\n$(COLOR_RESET)"; \
			doxygen -g Doxyfile; \
			$(SED_INPLACE) 's/PROJECT_NAME.*=.*/PROJECT_NAME = "$(PROJECT_NAME)"/' Doxyfile; \
			$(SED_INPLACE) 's|OUTPUT_DIRECTORY.*=.*|OUTPUT_DIRECTORY = $(DOCS_DIR)|' Doxyfile; \
			$(SED_INPLACE) 's|INPUT.*=.*|INPUT = $(SRC_DIRS) $(INCLUDE_DIRS)|' Doxyfile; \
			$(SED_INPLACE) 's/RECURSIVE.*=.*/RECURSIVE = YES/' Doxyfile; \
			$(SED_INPLACE) 's/EXTRACT_ALL.*=.*/EXTRACT_ALL = YES/' Doxyfile; \
			$(SED_INPLACE) 's/GENERATE_LATEX.*=.*/GENERATE_LATEX = NO/' Doxyfile; \
			printf "$(COLOR_GREEN)Created Doxyfile. Run 'make docs' again.\n$(COLOR_RESET)"; \
		fi; \
	else \
		printf "$(COLOR_RED)$(COLOR_BOLD)Error: doxygen not found!\n$(COLOR_RESET)"; \
		printf "$(COLOR_YELLOW)Please install doxygen first.\n$(COLOR_RESET)"; \
		echo ""; \
		echo "Installation:"; \
		echo "  macOS:   brew install doxygen graphviz"; \
		echo "  Ubuntu:  sudo apt-get install doxygen graphviz"; \
		exit 1; \
	fi
	$(call ts_done,Documentation Complete)

# ============================================================================
# Code Coverage Build
# ============================================================================
coverage:
	$(call ts_msg,Building with Code Coverage)
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)Building with Code Coverage\n$(COLOR_RESET)"
	@printf "$(COLOR_CYAN)Using $(NPROC) CPU cores\n$(COLOR_RESET)"
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"
	@$(CMAKE) -B $(BUILD_DIR) $(CMAKE_GENERATOR) $(CMAKE_COMMON_OPTS) \
		-DCMAKE_BUILD_TYPE=Debug \
		-DENABLE_COVERAGE=ON
	@$(CMAKE) --build $(BUILD_DIR) --parallel $(NPROC)
	@echo ""
	@printf "$(COLOR_CYAN)Running tests with coverage...\n$(COLOR_RESET)"
	@rm -f $(BUILD_DIR)/*.profraw
	@LLVM_PROFILE_FILE="$(PWD)/$(BUILD_DIR)/lexer_unittest.profraw" $(BUILD_DIR)/lexer_unittest
	@LLVM_PROFILE_FILE="$(PWD)/$(BUILD_DIR)/cli_unittest.profraw" $(BUILD_DIR)/cli_unittest
	@LLVM_PROFILE_FILE="$(PWD)/$(BUILD_DIR)/lexer_integration.profraw" $(BUILD_DIR)/lexer_integration_tests
	@LLVM_PROFILE_FILE="$(PWD)/$(BUILD_DIR)/cli_integration.profraw" $(BUILD_DIR)/cli_integration_tests
	@echo ""
	@printf "$(COLOR_GREEN)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"
	@printf "$(COLOR_GREEN)$(COLOR_BOLD)Coverage build completed!\n$(COLOR_RESET)"
	@printf "$(COLOR_YELLOW)Run 'make coverage-report' to generate HTML report\n$(COLOR_RESET)"
	@printf "$(COLOR_GREEN)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"
	$(call ts_done,Coverage Build Complete)

# ============================================================================
# Generate Coverage Report
# ============================================================================
coverage-report:
	$(call ts_msg,Generating Coverage Report)
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)Generating Coverage Report\n$(COLOR_RESET)"
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"
	@if command -v llvm-profdata >/dev/null 2>&1 && command -v llvm-cov >/dev/null 2>&1; then \
		printf "$(COLOR_CYAN)Using LLVM coverage tools...\n$(COLOR_RESET)"; \
		PROFRAW_FILES=$$(find $(BUILD_DIR) -name "*.profraw" 2>/dev/null); \
		if [ -n "$$PROFRAW_FILES" ]; then \
			printf "$(COLOR_CYAN)Found profraw files:\n$$PROFRAW_FILES\n$(COLOR_RESET)"; \
			llvm-profdata merge -sparse $$PROFRAW_FILES -o $(BUILD_DIR)/coverage.profdata; \
			TEST_BINS=""; \
			for bin in lexer_unittest cli_unittest lexer_integration_tests cli_integration_tests; do \
				if [ -f "$(BUILD_DIR)/$$bin" ]; then \
					TEST_BINS="$$TEST_BINS -object $(BUILD_DIR)/$$bin"; \
				fi; \
			done; \
			if [ -n "$$TEST_BINS" ]; then \
				printf "$(COLOR_CYAN)Using test binaries for coverage...\n$(COLOR_RESET)"; \
				FIRST_BIN=$$(echo $$TEST_BINS | awk '{print $$2}'); \
				llvm-cov show $$FIRST_BIN $$TEST_BINS -instr-profile=$(BUILD_DIR)/coverage.profdata \
					--sources src/ include/ \
					-format=html -output-dir=$(BUILD_DIR)/coverage_html; \
				echo ""; \
				printf "$(COLOR_CYAN)Coverage Summary (source files only):\n$(COLOR_RESET)"; \
				llvm-cov report $$FIRST_BIN $$TEST_BINS -instr-profile=$(BUILD_DIR)/coverage.profdata \
					--sources src/ include/; \
				printf "\n$(COLOR_GREEN)Report: $(BUILD_DIR)/coverage_html/index.html\n$(COLOR_RESET)"; \
			else \
				printf "$(COLOR_YELLOW)Test executable not found.\n$(COLOR_RESET)"; \
			fi; \
		else \
			printf "$(COLOR_YELLOW)No coverage data found. Run 'make coverage' first.\n$(COLOR_RESET)"; \
		fi; \
	elif command -v lcov >/dev/null 2>&1; then \
		printf "$(COLOR_CYAN)Using lcov for coverage...\n$(COLOR_RESET)"; \
		lcov --capture --directory $(BUILD_DIR) --output-file $(BUILD_DIR)/coverage.info \
			--ignore-errors inconsistent,unsupported 2>/dev/null; \
		lcov --remove $(BUILD_DIR)/coverage.info '/usr/*' '/Library/*' '*/_deps/*' '*/vcpkg_installed/*' '*/test/*' \
			--output-file $(BUILD_DIR)/coverage_filtered.info \
			--ignore-errors inconsistent,unsupported,empty 2>/dev/null; \
		genhtml $(BUILD_DIR)/coverage_filtered.info --output-directory $(BUILD_DIR)/coverage_html \
			--ignore-errors inconsistent,unsupported,empty,category 2>/dev/null; \
		SUMMARY=$$(lcov --summary $(BUILD_DIR)/coverage_filtered.info --ignore-errors inconsistent,corrupt,count 2>&1); \
		LINE_COV=$$(echo "$$SUMMARY" | grep "lines" | grep -oE '[0-9]+\.[0-9]+%' | head -1); \
		FUNC_COV=$$(echo "$$SUMMARY" | grep "functions" | grep -oE '[0-9]+\.[0-9]+%' | head -1); \
		printf "$(COLOR_GREEN)$(COLOR_BOLD)Line coverage:     $$LINE_COV\n$(COLOR_RESET)"; \
		printf "$(COLOR_GREEN)$(COLOR_BOLD)Function coverage: $$FUNC_COV\n$(COLOR_RESET)"; \
		echo ""; \
		printf "$(COLOR_GREEN)Report: $(BUILD_DIR)/coverage_html/index.html\n$(COLOR_RESET)"; \
	else \
		printf "$(COLOR_RED)$(COLOR_BOLD)Error: Coverage tools not found!\n$(COLOR_RESET)"; \
		printf "$(COLOR_YELLOW)Please install lcov or llvm:\n$(COLOR_RESET)"; \
		echo "  macOS:   brew install lcov  OR  brew install llvm"; \
		echo "  Ubuntu:  sudo apt-get install lcov"; \
		exit 1; \
	fi
	$(call ts_done,Coverage Report Complete)

# ============================================================================
# Benchmark
# ============================================================================
benchmark:
	$(call ts_msg,Building and Running Benchmarks)
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)Building Performance Benchmarks\n$(COLOR_RESET)"
	@printf "$(COLOR_CYAN)Using $(NPROC) CPU cores\n$(COLOR_RESET)"
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"
	@$(CMAKE) -B $(BUILD_DIR) $(CMAKE_GENERATOR) $(CMAKE_COMMON_OPTS) \
		-DCMAKE_BUILD_TYPE=Release \
		-DBUILD_BENCHMARKS=ON
	@$(CMAKE) --build $(BUILD_DIR) --parallel $(NPROC)
	@echo ""
	@printf "$(COLOR_CYAN)Running benchmarks...\n$(COLOR_RESET)"
	@if [ -f $(BUILD_DIR)/$(BENCHMARK_DIRS)/benchmark_$(PROJECT_NAME)$(EXE_EXT) ]; then \
		./$(BUILD_DIR)/$(BENCHMARK_DIRS)/benchmark_$(PROJECT_NAME)$(EXE_EXT); \
	else \
		printf "$(COLOR_YELLOW)No benchmark executable found.\n$(COLOR_RESET)"; \
	fi
	@echo ""
	@printf "$(COLOR_GREEN)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"
	@printf "$(COLOR_GREEN)$(COLOR_BOLD)Benchmark completed!\n$(COLOR_RESET)"
	@printf "$(COLOR_GREEN)$(COLOR_BOLD)===================================\n$(COLOR_RESET)"
	$(call ts_done,Benchmark Complete)

# ============================================================================
# Pre-Commit Quality Check
# ============================================================================
runbeforecommit:
	$(call ts_msg,Pre-Commit Quality Check)
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)========================================\n$(COLOR_RESET)"
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)Pre-Commit Quality Check\n$(COLOR_RESET)"
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)========================================\n$(COLOR_RESET)"
	@echo ""
	@printf "$(COLOR_CYAN)Step 1/5: Cleaning previous build...\n$(COLOR_RESET)"
	@$(MAKE) clean
	@echo ""
	@printf "$(COLOR_CYAN)Step 2/5: Building project (Debug with coverage)...\n$(COLOR_RESET)"
	@$(CMAKE) -B $(BUILD_DIR) $(CMAKE_GENERATOR) $(CMAKE_COMMON_OPTS) \
		-DCMAKE_BUILD_TYPE=Debug \
		-DENABLE_COVERAGE=ON
	@$(CMAKE) --build $(BUILD_DIR) --parallel $(NPROC)
	@echo ""
	@printf "$(COLOR_CYAN)Step 3/5: Running all tests...\n$(COLOR_RESET)"
	@cd $(BUILD_DIR) && $(CTEST) --output-on-failure --parallel $(NPROC) || \
		(printf "$(COLOR_RED)$(COLOR_BOLD)[FAIL]$(COLOR_RESET) Tests failed! Fix errors before committing.\n" && exit 1)
	@echo ""
	@printf "$(COLOR_GREEN)$(COLOR_BOLD)[PASS]$(COLOR_RESET) All tests passed!\n"
	@echo ""
	@printf "$(COLOR_CYAN)Step 4/5: Checking coverage...\n$(COLOR_RESET)"
	@if command -v lcov >/dev/null 2>&1; then \
		lcov --capture --directory $(BUILD_DIR) --output-file $(BUILD_DIR)/coverage.info \
			--ignore-errors inconsistent,unsupported 2>/dev/null; \
		lcov --remove $(BUILD_DIR)/coverage.info '/usr/*' '/Library/*' '*/_deps/*' '*/vcpkg_installed/*' \
			--output-file $(BUILD_DIR)/coverage_filtered.info \
			--ignore-errors inconsistent,unsupported,empty 2>/dev/null; \
		SUMMARY=$$(lcov --summary $(BUILD_DIR)/coverage_filtered.info --ignore-errors inconsistent,corrupt,count 2>&1); \
		LINE_COV=$$(echo "$$SUMMARY" | grep "lines" | grep -oE '[0-9]+\.?[0-9]*%' | head -1 | sed 's/%//'); \
		FUNC_COV=$$(echo "$$SUMMARY" | grep "functions" | grep -oE '[0-9]+\.?[0-9]*%' | head -1 | sed 's/%//'); \
		if [ -z "$$LINE_COV" ]; then LINE_COV="0"; fi; \
		if [ -z "$$FUNC_COV" ]; then FUNC_COV="0"; fi; \
		printf "$(COLOR_CYAN)Line coverage:     $(COLOR_BOLD)$$LINE_COV%%$(COLOR_RESET)\n"; \
		printf "$(COLOR_CYAN)Function coverage: $(COLOR_BOLD)$$FUNC_COV%%$(COLOR_RESET)\n"; \
		printf "$(COLOR_CYAN)Required coverage: $(COLOR_BOLD)$(COVERAGE_THRESHOLD)%%$(COLOR_RESET)\n"; \
		LINE_FAIL=0; FUNC_FAIL=0; \
		if [ $$(awk "BEGIN {print ($$LINE_COV < $(COVERAGE_THRESHOLD))}") -eq 1 ]; then LINE_FAIL=1; fi; \
		if [ $$(awk "BEGIN {print ($$FUNC_COV < $(COVERAGE_THRESHOLD))}") -eq 1 ]; then FUNC_FAIL=1; fi; \
		if [ $$LINE_FAIL -eq 1 ] || [ $$FUNC_FAIL -eq 1 ]; then \
			printf "$(COLOR_RED)$(COLOR_BOLD)[FAIL]$(COLOR_RESET) Coverage below $(COVERAGE_THRESHOLD)%% threshold!\n"; \
			exit 1; \
		else \
			printf "$(COLOR_GREEN)$(COLOR_BOLD)[PASS]$(COLOR_RESET) Coverage check passed!\n"; \
		fi; \
	else \
		printf "$(COLOR_YELLOW)[WARN]$(COLOR_RESET) lcov not found, skipping coverage check\n"; \
	fi
	@echo ""
	@printf "$(COLOR_CYAN)Step 5/5: Running code formatter...\n$(COLOR_RESET)"
	@$(MAKE) fmt
	@echo ""
	@printf "$(COLOR_GREEN)$(COLOR_BOLD)========================================\n$(COLOR_RESET)"
	@printf "$(COLOR_GREEN)$(COLOR_BOLD)Pre-Commit Check PASSED!\n$(COLOR_RESET)"
	@printf "$(COLOR_GREEN)$(COLOR_BOLD)========================================\n$(COLOR_RESET)"
	@echo ""
	@printf "$(COLOR_BOLD)Summary:\n$(COLOR_RESET)"
	@printf "  $(COLOR_GREEN)[PASS]$(COLOR_RESET) Build successful\n"
	@printf "  $(COLOR_GREEN)[PASS]$(COLOR_RESET) All tests passed\n"
	@printf "  $(COLOR_GREEN)[PASS]$(COLOR_RESET) Coverage >= $(COVERAGE_THRESHOLD)%%\n"
	@printf "  $(COLOR_GREEN)[PASS]$(COLOR_RESET) Code formatted\n"
	@echo ""
	@printf "$(COLOR_GREEN)$(COLOR_BOLD)You are ready to commit!$(COLOR_RESET)\n"
	@echo ""
	$(call ts_done,Pre-Commit Check Complete)

# ============================================================================
# Code Statistics
# ============================================================================
stats:
	$(call ts_msg,Code Statistics)
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)╔═══════════════════════════════════════════════════╗\n$(COLOR_RESET)"
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)║    $(PROJECT_NAME) - Code Statistics               \n$(COLOR_RESET)"
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)╚═══════════════════════════════════════════════════╝\n$(COLOR_RESET)"
	@echo ""
	@if command -v cloc >/dev/null 2>&1; then \
		printf "$(COLOR_CYAN)Using cloc for detailed statistics...\n$(COLOR_RESET)"; \
		echo ""; \
		DIRS=""; \
		for dir in $(SRC_DIRS) $(INCLUDE_DIRS) $(TEST_DIRS); do \
			if [ -d "$$dir" ]; then \
				DIRS="$$DIRS $$dir"; \
			fi; \
		done; \
		if [ -n "$$DIRS" ]; then \
			cloc $$DIRS --exclude-dir=build,_deps,vcpkg_installed; \
		else \
			printf "$(COLOR_YELLOW)No source directories found.\n$(COLOR_RESET)"; \
		fi; \
	else \
		printf "$(COLOR_CYAN)Using built-in counter (install cloc for detailed stats)...\n$(COLOR_RESET)"; \
		echo ""; \
		printf "$(COLOR_BOLD)%-40s %10s %10s %10s %10s\n$(COLOR_RESET)" "Directory" "Files" "Blank" "Comment" "Code"; \
		printf "$(COLOR_CYAN)──────────────────────────────────────────────────────────────────────────────────\n$(COLOR_RESET)"; \
		TOTAL_FILES=0; TOTAL_BLANK=0; TOTAL_COMMENT=0; TOTAL_CODE=0; \
		for dir in $(SRC_DIRS) $(INCLUDE_DIRS) $(TEST_DIRS); do \
			if [ -d "$$dir" ]; then \
				FILES=$$(find $$dir -type f \( -name "*.cpp" -o -name "*.cc" -o -name "*.c" -o -name "*.hpp" -o -name "*.h" \) 2>/dev/null | wc -l | tr -d ' '); \
				if [ "$$FILES" -gt 0 ]; then \
					STATS=$$(find $$dir -type f \( -name "*.cpp" -o -name "*.cc" -o -name "*.c" -o -name "*.hpp" -o -name "*.h" \) -exec cat {} + 2>/dev/null | awk ' \
						BEGIN { blank=0; comment=0; code=0; in_block=0 } \
						/^[[:space:]]*$$/ { blank++; next } \
						/^[[:space:]]*\/\// { comment++; next } \
						/^[[:space:]]*\/\*/ { comment++; in_block=1; if (/\*\//) in_block=0; next } \
						in_block { comment++; if (/\*\//) in_block=0; next } \
						{ code++ } \
						END { printf "%d %d %d", blank, comment, code } \
					'); \
					BLANK=$$(echo $$STATS | cut -d" " -f1); \
					COMMENT=$$(echo $$STATS | cut -d" " -f2); \
					CODE=$$(echo $$STATS | cut -d" " -f3); \
					printf "%-40s %10d %10d %10d %10d\n" "$$dir" "$$FILES" "$$BLANK" "$$COMMENT" "$$CODE"; \
					TOTAL_FILES=$$((TOTAL_FILES + FILES)); \
					TOTAL_BLANK=$$((TOTAL_BLANK + BLANK)); \
					TOTAL_COMMENT=$$((TOTAL_COMMENT + COMMENT)); \
					TOTAL_CODE=$$((TOTAL_CODE + CODE)); \
				fi; \
			fi; \
		done; \
		if [ $$TOTAL_FILES -eq 0 ]; then \
			printf "$(COLOR_YELLOW)No source files found in any directory.\n$(COLOR_RESET)"; \
		else \
			printf "$(COLOR_CYAN)──────────────────────────────────────────────────────────────────────────────────\n$(COLOR_RESET)"; \
			printf "$(COLOR_GREEN)$(COLOR_BOLD)%-40s %10d %10d %10d %10d\n$(COLOR_RESET)" "TOTAL" "$$TOTAL_FILES" "$$TOTAL_BLANK" "$$TOTAL_COMMENT" "$$TOTAL_CODE"; \
		fi; \
		echo ""; \
		printf "$(COLOR_YELLOW)Tip: Install cloc for more accurate statistics:\n$(COLOR_RESET)"; \
		echo "  macOS:  brew install cloc"; \
		echo "  Ubuntu: sudo apt-get install cloc"; \
	fi
	@echo ""
	$(call ts_done,Code Statistics Complete)

# ============================================================================
# Run Main Executable
# ============================================================================
run: release
	$(call ts_msg,Running $(PROJECT_NAME))
	@if [ -f "$(MAIN_EXECUTABLE_PATH)" ]; then \
		printf "$(COLOR_CYAN)Executing: $(MAIN_EXECUTABLE_PATH)\n$(COLOR_RESET)"; \
		echo ""; \
		./$(MAIN_EXECUTABLE_PATH) $(ARGS); \
	else \
		printf "$(COLOR_RED)$(COLOR_BOLD)Error: Executable not found!\n$(COLOR_RESET)"; \
		printf "$(COLOR_YELLOW)Expected: $(MAIN_EXECUTABLE_PATH)\n$(COLOR_RESET)"; \
		printf "$(COLOR_YELLOW)Make sure your CMakeLists.txt creates an executable named '$(MAIN_EXECUTABLE)'\n$(COLOR_RESET)"; \
		exit 1; \
	fi
	$(call ts_done,Execution Complete)

# ============================================================================
# Build Information Dashboard
# ============================================================================

info:
	$(call ts_msg,Build Information)
	@echo ""
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)  ╔═══════════════════════════════════════════════════════════════╗\n$(COLOR_RESET)"
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)  ║  $(PROJECT_NAME) v$(PROJECT_VERSION) - Build Information\n$(COLOR_RESET)"
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)  ╚═══════════════════════════════════════════════════════════════╝\n$(COLOR_RESET)"
	@echo ""
	@# ===== System Section =====
	@printf "$(COLOR_CYAN)$(COLOR_BOLD)  ┌─ System ───────────────────────────────────────────────────────\n$(COLOR_RESET)"
	@OS_NAME="$(UNAME_S)"; \
	if [ "$$OS_NAME" = "Darwin" ]; then OS_DISPLAY="macOS (Darwin)"; else OS_DISPLAY="Linux ($$OS_NAME)"; fi; \
	ARCH=$$(uname -m); \
	if [ "$(UNAME_S)" = "Darwin" ]; then \
		MEM_BYTES=$$(sysctl -n hw.memsize 2>/dev/null || echo 0); \
		MEM_GB=$$(awk "BEGIN {printf \"%.0f\", $$MEM_BYTES/1024/1024/1024}"); \
	else \
		MEM_KB=$$(grep MemTotal /proc/meminfo 2>/dev/null | awk '{print $$2}' || echo 0); \
		MEM_GB=$$(awk "BEGIN {printf \"%.0f\", $$MEM_KB/1024/1024}"); \
	fi; \
	printf "$(COLOR_CYAN)  │$(COLOR_RESET)  %-14s $(COLOR_BOLD)%s$(COLOR_RESET)\n" "OS:" "$$OS_DISPLAY"; \
	printf "$(COLOR_CYAN)  │$(COLOR_RESET)  %-14s $(COLOR_BOLD)%s$(COLOR_RESET)\n" "Architecture:" "$$ARCH"; \
	printf "$(COLOR_CYAN)  │$(COLOR_RESET)  %-14s $(COLOR_BOLD)%s$(COLOR_RESET)\n" "CPU Cores:" "$(CPU_CORES)"; \
	printf "$(COLOR_CYAN)  │$(COLOR_RESET)  %-14s $(COLOR_BOLD)%s$(COLOR_RESET)\n" "Memory:" "$${MEM_GB} GB"
	@printf "$(COLOR_CYAN)  └─────────────────────────────────────────────────────────────────\n$(COLOR_RESET)"
	@echo ""
	@# ===== Compiler Section =====
	@printf "$(COLOR_CYAN)$(COLOR_BOLD)  ┌─ Compiler ─────────────────────────────────────────────────────\n$(COLOR_RESET)"
	@CC_VER=$$($(CC) --version 2>&1 | head -1 | sed 's/.*version //' | cut -d' ' -f1 || echo "N/A"); \
	CXX_VER=$$($(CXX) --version 2>&1 | head -1 | sed 's/.*version //' | cut -d' ' -f1 || echo "N/A"); \
	CMAKE_VER=$$($(CMAKE) --version 2>&1 | head -1 | sed 's/cmake version //' || echo "N/A"); \
	printf "$(COLOR_CYAN)  │$(COLOR_RESET)  %-14s $(COLOR_BOLD)%s %s$(COLOR_RESET)\n" "C Compiler:" "$(CC)" "$$CC_VER"; \
	printf "$(COLOR_CYAN)  │$(COLOR_RESET)  %-14s $(COLOR_BOLD)%s %s$(COLOR_RESET)\n" "C++ Compiler:" "$(CXX)" "$$CXX_VER"; \
	printf "$(COLOR_CYAN)  │$(COLOR_RESET)  %-14s $(COLOR_BOLD)%s$(COLOR_RESET)\n" "C++ Standard:" "C++$(CXX_STANDARD)"; \
	printf "$(COLOR_CYAN)  │$(COLOR_RESET)  %-14s $(COLOR_BOLD)%s$(COLOR_RESET)\n" "CMake:" "$$CMAKE_VER"
	@printf "$(COLOR_CYAN)  └─────────────────────────────────────────────────────────────────\n$(COLOR_RESET)"
	@echo ""
	@# ===== Build Configuration Section =====
	@printf "$(COLOR_CYAN)$(COLOR_BOLD)  ┌─ Build Configuration ──────────────────────────────────────────\n$(COLOR_RESET)"
	@OPT_STR="-$(OPTIMIZATION)"; \
	if [ "$(ENABLE_LTO)" = "ON" ]; then OPT_STR="$$OPT_STR +LTO"; fi; \
	if [ "$(NATIVE_ARCH)" = "ON" ]; then OPT_STR="$$OPT_STR +native"; fi; \
	printf "$(COLOR_CYAN)  │$(COLOR_RESET)  %-14s $(COLOR_BOLD)%s$(COLOR_RESET)\n" "Optimization:" "$$OPT_STR"; \
	printf "$(COLOR_CYAN)  │$(COLOR_RESET)  %-14s $(COLOR_BOLD)%s$(COLOR_RESET)\n" "LTO:" "$(ENABLE_LTO)"; \
	printf "$(COLOR_CYAN)  │$(COLOR_RESET)  %-14s $(COLOR_BOLD)%s$(COLOR_RESET)\n" "Native Arch:" "$(NATIVE_ARCH)"; \
	printf "$(COLOR_CYAN)  │$(COLOR_RESET)  %-14s $(COLOR_BOLD)%s$(COLOR_RESET)\n" "Parallel:" "$(NPROC) jobs (of $(CPU_CORES) cores)"
	@printf "$(COLOR_CYAN)  └─────────────────────────────────────────────────────────────────\n$(COLOR_RESET)"
	@echo ""
	@# ===== Last Build Status Section =====
	@printf "$(COLOR_CYAN)$(COLOR_BOLD)  ┌─ Last Build Status ────────────────────────────────────────────\n$(COLOR_RESET)"
	@if [ -d "$(BUILD_DIR)" ]; then \
		printf "$(COLOR_CYAN)  │$(COLOR_RESET)  %-14s $(COLOR_GREEN)[OK]$(COLOR_RESET) exists\n" "Build Dir:"; \
	else \
		printf "$(COLOR_CYAN)  │$(COLOR_RESET)  %-14s $(COLOR_YELLOW)[--]$(COLOR_RESET) not found\n" "Build Dir:"; \
	fi
	@if [ -f "$(BUILD_DIR)/CMakeCache.txt" ]; then \
		BUILD_TYPE=$$(grep 'CMAKE_BUILD_TYPE:STRING=' $(BUILD_DIR)/CMakeCache.txt 2>/dev/null | cut -d'=' -f2 || echo "Unknown"); \
		printf "$(COLOR_CYAN)  │$(COLOR_RESET)  %-14s $(COLOR_BOLD)%s$(COLOR_RESET)\n" "Build Type:" "$$BUILD_TYPE"; \
	else \
		printf "$(COLOR_CYAN)  │$(COLOR_RESET)  %-14s $(COLOR_YELLOW)[--]$(COLOR_RESET) not configured\n" "Build Type:"; \
	fi
	@if [ -f "$(MAIN_EXECUTABLE_PATH)" ]; then \
		if [ "$(UNAME_S)" = "Darwin" ]; then \
			MOD_TIME=$$(stat -f '%Sm' -t '%Y-%m-%d %H:%M:%S' "$(MAIN_EXECUTABLE_PATH)" 2>/dev/null || echo "unknown"); \
		else \
			MOD_TIME=$$(stat -c '%y' "$(MAIN_EXECUTABLE_PATH)" 2>/dev/null | cut -d'.' -f1 || echo "unknown"); \
		fi; \
		printf "$(COLOR_CYAN)  │$(COLOR_RESET)  %-14s $(COLOR_GREEN)[OK]$(COLOR_RESET) %s\n" "Main Exe:" "$$MOD_TIME"; \
	else \
		printf "$(COLOR_CYAN)  │$(COLOR_RESET)  %-14s $(COLOR_YELLOW)[--]$(COLOR_RESET) not built\n" "Main Exe:"; \
	fi
	@if [ -f "$(TEST_EXECUTABLE_PATH)" ]; then \
		if [ "$(UNAME_S)" = "Darwin" ]; then \
			MOD_TIME=$$(stat -f '%Sm' -t '%Y-%m-%d %H:%M:%S' "$(TEST_EXECUTABLE_PATH)" 2>/dev/null || echo "unknown"); \
		else \
			MOD_TIME=$$(stat -c '%y' "$(TEST_EXECUTABLE_PATH)" 2>/dev/null | cut -d'.' -f1 || echo "unknown"); \
		fi; \
		printf "$(COLOR_CYAN)  │$(COLOR_RESET)  %-14s $(COLOR_GREEN)[OK]$(COLOR_RESET) %s\n" "Test Exe:" "$$MOD_TIME"; \
	else \
		printf "$(COLOR_CYAN)  │$(COLOR_RESET)  %-14s $(COLOR_YELLOW)[--]$(COLOR_RESET) not built\n" "Test Exe:"; \
	fi
	@printf "$(COLOR_CYAN)  └─────────────────────────────────────────────────────────────────\n$(COLOR_RESET)"
	@echo ""
	@# ===== Git Status Section =====
	@printf "$(COLOR_CYAN)$(COLOR_BOLD)  ┌─ Git Status ────────────────────────────────────────────────────\n$(COLOR_RESET)"
	@if command -v git >/dev/null 2>&1 && git rev-parse --is-inside-work-tree >/dev/null 2>&1; then \
		BRANCH=$$(git branch --show-current 2>/dev/null || echo "detached"); \
		COMMIT=$$(git log -1 --format='%h - %s' 2>/dev/null | cut -c1-50 || echo "N/A"); \
		COMMIT_TIME=$$(git log -1 --format='%cr' 2>/dev/null || echo ""); \
		MODIFIED=$$(git status --porcelain 2>/dev/null | grep -c '^.M' || echo 0); \
		UNTRACKED=$$(git status --porcelain 2>/dev/null | grep -c '^??' || echo 0); \
		printf "$(COLOR_CYAN)  │$(COLOR_RESET)  %-14s $(COLOR_BOLD)%s$(COLOR_RESET)\n" "Branch:" "$$BRANCH"; \
		printf "$(COLOR_CYAN)  │$(COLOR_RESET)  %-14s $(COLOR_BOLD)%s$(COLOR_RESET)\n" "Last Commit:" "$$COMMIT"; \
		if [ -n "$$COMMIT_TIME" ]; then \
			printf "$(COLOR_CYAN)  │$(COLOR_RESET)  %-14s %s\n" "" "($$COMMIT_TIME)"; \
		fi; \
		if [ "$$MODIFIED" -gt 0 ] || [ "$$UNTRACKED" -gt 0 ]; then \
			printf "$(COLOR_CYAN)  │$(COLOR_RESET)  %-14s $(COLOR_YELLOW)[!!]$(COLOR_RESET) %s modified, %s untracked\n" "Working Tree:" "$$MODIFIED" "$$UNTRACKED"; \
		else \
			printf "$(COLOR_CYAN)  │$(COLOR_RESET)  %-14s $(COLOR_GREEN)[OK]$(COLOR_RESET) clean\n" "Working Tree:"; \
		fi; \
	else \
		printf "$(COLOR_CYAN)  │$(COLOR_RESET)  %-14s $(COLOR_YELLOW)[--]$(COLOR_RESET) Not a git repository\n" ""; \
	fi
	@printf "$(COLOR_CYAN)  └─────────────────────────────────────────────────────────────────\n$(COLOR_RESET)"
	@echo ""
	@# ===== Dependencies Section =====
	@printf "$(COLOR_CYAN)$(COLOR_BOLD)  ┌─ Dependencies ─────────────────────────────────────────────────\n$(COLOR_RESET)"
	@if [ -d "$(VCPKG_ROOT)" ]; then \
		printf "$(COLOR_CYAN)  │$(COLOR_RESET)  %-14s $(COLOR_GREEN)[OK]$(COLOR_RESET) %s\n" "vcpkg:" "$(VCPKG_ROOT)"; \
	else \
		printf "$(COLOR_CYAN)  │$(COLOR_RESET)  %-14s $(COLOR_YELLOW)[--]$(COLOR_RESET) not found\n" "vcpkg:"; \
	fi
	@if [ -f "vcpkg.json" ]; then \
		printf "$(COLOR_CYAN)  │$(COLOR_RESET)  %-14s $(COLOR_GREEN)[OK]$(COLOR_RESET) found\n" "vcpkg.json:"; \
	else \
		printf "$(COLOR_CYAN)  │$(COLOR_RESET)  %-14s $(COLOR_YELLOW)[--]$(COLOR_RESET) not found\n" "vcpkg.json:"; \
	fi
	@if [ -f "CMakeLists.txt" ]; then \
		printf "$(COLOR_CYAN)  │$(COLOR_RESET)  %-14s $(COLOR_GREEN)[OK]$(COLOR_RESET) found\n" "CMakeLists:"; \
	else \
		printf "$(COLOR_CYAN)  │$(COLOR_RESET)  %-14s $(COLOR_RED)[!!]$(COLOR_RESET) MISSING - required!\n" "CMakeLists:"; \
	fi
	@printf "$(COLOR_CYAN)  └─────────────────────────────────────────────────────────────────\n$(COLOR_RESET)"
	@echo ""
	@# ===== Source Files Section =====
	@printf "$(COLOR_CYAN)$(COLOR_BOLD)  ┌─ Source Files ─────────────────────────────────────────────────\n$(COLOR_RESET)"
	@HEADER_COUNT=0; SRC_COUNT=0; TEST_COUNT=0; \
	if [ -d "$(INCLUDE_DIRS)" ]; then \
		HEADER_COUNT=$$(find $(INCLUDE_DIRS) -type f \( -name "*.hpp" -o -name "*.h" \) 2>/dev/null | wc -l | tr -d ' '); \
	fi; \
	if [ -d "$(SRC_DIRS)" ]; then \
		SRC_COUNT=$$(find $(SRC_DIRS) -type f \( -name "*.cpp" -o -name "*.cc" -o -name "*.c" \) 2>/dev/null | wc -l | tr -d ' '); \
	fi; \
	if [ -d "$(TEST_DIRS)" ]; then \
		TEST_COUNT=$$(find $(TEST_DIRS) -type f \( -name "*.cpp" -o -name "*.cc" -o -name "*.c" \) 2>/dev/null | wc -l | tr -d ' '); \
	fi; \
	TOTAL=$$((HEADER_COUNT + SRC_COUNT + TEST_COUNT)); \
	printf "$(COLOR_CYAN)  │$(COLOR_RESET)  %-14s $(COLOR_BOLD)%s$(COLOR_RESET) files (%s/)\n" "Headers:" "$$HEADER_COUNT" "$(INCLUDE_DIRS)"; \
	printf "$(COLOR_CYAN)  │$(COLOR_RESET)  %-14s $(COLOR_BOLD)%s$(COLOR_RESET) files (%s/)\n" "Sources:" "$$SRC_COUNT" "$(SRC_DIRS)"; \
	printf "$(COLOR_CYAN)  │$(COLOR_RESET)  %-14s $(COLOR_BOLD)%s$(COLOR_RESET) files (%s/)\n" "Tests:" "$$TEST_COUNT" "$(TEST_DIRS)"; \
	printf "$(COLOR_CYAN)  │$(COLOR_RESET)  %-14s $(COLOR_BOLD)%s$(COLOR_RESET) files\n" "Total:" "$$TOTAL"
	@printf "$(COLOR_CYAN)  └─────────────────────────────────────────────────────────────────\n$(COLOR_RESET)"
	@echo ""
	@# ===== Config Files Section =====
	@printf "$(COLOR_CYAN)$(COLOR_BOLD)  ┌─ Config Files ─────────────────────────────────────────────────\n$(COLOR_RESET)"
	@CF="--"; CT="--"; DX="--"; GI="--"; \
	CFC="$(COLOR_YELLOW)"; CTC="$(COLOR_YELLOW)"; DXC="$(COLOR_YELLOW)"; GIC="$(COLOR_YELLOW)"; \
	if [ -f ".clang-format" ]; then CF="OK"; CFC="$(COLOR_GREEN)"; fi; \
	if [ -f ".clang-tidy" ]; then CT="OK"; CTC="$(COLOR_GREEN)"; fi; \
	if [ -f "Doxyfile" ]; then DX="OK"; DXC="$(COLOR_GREEN)"; fi; \
	if [ -f ".gitignore" ]; then GI="OK"; GIC="$(COLOR_GREEN)"; fi; \
	printf "$(COLOR_CYAN)  │$(COLOR_RESET)  .clang-format $${CFC}[%s]$(COLOR_RESET)    .clang-tidy $${CTC}[%s]$(COLOR_RESET)\n" "$$CF" "$$CT"; \
	printf "$(COLOR_CYAN)  │$(COLOR_RESET)  Doxyfile      $${DXC}[%s]$(COLOR_RESET)    .gitignore  $${GIC}[%s]$(COLOR_RESET)\n" "$$DX" "$$GI"
	@printf "$(COLOR_CYAN)  └─────────────────────────────────────────────────────────────────\n$(COLOR_RESET)"
	@echo ""
	$(call ts_done,Build Information Complete)

# ============================================================================
# Help
# ============================================================================
help:
	$(call ts_msg,Help)
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)╔═══════════════════════════════════════════════════════╗\n$(COLOR_RESET)"
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)║  $(PROJECT_NAME) - Makefile Commands                   \n$(COLOR_RESET)"
	@printf "$(COLOR_BLUE)$(COLOR_BOLD)╚═══════════════════════════════════════════════════════╝\n$(COLOR_RESET)"
	@echo ""
	@printf "$(COLOR_CYAN)$(COLOR_BOLD)Build Commands:$(COLOR_RESET)\n"
	@echo "  make [all|build]     - Build project in Release mode (default)"
	@echo "  make release         - Build project in Release mode"
	@echo "  make debug           - Build project in Debug mode"
	@echo "  make clean           - Clean all build artifacts"
	@echo "  make rebuild         - Clean and rebuild"
	@echo "  make run             - Build and run main executable"
	@echo "  make run ARGS='...'  - Run with arguments"
	@echo "  make info            - Show build information dashboard"
	@echo "  make check-deps      - Check if all dependencies are installed"
	@echo ""
	@printf "$(COLOR_CYAN)$(COLOR_BOLD)Testing & Quality:$(COLOR_RESET)\n"
	@echo "  make test            - Build and run tests (Google Test)"
	@echo "  make coverage        - Build with coverage instrumentation"
	@echo "  make coverage-report - Generate HTML coverage report"
	@echo "  make benchmark       - Build and run performance benchmarks"
	@echo "  make runbeforecommit - Full quality check before committing"
	@echo ""
	@printf "$(COLOR_CYAN)$(COLOR_BOLD)Code Quality:$(COLOR_RESET)\n"
	@echo "  make fmt             - Format code with clang-format"
	@echo "  make tidy            - Run clang-tidy static analysis"
	@echo "  make analyze         - Run static analysis (alias for tidy)"
	@echo "  make analyze-cppcheck- Run cppcheck static analysis"
	@echo "  make analyze-full    - Run all static analyzers"
	@echo ""
	@printf "$(COLOR_CYAN)$(COLOR_BOLD)Documentation:$(COLOR_RESET)\n"
	@echo "  make docs            - Generate Doxygen documentation"
	@echo "  make stats           - Show code line statistics"
	@echo ""
	@printf "$(COLOR_CYAN)$(COLOR_BOLD)Dependencies:$(COLOR_RESET)\n"
	@echo "  make vcpkg-install   - Install dependencies via vcpkg"
	@echo ""
	@printf "$(COLOR_CYAN)$(COLOR_BOLD)Installation:$(COLOR_RESET)\n"
	@echo "  make install         - Install to /usr/local"
	@echo ""
	@printf "$(COLOR_CYAN)$(COLOR_BOLD)Configuration:$(COLOR_RESET)\n"
	@echo "  Compiler:      $(CXX)"
	@echo "  C++ Standard:  C++$(CXX_STANDARD)"
	@echo "  Optimization:  -$(OPTIMIZATION)"
	@echo "  LTO:           $(ENABLE_LTO)"
	@echo "  Native Arch:   $(NATIVE_ARCH)"
	@echo "  Parallel Jobs: $(NPROC) (of $(CPU_CORES) cores)"
	@echo "  vcpkg Root:    $(VCPKG_ROOT)"
	@echo ""
	@printf "$(COLOR_CYAN)$(COLOR_BOLD)Override Examples:$(COLOR_RESET)\n"
	@echo "  make release OPTIMIZATION=Ofast    # Use -Ofast"
	@echo "  make release ENABLE_LTO=ON         # Enable Link-Time Optimization"
	@echo "  make release NATIVE_ARCH=ON        # Use -march=native"
	@echo "  make release PARALLEL_JOBS=4       # Limit to 4 parallel jobs"
	@echo "  make release OPTIMIZATION=O3 ENABLE_LTO=ON NATIVE_ARCH=ON  # Max performance"
	@echo ""
	$(call ts_done,Help Complete)
