/**
 * @file format_visitor.hpp
 * @brief 定义了格式化访问者接口，用于实现访问者模式格式化 CST 节点。
 * @author BegoniaHe
 * @date 2025-11-07
 */

#ifndef CZC_FORMAT_VISITOR_HPP
#define CZC_FORMAT_VISITOR_HPP

#include "czc/cst/cst_node.hpp"
#include <string>

namespace czc {
namespace formatter {

/**
 * @brief CST 节点格式化访问者接口。
 * @details
 *   采用访问者模式设计，每种 CST 节点类型对应一个 visit 方法。
 *   相比巨大的 switch-case，访问者模式符合开闭原则，更易于扩展和维护。
 */
class FormatVisitor {
public:
  virtual ~FormatVisitor() = default;

  // --- 程序结构 ---
  virtual std::string visit_program(const cst::CSTNode *node) = 0;

  // --- 声明 ---
  virtual std::string visit_var_declaration(const cst::CSTNode *node) = 0;
  virtual std::string visit_fn_declaration(const cst::CSTNode *node) = 0;

  // --- 语句 ---
  virtual std::string visit_return_stmt(const cst::CSTNode *node) = 0;
  virtual std::string visit_if_stmt(const cst::CSTNode *node) = 0;
  virtual std::string visit_while_stmt(const cst::CSTNode *node) = 0;
  virtual std::string visit_block_stmt(const cst::CSTNode *node) = 0;
  virtual std::string visit_expr_stmt(const cst::CSTNode *node) = 0;

  // --- 表达式 ---
  virtual std::string visit_binary_expr(const cst::CSTNode *node) = 0;
  virtual std::string visit_unary_expr(const cst::CSTNode *node) = 0;
  virtual std::string visit_call_expr(const cst::CSTNode *node) = 0;
  virtual std::string visit_index_expr(const cst::CSTNode *node) = 0;
  virtual std::string visit_member_expr(const cst::CSTNode *node) = 0;
  virtual std::string visit_assign_expr(const cst::CSTNode *node) = 0;
  virtual std::string visit_index_assign_expr(const cst::CSTNode *node) = 0;
  virtual std::string visit_array_literal(const cst::CSTNode *node) = 0;
  virtual std::string visit_paren_expr(const cst::CSTNode *node) = 0;

  // --- 字面量 ---
  virtual std::string visit_integer_literal(const cst::CSTNode *node) = 0;
  virtual std::string visit_float_literal(const cst::CSTNode *node) = 0;
  virtual std::string visit_string_literal(const cst::CSTNode *node) = 0;
  virtual std::string visit_boolean_literal(const cst::CSTNode *node) = 0;
  virtual std::string visit_identifier(const cst::CSTNode *node) = 0;

  // --- 类型 ---
  virtual std::string visit_type_annotation(const cst::CSTNode *node) = 0;
  virtual std::string visit_array_type(const cst::CSTNode *node) = 0;

  // --- 参数和列表 ---
  virtual std::string visit_parameter(const cst::CSTNode *node) = 0;
  virtual std::string visit_parameter_list(const cst::CSTNode *node) = 0;
  virtual std::string visit_argument_list(const cst::CSTNode *node) = 0;
  virtual std::string visit_statement_list(const cst::CSTNode *node) = 0;

  // --- 符号 ---
  virtual std::string visit_operator(const cst::CSTNode *node) = 0;
  virtual std::string visit_delimiter(const cst::CSTNode *node) = 0;
  virtual std::string visit_comment(const cst::CSTNode *node) = 0;
};

} // namespace formatter
} // namespace czc

#endif // CZC_FORMAT_VISITOR_HPP
