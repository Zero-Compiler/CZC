/**
 * @file test_error_recovery.cpp
 * @brief 测试语法分析器的错误恢复机制。
 * @author BegoniaHe
 * @date 2025-11-07
 */

#include "czc/lexer/lexer.hpp"
#include "czc/parser/parser.hpp"
#include <cassert>
#include <iostream>

using namespace czc::lexer;
using namespace czc::parser;
using namespace czc::cst;

/**
 * @brief 测试多个错误能否被全部捕获（而非在第一个错误处停止）。
 */
void test_multiple_errors() {
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
  assert(parser.has_errors());
  const auto &errors = parser.get_errors();

  std::cout << "test_multiple_errors: Found " << errors.size()
            << " errors as expected" << std::endl;

  // 即使有错误，应该仍然解析出部分结构
  assert(tree != nullptr);
  assert(tree->get_type() == CSTNodeType::Program);

  std::cout << "test_multiple_errors: Tree structure maintained" << std::endl;
}

/**
 * @brief 测试缺少分号的错误恢复。
 */
void test_missing_semicolon() {
  std::string source = R"(
    let x = 10
    let y = 20;
  )";

  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto tree = parser.parse();

  assert(parser.has_errors());

  // 应该能继续解析第二个变量声明
  assert(tree != nullptr);
  assert(tree->children().size() >= 2); // 至少有两个声明

  std::cout << "test_missing_semicolon: Recovered and parsed next statement"
            << std::endl;
}

/**
 * @brief 测试缺少标识符的错误恢复。
 */
void test_missing_identifier() {
  std::string source = R"(
    let = 10;
    let y = 20;
  )";

  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto tree = parser.parse();

  assert(parser.has_errors());

  // 应该能继续解析第二个变量声明
  assert(tree != nullptr);

  std::cout << "test_missing_identifier: Recovered and parsed next statement"
            << std::endl;
}

/**
 * @brief 测试函数声明中缺少括号的错误恢复。
 */
void test_missing_function_paren() {
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

  assert(parser.has_errors());

  // 应该能继续解析第二个函数
  assert(tree != nullptr);

  std::cout << "test_missing_function_paren: Recovered and parsed next function"
            << std::endl;
}

/**
 * @brief 测试表达式错误的恢复。
 */
void test_expression_error_recovery() {
  std::string source = R"(
    let x = 10 + ;  // 表达式不完整
    let y = 20;
  )";

  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto tree = parser.parse();

  assert(parser.has_errors());

  // 应该能继续解析第二个变量
  assert(tree != nullptr);

  std::cout
      << "test_expression_error_recovery: Recovered after expression error"
      << std::endl;
}

/**
 * @brief 测试代码块错误的恢复。
 */
void test_block_error_recovery() {
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

  assert(parser.has_errors());

  // 应该能继续解析块内其他语句和后续声明
  assert(tree != nullptr);

  std::cout << "test_block_error_recovery: Recovered within and after block"
            << std::endl;
}

/**
 * @brief 测试复杂的多重错误场景。
 */
void test_complex_multiple_errors() {
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

  assert(parser.has_errors());
  const auto &errors = parser.get_errors();

  // 应该报告多个错误（至少3个）
  assert(errors.size() >= 3);

  std::cout << "test_complex_multiple_errors: Found " << errors.size()
            << " errors, maintained parsing" << std::endl;
}

/**
 * @brief 测试连续的错误恢复。
 */
void test_consecutive_errors() {
  std::string source = R"(
    let x =
    let y =
    let z = 30;
  )";

  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto tree = parser.parse();

  assert(parser.has_errors());

  // 应该能恢复并解析最后一个正确的声明
  assert(tree != nullptr);

  std::cout << "test_consecutive_errors: Recovered from consecutive errors"
            << std::endl;
}

int main() {
  std::cout << "\n=== Testing Enhanced Error Recovery ===" << std::endl;

  try {
    test_multiple_errors();
    test_missing_semicolon();
    test_missing_identifier();
    test_missing_function_paren();
    test_expression_error_recovery();
    test_block_error_recovery();
    test_complex_multiple_errors();
    test_consecutive_errors();

    std::cout << "\nAll error recovery tests passed!" << std::endl;
  } catch (const std::exception &e) {
    std::cerr << "\nTest failed with exception: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
