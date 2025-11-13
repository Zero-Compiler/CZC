/**
 * @file ast_node.hpp
 * @brief AST（抽象语法树）节点定义
 * @details 定义 AST 节点的基础类型和结构，AST 是从 CST 转换而来的语义层表示
 * @author BegoniaHe
 * @date 2025-11-13
 */

#ifndef CZC_AST_NODE_HPP
#define CZC_AST_NODE_HPP

#include "czc/utils/source_location.hpp"

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace czc::ast {

// 前向声明
class ASTNode;
class Expression;
class Statement;
class Declaration;
class Type;

/**
 * @enum ASTNodeKind
 * @brief AST 节点类型枚举
 * @details 定义所有可能的 AST 节点类型，相比 CST 更加语义化和简化
 */
enum class ASTNodeKind {
  // === 程序结构 ===
  Program, ///< 程序根节点
  Module,  ///< 模块

  // === 声明 (Declarations) ===
  VarDecl,       ///< 变量声明: let/var name: type = expr
  FunctionDecl,  ///< 函数声明: fn name(params) -> type { body }
  StructDecl,    ///< 结构体声明: struct Name { fields }
  StructField,   ///< 结构体字段: name: type
  TypeAliasDecl, ///< 类型别名: type Name = Type

  // === 语句 (Statements) ===
  BlockStmt,    ///< 块语句: { stmts }
  ExprStmt,     ///< 表达式语句
  ReturnStmt,   ///< 返回语句: return expr
  IfStmt,       ///< if 语句: if cond { then } else { else }
  WhileStmt,    ///< while 循环: while cond { body }
  BreakStmt,    ///< break 语句
  ContinueStmt, ///< continue 语句

  // === 表达式 (Expressions) ===
  // 字面量
  IntegerLiteral,  ///< 整数字面量
  FloatLiteral,    ///< 浮点数字面量
  StringLiteral,   ///< 字符串字面量
  BooleanLiteral,  ///< 布尔字面量
  ArrayLiteral,    ///< 数组字面量: [elem1, elem2, ...]
  TupleLiteral,    ///< 元组字面量: (elem1, elem2, ...)
  StructLiteral,   ///< 结构体字面量: StructName { field: value, ... }
  FunctionLiteral, ///< 函数字面量: fn (params) { body }

  // 标识符和引用
  Identifier, ///< 标识符

  // 运算符表达式
  BinaryOp, ///< 二元运算: left op right
  UnaryOp,  ///< 一元运算: op expr
  AssignOp, ///< 赋值: target = value

  // 调用和访问
  CallExpr,   ///< 函数调用: func(args)
  IndexExpr,  ///< 索引访问: array[index]
  MemberExpr, ///< 成员访问: struct.field

  // 其他
  ParenExpr, ///< 括号表达式: (expr)
  IfExpr,    ///< if 表达式（可作为值）

  // === 类型 (Types) ===
  PrimitiveType,  ///< 基本类型: Integer, Float, String, Boolean
  ArrayType,      ///< 动态数组类型: T[]
  SizedArrayType, ///< 固定大小数组类型: T[N]
  TupleType,      ///< 元组类型: (T1, T2, ...)
  FunctionType,   ///< 函数类型: (T1, T2) -> T3
  StructType,     ///< 结构体类型引用
  TypeName,       ///< 类型名称引用
};

/**
 * @enum BinaryOperator
 * @brief 二元运算符类型
 */
enum class BinaryOperator {
  // 算术运算
  Add, ///< +
  Sub, ///< -
  Mul, ///< *
  Div, ///< /
  Mod, ///< %

  // 比较运算
  Eq, ///< ==
  Ne, ///< !=
  Lt, ///< <
  Le, ///< <=
  Gt, ///< >
  Ge, ///< >=

  // 逻辑运算
  And, ///< &&
  Or,  ///< ||
};

/**
 * @enum UnaryOperator
 * @brief 一元运算符类型
 */
enum class UnaryOperator {
  Plus,  ///< +
  Minus, ///< -
  Not,   ///< !
};

/**
 * @class ASTNode
 * @brief AST 节点基类
 * @details 所有 AST 节点的基类，提供共同的属性和接口
 */
class ASTNode {
public:
  /**
   * @brief 构造函数
   * @param kind 节点类型
   * @param location 源码位置
   */
  ASTNode(ASTNodeKind kind, const utils::SourceLocation& location)
      : kind_(kind), location_(location) {}

  virtual ~ASTNode() = default;

  /**
   * @brief 获取节点类型
   */
  ASTNodeKind get_kind() const {
    return kind_;
  }

  /**
   * @brief 获取源码位置
   */
  const utils::SourceLocation& get_location() const {
    return location_;
  }

  /**
   * @brief 设置类型信息（用于类型检查阶段）
   */
  void set_type(std::shared_ptr<Type> type) {
    type_ = type;
  }

  /**
   * @brief 获取类型信息
   */
  std::shared_ptr<Type> get_type() const {
    return type_;
  }

protected:
  ASTNodeKind kind_;               ///< 节点类型
  utils::SourceLocation location_; ///< 源码位置
  std::shared_ptr<Type> type_;     ///< 类型信息（类型检查后填充）
};

/**
 * @class Expression
 * @brief 表达式节点基类
 */
class Expression : public ASTNode {
public:
  Expression(ASTNodeKind kind, const utils::SourceLocation& location)
      : ASTNode(kind, location) {}
};

/**
 * @class Statement
 * @brief 语句节点基类
 */
class Statement : public ASTNode {
public:
  Statement(ASTNodeKind kind, const utils::SourceLocation& location)
      : ASTNode(kind, location) {}
};

/**
 * @class Declaration
 * @brief 声明节点基类
 */
class Declaration : public ASTNode {
public:
  Declaration(ASTNodeKind kind, const utils::SourceLocation& location)
      : ASTNode(kind, location) {}
};

/**
 * @class Type
 * @brief 类型节点基类
 */
class Type : public ASTNode {
public:
  Type(ASTNodeKind kind, const utils::SourceLocation& location)
      : ASTNode(kind, location) {}
};

// === 具体节点类型 ===

/**
 * @class Program
 * @brief 程序根节点
 */
class Program : public ASTNode {
public:
  Program(const utils::SourceLocation& location)
      : ASTNode(ASTNodeKind::Program, location) {}

  void add_declaration(std::shared_ptr<Declaration> decl) {
    declarations_.push_back(decl);
  }

  const std::vector<std::shared_ptr<Declaration>>& get_declarations() const {
    return declarations_;
  }

private:
  std::vector<std::shared_ptr<Declaration>> declarations_;
};

/**
 * @class Identifier
 * @brief 标识符节点
 */
class Identifier : public Expression {
public:
  Identifier(const std::string& name, const utils::SourceLocation& location)
      : Expression(ASTNodeKind::Identifier, location), name_(name) {}

  const std::string& get_name() const {
    return name_;
  }

private:
  std::string name_;
};

/**
 * @class IntegerLiteral
 * @brief 整数字面量节点
 */
class IntegerLiteral : public Expression {
public:
  IntegerLiteral(int64_t value, const utils::SourceLocation& location)
      : Expression(ASTNodeKind::IntegerLiteral, location), value_(value) {}

  int64_t get_value() const {
    return value_;
  }

private:
  int64_t value_;
};

/**
 * @class BinaryOpExpr
 * @brief 二元运算表达式
 */
class BinaryOpExpr : public Expression {
public:
  BinaryOpExpr(BinaryOperator op, std::shared_ptr<Expression> left,
               std::shared_ptr<Expression> right,
               const utils::SourceLocation& location)
      : Expression(ASTNodeKind::BinaryOp, location), op_(op), left_(left),
        right_(right) {}

  BinaryOperator get_operator() const {
    return op_;
  }
  std::shared_ptr<Expression> get_left() const {
    return left_;
  }
  std::shared_ptr<Expression> get_right() const {
    return right_;
  }

private:
  BinaryOperator op_;
  std::shared_ptr<Expression> left_;
  std::shared_ptr<Expression> right_;
};

/**
 * @class BlockStmt
 * @brief 块语句
 */
class BlockStmt : public Statement {
public:
  BlockStmt(const utils::SourceLocation& location)
      : Statement(ASTNodeKind::BlockStmt, location) {}

  void add_statement(std::shared_ptr<Statement> stmt) {
    statements_.push_back(stmt);
  }

  const std::vector<std::shared_ptr<Statement>>& get_statements() const {
    return statements_;
  }

private:
  std::vector<std::shared_ptr<Statement>> statements_;
};

/**
 * @class VarDecl
 * @brief 变量声明节点
 */
class VarDecl : public Declaration {
public:
  VarDecl(const std::string& name, std::shared_ptr<Type> type,
          std::shared_ptr<Expression> init,
          const utils::SourceLocation& location)
      : Declaration(ASTNodeKind::VarDecl, location), name_(name), type_(type),
        init_(init) {}

  const std::string& get_name() const {
    return name_;
  }
  std::shared_ptr<Type> get_type_annotation() const {
    return type_;
  }
  std::shared_ptr<Expression> get_initializer() const {
    return init_;
  }

private:
  std::string name_;
  std::shared_ptr<Type> type_;       // 可选
  std::shared_ptr<Expression> init_; // 可选
};

/**
 * @class FloatLiteral
 * @brief 浮点数字面量节点
 */
class FloatLiteral : public Expression {
public:
  FloatLiteral(double value, const utils::SourceLocation& location)
      : Expression(ASTNodeKind::FloatLiteral, location), value_(value) {}

  double get_value() const {
    return value_;
  }

private:
  double value_;
};

/**
 * @class StringLiteral
 * @brief 字符串字面量节点
 */
class StringLiteral : public Expression {
public:
  StringLiteral(const std::string& value, const utils::SourceLocation& location)
      : Expression(ASTNodeKind::StringLiteral, location), value_(value) {}

  const std::string& get_value() const {
    return value_;
  }

private:
  std::string value_;
};

/**
 * @class BooleanLiteral
 * @brief 布尔字面量节点
 */
class BooleanLiteral : public Expression {
public:
  BooleanLiteral(bool value, const utils::SourceLocation& location)
      : Expression(ASTNodeKind::BooleanLiteral, location), value_(value) {}

  bool get_value() const {
    return value_;
  }

private:
  bool value_;
};

/**
 * @class UnaryOpExpr
 * @brief 一元运算表达式
 */
class UnaryOpExpr : public Expression {
public:
  UnaryOpExpr(UnaryOperator op, std::shared_ptr<Expression> operand,
              const utils::SourceLocation& location)
      : Expression(ASTNodeKind::UnaryOp, location), op_(op), operand_(operand) {
  }

  UnaryOperator get_operator() const {
    return op_;
  }
  std::shared_ptr<Expression> get_operand() const {
    return operand_;
  }

private:
  UnaryOperator op_;
  std::shared_ptr<Expression> operand_;
};

/**
 * @class ParenExpr
 * @brief 括号表达式
 * @details 表示括号包围的表达式: (expr)
 */
class ParenExpr : public Expression {
public:
  ParenExpr(std::shared_ptr<Expression> expr,
            const utils::SourceLocation& location)
      : Expression(ASTNodeKind::ParenExpr, location), expr_(expr) {}

  std::shared_ptr<Expression> get_expression() const {
    return expr_;
  }

private:
  std::shared_ptr<Expression> expr_;
};

/**
 * @class CallExpr
 * @brief 函数调用表达式
 * @details 表示函数调用: func(arg1, arg2, ...)
 */
class CallExpr : public Expression {
public:
  CallExpr(std::shared_ptr<Expression> callee,
           std::vector<std::shared_ptr<Expression>> arguments,
           const utils::SourceLocation& location)
      : Expression(ASTNodeKind::CallExpr, location), callee_(callee),
        arguments_(std::move(arguments)) {}

  std::shared_ptr<Expression> get_callee() const {
    return callee_;
  }
  const std::vector<std::shared_ptr<Expression>>& get_arguments() const {
    return arguments_;
  }

private:
  std::shared_ptr<Expression> callee_;
  std::vector<std::shared_ptr<Expression>> arguments_;
};

/**
 * @class IndexExpr
 * @brief 索引访问表达式
 * @details 表示数组索引访问: array[index]
 */
class IndexExpr : public Expression {
public:
  IndexExpr(std::shared_ptr<Expression> object,
            std::shared_ptr<Expression> index,
            const utils::SourceLocation& location)
      : Expression(ASTNodeKind::IndexExpr, location), object_(object),
        index_(index) {}

  std::shared_ptr<Expression> get_object() const {
    return object_;
  }
  std::shared_ptr<Expression> get_index() const {
    return index_;
  }

private:
  std::shared_ptr<Expression> object_;
  std::shared_ptr<Expression> index_;
};

/**
 * @class MemberExpr
 * @brief 成员访问表达式
 * @details 表示结构体成员访问: struct.member
 */
class MemberExpr : public Expression {
public:
  MemberExpr(std::shared_ptr<Expression> object, const std::string& member,
             const utils::SourceLocation& location)
      : Expression(ASTNodeKind::MemberExpr, location), object_(object),
        member_(member) {}

  std::shared_ptr<Expression> get_object() const {
    return object_;
  }
  std::string get_member() const {
    return member_;
  }

private:
  std::shared_ptr<Expression> object_;
  std::string member_;
};

/**
 * @class ExprStmt
 * @brief 表达式语句
 */
class ExprStmt : public Statement {
public:
  ExprStmt(std::shared_ptr<Expression> expr,
           const utils::SourceLocation& location)
      : Statement(ASTNodeKind::ExprStmt, location), expr_(expr) {}

  std::shared_ptr<Expression> get_expression() const {
    return expr_;
  }

private:
  std::shared_ptr<Expression> expr_;
};

/**
 * @class ReturnStmt
 * @brief 返回语句
 */
class ReturnStmt : public Statement {
public:
  ReturnStmt(std::shared_ptr<Expression> value,
             const utils::SourceLocation& location)
      : Statement(ASTNodeKind::ReturnStmt, location), value_(value) {}

  std::shared_ptr<Expression> get_value() const {
    return value_;
  }

private:
  std::shared_ptr<Expression> value_; // 可选
};

/**
 * @class IfStmt
 * @brief 条件语句
 */
class IfStmt : public Statement {
public:
  IfStmt(std::shared_ptr<Expression> condition,
         std::shared_ptr<Statement> then_branch,
         std::shared_ptr<Statement> else_branch,
         const utils::SourceLocation& location)
      : Statement(ASTNodeKind::IfStmt, location), condition_(condition),
        then_branch_(then_branch), else_branch_(else_branch) {}

  std::shared_ptr<Expression> get_condition() const {
    return condition_;
  }
  std::shared_ptr<Statement> get_then_branch() const {
    return then_branch_;
  }
  std::shared_ptr<Statement> get_else_branch() const {
    return else_branch_;
  }

private:
  std::shared_ptr<Expression> condition_;
  std::shared_ptr<Statement> then_branch_;
  std::shared_ptr<Statement> else_branch_; // 可选
};

/**
 * @class Parameter
 * @brief 函数参数节点
 */
class Parameter : public ASTNode {
public:
  Parameter(const std::string& name, std::shared_ptr<Type> type,
            const utils::SourceLocation& location)
      : ASTNode(ASTNodeKind::VarDecl, location), name_(name), type_(type) {}

  const std::string& get_name() const {
    return name_;
  }
  std::shared_ptr<Type> get_type() const {
    return type_;
  }

private:
  std::string name_;
  std::shared_ptr<Type> type_; // 可选
};

/**
 * @class FunctionDecl
 * @brief 函数声明节点
 */
class FunctionDecl : public Declaration {
public:
  FunctionDecl(const std::string& name,
               std::vector<std::shared_ptr<Parameter>> parameters,
               std::shared_ptr<Type> return_type,
               std::shared_ptr<BlockStmt> body,
               const utils::SourceLocation& location)
      : Declaration(ASTNodeKind::FunctionDecl, location), name_(name),
        parameters_(parameters), return_type_(return_type), body_(body) {}

  const std::string& get_name() const {
    return name_;
  }
  const std::vector<std::shared_ptr<Parameter>>& get_parameters() const {
    return parameters_;
  }
  std::shared_ptr<Type> get_return_type() const {
    return return_type_;
  }
  std::shared_ptr<BlockStmt> get_body() const {
    return body_;
  }

private:
  std::string name_;
  std::vector<std::shared_ptr<Parameter>> parameters_;
  std::shared_ptr<Type> return_type_; // 可选
  std::shared_ptr<BlockStmt> body_;
};

/**
 * @class StructField
 * @brief 结构体字段节点
 * @details 表示结构体中的一个字段,包含字段名和类型
 */
class StructField : public ASTNode {
public:
  StructField(const std::string& name, std::shared_ptr<Type> type,
              const utils::SourceLocation& loc)
      : ASTNode(ASTNodeKind::StructField, loc), name_(name), type_(type) {}

  std::string get_name() const {
    return name_;
  }
  std::shared_ptr<Type> get_type() const {
    return type_;
  }

private:
  std::string name_;
  std::shared_ptr<Type> type_;
};

/**
 * @class StructDecl
 * @brief 结构体声明节点
 * @details 表示结构体声明: struct Name { field1: Type1, field2: Type2, ... }
 */
class StructDecl : public Declaration {
public:
  StructDecl(const std::string& name,
             std::vector<std::shared_ptr<StructField>> fields,
             const utils::SourceLocation& loc)
      : Declaration(ASTNodeKind::StructDecl, loc), name_(name),
        fields_(std::move(fields)) {}

  std::string get_name() const {
    return name_;
  }
  const std::vector<std::shared_ptr<StructField>>& get_fields() const {
    return fields_;
  }

private:
  std::string name_;
  std::vector<std::shared_ptr<StructField>> fields_;
};

// 更多具体节点类型将在后续实现中添加...

} // namespace czc::ast

#endif // CZC_AST_NODE_HPP
