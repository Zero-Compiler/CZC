/**
 * @file test_synthetic_tokens.cpp
 * @brief 测试虚拟 Token（synthetic tokens）的处理。
 * @author BegoniaHe
 * @date 2025-11-07
 */

#include "czc/formatter/formatter.hpp"
#include "czc/lexer/lexer.hpp"
#include "czc/parser/parser.hpp"
#include <cassert>
#include <iostream>

using namespace czc::formatter;
using namespace czc::lexer;
using namespace czc::parser;

/**
 * @brief 测试虚拟分号不会被格式化输出。
 */
void test_synthetic_semicolon_not_output() {
  std::string source = R"(
let x = 10
let y = 20;
)";

  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto tree = parser.parse();

  assert(parser.has_errors() && "应该检测到缺少分号的错误");

  // 格式化
  FormatOptions options;
  Formatter formatter(options);
  std::string formatted = formatter.format(tree.get());

  // 虚拟分号不应该被添加到输出中
  // 输出应该仍然缺少第一个分号
  std::cout << "Formatted output:\n" << formatted << std::endl;

  // 统计分号数量
  size_t semicolon_count = 0;
  for (char c : formatted) {
    if (c == ';') {
      semicolon_count++;
    }
  }

  assert(semicolon_count == 1 &&
         "虚拟分号不应该被输出，应该只有一个真实的分号");

  std::cout << "test_synthetic_semicolon_not_output: 虚拟分号没有被输出"
            << std::endl;
}

/**
 * @brief 测试虚拟括号不会被格式化输出。
 */
void test_synthetic_paren_not_output() {
  std::string source = R"(
fn test {
  return 42;
}
)";

  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto tree = parser.parse();

  assert(parser.has_errors() && "应该检测到缺少括号的错误");

  // 格式化
  FormatOptions options;
  Formatter formatter(options);
  std::string formatted = formatter.format(tree.get());

  std::cout << "Formatted output:\n" << formatted << std::endl;

  // 虚拟括号不应该被添加
  // 原始代码缺少 ()，格式化后也不应该自动添加
  size_t left_paren_count = 0;
  size_t right_paren_count = 0;
  for (char c : formatted) {
    if (c == '(')
      left_paren_count++;
    if (c == ')')
      right_paren_count++;
  }

  assert(left_paren_count == 0 && right_paren_count == 0 &&
         "虚拟括号不应该被输出");

  std::cout << "test_synthetic_paren_not_output: 虚拟括号没有被输出"
            << std::endl;
}

/**
 * @brief 测试真实 Token 仍然被正常输出。
 */
void test_real_tokens_output() {
  std::string source = R"(
let x = 10;
let y = 20;
)";

  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto tree = parser.parse();

  assert(!parser.has_errors() && "正确的代码不应该有错误");

  // 格式化
  FormatOptions options;
  Formatter formatter(options);
  std::string formatted = formatter.format(tree.get());

  std::cout << "Formatted output:\n" << formatted << std::endl;

  // 应该有两个真实的分号
  size_t semicolon_count = 0;
  for (char c : formatted) {
    if (c == ';') {
      semicolon_count++;
    }
  }

  assert(semicolon_count == 2 && "两个真实的分号应该都被输出");

  std::cout << "test_real_tokens_output: 真实 Token 正常输出" << std::endl;
}

/**
 * @brief 测试混合场景：有真实 Token 也有虚拟 Token。
 */
void test_mixed_tokens() {
  std::string source = R"(
let x = 10;
let y = 20
let z = 30;
)";

  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto tree = parser.parse();

  assert(parser.has_errors() && "第二行缺少分号");

  // 格式化
  FormatOptions options;
  Formatter formatter(options);
  std::string formatted = formatter.format(tree.get());

  std::cout << "Formatted output:\n" << formatted << std::endl;

  // 应该只有两个真实的分号（第1行和第3行）
  size_t semicolon_count = 0;
  for (char c : formatted) {
    if (c == ';') {
      semicolon_count++;
    }
  }

  assert(semicolon_count == 2 && "只有真实的分号应该被输出");

  std::cout << "test_mixed_tokens: 混合场景中虚拟 Token 被正确过滤"
            << std::endl;
}

int main() {
  std::cout << "\n=== Testing Synthetic Token Handling ===" << std::endl;

  try {
    test_synthetic_semicolon_not_output();
    test_synthetic_paren_not_output();
    test_real_tokens_output();
    test_mixed_tokens();

    std::cout << "\nAll synthetic token tests passed!" << std::endl;
  } catch (const std::exception &e) {
    std::cerr << "\nTest failed with exception: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
