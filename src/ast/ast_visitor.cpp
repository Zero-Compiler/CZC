/**
 * @file ast_visitor.cpp
 * @brief AST 访问者实现
 * @author BegoniaHe
 * @date 2025-11-13
 */

#include "czc/ast/ast_visitor.hpp"

#include "czc/ast/ast_node.hpp"

#include <iostream>

namespace czc {
namespace ast {

// === ASTPrinter 实现 ===

void ASTPrinter::print_indent() {
  for (int i = 0; i < indent_level_; i++) {
    std::cout << "  ";
  }
}

void ASTPrinter::visit_program(Program* node) {
  print_indent();
  std::cout << "Program" << std::endl;

  increase_indent();
  for (const auto& decl : node->get_declarations()) {
    // TODO: 访问每个声明
    print_indent();
    std::cout << "Declaration (TODO)" << std::endl;
  }
  decrease_indent();
}

void ASTPrinter::visit_identifier(Identifier* node) {
  print_indent();
  std::cout << "Identifier: " << node->get_name() << std::endl;
}

void ASTPrinter::visit_integer_literal(IntegerLiteral* node) {
  print_indent();
  std::cout << "IntegerLiteral: " << node->get_value() << std::endl;
}

void ASTPrinter::visit_binary_op(BinaryOpExpr* node) {
  print_indent();
  std::cout << "BinaryOp" << std::endl;

  increase_indent();

  print_indent();
  std::cout << "Left:" << std::endl;
  increase_indent();
  // TODO: 访问左操作数
  decrease_indent();

  print_indent();
  std::cout << "Right:" << std::endl;
  increase_indent();
  // TODO: 访问右操作数
  decrease_indent();

  decrease_indent();
}

void ASTPrinter::visit_block_stmt(BlockStmt* node) {
  print_indent();
  std::cout << "BlockStmt" << std::endl;

  increase_indent();
  for (const auto& stmt : node->get_statements()) {
    // TODO: 访问每个语句
    print_indent();
    std::cout << "Statement (TODO)" << std::endl;
  }
  decrease_indent();
}

} // namespace ast
} // namespace czc
