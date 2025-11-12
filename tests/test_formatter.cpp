/**
 * @file test_formatter.cpp
 * @brief 代码格式化器测试套件。
 * @author BegoniaHe
 * @date 2025-11-11
 */

#include "czc/formatter/formatter.hpp"
#include "czc/lexer/lexer.hpp"
#include "czc/parser/parser.hpp"

#include <cassert>
#include <iostream>

using namespace czc::formatter;
using namespace czc::lexer;
using namespace czc::parser;
using namespace czc::cst;

/**
 * @brief 测试基本的格式化功能。
 */
void test_basic_formatting() {
  std::cout << "Testing basic formatting..." << std::endl;

  // 创建一个简单的 CST 用于测试
  const char* source = "let x = 42;";
  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto root = parser.parse();

  // 使用默认选项格式化
  FormatOptions options;
  Formatter formatter(options);
  std::string formatted = formatter.format(root.get());

  std::cout << "Original: " << source << std::endl;
  std::cout << "Formatted: " << formatted << std::endl;

  assert(!formatter.get_error_collector().has_errors() &&
         "Formatting should not produce errors");

  std::cout << "Basic formatting test passed" << std::endl;
}

/**
 * @brief 测试不同的缩进风格。
 */
void test_indent_styles() {
  std::cout << "Testing indent styles..." << std::endl;

  const char* source = "let x = 42;";
  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto root = parser.parse();

  // 测试空格缩进
  FormatOptions spaces_options(IndentStyle::SPACES, 2, 80, true, true, false);
  Formatter spaces_formatter(spaces_options);
  std::string spaces_result = spaces_formatter.format(root.get());

  // 测试制表符缩进
  FormatOptions tabs_options(IndentStyle::TABS, 1, 80, true, true, false);
  Formatter tabs_formatter(tabs_options);
  std::string tabs_result = tabs_formatter.format(root.get());

  std::cout << "Spaces result: " << spaces_result << std::endl;
  std::cout << "Tabs result: " << tabs_result << std::endl;

  std::cout << "Indent styles test passed" << std::endl;
}

/**
 * @brief 测试错误收集功能。
 */
void test_error_collection() {
  std::cout << "Testing error collection..." << std::endl;

  FormatOptions options;
  Formatter formatter(options);

  // 测试空指针
  std::string result = formatter.format(nullptr);
  assert(result.empty() && "Formatting nullptr should return empty string");
  assert(!formatter.get_error_collector().has_errors() &&
         "Formatting nullptr should not produce errors");

  std::cout << "Error collection test passed" << std::endl;
}

/**
 * @brief 测试行内注释格式化。
 */
void test_inline_comment_spacing() {
  std::cout << "Testing inline comment spacing..." << std::endl;

  const char* source = "let x = 42;  // comment";
  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto root = parser.parse();

  FormatOptions options;
  Formatter formatter(options);
  std::string formatted = formatter.format(root.get());

  std::cout << "Original: " << source << std::endl;
  std::cout << "Formatted: " << formatted;

  // 验证注释前有空格（格式化器默认使用3个空格）
  assert(formatted.find("42;   //") != std::string::npos &&
         "Inline comment should have spaces before it");

  std::cout << "Inline comment spacing test passed" << std::endl;
}

/**
 * @brief 测试独立行注释格式化。
 */
void test_standalone_comment() {
  std::cout << "Testing standalone comment..." << std::endl;

  const char* source = "// This is a comment\nlet x = 42;";
  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto root = parser.parse();

  FormatOptions options;
  Formatter formatter(options);
  std::string formatted = formatter.format(root.get());

  std::cout << "Original: " << source << std::endl;
  std::cout << "Formatted: " << formatted;

  // 验证独立行注释在行首（无缩进）
  assert(formatted.find("// This is a comment\n") != std::string::npos &&
         "Standalone comment should be on its own line");

  std::cout << "Standalone comment test passed" << std::endl;
}

/**
 * @brief 测试嵌套代码块缩进。
 */
void test_nested_blocks() {
  std::cout << "Testing nested blocks..." << std::endl;

  const char* source = "fn f() { let x = 1; }";
  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto root = parser.parse();

  FormatOptions options(IndentStyle::SPACES, 4, 80, true, true, false);
  Formatter formatter(options);
  std::string formatted = formatter.format(root.get());

  std::cout << "Original: " << source << std::endl;
  std::cout << "Formatted:\n" << formatted;

  // 验证函数体内有缩进
  assert(formatted.find("    let x = 1;") != std::string::npos &&
         "Function body should be indented with 4 spaces");

  std::cout << "Nested blocks test passed" << std::endl;
}

/**
 * @brief 测试二元表达式空格。
 */
void test_binary_expr_spacing() {
  std::cout << "Testing binary expression spacing..." << std::endl;

  const char* source = "let x = 1+2*3;";
  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto root = parser.parse();

  FormatOptions options;
  Formatter formatter(options);
  std::string formatted = formatter.format(root.get());

  std::cout << "Original: " << source << std::endl;
  std::cout << "Formatted: " << formatted;

  // 验证运算符前后有空格
  assert(formatted.find(" + ") != std::string::npos &&
         "Binary operators should have spaces around them");
  assert(formatted.find(" * ") != std::string::npos &&
         "Binary operators should have spaces around them");

  std::cout << "Binary expression spacing test passed" << std::endl;
}

/**
 * @brief 测试空程序格式化。
 */
void test_empty_program() {
  std::cout << "Testing empty program..." << std::endl;

  const char* source = "";
  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto root = parser.parse();

  FormatOptions options;
  Formatter formatter(options);
  std::string formatted = formatter.format(root.get());

  assert(formatted.empty() && "Empty program should format to empty string");

  std::cout << "Empty program test passed" << std::endl;
}

/**
 * @brief 运行所有测试。
 */
int main() {
  std::cout << "=== Formatter Test Suite ===" << std::endl;

  try {
    test_basic_formatting();
    test_indent_styles();
    test_error_collection();
    test_inline_comment_spacing();
    test_standalone_comment();
    test_nested_blocks();
    test_binary_expr_spacing();
    test_empty_program();

    std::cout << "\nAll formatter tests passed!" << std::endl;
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "✗ Test failed with exception: " << e.what() << std::endl;
    return 1;
  }
}
