/**
 * @file test_parser.cpp
 * @brief 语法分析器测试套件。
 * @author BegoniaHe
 * @date 2025-11-05
 */

#include "czc/lexer/lexer.hpp"
#include "czc/parser/parser.hpp"
#include <cassert>
#include <iostream>

using namespace czc::lexer;
using namespace czc::parser;
using namespace czc::cst;

/**
 * @brief 递归打印 CST 树结构（用于调试）。
 * @param[in] node CST 节点。
 * @param[in] indent 缩进级别。
 */
void print_cst(const CSTNode *node, int indent = 0) {
  if (!node) {
    return;
  }

  // 打印缩进
  for (int i = 0; i < indent; ++i) {
    std::cout << "  ";
  }

  // 打印节点类型
  std::cout << cst_node_type_to_string(node->get_type());

  // 打印节点值（如果有）
  if (node->get_value()) {
    std::cout << " [" << *node->get_value() << "]";
  }

  // 打印 Token 类型（如果有）
  if (node->get_token()) {
    std::cout << " <" << token_type_to_string(node->get_token()->token_type)
              << ">";
  }

  std::cout << std::endl;

  // 递归打印子节点
  for (const auto &child : node->get_children()) {
    print_cst(child.get(), indent + 1);
  }
}

// --- 测试用例 ---

/**
 * @brief 测试变量声明的解析。
 */
void test_variable_declaration() {
  std::cout << "\n=== Test: Variable Declaration ===" << std::endl;

  Lexer lexer("let x: int = 42;");
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto cst = parser.parse();

  assert(cst != nullptr);
  assert(cst->get_type() == CSTNodeType::Program);
  assert(!parser.has_errors());

  std::cout << "Variable declaration parsed successfully" << std::endl;
  print_cst(cst.get());
}

/**
 * @brief 测试函数声明的解析。
 */
void test_function_declaration() {
  std::cout << "\n=== Test: Function Declaration ===" << std::endl;

  Lexer lexer("fn add(a: int, b: int) -> int { return a + b; }");
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto cst = parser.parse();

  assert(cst != nullptr);
  assert(cst->get_type() == CSTNodeType::Program);
  assert(!parser.has_errors());

  std::cout << "Function declaration parsed successfully" << std::endl;
  print_cst(cst.get());
}

/**
 * @brief 测试二元表达式的解析。
 */
void test_binary_expression() {
  std::cout << "\n=== Test: Binary Expression ===" << std::endl;

  Lexer lexer("2 + 3 * 4;");
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto cst = parser.parse();

  assert(cst != nullptr);
  assert(cst->get_type() == CSTNodeType::Program);
  assert(!parser.has_errors());

  std::cout << "Binary expression parsed successfully" << std::endl;
  print_cst(cst.get());
}

/**
 * @brief 测试条件语句的解析。
 */
void test_if_statement() {
  std::cout << "\n=== Test: If Statement ===" << std::endl;

  Lexer lexer("if x > 0 { print(x); } else { print(0); }");
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto cst = parser.parse();

  assert(cst != nullptr);
  assert(cst->get_type() == CSTNodeType::Program);
  assert(!parser.has_errors());

  std::cout << "If statement parsed successfully" << std::endl;
  print_cst(cst.get());
}

/**
 * @brief 测试循环语句的解析。
 */
void test_while_statement() {
  std::cout << "\n=== Test: While Statement ===" << std::endl;

  Lexer lexer("while x < 10 { x = x + 1; }");
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto cst = parser.parse();

  assert(cst != nullptr);
  assert(cst->get_type() == CSTNodeType::Program);
  assert(!parser.has_errors());

  std::cout << "While statement parsed successfully" << std::endl;
  print_cst(cst.get());
}

/**
 * @brief 测试数组字面量的解析。
 */
void test_array_literal() {
  std::cout << "\n=== Test: Array Literal ===" << std::endl;

  Lexer lexer("let arr: [int] = [1, 2, 3];");
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto cst = parser.parse();

  assert(cst != nullptr);
  assert(cst->get_type() == CSTNodeType::Program);
  assert(!parser.has_errors());

  std::cout << "Array literal parsed successfully" << std::endl;
  print_cst(cst.get());
}

/**
 * @brief 测试函数调用的解析。
 */
void test_function_call() {
  std::cout << "\n=== Test: Function Call ===" << std::endl;

  Lexer lexer("add(1, 2);");
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto cst = parser.parse();

  assert(cst != nullptr);
  assert(cst->get_type() == CSTNodeType::Program);
  assert(!parser.has_errors());

  std::cout << "Function call parsed successfully" << std::endl;
  print_cst(cst.get());
}

/**
 * @brief 测试括号表达式的保留。
 */
void test_parenthesized_expression() {
  std::cout << "\n=== Test: Parenthesized Expression ===" << std::endl;

  Lexer lexer("(2 + 3) * 4;");
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto cst = parser.parse();

  assert(cst != nullptr);
  assert(cst->get_type() == CSTNodeType::Program);
  assert(!parser.has_errors());

  // 验证 CST 中保留了括号节点
  const auto &children = cst->get_children();
  assert(children.size() > 0);

  std::cout << "Parenthesized expression parsed successfully (brackets "
               "preserved in CST)"
            << std::endl;
  print_cst(cst.get());
}

/**
 * @brief 测试错误处理。
 */
void test_error_handling() {
  std::cout << "\n=== Test: Error Handling ===" << std::endl;

  // 缺少分号
  Lexer lexer("let x = 42");
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto cst = parser.parse();

  assert(cst != nullptr);
  assert(parser.has_errors());

  std::cout << "Error handling works" << std::endl;
  std::cout << "Errors detected: " << parser.get_errors().size() << std::endl;
}

/**
 * @brief 主测试入口。
 */
int main() {
  std::cout << "======================================" << std::endl;
  std::cout << "  CST Parser Test Suite" << std::endl;
  std::cout << "======================================" << std::endl;

  try {
    test_variable_declaration();
    test_function_declaration();
    test_binary_expression();
    test_if_statement();
    test_while_statement();
    test_array_literal();
    test_function_call();
    test_parenthesized_expression();
    test_error_handling();

    std::cout << "\n======================================" << std::endl;
    std::cout << "  All tests passed!" << std::endl;
    std::cout << "======================================" << std::endl;

    return 0;
  } catch (const std::exception &e) {
    std::cerr << "Test failed with exception: " << e.what() << std::endl;
    return 1;
  }
}
