/**
 * @file test_comments.cpp
 * @brief 测试注释功能 (Google Test 版本)。
 * @author BegoniaHe
 * @date 2025-11-11
 */

#include "czc/formatter/formatter.hpp"
#include "czc/lexer/lexer.hpp"
#include "czc/parser/parser.hpp"

#include <fstream>
#include <sstream>

#include "test_helpers.hpp"
#include <gtest/gtest.h>

using namespace czc::lexer;
using namespace czc::parser;
using namespace czc::formatter;
using namespace czc::cst;
using namespace czc::test;

// --- Test Fixtures ---

class CommentsTest : public ::testing::Test {
protected:
  // Empty for now
};

// --- Comment Parsing Tests ---

TEST_F(CommentsTest, CommentTokenization) {
  std::string source = "let x = 10; // comment\nlet y = 20;";

  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  int comment_count = 0;
  for (const auto& token : tokens) {
    if (token.token_type == TokenType::Comment) {
      comment_count++;
    }
  }

  EXPECT_GT(comment_count, 0);
}

TEST_F(CommentsTest, CommentInCST) {
  std::string source = "let x = 10; // comment\nlet y = 20;";

  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto cst = parser.parse();

  ASSERT_NE(cst, nullptr);
  verify_node(cst.get(), CSTNodeType::Program);

  // 验证有两个变量声明
  size_t var_count = count_nodes(cst.get(), CSTNodeType::VarDeclaration);
  EXPECT_EQ(var_count, 2) << "Should have 2 variable declarations";
}

TEST_F(CommentsTest, CommentFormatting) {
  std::string source = "let x = 10; // comment";

  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto cst = parser.parse();

  FormatOptions options;
  Formatter formatter(options);
  std::string formatted = formatter.format(cst.get());

  EXPECT_NE(formatted.find("//"), std::string::npos);
}
