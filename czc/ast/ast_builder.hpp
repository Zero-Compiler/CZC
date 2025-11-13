/**
 * @file ast_builder.hpp
 * @brief CST 到 AST 的转换器
 * @details 将 CST（具体语法树）转换为 AST（抽象语法树）
 * @author BegoniaHe
 * @date 2025-11-13
 */

#ifndef CZC_AST_BUILDER_HPP
#define CZC_AST_BUILDER_HPP

#include "czc/ast/ast_node.hpp"
#include "czc/cst/cst_node.hpp"

#include <memory>
#include <string>

namespace czc::ast {

/**
 * @class ASTBuilder
 * @brief CST 到 AST 转换器
 * @details
 *   遍历 CST 并构建对应的 AST。
 *   转换过程中会：
 *   1. 简化语法结构（移除冗余的语法信息）
 *   2. 解析字面量值
 *   3. 建立语义连接
 *   4. 保留源码位置信息（用于错误报告）
 */
class ASTBuilder {
public:
  ASTBuilder() = default;

  /**
   * @brief 从 CST 构建 AST
   * @param cst_root CST 根节点
   * @return AST 根节点（Program）
   */
  std::shared_ptr<Program> build(const cst::CSTNode* cst_root);

private:
  // === CST -> AST 转换方法 ===

  /**
   * @brief 转换 Program 节点
   */
  std::shared_ptr<Program> build_program(const cst::CSTNode* cst_node);

  /**
   * @brief 转换 Declaration 节点
   */
  std::shared_ptr<Declaration> build_declaration(const cst::CSTNode* cst_node);

  /**
   * @brief 转换 Statement 节点
   */
  std::shared_ptr<Statement> build_statement(const cst::CSTNode* cst_node);

  /**
   * @brief 转换 Expression 节点
   */
  std::shared_ptr<Expression> build_expression(const cst::CSTNode* cst_node);

  /**
   * @brief 转换 Type 节点
   */
  std::shared_ptr<Type> build_type(const cst::CSTNode* cst_node);

  // === 具体节点转换方法 ===

  /**
   * @brief 转换变量声明
   */
  std::shared_ptr<Declaration>
  build_var_declaration(const cst::CSTNode* cst_node);

  /**
   * @brief 转换函数声明
   */
  std::shared_ptr<Declaration>
  build_function_declaration(const cst::CSTNode* cst_node);

  /**
   * @brief 转换结构体声明
   */
  std::shared_ptr<Declaration>
  build_struct_declaration(const cst::CSTNode* cst_node);

  /**
   * @brief 转换 Block 语句
   */
  std::shared_ptr<Statement>
  build_block_statement(const cst::CSTNode* cst_node);

  /**
   * @brief 转换 Expression 语句
   */
  std::shared_ptr<Statement> build_expr_statement(const cst::CSTNode* cst_node);

  /**
   * @brief 转换 Return 语句
   */
  std::shared_ptr<Statement>
  build_return_statement(const cst::CSTNode* cst_node);

  /**
   * @brief 转换 If 语句
   */
  std::shared_ptr<Statement> build_if_statement(const cst::CSTNode* cst_node);

  /**
   * @brief 转换二元运算表达式
   */
  std::shared_ptr<Expression> build_binary_expr(const cst::CSTNode* cst_node);

  /**
   * @brief 转换一元运算表达式
   */
  std::shared_ptr<Expression> build_unary_expr(const cst::CSTNode* cst_node);

  /**
   * @brief 转换字面量表达式
   */
  std::shared_ptr<Expression> build_literal(const cst::CSTNode* cst_node);

  /**
   * @brief 转换标识符
   */
  std::shared_ptr<Expression> build_identifier(const cst::CSTNode* cst_node);

  /**
   * @brief 转换括号表达式
   */
  std::shared_ptr<Expression> build_paren_expr(const cst::CSTNode* cst_node);

  /**
   * @brief 转换函数调用表达式
   */
  std::shared_ptr<Expression> build_call_expr(const cst::CSTNode* cst_node);

  /**
   * @brief 转换索引访问表达式
   */
  std::shared_ptr<Expression> build_index_expr(const cst::CSTNode* cst_node);

  /**
   * @brief 转换成员访问表达式
   */
  std::shared_ptr<Expression> build_member_expr(const cst::CSTNode* cst_node);

  /**
   * @brief 转换函数参数
   */
  std::shared_ptr<Parameter> build_parameter(const cst::CSTNode* cst_node);

  /**
   * @brief 转换结构体字段
   */
  std::shared_ptr<StructField> build_struct_field(const cst::CSTNode* cst_node);

  // === 辅助方法 ===

  /**
   * @brief 从 CST Token 解析二元运算符
   */
  BinaryOperator parse_binary_operator(const std::string& op_str);

  /**
   * @brief 从 CST Token 解析一元运算符
   */
  UnaryOperator parse_unary_operator(const std::string& op_str);

  /**
   * @brief 解析整数字面量
   */
  int64_t parse_integer_literal(const std::string& literal_str);

  /**
   * @brief 解析浮点数字面量
   */
  double parse_float_literal(const std::string& literal_str);

  /**
   * @brief 解析字符串字面量（处理转义）
   */
  std::string parse_string_literal(const std::string& literal_str);
};

} // namespace czc::ast

#endif // CZC_AST_BUILDER_HPP
