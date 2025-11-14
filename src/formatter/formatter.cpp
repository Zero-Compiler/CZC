/**
 * @file formatter.cpp
 * @brief `Formatter` 类的功能实现。
 * @author BegoniaHe
 * @date 2025-11-11
 */

#include "czc/formatter/formatter.hpp"

#include <cstdio>
#include <sstream>

namespace czc::formatter {

Formatter::Formatter(const FormatOptions& options)
    : options(options), error_collector(), indent_level(0) {}

std::string Formatter::format(const cst::CSTNode* root) {
  if (!root) {
    return "";
  }
  indent_level = 0;
  error_collector.clear();
  return format_node(root);
}

std::string Formatter::format_node(const cst::CSTNode* node) {
  if (!node) {
    return "";
  }

  // 使用访问者模式分派到具体的 visit 方法
  switch (node->get_type()) {
  case cst::CSTNodeType::Program:
    return visit_program(node);
  case cst::CSTNodeType::VarDeclaration:
    return visit_var_declaration(node);
  case cst::CSTNodeType::FnDeclaration:
    return visit_fn_declaration(node);
  case cst::CSTNodeType::StructDeclaration:
    return visit_struct_declaration(node);
  case cst::CSTNodeType::TypeAliasDeclaration:
    return visit_type_alias_declaration(node);
  case cst::CSTNodeType::ReturnStmt:
    return visit_return_stmt(node);
  case cst::CSTNodeType::IfStmt:
    return visit_if_stmt(node);
  case cst::CSTNodeType::WhileStmt:
    return visit_while_stmt(node);
  case cst::CSTNodeType::BlockStmt:
    return visit_block_stmt(node);
  case cst::CSTNodeType::ExprStmt:
    return visit_expr_stmt(node);
  case cst::CSTNodeType::BinaryExpr:
    return visit_binary_expr(node);
  case cst::CSTNodeType::UnaryExpr:
    return visit_unary_expr(node);
  case cst::CSTNodeType::CallExpr:
    return visit_call_expr(node);
  case cst::CSTNodeType::IndexExpr:
    return visit_index_expr(node);
  case cst::CSTNodeType::MemberExpr:
    return visit_member_expr(node);
  case cst::CSTNodeType::AssignExpr:
    return visit_assign_expr(node);
  case cst::CSTNodeType::IndexAssignExpr:
    return visit_index_assign_expr(node);
  case cst::CSTNodeType::MemberAssignExpr:
    return visit_member_assign_expr(node);
  case cst::CSTNodeType::ArrayLiteral:
    return visit_array_literal(node);
  case cst::CSTNodeType::TupleLiteral:
    return visit_tuple_literal(node);
  case cst::CSTNodeType::FunctionLiteral:
    return visit_function_literal(node);
  case cst::CSTNodeType::StructLiteral:
    return visit_struct_literal(node);
  case cst::CSTNodeType::ParenExpr:
    return visit_paren_expr(node);
  case cst::CSTNodeType::IntegerLiteral:
    return visit_integer_literal(node);
  case cst::CSTNodeType::FloatLiteral:
    return visit_float_literal(node);
  case cst::CSTNodeType::StringLiteral:
    return visit_string_literal(node);
  case cst::CSTNodeType::BooleanLiteral:
    return visit_boolean_literal(node);
  case cst::CSTNodeType::Identifier:
    return visit_identifier(node);
  case cst::CSTNodeType::TypeAnnotation:
    return visit_type_annotation(node);
  case cst::CSTNodeType::ArrayType:
    return visit_array_type(node);
  case cst::CSTNodeType::SizedArrayType:
    return visit_sized_array_type(node);
  case cst::CSTNodeType::UnionType:
    return visit_union_type(node);
  case cst::CSTNodeType::IntersectionType:
    return visit_intersection_type(node);
  case cst::CSTNodeType::NegationType:
    return visit_negation_type(node);
  case cst::CSTNodeType::TupleType:
    return visit_tuple_type(node);
  case cst::CSTNodeType::FunctionSignatureType:
    return visit_function_signature_type(node);
  case cst::CSTNodeType::AnonymousStructType:
    return visit_anonymous_struct_type(node);
  case cst::CSTNodeType::StructField:
    return visit_struct_field(node);
  case cst::CSTNodeType::Parameter:
    return visit_parameter(node);
  case cst::CSTNodeType::ParameterList:
    return visit_parameter_list(node);
  case cst::CSTNodeType::ArgumentList:
    return visit_argument_list(node);
  case cst::CSTNodeType::StatementList:
    return visit_statement_list(node);
  case cst::CSTNodeType::Operator:
    return visit_operator(node);
  case cst::CSTNodeType::Delimiter:
    return visit_delimiter(node);
  case cst::CSTNodeType::Comment:
    return visit_comment(node);
  default:
    // 未处理的节点类型，递归格式化子节点
    std::ostringstream result;
    for (const auto& child : node->get_children()) {
      result << format_node(child.get());
    }
    return result.str();
  }
}

std::string Formatter::get_indent() const {
  if (options.indent_style == IndentStyle::SPACES) {
    return std::string(indent_level * options.indent_width, ' ');
  } else {
    return std::string(indent_level, '\t');
  }
}

std::string Formatter::format_inline_comment(const cst::CSTNode* comment) {
  if (!comment) {
    return "";
  }
  std::ostringstream result;
  result << TWO_WIDTH_SPACE_STRING;
  if (comment->get_token().has_value()) {
    result << comment->get_token()->value;
  }
  return result.str();
}

std::string Formatter::format_standalone_comment(const cst::CSTNode* comment) {
  if (!comment) {
    return "";
  }
  std::ostringstream result;
  result << get_indent();
  if (comment->get_token().has_value()) {
    result << comment->get_token()->value;
  }
  result << "\n";
  return result.str();
}

} // namespace czc::formatter
