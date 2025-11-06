/**
 * @file cst_node.cpp
 * @brief `CSTNode` 类及相关工具函数的实现。
 * @author BegoniaHe
 * @date 2025-11-05
 */

#include "czc/cst/cst_node.hpp"

namespace czc {
namespace cst {

CSTNode::CSTNode(CSTNodeType type, const utils::SourceLocation &location)
    : node_type(type), location(location), children(), token(), value() {}

void CSTNode::add_child(std::unique_ptr<CSTNode> child) {
  // NOTE: 使用 emplace_back 和 std::move 可以最高效地将 unique_ptr 的所有权
  //       转移到 vector 中，避免了不必要的内存分配或拷贝操作。
  children.emplace_back(std::move(child));
}

void CSTNode::set_token(const lexer::Token &tok) { token = tok; }

std::string cst_node_type_to_string(CSTNodeType type) {
  switch (type) {
  case CSTNodeType::Program:
    return "Program";
  case CSTNodeType::VarDeclaration:
    return "VarDeclaration";
  case CSTNodeType::FnDeclaration:
    return "FnDeclaration";
  case CSTNodeType::ReturnStmt:
    return "ReturnStmt";
  case CSTNodeType::IfStmt:
    return "IfStmt";
  case CSTNodeType::WhileStmt:
    return "WhileStmt";
  case CSTNodeType::BlockStmt:
    return "BlockStmt";
  case CSTNodeType::ExprStmt:
    return "ExprStmt";
  case CSTNodeType::BinaryExpr:
    return "BinaryExpr";
  case CSTNodeType::UnaryExpr:
    return "UnaryExpr";
  case CSTNodeType::CallExpr:
    return "CallExpr";
  case CSTNodeType::IndexExpr:
    return "IndexExpr";
  case CSTNodeType::MemberExpr:
    return "MemberExpr";
  case CSTNodeType::AssignExpr:
    return "AssignExpr";
  case CSTNodeType::IndexAssignExpr:
    return "IndexAssignExpr";
  case CSTNodeType::ArrayLiteral:
    return "ArrayLiteral";
  case CSTNodeType::IntegerLiteral:
    return "IntegerLiteral";
  case CSTNodeType::FloatLiteral:
    return "FloatLiteral";
  case CSTNodeType::StringLiteral:
    return "StringLiteral";
  case CSTNodeType::BooleanLiteral:
    return "BooleanLiteral";
  case CSTNodeType::Identifier:
    return "Identifier";
  case CSTNodeType::ParenExpr:
    return "ParenExpr";
  case CSTNodeType::TypeAnnotation:
    return "TypeAnnotation";
  case CSTNodeType::ArrayType:
    return "ArrayType";
  case CSTNodeType::Parameter:
    return "Parameter";
  case CSTNodeType::ParameterList:
    return "ParameterList";
  case CSTNodeType::ArgumentList:
    return "ArgumentList";
  case CSTNodeType::StatementList:
    return "StatementList";
  case CSTNodeType::Operator:
    return "Operator";
  case CSTNodeType::Delimiter:
    return "Delimiter";
  default:
    return "Unknown";
  }
}

std::unique_ptr<CSTNode> make_cst_node(CSTNodeType type,
                                       const utils::SourceLocation &location) {
  return std::make_unique<CSTNode>(type, location);
}

std::unique_ptr<CSTNode> make_cst_node(CSTNodeType type,
                                       const lexer::Token &token) {
  // NOTE: 从 Token 创建 CST 节点时，我们只关心其起始位置。
  //       文件名此时未知，因此留空，将在更高层级的组件中填充。
  auto location = utils::SourceLocation("", token.line, token.column);
  auto node = std::make_unique<CSTNode>(type, location);
  node->set_token(token);
  return node;
}

} // namespace cst
} // namespace czc
