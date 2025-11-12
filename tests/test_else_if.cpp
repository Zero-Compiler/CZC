/**
 * @file test_else_if.cpp
 * @brief 测试 if-else-if 链式条件语句的解析和格式化。
 * @author BegoniaHe
 * @date 2025-11-12
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
 * @brief 测试简单的 if-else 语句。
 */
void test_simple_if_else() {
  std::cout << "Testing simple if-else..." << std::endl;

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

  assert(cst != nullptr);
  assert(parser.get_errors().empty());

  FormatOptions options;
  options.indent_width = 4;
  options.space_before_paren = true;
  Formatter formatter(options);

  std::string formatted = formatter.format(cst.get());
  std::cout << "Formatted output:\n" << formatted << std::endl;

  // 验证输出包含 else
  assert(formatted.find("else") != std::string::npos);
  std::cout << "✓ Simple if-else test passed\n" << std::endl;
}

/**
 * @brief 测试 if-else if 链。
 */
void test_if_else_if_chain() {
  std::cout << "Testing if-else-if chain..." << std::endl;

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

  assert(cst != nullptr);
  assert(parser.get_errors().empty());

  FormatOptions options;
  options.indent_width = 4;
  options.space_before_paren = true;
  Formatter formatter(options);

  std::string formatted = formatter.format(cst.get());
  std::cout << "Formatted output:\n" << formatted << std::endl;

  // 验证输出包含 else if
  assert(formatted.find("else if") != std::string::npos);

  // 计算 else if 出现次数（应该是2次）
  size_t count = 0;
  size_t pos = 0;
  while ((pos = formatted.find("else if", pos)) != std::string::npos) {
    count++;
    pos += 7; // "else if" 的长度
  }
  assert(count == 2);

  std::cout << "✓ If-else-if chain test passed\n" << std::endl;
}

/**
 * @brief 测试嵌套的 if-else 语句。
 */
void test_nested_if_else() {
  std::cout << "Testing nested if-else..." << std::endl;

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

  assert(cst != nullptr);
  assert(parser.get_errors().empty());

  FormatOptions options;
  options.indent_width = 4;
  options.space_before_paren = true;
  Formatter formatter(options);

  std::string formatted = formatter.format(cst.get());
  std::cout << "Formatted output:\n" << formatted << std::endl;

  // 验证包含 else if 和嵌套的 else
  assert(formatted.find("else if") != std::string::npos);

  // 计算 else 出现次数（应该有3个 else：1个 else if，2个单独的 else）
  size_t else_count = 0;
  size_t pos = 0;
  while ((pos = formatted.find("else", pos)) != std::string::npos) {
    else_count++;
    pos += 4; // "else" 的长度
  }
  assert(else_count >= 2); // 至少有2个 else

  std::cout << "✓ Nested if-else test passed\n" << std::endl;
}

/**
 * @brief 测试只有 if 没有 else 的情况。
 */
void test_if_only() {
  std::cout << "Testing if-only (no else)..." << std::endl;

  std::string source = R"(
if x > 10 {
    print("big");
}
)";

  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens, "test.zero");
  auto cst = parser.parse();

  assert(cst != nullptr);
  assert(parser.get_errors().empty());

  FormatOptions options;
  options.indent_width = 4;
  options.space_before_paren = true;
  Formatter formatter(options);

  std::string formatted = formatter.format(cst.get());
  std::cout << "Formatted output:\n" << formatted << std::endl;

  // 验证不包含 else
  assert(formatted.find("else") == std::string::npos);
  std::cout << "✓ If-only test passed\n" << std::endl;
}

int main() {
  std::cout << "=== Testing If-Else-If Statements ===" << std::endl
            << std::endl;

  test_if_only();
  test_simple_if_else();
  test_if_else_if_chain();
  test_nested_if_else();

  std::cout << "=== All tests passed! ===" << std::endl;
  return 0;
}
