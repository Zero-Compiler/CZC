/**
 * @file test_lexer_gtest.cpp
 * @brief 词法分析器测试套件 (Google Test 版本)。
 * @author BegoniaHe
 * @date 2025-11-11
 */

#include "czc/lexer/lexer.hpp"

#include <vector>

#include <gtest/gtest.h>

using namespace czc::lexer;

// --- Test Fixtures ---

class LexerTest : public ::testing::Test {
protected:
  // Helper function to tokenize and return tokens
  std::vector<Token> tokenize(const std::string& source) {
    Lexer lexer(source);
    return lexer.tokenize();
  }
};

// --- Integer Tests ---

TEST_F(LexerTest, BasicIntegers) {
  auto tokens = tokenize("123 456 789");

  ASSERT_EQ(tokens.size(), 4); // 3 integers + EOF
  EXPECT_EQ(tokens[0].token_type, TokenType::Integer);
  EXPECT_EQ(tokens[0].value, "123");
  EXPECT_EQ(tokens[1].token_type, TokenType::Integer);
  EXPECT_EQ(tokens[1].value, "456");
  EXPECT_EQ(tokens[2].token_type, TokenType::Integer);
  EXPECT_EQ(tokens[2].value, "789");
  EXPECT_EQ(tokens[3].token_type, TokenType::EndOfFile);
}

// --- Float Tests ---

TEST_F(LexerTest, BasicFloats) {
  auto tokens = tokenize("3.14 2.71828 0.5");

  ASSERT_EQ(tokens.size(), 4); // 3 floats + EOF
  EXPECT_EQ(tokens[0].token_type, TokenType::Float);
  EXPECT_EQ(tokens[0].value, "3.14");
  EXPECT_EQ(tokens[1].token_type, TokenType::Float);
  EXPECT_EQ(tokens[1].value, "2.71828");
  EXPECT_EQ(tokens[2].token_type, TokenType::Float);
  EXPECT_EQ(tokens[2].value, "0.5");
  EXPECT_EQ(tokens[3].token_type, TokenType::EndOfFile);
}

// --- Scientific Notation Tests ---

TEST_F(LexerTest, ScientificNotation) {
  auto tokens = tokenize("1.5e10 2.0e-5 3e8");

  ASSERT_EQ(tokens.size(), 4);
  EXPECT_EQ(tokens[0].token_type, TokenType::ScientificExponent);
  EXPECT_EQ(tokens[0].value, "1.5e10");
  EXPECT_EQ(tokens[1].token_type, TokenType::ScientificExponent);
  EXPECT_EQ(tokens[1].value, "2.0e-5");
  EXPECT_EQ(tokens[2].token_type, TokenType::ScientificExponent);
  EXPECT_EQ(tokens[2].value, "3e8");
}

// --- Hex, Binary, Octal Tests ---

TEST_F(LexerTest, HexadecimalNumbers) {
  auto tokens = tokenize("0xFF 0x1A2B 0x0");

  ASSERT_EQ(tokens.size(), 4);
  EXPECT_EQ(tokens[0].token_type, TokenType::Integer);
  EXPECT_EQ(tokens[0].value, "0xFF");
  EXPECT_EQ(tokens[1].token_type, TokenType::Integer);
  EXPECT_EQ(tokens[1].value, "0x1A2B");
  EXPECT_EQ(tokens[2].token_type, TokenType::Integer);
  EXPECT_EQ(tokens[2].value, "0x0");
}

TEST_F(LexerTest, BinaryNumbers) {
  auto tokens = tokenize("0b1010 0b1111 0b0");

  ASSERT_EQ(tokens.size(), 4);
  EXPECT_EQ(tokens[0].token_type, TokenType::Integer);
  EXPECT_EQ(tokens[0].value, "0b1010");
  EXPECT_EQ(tokens[1].token_type, TokenType::Integer);
  EXPECT_EQ(tokens[1].value, "0b1111");
}

TEST_F(LexerTest, OctalNumbers) {
  auto tokens = tokenize("0o755 0o17");

  ASSERT_EQ(tokens.size(), 3);
  EXPECT_EQ(tokens[0].token_type, TokenType::Integer);
  EXPECT_EQ(tokens[0].value, "0o755");
  EXPECT_EQ(tokens[1].token_type, TokenType::Integer);
  EXPECT_EQ(tokens[1].value, "0o17");
}

// --- String Tests ---

TEST_F(LexerTest, BasicStrings) {
  auto tokens = tokenize(R"("hello" "world")");

  ASSERT_EQ(tokens.size(), 3);
  EXPECT_EQ(tokens[0].token_type, TokenType::String);
  EXPECT_EQ(tokens[0].value, "hello");
  EXPECT_EQ(tokens[1].token_type, TokenType::String);
  EXPECT_EQ(tokens[1].value, "world");
}

TEST_F(LexerTest, StringEscapeSequences) {
  auto tokens = tokenize(R"("line1\nline2\ttab")");

  ASSERT_EQ(tokens.size(), 2);
  EXPECT_EQ(tokens[0].token_type, TokenType::String);
  EXPECT_EQ(tokens[0].value, "line1\nline2\ttab");
}

TEST_F(LexerTest, RawStrings) {
  auto tokens = tokenize(R"(r"C:\path\to\file")");

  ASSERT_EQ(tokens.size(), 2);
  EXPECT_EQ(tokens[0].token_type, TokenType::String);
  EXPECT_EQ(tokens[0].value, R"(C:\path\to\file)");
}

// --- Identifier and Keyword Tests ---

