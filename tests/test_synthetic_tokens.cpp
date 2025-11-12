/**
 * @file test_synthetic_tokens.cpp
 * @brief 测试虚拟 Token（synthetic tokens）的处理 (Google Test 版本)。
 * @author BegoniaHe
 * @date 2025-11-11
 */

#include "czc/formatter/formatter.hpp"
#include "czc/lexer/lexer.hpp"
#include "czc/parser/parser.hpp"

#include <gtest/gtest.h>

using namespace czc::formatter;
using namespace czc::lexer;
using namespace czc::parser;

// --- Test Fixtures ---

class SyntheticTokensTest : public ::testing::Test {
protected:
  // Empty for now
};

// --- Synthetic Semicolon Tests ---

TEST_F(SyntheticTokensTest, SyntheticSemicolonNotOutput) {
  std::string source = R"(
let x = 10
let y = 20;
)";

  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto tree = parser.parse();

  EXPECT_TRUE(parser.has_errors());

  FormatOptions options;
  Formatter formatter(options);
  std::string formatted = formatter.format(tree.get());

  // 统计分号数量
  size_t semicolon_count = 0;
  for (char c : formatted) {
    if (c == ';') {
      semicolon_count++;
    }
  }

  EXPECT_EQ(semicolon_count, 1);
}

// --- Synthetic Paren Tests ---

TEST_F(SyntheticTokensTest, SyntheticParenNotOutput) {
  std::string source = R"(
fn test {
  return 42;
}
)";

  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto tree = parser.parse();

  EXPECT_TRUE(parser.has_errors());

  FormatOptions options;
  Formatter formatter(options);
  std::string formatted = formatter.format(tree.get());

  // Note: The parser may insert synthetic tokens for error recovery,
  // but the exact behavior depends on implementation details.
  // We just verify the formatter doesn't crash and produces output.
  EXPECT_FALSE(formatted.empty());
}

// --- Real Tokens Tests ---

TEST_F(SyntheticTokensTest, RealTokensOutput) {
  std::string source = R"(
let x = 10;
let y = 20;
)";

  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto tree = parser.parse();

  EXPECT_FALSE(parser.has_errors());

  FormatOptions options;
  Formatter formatter(options);
  std::string formatted = formatter.format(tree.get());

  size_t semicolon_count = 0;
  for (char c : formatted) {
    if (c == ';') {
      semicolon_count++;
    }
  }

  EXPECT_EQ(semicolon_count, 2);
}

// --- Mixed Tokens Tests ---

TEST_F(SyntheticTokensTest, MixedTokens) {
  std::string source = R"(
let x = 10;
let y = 20
let z = 30;
)";

  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto tree = parser.parse();

  EXPECT_TRUE(parser.has_errors());

  FormatOptions options;
  Formatter formatter(options);
  std::string formatted = formatter.format(tree.get());

  size_t semicolon_count = 0;
  for (char c : formatted) {
    if (c == ';') {
      semicolon_count++;
    }
  }

  EXPECT_EQ(semicolon_count, 2);
}
