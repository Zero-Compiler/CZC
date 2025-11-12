/**
 * @file test_else_if.cpp
 * @brief 测试 if-else-if 链式条件语句的解析和格式化 (Google Test 版本)。
 * @author BegoniaHe
 * @date 2025-11-12
 */

#include "czc/formatter/formatter.hpp"
#include "czc/lexer/lexer.hpp"
#include "czc/parser/parser.hpp"

#include <gtest/gtest.h>

using namespace czc::formatter;
using namespace czc::lexer;
using namespace czc::parser;
using namespace czc::cst;

// --- Test Fixtures ---

class ElseIfTest : public ::testing::Test {
protected:
  // Empty for now
};

// --- If-Else Tests ---

TEST_F(ElseIfTest, SimpleIfElse) {
  std::string source = R"(
if x > 10 {
    print("big");
} else {
    print("small");
}
)";

  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens, "test.zero");
  auto cst = parser.parse();

  ASSERT_NE(cst, nullptr);
  EXPECT_TRUE(parser.get_errors().empty());

  FormatOptions options;
  options.indent_width = 4;
  options.space_before_paren = true;
  Formatter formatter(options);

  std::string formatted = formatter.format(cst.get());
  EXPECT_NE(formatted.find("else"), std::string::npos);
}

TEST_F(ElseIfTest, IfElseIfChain) {
  std::string source = R"(
if score >= 90 {
    print("A");
} else if score >= 80 {
    print("B");
} else if score >= 70 {
    print("C");
} else {
    print("F");
}
)";

  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens, "test.zero");
  auto cst = parser.parse();

  ASSERT_NE(cst, nullptr);
  EXPECT_TRUE(parser.get_errors().empty());

  FormatOptions options;
  options.indent_width = 4;
  options.space_before_paren = true;
  Formatter formatter(options);

  std::string formatted = formatter.format(cst.get());
  EXPECT_NE(formatted.find("else if"), std::string::npos);
}

TEST_F(ElseIfTest, NestedIfElse) {
  std::string source = R"(
if x > y {
    print("x greater");
} else if x < y {
    if y > 8 {
        print("nested true");
    } else {
        print("nested false");
    }
} else {
    print("equal");
}
)";

  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens, "test.zero");
  auto cst = parser.parse();

  ASSERT_NE(cst, nullptr);
  EXPECT_TRUE(parser.get_errors().empty());

  FormatOptions options;
  options.indent_width = 4;
  options.space_before_paren = true;
  Formatter formatter(options);

  std::string formatted = formatter.format(cst.get());
  EXPECT_NE(formatted.find("else if"), std::string::npos);
}

TEST_F(ElseIfTest, IfOnlyNoElse) {
  std::string source = R"(
if x > 10 {
    print("big");
}
)";

  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens, "test.zero");
  auto cst = parser.parse();

  ASSERT_NE(cst, nullptr);
  EXPECT_TRUE(parser.get_errors().empty());

  FormatOptions options;
  options.indent_width = 4;
  options.space_before_paren = true;
  Formatter formatter(options);

  std::string formatted = formatter.format(cst.get());
  EXPECT_EQ(formatted.find("else"), std::string::npos);
}
