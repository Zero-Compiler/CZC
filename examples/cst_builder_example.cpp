/**
 * @file cst_builder_example.cpp
 * @brief CSTBuilder 使用示例
 * @details 展示如何使用 CSTBuilder 解耦 CST 节点创建
 * @author BegoniaHe
 * @date 2025-11-14
 */

#include "czc/cst/cst_builder.hpp"
#include "czc/lexer/token.hpp"

#include <iostream>

using namespace czc::cst;
using namespace czc::lexer;
using namespace czc::utils;

int main() {
  // 示例 1: 创建程序根节点
  auto program = CSTBuilder::create_program(SourceLocation("example.zero", 1, 1));
  std::cout << "Created program node" << std::endl;

  // 示例 2: 创建变量声明
  auto var_decl = CSTBuilder::create_var_declaration(SourceLocation("example.zero", 2, 1));
  
  // 示例 3: 创建标识符
  Token id_token(TokenType::Identifier, "my_var", 2, 5);
  auto identifier = CSTBuilder::create_identifier(id_token);
  
  // 示例 4: 添加子节点
  CSTBuilder::add_child(var_decl.get(), std::move(identifier));
  CSTBuilder::add_child(program.get(), std::move(var_decl));
  
  std::cout << "Built CST with " << program->get_children().size() << " declarations" << std::endl;
  
  return 0;
}