TEST_F(LexerTest, Identifiers) {
  auto tokens = tokenize("foo bar baz123 _underscore");

  ASSERT_EQ(tokens.size(), 5);
  EXPECT_EQ(tokens[0].token_type, TokenType::Identifier);
  EXPECT_EQ(tokens[0].value, "foo");
  EXPECT_EQ(tokens[1].token_type, TokenType::Identifier);
  EXPECT_EQ(tokens[1].value, "bar");
  EXPECT_EQ(tokens[2].token_type, TokenType::Identifier);
  EXPECT_EQ(tokens[2].value, "baz123");
  EXPECT_EQ(tokens[3].token_type, TokenType::Identifier);
  EXPECT_EQ(tokens[3].value, "_underscore");
}

TEST_F(LexerTest, Keywords) {
  auto tokens = tokenize("let fn if else while return");

  ASSERT_EQ(tokens.size(), 7);
  EXPECT_EQ(tokens[0].token_type, TokenType::Let);
  EXPECT_EQ(tokens[1].token_type, TokenType::Fn);
  EXPECT_EQ(tokens[2].token_type, TokenType::If);
  EXPECT_EQ(tokens[3].token_type, TokenType::Else);
  EXPECT_EQ(tokens[4].token_type, TokenType::While);
  EXPECT_EQ(tokens[5].token_type, TokenType::Return);
}

// --- Operator Tests ---

TEST_F(LexerTest, ArithmeticOperators) {
  auto tokens = tokenize("+ - * / %");

  ASSERT_EQ(tokens.size(), 6);
  EXPECT_EQ(tokens[0].token_type, TokenType::Plus);
  EXPECT_EQ(tokens[1].token_type, TokenType::Minus);
  EXPECT_EQ(tokens[2].token_type, TokenType::Star);
  EXPECT_EQ(tokens[3].token_type, TokenType::Slash);
  EXPECT_EQ(tokens[4].token_type, TokenType::Percent);
}

TEST_F(LexerTest, ComparisonOperators) {
  auto tokens = tokenize("== != < > <= >=");

  ASSERT_EQ(tokens.size(), 7);
  EXPECT_EQ(tokens[0].token_type, TokenType::EqualEqual);
  EXPECT_EQ(tokens[1].token_type, TokenType::BangEqual);
  EXPECT_EQ(tokens[2].token_type, TokenType::Less);
  EXPECT_EQ(tokens[3].token_type, TokenType::Greater);
  EXPECT_EQ(tokens[4].token_type, TokenType::LessEqual);
  EXPECT_EQ(tokens[5].token_type, TokenType::GreaterEqual);
}

// --- Comment Tests ---

TEST_F(LexerTest, SingleLineComments) {
  auto tokens = tokenize("123 // this is a comment\n456");

  ASSERT_EQ(tokens.size(), 4); // 123, comment, 456, EOF
  EXPECT_EQ(tokens[0].token_type, TokenType::Integer);
  EXPECT_EQ(tokens[0].value, "123");
  EXPECT_EQ(tokens[1].token_type, TokenType::Comment);
  EXPECT_EQ(tokens[2].token_type, TokenType::Integer);
  EXPECT_EQ(tokens[2].value, "456");
}

// TODO: Multi-line comments (/* */) are not yet implemented in the lexer
// TEST_F(LexerTest, MultiLineComments) {
//   auto tokens = tokenize("123 /* comment\nspanning\nlines */ 456");
//
//   ASSERT_EQ(tokens.size(), 4); // 123, comment, 456, EOF
//   EXPECT_EQ(tokens[0].token_type, TokenType::Integer);
//   EXPECT_EQ(tokens[0].value, "123");
//   EXPECT_EQ(tokens[1].token_type, TokenType::Comment);
//   EXPECT_EQ(tokens[2].token_type, TokenType::Integer);
//   EXPECT_EQ(tokens[2].value, "456");
// }

// --- UTF-8 Tests ---

TEST_F(LexerTest, UTF8Identifiers) {
  auto tokens = tokenize("变量 función переменная");

  ASSERT_EQ(tokens.size(), 4);
  EXPECT_EQ(tokens[0].token_type, TokenType::Identifier);
  EXPECT_EQ(tokens[0].value, "变量");
  EXPECT_EQ(tokens[1].token_type, TokenType::Identifier);
  EXPECT_EQ(tokens[1].value, "función");
  EXPECT_EQ(tokens[2].token_type, TokenType::Identifier);
  EXPECT_EQ(tokens[2].value, "переменная");
}

TEST_F(LexerTest, UTF8Strings) {
  auto tokens = tokenize(R"("你好" "🌍" "Привет")");

  ASSERT_EQ(tokens.size(), 4);
  EXPECT_EQ(tokens[0].token_type, TokenType::String);
  EXPECT_EQ(tokens[0].value, "你好");
  EXPECT_EQ(tokens[1].token_type, TokenType::String);
  EXPECT_EQ(tokens[1].value, "🌍");
  EXPECT_EQ(tokens[2].token_type, TokenType::String);
  EXPECT_EQ(tokens[2].value, "Привет");
}

// --- Error Cases ---

TEST_F(LexerTest, UnterminatedString) {
  Lexer lexer(R"("unterminated)");
  auto tokens = lexer.tokenize();
  auto errors = lexer.get_errors();

  EXPECT_TRUE(errors.has_errors());
  // The lexer should report an error for unterminated string
}

TEST_F(LexerTest, InvalidHexNumber) {
  Lexer lexer("0x");
  auto tokens = lexer.tokenize();
  auto errors = lexer.get_errors();

  EXPECT_TRUE(errors.has_errors());
}

TEST_F(LexerTest, InvalidEscapeSequence) {
  Lexer lexer(R"("\q")");
  auto tokens = lexer.tokenize();
  auto errors = lexer.get_errors();

  EXPECT_TRUE(errors.has_errors());
}
