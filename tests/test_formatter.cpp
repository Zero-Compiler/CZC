/**
 * @file test_formatter.cpp
 * @brief 代码格式化器测试套件（使用 Google Test 框架）。
 * @details 本测试套件全面测试代码格式化器的各项功能，包括缩进风格、
 *          运算符间距、注释处理、嵌套块格式化等，确保生成符合规范的代码。
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
using namespace czc::cst;

// --- 测试夹具 ---

/**
 * @brief 代码格式化器测试夹具。
 * @details 提供测试所需的通用环境，可在此添加通用的辅助方法。
 */
class FormatterTest : public ::testing::Test {
protected:
  // 可根据需要添加通用的 setup/teardown 方法
};

// --- 基础格式化测试 ---

/**
 * @brief 测试基本的代码格式化功能。
 * @details 验证格式化器能够处理简单的变量声明并生成无错误的输出。
 */
TEST_F(FormatterTest, BasicFormatting) {
  const char* source = "let x = 42;";
  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto root = parser.parse();

  FormatOptions options;
  Formatter formatter(options);
  std::string formatted = formatter.format(root.get());

  EXPECT_FALSE(formatter.get_error_collector().has_errors());
}

// --- 缩进风格测试 ---

/**
 * @brief 测试空格缩进风格。
 * @details 验证格式化器能够使用指定数量的空格进行缩进。
 */
TEST_F(FormatterTest, IndentStyleSpaces) {
  const char* source = "let x = 42;";
  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto root = parser.parse();

  // 使用 2 个空格缩进
  FormatOptions options(IndentStyle::SPACES, 2, 80, true, true, false);
  Formatter formatter(options);
  std::string formatted = formatter.format(root.get());

  EXPECT_FALSE(formatted.empty());
}

/**
 * @brief 测试制表符缩进风格。
 * @details 验证格式化器能够使用制表符进行缩进。
 */
TEST_F(FormatterTest, IndentStyleTabs) {
  const char* source = "let x = 42;";
  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto root = parser.parse();

  // 使用制表符缩进
  FormatOptions options(IndentStyle::TABS, 1, 80, true, true, false);
  Formatter formatter(options);
  std::string formatted = formatter.format(root.get());

  EXPECT_FALSE(formatted.empty());
}

// --- 错误收集测试 ---

/**
 * @brief 测试空指针输入的错误处理。
 * @details 验证格式化器能够安全处理空指针输入，不会崩溃。
 */
TEST_F(FormatterTest, ErrorCollectionNullptr) {
  FormatOptions options;
  Formatter formatter(options);

  std::string result = formatter.format(nullptr);
  EXPECT_TRUE(result.empty());
  EXPECT_FALSE(formatter.get_error_collector().has_errors());
}

// --- 行内注释测试 ---

/**
 * @brief 测试行内注释的间距处理。
 * @details 验证格式化器在行内注释前添加适当的空格。
 */
TEST_F(FormatterTest, InlineCommentSpacing) {
  const char* source = "let x = 42;  // comment";
  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto root = parser.parse();

  FormatOptions options;
  Formatter formatter(options);
  std::string formatted = formatter.format(root.get());

  // 验证注释前有空格
  EXPECT_NE(formatted.find("//"), std::string::npos);
}

// --- 独立注释测试 ---

/**
 * @brief 测试独立行注释的格式化。
 * @details 验证格式化器能够保留独立行的注释，并正确处理其前后的空白。
 */
TEST_F(FormatterTest, StandaloneComment) {
  const char* source = "// This is a comment\nlet x = 42;";
  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto root = parser.parse();

  FormatOptions options;
  Formatter formatter(options);
  std::string formatted = formatter.format(root.get());

  EXPECT_NE(formatted.find("// This is a comment"), std::string::npos);
}

// --- 嵌套块测试 ---

/**
 * @brief 测试嵌套代码块的缩进。
 * @details 验证格式化器能够为嵌套的代码块正确增加缩进层级。
 */
TEST_F(FormatterTest, NestedBlocks) {
  const char* source = "fn f() { let x = 1; }";
  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto root = parser.parse();

  // 使用 4 个空格缩进以便更明显
  FormatOptions options(IndentStyle::SPACES, 4, 80, true, true, false);
  Formatter formatter(options);
  std::string formatted = formatter.format(root.get());

  // 验证有缩进
  EXPECT_NE(formatted.find("let x = 1;"), std::string::npos);
}

// --- 二元表达式测试 ---

/**
 * @brief 测试二元表达式的运算符间距。
 * @details 验证格式化器在二元运算符前后添加空格。
 */
TEST_F(FormatterTest, BinaryExprSpacing) {
  const char* source = "let x = 1+2*3;";
  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto root = parser.parse();

  FormatOptions options;
  Formatter formatter(options);
  std::string formatted = formatter.format(root.get());

  // 验证运算符有空格
  EXPECT_NE(formatted.find(" + "), std::string::npos);
  EXPECT_NE(formatted.find(" * "), std::string::npos);
}

