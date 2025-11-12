/**
 * @file test_error_recovery.cpp
 * @brief 测试语法分析器的错误恢复机制 (Google Test 版本)。
 * @author BegoniaHe
 * @date 2025-11-11
 */

#include "czc/lexer/lexer.hpp"
#include "czc/parser/parser.hpp"

#include <gtest/gtest.h>

using namespace czc::lexer;
using namespace czc::parser;
using namespace czc::cst;

// --- Test Fixtures ---

class ErrorRecoveryTest : public ::testing::Test {
protected:
  // Empty for now, can add common setup if needed
};

// --- Multiple Errors Tests ---

TEST_F(ErrorRecoveryTest, MultipleErrors) {
  std::string source = R"(
    let x = 10;
    let y  // 缺少分号
    let z = 30;
    fn test( {  // 缺少右括号
      return 42;
    }
    let a = 50;
  )";

  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto tree = parser.parse();

  // 应该报告多个错误
  EXPECT_TRUE(parser.has_errors());
  const auto& errors = parser.get_errors();
  EXPECT_GT(errors.size(), 0);

  // 即使有错误，应该仍然解析出部分结构
  ASSERT_NE(tree, nullptr);
  EXPECT_EQ(tree->get_type(), CSTNodeType::Program);
}

// --- Missing Semicolon Tests ---

TEST_F(ErrorRecoveryTest, MissingSemicolon) {
  std::string source = R"(
    let x = 10
    let y = 20;
  )";

  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto tree = parser.parse();

  EXPECT_TRUE(parser.has_errors());

  // 应该能继续解析第二个变量声明
  ASSERT_NE(tree, nullptr);
  EXPECT_GE(tree->get_children().size(), 2); // 至少有两个声明
}

// --- Missing Identifier Tests ---

TEST_F(ErrorRecoveryTest, MissingIdentifier) {
  std::string source = R"(
    let = 10;
    let y = 20;
  )";

  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto tree = parser.parse();

  EXPECT_TRUE(parser.has_errors());
  ASSERT_NE(tree, nullptr);
}

// --- Missing Function Paren Tests ---

TEST_F(ErrorRecoveryTest, MissingFunctionParen) {
  std::string source = R"(
    fn test {
      return 42;
    }
    fn valid() {
      return 1;
    }
  )";

  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto tree = parser.parse();

  EXPECT_TRUE(parser.has_errors());
  ASSERT_NE(tree, nullptr);
}

// --- Expression Error Recovery Tests ---

TEST_F(ErrorRecoveryTest, ExpressionErrorRecovery) {
  std::string source = R"(
    let x = 10 + ;  // 表达式不完整
    let y = 20;
  )";

  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto tree = parser.parse();

  EXPECT_TRUE(parser.has_errors());
  ASSERT_NE(tree, nullptr);
}

// --- Block Error Recovery Tests ---

TEST_F(ErrorRecoveryTest, BlockErrorRecovery) {
  std::string source = R"(
    fn test() {
      let x = ;  // 错误
      let y = 20;
      return y;
    }
    let z = 30;
  )";

  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto tree = parser.parse();

  EXPECT_TRUE(parser.has_errors());
  ASSERT_NE(tree, nullptr);
}

// --- Complex Multiple Errors Tests ---

TEST_F(ErrorRecoveryTest, ComplexMultipleErrors) {
  std::string source = R"(
    let x = 10;
    let  = 20;      // 错误1: 缺少标识符
    fn test(         // 错误2: 缺少右括号
      let z = 30;
    }
    let a = + 5;    // 错误3: 表达式错误
    fn valid() {
      return 1;
    }
  )";

  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto tree = parser.parse();

  EXPECT_TRUE(parser.has_errors());
  const auto& errors = parser.get_errors();

  // 应该报告多个错误（至少2个）
  EXPECT_GE(errors.size(), 2);
}

// --- Consecutive Errors Tests ---

TEST_F(ErrorRecoveryTest, ConsecutiveErrors) {
  std::string source = R"(
    let x =
    let y =
    let z = 30;
  )";

  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto tree = parser.parse();

  EXPECT_TRUE(parser.has_errors());
  ASSERT_NE(tree, nullptr);
}
