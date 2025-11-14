/**
 * @file ast_visitor.hpp
 * @brief AST 访问者模式接口
 * @details 提供访问者模式的抽象接口，用于遍历和处理 AST
 * @author BegoniaHe
 * @date 2025-11-13
 */

#ifndef CZC_AST_VISITOR_HPP
#define CZC_AST_VISITOR_HPP

#include <memory>

namespace czc::ast {

// 前向声明所有 AST 节点类型
class Program;
class Identifier;
class IntegerLiteral;
class FloatLiteral;
class StringLiteral;
class BooleanLiteral;
class BinaryOpExpr;
class UnaryOpExpr;
class BlockStmt;
class ExprStmt;
class ReturnStmt;
class IfStmt;
class VarDecl;
class FunctionDecl;
class StructDecl;
class StructField;

/**
 * @class ASTVisitor
 * @brief AST 访问者接口
 * @details
 *   定义访问各种 AST 节点的接口。实现此接口的类可以遍历和处理 AST。
 *   典型用途包括：
 *   - 类型检查
 *   - 代码生成
 *   - 代码优化
 *   - 静态分析
 *   - Pretty printing
 */
class ASTVisitor {
public:
  virtual ~ASTVisitor() = default;

  // === 程序结构 ===
  virtual void visit_program(Program* node) = 0;

  // === 表达式 ===
  virtual void visit_identifier(Identifier* node) = 0;
  virtual void visit_integer_literal(IntegerLiteral* node) = 0;
  virtual void visit_float_literal(FloatLiteral* node) = 0;
  virtual void visit_string_literal(StringLiteral* node) = 0;
  virtual void visit_boolean_literal(BooleanLiteral* node) = 0;
  virtual void visit_binary_op(BinaryOpExpr* node) = 0;
  virtual void visit_unary_op(UnaryOpExpr* node) = 0;

  // === 语句 ===
  virtual void visit_block_stmt(BlockStmt* node) = 0;
  virtual void visit_expr_stmt(ExprStmt* node) = 0;
  virtual void visit_return_stmt(ReturnStmt* node) = 0;
  virtual void visit_if_stmt(IfStmt* node) = 0;

  // === 声明 ===
  virtual void visit_var_decl(VarDecl* node) = 0;
  virtual void visit_function_decl(FunctionDecl* node) = 0;
  virtual void visit_struct_decl(StructDecl* node) = 0;
  virtual void visit_struct_field(StructField* node) = 0;
};

/**
 * @class ASTBaseVisitor
 * @brief AST 访问者基类
 * @details
 *   提供默认的空实现，派生类只需要重写感兴趣的方法。
 *   这是一个便利类，避免每个访问者都要实现所有方法。
 */
class ASTBaseVisitor : public ASTVisitor {
public:
  ~ASTBaseVisitor() override = default;

  // 默认实现：什么都不做
  void visit_program(Program* node) override {}
  void visit_identifier(Identifier* node) override {}
  void visit_integer_literal(IntegerLiteral* node) override {}
  void visit_float_literal(FloatLiteral* node) override {}
  void visit_string_literal(StringLiteral* node) override {}
  void visit_boolean_literal(BooleanLiteral* node) override {}
  void visit_binary_op(BinaryOpExpr* node) override {}
  void visit_unary_op(UnaryOpExpr* node) override {}
  void visit_block_stmt(BlockStmt* node) override {}
  void visit_expr_stmt(ExprStmt* node) override {}
  void visit_return_stmt(ReturnStmt* node) override {}
  void visit_if_stmt(IfStmt* node) override {}
  void visit_var_decl(VarDecl* node) override {}
  void visit_function_decl(FunctionDecl* node) override {}
  void visit_struct_decl(StructDecl* node) override {}
  void visit_struct_field(StructField* node) override {}
};

/**
 * @class ASTPrinter
 * @brief AST 打印访问者（调试用）
 * @details 遍历 AST 并打印其结构，用于调试和可视化
 */
class ASTPrinter : public ASTBaseVisitor {
public:
  ASTPrinter() : indent_level_(0) {}

  void visit_program(Program* node) override;
  void visit_identifier(Identifier* node) override;
  void visit_integer_literal(IntegerLiteral* node) override;
  void visit_binary_op(BinaryOpExpr* node) override;
  void visit_block_stmt(BlockStmt* node) override;

private:
  int indent_level_;
  void print_indent();
  void increase_indent() {
    indent_level_++;
  }
  void decrease_indent() {
    indent_level_--;
  }
};

} // namespace czc::ast

#endif // CZC_AST_VISITOR_HPP