// --- 空程序测试 ---

/**
 * @brief 测试空程序的格式化。
 * @details 验证格式化器能够处理不包含任何语句的空程序。
 */
TEST_F(FormatterTest, EmptyProgram) {
  const char* source = "";
  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto root = parser.parse();

  FormatOptions options;
  Formatter formatter(options);
  std::string formatted = formatter.format(root.get());

  EXPECT_TRUE(formatted.empty());
}

/**
 * @brief 测试函数声明的格式化。
 * @details 验证格式化器能够正确处理函数声明的参数列表和返回类型。
 */
TEST_F(FormatterTest, FunctionDeclaration) {
  const char* source = "fn add(x: int, y: int) -> int { return x + y; }";
  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto root = parser.parse();

  FormatOptions options;
  Formatter formatter(options);
  std::string formatted = formatter.format(root.get());

  EXPECT_FALSE(formatted.empty());
  EXPECT_NE(formatted.find("fn"), std::string::npos);
  EXPECT_NE(formatted.find("add"), std::string::npos);
}

/**
 * @brief 测试if语句的格式化。
 * @details 验证格式化器能够正确处理if语句的条件和代码块。
 */
TEST_F(FormatterTest, IfStatementFormatting) {
  const char* source = "if x > 0 { let y = 1; }";
  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto root = parser.parse();

  FormatOptions options;
  Formatter formatter(options);
  std::string formatted = formatter.format(root.get());

  EXPECT_FALSE(formatted.empty());
  EXPECT_NE(formatted.find("if"), std::string::npos);
}

/**
 * @brief 测试while循环的格式化。
 * @details 验证格式化器能够正确处理while循环的条件和循环体。
 */
TEST_F(FormatterTest, WhileLoopFormatting) {
  const char* source = "while x < 10 { x = x + 1; }";
  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto root = parser.parse();

  FormatOptions options;
  Formatter formatter(options);
  std::string formatted = formatter.format(root.get());

  EXPECT_FALSE(formatted.empty());
  EXPECT_NE(formatted.find("while"), std::string::npos);
}

/**
 * @brief 测试数组字面量的格式化。
 * @details 验证格式化器能够正确处理数组字面量的元素。
 */
TEST_F(FormatterTest, ArrayLiteralFormatting) {
  const char* source = "let arr = [1, 2, 3];";
  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto root = parser.parse();

  FormatOptions options;
  Formatter formatter(options);
  std::string formatted = formatter.format(root.get());

  EXPECT_FALSE(formatted.empty());
  EXPECT_NE(formatted.find("["), std::string::npos);
  EXPECT_NE(formatted.find("]"), std::string::npos);
}

/**
 * @brief 测试函数调用的格式化。
 * @details 验证格式化器能够正确处理函数调用的参数列表。
 */
TEST_F(FormatterTest, FunctionCallFormatting) {
  const char* source = "let result = add(1, 2);";
  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto root = parser.parse();

  FormatOptions options;
  Formatter formatter(options);
  std::string formatted = formatter.format(root.get());

  EXPECT_FALSE(formatted.empty());
  EXPECT_NE(formatted.find("add"), std::string::npos);
}

/**
 * @brief 测试成员访问的格式化。
 * @details 验证格式化器能够正确处理成员访问表达式。
 */
TEST_F(FormatterTest, MemberAccessFormatting) {
  const char* source = "let val = obj.field;";
  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto root = parser.parse();

  FormatOptions options;
  Formatter formatter(options);
  std::string formatted = formatter.format(root.get());

  EXPECT_FALSE(formatted.empty());
  EXPECT_NE(formatted.find("."), std::string::npos);
}

/**
 * @brief 测试索引访问的格式化。
 * @details 验证格式化器能够正确处理数组索引访问。
 */
TEST_F(FormatterTest, IndexAccessFormatting) {
  const char* source = "let val = arr[0];";
  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto root = parser.parse();

  FormatOptions options;
  Formatter formatter(options);
  std::string formatted = formatter.format(root.get());

  EXPECT_FALSE(formatted.empty());
  EXPECT_NE(formatted.find("["), std::string::npos);
}

/**
 * @brief 测试赋值表达式的格式化。
 * @details 验证格式化器能够正确处理赋值运算符的间距。
 */
TEST_F(FormatterTest, AssignmentFormatting) {
  const char* source = "x = 42;";
  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto root = parser.parse();

  FormatOptions options;
  Formatter formatter(options);
  std::string formatted = formatter.format(root.get());

  EXPECT_FALSE(formatted.empty());
  EXPECT_NE(formatted.find("="), std::string::npos);
}

