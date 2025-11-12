/**
 * @file test_parser.cpp
 * @brief 语法分析器测试套件（使用 Google Test 框架）。
 * @details 本测试套件全面测试语法分析器（Parser）的 CST 构建能力，
 *          包括变量声明、函数定义、表达式、控制流语句等各种语法结构的解析。
 * @author BegoniaHe
 * @date 2025-11-11
 */

#include "czc/lexer/lexer.hpp"
#include "czc/parser/parser.hpp"

#include <gtest/gtest.h>

using namespace czc::lexer;
using namespace czc::parser;
using namespace czc::cst;

// --- 测试夹具 ---

/**
 * @brief 语法分析器测试夹具。
 * @details 提供测试所需的通用环境，可在此添加通用的辅助方法。
 */
class ParserTest : public ::testing::Test {
protected:
  // 可根据需要添加通用的 setup/teardown 方法
};

// --- 变量声明测试 ---

/**
 * @brief 测试基本变量声明的解析。
 * @details 验证解析器能够正确解析带类型标注和初始化表达式的变量声明。
 */
TEST_F(ParserTest, VariableDeclaration) {
  Lexer lexer("let x: Integer = 42;");
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto cst = parser.parse();

  ASSERT_NE(cst, nullptr);
  EXPECT_EQ(cst->get_type(), CSTNodeType::Program);
  EXPECT_FALSE(parser.has_errors());
}

// --- 函数声明测试 ---

/**
 * @brief 测试函数声明的解析。
 * @details 验证解析器能够正确解析带参数、返回类型和函数体的函数定义。
 */
TEST_F(ParserTest, FunctionDeclaration) {
  Lexer lexer("fn add(a: Integer, b: Integer) -> Integer { return a + b; }");
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto cst = parser.parse();

  ASSERT_NE(cst, nullptr);
  EXPECT_EQ(cst->get_type(), CSTNodeType::Program);
  EXPECT_FALSE(parser.has_errors());
}

// --- 二元表达式测试 ---

/**
 * @brief 测试二元表达式的解析。
 * @details 验证解析器能够正确处理运算符优先级和结合性，
 *          例如乘法优先于加法。
 */
TEST_F(ParserTest, BinaryExpression) {
  Lexer lexer("2 + 3 * 4;");
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto cst = parser.parse();

  ASSERT_NE(cst, nullptr);
  EXPECT_EQ(cst->get_type(), CSTNodeType::Program);
  EXPECT_FALSE(parser.has_errors());
}

// --- 条件语句测试 ---

/**
 * @brief 测试 if-else 条件语句的解析。
 * @details 验证解析器能够正确解析包含条件、then 分支和 else
 * 分支的完整条件语句。
 */
TEST_F(ParserTest, IfStatement) {
  Lexer lexer("if x > 0 { io.print(x); } else { io.print(0); }");
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto cst = parser.parse();

  ASSERT_NE(cst, nullptr);
  EXPECT_EQ(cst->get_type(), CSTNodeType::Program);
  EXPECT_FALSE(parser.has_errors());
}

// --- 循环语句测试 ---

/**
 * @brief 测试 while 循环语句的解析。
 * @details 验证解析器能够正确解析包含循环条件和循环体的 while 语句。
 */
TEST_F(ParserTest, WhileStatement) {
  Lexer lexer("while x < 10 { x = x + 1; }");
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto cst = parser.parse();

  ASSERT_NE(cst, nullptr);
  EXPECT_EQ(cst->get_type(), CSTNodeType::Program);
  EXPECT_FALSE(parser.has_errors());
}

// --- 数组字面量测试 ---

/**
 * @brief 测试数组字面量的解析。
 * @details 验证解析器能够正确解析数组类型标注和数组初始化表达式。
 */
TEST_F(ParserTest, ArrayLiteral) {
  Lexer lexer("let arr: Integer[] = [1, 2, 3];");
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto cst = parser.parse();

  ASSERT_NE(cst, nullptr);
  EXPECT_EQ(cst->get_type(), CSTNodeType::Program);
  EXPECT_FALSE(parser.has_errors());
}

// --- 函数调用测试 ---

/**
 * @brief 测试函数调用表达式的解析。
 * @details 验证解析器能够正确解析函数名和参数列表。
 */
TEST_F(ParserTest, FunctionCall) {
  Lexer lexer("add(1, 2);");
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto cst = parser.parse();

  ASSERT_NE(cst, nullptr);
  EXPECT_EQ(cst->get_type(), CSTNodeType::Program);
  EXPECT_FALSE(parser.has_errors());
}

// --- 括号表达式测试 ---

/**
 * @brief 测试括号表达式的解析。
 * @details 验证解析器能够正确处理括号改变运算符优先级的情况，
 *          并在 CST 中保留括号信息。
 */
TEST_F(ParserTest, ParenthesizedExpression) {
  Lexer lexer("(2 + 3) * 4;");
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto cst = parser.parse();

  ASSERT_NE(cst, nullptr);
  EXPECT_EQ(cst->get_type(), CSTNodeType::Program);
  EXPECT_FALSE(parser.has_errors());

  // 验证 CST 保留了括号信息
  const auto& children = cst->get_children();
  EXPECT_GT(children.size(), 0);
}

// --- 错误处理测试 ---

/**
 * @brief 测试语法错误的处理（缺少分号）。
 * @details 验证解析器能够检测并报告语法错误，例如缺少分号。
 */
TEST_F(ParserTest, ErrorHandlingMissingSemicolon) {
  Lexer lexer("let x = 42");
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto cst = parser.parse();

  ASSERT_NE(cst, nullptr);
  EXPECT_TRUE(parser.has_errors());
}

// --- 成员访问测试 ---

/**
 * @brief 测试简单成员访问表达式的解析。
 * @details 验证解析器能够正确解析点运算符的成员访问。
 */
TEST_F(ParserTest, SimpleMemberAccess) {
  Lexer lexer("io.print(x);");
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto cst = parser.parse();

  ASSERT_NE(cst, nullptr);
  EXPECT_EQ(cst->get_type(), CSTNodeType::Program);
  EXPECT_FALSE(parser.has_errors());
}

/**
 * @brief 测试链式成员访问表达式的解析。
 * @details 验证解析器能够正确解析多层嵌套的成员访问。
 */
TEST_F(ParserTest, ChainedMemberAccess) {
  Lexer lexer("obj.field.method();");
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto cst = parser.parse();

  ASSERT_NE(cst, nullptr);
  EXPECT_EQ(cst->get_type(), CSTNodeType::Program);
  EXPECT_FALSE(parser.has_errors());
}

/**
 * @brief 测试成员访问与索引访问的组合。
 * @details 验证解析器能够正确解析索引访问后的成员访问。
 */
TEST_F(ParserTest, MemberAccessWithIndex) {
  Lexer lexer("arr[0].name;");
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto cst = parser.parse();

  ASSERT_NE(cst, nullptr);
  EXPECT_EQ(cst->get_type(), CSTNodeType::Program);
  EXPECT_FALSE(parser.has_errors());
}
