/**
 * @file cst_node.hpp
 * @brief 定义了具体语法树（CST）的核心数据结构 `CSTNode` 及其相关类型。
 * @author BegoniaHe
 * @date 2025-11-05
 */

#ifndef CZC_CST_NODE_HPP
#define CZC_CST_NODE_HPP

#include "czc/lexer/token.hpp"
#include "czc/utils/source_location.hpp"
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace czc {
namespace cst {

/**
 * @brief 定义 CST 节点的所有类型。
 * @details
 *   CST 节点类型与语法结构一一对应，保留所有语法细节，
 *   包括括号、分号、关键字等符号的位置信息。
 */
enum class CSTNodeType {
  // --- 程序结构 ---
  Program, ///< 程序根节点

  // --- 声明 ---
  VarDeclaration, ///< 变量声明: let/var name: type = expr;
  FnDeclaration,  ///< 函数声明: fn name(params) -> type { body }

  // --- 语句 ---
  ReturnStmt, ///< 返回语句: return expr;
  IfStmt,     ///< 条件语句: if expr { stmts } else { stmts }
  WhileStmt,  ///< 循环语句: while expr { stmts }
  BlockStmt,  ///< 代码块: { stmts }
  ExprStmt,   ///< 表达式语句: expr;

  // --- 表达式 ---
  BinaryExpr,      ///< 二元表达式: left op right
  UnaryExpr,       ///< 一元表达式: op operand
  CallExpr,        ///< 函数调用: callee(args)
  IndexExpr,       ///< 索引访问: object[index]
  MemberExpr,      ///< 成员访问: object.member
  AssignExpr,      ///< 赋值表达式: lvalue = rvalue
  IndexAssignExpr, ///< 索引赋值: object[index] = value
  ArrayLiteral,    ///< 数组字面量: [elem1, elem2, ...]
  IntegerLiteral,  ///< 整数字面量: 42
  FloatLiteral,    ///< 浮点数字面量: 3.14
  StringLiteral,   ///< 字符串字面量: "hello"
  BooleanLiteral,  ///< 布尔字面量: true | false
  Identifier,      ///< 标识符: var_name
  ParenExpr,       ///< 括号表达式: (expr)

  // --- 类型 ---
  TypeAnnotation, ///< 类型注解节点
  ArrayType,      ///< 数组类型: [element_type]

  // --- 参数 ---
  Parameter,     ///< 函数参数: name: type
  ParameterList, ///< 参数列表

  // --- 列表 ---
  ArgumentList,  ///< 实参列表
  StatementList, ///< 语句列表

  // --- 运算符 ---
  Operator, ///< 运算符 token

  // --- 分隔符 ---
  Delimiter, ///< 分隔符（括号、分号等）

  // --- 注释 ---
  Comment, ///< 注释
};

/**
 * @brief 具体语法树（CST）的基类节点，忠实保留源码的所有语法细节。
 * @details
 *   与抽象语法树（AST）旨在表达代码的语义结构不同，CST 的核心使命是
 *   **无损地** 表示源码的语法结构。它保留了所有词法单元（Tokens），
 *   包括关键字、括号、分号、运算符等，是实现源码格式化、重构工具、
 *   精确错误提示和高级 IDE 功能（如语法高亮）的基石。
 *
 * @property {生命周期} 节点的生命周期由 `std::unique_ptr` 自动管理。
 * @property {线程安全} 非线程安全。所有对 CST 的操作都应在单线程内完成。
 */
class CSTNode {
public:
  /**
   * @brief 构造一个 CST 节点。
   * @param[in] type 节点类型。
   * @param[in] location 节点在源码中的位置。
   */
  CSTNode(CSTNodeType type, const utils::SourceLocation &location);

  /**
   * @brief 虚析构函数。
   */
  virtual ~CSTNode() = default;

  /**
   * @brief 获取节点类型。
   * @return 节点的类型枚举值。
   */
  CSTNodeType get_type() const { return node_type; }

  /**
   * @brief 获取节点的源码位置。
   * @return 节点的源码位置信息。
   */
  const utils::SourceLocation &get_location() const { return location; }

  /**
   * @brief 添加一个子节点。
   * @param[in] child 要添加的子节点。
   */
  void add_child(std::unique_ptr<CSTNode> child);

  /**
   * @brief 获取所有子节点。
   * @return 子节点列表的常量引用。
   */
  const std::vector<std::unique_ptr<CSTNode>> &get_children() const {
    return children;
  }

  /**
   * @brief 关联一个 Token 到此节点。
   * @details
   *   用于保留关键字、运算符、分隔符等语法符号的精确位置。
   * @param[in] token Token 对象。
   */
  void set_token(const lexer::Token &token);

  /**
   * @brief 获取关联的 Token。
   * @return Token 的可选值。
   */
  const std::optional<lexer::Token> &get_token() const { return token; }

protected:
  // 节点的具体语法类型。
  CSTNodeType node_type;

  // 节点在源码中的起始与结束位置。
  utils::SourceLocation location;

  // 子节点列表，所有权由本节点通过 `std::unique_ptr` 管理。
  std::vector<std::unique_ptr<CSTNode>> children;

  // 关联的单个 Token，用于表示关键字、运算符、分隔符等叶子节点。
  // @note 对于复合节点，此项通常为空。
  std::optional<lexer::Token> token;
};

// --- 辅助函数 ---

/**
 * @brief 将 CST 节点类型转换为字符串。
 * @param[in] type 节点类型。
 * @return 类型的字符串表示。
 */
std::string cst_node_type_to_string(CSTNodeType type);

/**
 * @brief 创建一个新的 CST 节点。
 * @param[in] type 节点类型。
 * @param[in] location 源码位置。
 * @return 新创建的节点的智能指针。
 */
std::unique_ptr<CSTNode> make_cst_node(CSTNodeType type,
                                       const utils::SourceLocation &location);

/**
 * @brief 创建一个带 Token 的 CST 节点。
 * @param[in] type 节点类型。
 * @param[in] token Token 对象。
 * @return 新创建的节点的智能指针。
 */
std::unique_ptr<CSTNode> make_cst_node(CSTNodeType type,
                                       const lexer::Token &token);

} // namespace cst
} // namespace czc

#endif // CZC_CST_NODE_HPP