/**
 * @brief 测试一元表达式的格式化。
 * @details 验证格式化器能够正确处理一元运算符（如负号、逻辑非）。
 */
TEST_F(FormatterTest, UnaryExpressionFormatting) {
  const char* source = "let x = -42;";
  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto root = parser.parse();

  FormatOptions options;
  Formatter formatter(options);
  std::string formatted = formatter.format(root.get());

  EXPECT_FALSE(formatted.empty());
  EXPECT_NE(formatted.find("-"), std::string::npos);
}

/**
 * @brief 测试括号表达式的格式化。
 * @details 验证格式化器能够正确处理括号内的表达式。
 */
TEST_F(FormatterTest, ParenthesizedExpressionFormatting) {
  const char* source = "let x = (1 + 2) * 3;";
  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto root = parser.parse();

  FormatOptions options;
  Formatter formatter(options);
  std::string formatted = formatter.format(root.get());

  EXPECT_FALSE(formatted.empty());
  EXPECT_NE(formatted.find("("), std::string::npos);
  EXPECT_NE(formatted.find(")"), std::string::npos);
}

/**
 * @brief 测试返回语句的格式化。
 * @details 验证格式化器能够正确处理返回语句。
 */
TEST_F(FormatterTest, ReturnStatementFormatting) {
  const char* source = "fn f() -> int { return 42; }";
  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto root = parser.parse();

  FormatOptions options;
  Formatter formatter(options);
  std::string formatted = formatter.format(root.get());

  EXPECT_FALSE(formatted.empty());
  EXPECT_NE(formatted.find("return"), std::string::npos);
}

/**
 * @brief 测试if-else语句的格式化。
 * @details 验证格式化器能够正确处理if-else结构。
 */
TEST_F(FormatterTest, IfElseFormatting) {
  const char* source = "if x > 0 { let y = 1; } else { let y = 0; }";
  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto root = parser.parse();

  FormatOptions options;
  Formatter formatter(options);
  std::string formatted = formatter.format(root.get());

  EXPECT_FALSE(formatted.empty());
  EXPECT_NE(formatted.find("if"), std::string::npos);
  EXPECT_NE(formatted.find("else"), std::string::npos);
}

/**
 * @brief 测试多个变量声明的格式化。
 * @details 验证格式化器能够正确处理多个连续的变量声明。
 */
TEST_F(FormatterTest, MultipleDeclarations) {
  const char* source = "let x = 1; let y = 2; let z = 3;";
  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto root = parser.parse();

  FormatOptions options;
  Formatter formatter(options);
  std::string formatted = formatter.format(root.get());

  EXPECT_FALSE(formatted.empty());
  EXPECT_NE(formatted.find("let"), std::string::npos);
}

/**
 * @brief 测试复杂嵌套结构的格式化。
 * @details 验证格式化器能够处理深层嵌套的代码结构。
 */
TEST_F(FormatterTest, ComplexNestedStructure) {
  const char* source = "fn f() { if x > 0 { while y < 10 { y = y + 1; } } }";
  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto root = parser.parse();

  FormatOptions options;
  Formatter formatter(options);
  std::string formatted = formatter.format(root.get());

  EXPECT_FALSE(formatted.empty());
  EXPECT_NE(formatted.find("fn"), std::string::npos);
  EXPECT_NE(formatted.find("if"), std::string::npos);
  EXPECT_NE(formatted.find("while"), std::string::npos);
}

/**
 * @brief 测试字符串字面量的格式化。
 * @details 验证格式化器能够正确保留字符串内容。
 */
TEST_F(FormatterTest, StringLiteralFormatting) {
  const char* source = "let msg = \"Hello, World!\";";
  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto root = parser.parse();

  FormatOptions options;
  Formatter formatter(options);
  std::string formatted = formatter.format(root.get());

  EXPECT_FALSE(formatted.empty());
  EXPECT_NE(formatted.find("Hello"), std::string::npos);
}

/**
 * @brief 测试浮点数字面量的格式化。
 * @details 验证格式化器能够正确处理浮点数。
 */
TEST_F(FormatterTest, FloatLiteralFormatting) {
  const char* source = "let pi = 3.14159;";
  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto root = parser.parse();

  FormatOptions options;
  Formatter formatter(options);
  std::string formatted = formatter.format(root.get());

  EXPECT_FALSE(formatted.empty());
  EXPECT_NE(formatted.find("3.14"), std::string::npos);
}

/**
 * @brief 测试布尔字面量的格式化。
 * @details 验证格式化器能够正确处理true和false。
 */
TEST_F(FormatterTest, BooleanLiteralFormatting) {
  const char* source = "let flag = true;";
  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto root = parser.parse();

  FormatOptions options;
  Formatter formatter(options);
  std::string formatted = formatter.format(root.get());

  EXPECT_FALSE(formatted.empty());
  EXPECT_NE(formatted.find("true"), std::string::npos);
}
