CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -I.

# Directories
SRC_DIR = lexer
TEST_DIR = tests
CMD_DIR = cmd
BUILD_DIR = build

# Source files
LEXER_SOURCES = $(SRC_DIR)/token.cpp
TEST_SOURCES = $(TEST_DIR)/test_lexer.cpp
MAIN_SOURCES = $(CMD_DIR)/main.cpp

# Output
TEST_BIN = $(BUILD_DIR)/test_lexer
MAIN_BIN = $(BUILD_DIR)/czc

.PHONY: all clean test czc

all: czc

czc: $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(LEXER_SOURCES) $(MAIN_SOURCES) -o $(MAIN_BIN)
	@echo "Build complete: $(MAIN_BIN)"

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

test: $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(LEXER_SOURCES) $(TEST_SOURCES) -o $(TEST_BIN)
	./$(TEST_BIN)

clean:
	rm -rf $(BUILD_DIR)
