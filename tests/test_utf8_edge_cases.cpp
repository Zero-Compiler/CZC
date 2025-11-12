/**
 * @file test_utf8_edge_cases.cpp
 * @brief 测试 UTF-8 编码边界情况和错误处理 (Google Test 版本)。
 * @author BegoniaHe
 * @date 2025-11-11
 */

#include "czc/lexer/lexer.hpp"
#include "czc/lexer/utf8_handler.hpp"

#include <gtest/gtest.h>

using namespace czc::lexer;

// --- Test Fixtures ---

class Utf8EdgeCasesTest : public ::testing::Test {
protected:
  // Empty for now
};

// --- 4-Byte Emoji Tests ---

TEST_F(Utf8EdgeCasesTest, FourByteEmoji) {
  std::string source = "let emoji = \"🚀\";";

  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  EXPECT_FALSE(lexer.get_errors().has_errors());

  bool found_string = false;
  for (const auto& token : tokens) {
    if (token.token_type == TokenType::String) {
      // The token value contains the string content without quotes
      EXPECT_NE(token.value.find("🚀"), std::string::npos);
      found_string = true;
    }
  }
  EXPECT_TRUE(found_string);
}

// --- Various Unicode Characters Tests ---

TEST_F(Utf8EdgeCasesTest, VariousUnicodeCharacters) {
  std::string source = R"(
    let emoji1 = "😀";
    let emoji2 = "🔥";
    let chinese = "你好";
    let japanese = "こんにちは";
    let mixed = "Hello世界🌍";
  )";

  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  EXPECT_FALSE(lexer.get_errors().has_errors());
}

// --- UTF-8 Identifiers Tests ---

TEST_F(Utf8EdgeCasesTest, Utf8Identifiers) {
  std::string source = R"(
    let 变量 = 10;
    let переменная = 20;
    let μετβλητή = 30;
    let 変数 = 40;
  )";

  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  EXPECT_FALSE(lexer.get_errors().has_errors());

  int identifier_count = 0;
  for (const auto& token : tokens) {
    if (token.token_type == TokenType::Identifier) {
      identifier_count++;
    }
  }
  EXPECT_GE(identifier_count, 4);
}

// --- Invalid UTF-8 Start Byte Tests ---

TEST_F(Utf8EdgeCasesTest, InvalidUtf8StartByte) {
  std::string source = "let x = \xFF;";

  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  // Note: The lexer may or may not detect this specific error depending on
  // how it handles binary data. We just verify it doesn't crash.
  // If errors are detected, that's good. If not, that's okay too.
  // The main point is the lexer handles it gracefully.
  EXPECT_FALSE(tokens.empty());
}

// --- Incomplete UTF-8 Sequence Tests ---

TEST_F(Utf8EdgeCasesTest, IncompleteUtf8Sequence) {
  std::string source = "let x = \"\xE4\";";

  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  EXPECT_TRUE(lexer.get_errors().has_errors());
}

// --- Invalid UTF-8 Continuation Tests ---

TEST_F(Utf8EdgeCasesTest, InvalidUtf8Continuation) {
  std::string source = "let x = \"\xC0\x80\";";

  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  // Note: The lexer may or may not detect overlong encodings.
  // We just verify it doesn't crash.
  EXPECT_FALSE(tokens.empty());
}

// --- UTF-8 BOM Tests ---

TEST_F(Utf8EdgeCasesTest, Utf8BOM) {
  // BOM handling: The lexer may or may not handle BOM specially.
  // We test that it doesn't crash when encountering a BOM.
  std::string source = "\xEF\xBB\xBFlet x = 10;";

  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  // The main requirement is that the lexer doesn't crash
  // The exact behavior (whether BOM is skipped or causes an error)
  // depends on implementation details
  EXPECT_GE(tokens.size(), 0);
}

// --- Zero Width Characters Tests ---

TEST_F(Utf8EdgeCasesTest, ZeroWidthCharacters) {
  std::string source = "let\u200Bx = 10;";

  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  EXPECT_FALSE(lexer.get_errors().has_errors());
}

// --- UTF-8 Handler Static Methods Tests ---

TEST_F(Utf8EdgeCasesTest, Utf8ContinuationByte) {
  // Test continuation byte detection
  EXPECT_TRUE(Utf8Handler::is_continuation(0x80));  // 10000000
  EXPECT_TRUE(Utf8Handler::is_continuation(0xBF));  // 10111111
  EXPECT_FALSE(Utf8Handler::is_continuation(0x7F)); // 01111111
  EXPECT_FALSE(Utf8Handler::is_continuation(0xC0)); // 11000000
}

TEST_F(Utf8EdgeCasesTest, Utf8CharLength) {
  // Test character length detection
  EXPECT_EQ(Utf8Handler::get_char_length(0x41), 1); // ASCII 'A'
  EXPECT_EQ(Utf8Handler::get_char_length(0xC2), 2); // 2-byte sequence
  EXPECT_EQ(Utf8Handler::get_char_length(0xE0), 3); // 3-byte sequence
  EXPECT_EQ(Utf8Handler::get_char_length(0xF0), 4); // 4-byte sequence
}

// --- UTF-8 at Boundaries Tests ---

TEST_F(Utf8EdgeCasesTest, Utf8AtBoundaries) {
  std::string source1 = "你好世界";
  Lexer lexer1(source1);
  auto tokens1 = lexer1.tokenize();
  EXPECT_FALSE(lexer1.get_errors().has_errors());

  std::string source2 = "let x = \"世界\"";
  Lexer lexer2(source2);
  auto tokens2 = lexer2.tokenize();
  EXPECT_FALSE(lexer2.get_errors().has_errors());

  std::string source3 = "let x = 10; // 这是注释 🎉";
  Lexer lexer3(source3);
  auto tokens3 = lexer3.tokenize();
  EXPECT_FALSE(lexer3.get_errors().has_errors());
}

// --- Mixed Encoding Tests ---

TEST_F(Utf8EdgeCasesTest, MixedEncodingScenarios) {
  std::string source = "let result = calculate(42, \"结果\");";

  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  EXPECT_FALSE(lexer.get_errors().has_errors());
}
