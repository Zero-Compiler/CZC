/**
 * @file cst_builder.hpp
 * @brief CST 构建器，解耦 Parser 和 CST 节点创建
 * @details 提供统一的 CST 节点创建接口，减少 Parser 对 CST 实现细节的依赖
 * @author BegoniaHe
 * @date 2025-11-14
 */

#ifndef CZC_CST_BUILDER_HPP
#define CZC_CST_BUILDER_HPP

#include "czc/cst/cst_node.hpp"
#include "czc/lexer/token.hpp"
#include "czc/utils/source_location.hpp"

#include <memory>
#include <vector>

namespace czc::cst {

/**
 * @class CSTBuilder
 * @brief CST 节点构建器
 * @details
 *   提供统一的 CST 节点创建和组装接口。通过引入此中间层，
 *   Parser 无需直接操作 CSTNode 的内部细节，降低了模块间的耦合度。
 *
 *   优势：
 *   - 解耦：Parser 只需要知道构建器接口，不需要了解 CST 实现细节
 *   - 扩展性：未来可以轻松切换到不同的 CST 实现
 *   - 可测试性：可以为测试提供 Mock 实现
 *   - 一致性：所有 CST 节点通过统一接口创建，确保格式一致
 *
 * @property {线程安全} 无状态，线程安全
 */
class CSTBuilder {
public:
  /**
   * @brief 创建一个新的 CST 节点
   * @param type 节点类型
   * @param location 源码位置
   * @return 新创建的 CST 节点
   */
  [[nodiscard]] static std::unique_ptr<CSTNode>
  create_node(CSTNodeType type, const utils::SourceLocation& location) {
    return std::make_unique<CSTNode>(type, location);
  }

  /**
   * @brief 创建一个带 Token 的 CST 节点
   * @param type 节点类型
   * @param token Token 对象
   * @return 新创建的 CST 节点
   */
  [[nodiscard]] static std::unique_ptr<CSTNode>
  create_node(CSTNodeType type, const lexer::Token& token) {
    auto node = create_node(
        type, utils::SourceLocation(token.value, token.line, token.column));
    node->set_token(token);
    return node;
  }

  /**
   * @brief 创建程序根节点
   * @param location 源码位置
   * @return 程序根节点
   */
  [[nodiscard]] static std::unique_ptr<CSTNode>
  create_program(const utils::SourceLocation& location) {
    return create_node(CSTNodeType::Program, location);
  }

  /**
   * @brief 创建变量声明节点
   * @param location 源码位置
   * @return 变量声明节点
   */
  [[nodiscard]] static std::unique_ptr<CSTNode>
  create_var_declaration(const utils::SourceLocation& location) {
    return create_node(CSTNodeType::VarDeclaration, location);
  }

  /**
   * @brief 创建函数声明节点
   * @param location 源码位置
   * @return 函数声明节点
   */
  [[nodiscard]] static std::unique_ptr<CSTNode>
  create_fn_declaration(const utils::SourceLocation& location) {
    return create_node(CSTNodeType::FnDeclaration, location);
  }

  /**
   * @brief 创建结构体声明节点
   * @param location 源码位置
   * @return 结构体声明节点
   */
  [[nodiscard]] static std::unique_ptr<CSTNode>
  create_struct_declaration(const utils::SourceLocation& location) {
    return create_node(CSTNodeType::StructDeclaration, location);
  }

  /**
   * @brief 创建类型别名声明节点
   * @param location 源码位置
   * @return 类型别名声明节点
   */
  [[nodiscard]] static std::unique_ptr<CSTNode>
  create_type_alias_declaration(const utils::SourceLocation& location) {
    return create_node(CSTNodeType::TypeAliasDeclaration, location);
  }

  /**
   * @brief 创建代码块节点
   * @param location 源码位置
   * @return 代码块节点
   */
  [[nodiscard]] static std::unique_ptr<CSTNode>
  create_block_stmt(const utils::SourceLocation& location) {
    return create_node(CSTNodeType::BlockStmt, location);
  }

  /**
   * @brief 创建表达式节点
   * @param type 表达式类型
   * @param location 源码位置
   * @return 表达式节点
   */
  [[nodiscard]] static std::unique_ptr<CSTNode>
  create_expression(CSTNodeType type, const utils::SourceLocation& location) {
    return create_node(type, location);
  }

  /**
   * @brief 创建二元表达式节点
   * @param location 源码位置
   * @return 二元表达式节点
   */
  [[nodiscard]] static std::unique_ptr<CSTNode>
  create_binary_expr(const utils::SourceLocation& location) {
    return create_node(CSTNodeType::BinaryExpr, location);
  }

  /**
   * @brief 创建标识符节点
   * @param token 标识符 Token
   * @return 标识符节点
   */
  [[nodiscard]] static std::unique_ptr<CSTNode>
  create_identifier(const lexer::Token& token) {
    return create_node(CSTNodeType::Identifier, token);
  }

  /**
   * @brief 创建字面量节点
   * @param token 字面量 Token
   * @return 字面量节点
   */
  [[nodiscard]] static std::unique_ptr<CSTNode>
  create_literal(const lexer::Token& token) {
    CSTNodeType type;
    switch (token.token_type) {
    case lexer::TokenType::Integer:
      type = CSTNodeType::IntegerLiteral;
      break;
    case lexer::TokenType::Float:
    case lexer::TokenType::ScientificExponent:
      type = CSTNodeType::FloatLiteral;
      break;
    case lexer::TokenType::String:
      type = CSTNodeType::StringLiteral;
      break;
    case lexer::TokenType::True:
    case lexer::TokenType::False:
      type = CSTNodeType::BooleanLiteral;
      break;
    default:
      type = CSTNodeType::Identifier;
      break;
    }
    return create_node(type, token);
  }

  /**
   * @brief 向节点添加子节点
   * @param parent 父节点
   * @param child 子节点
   */
  static void add_child(CSTNode* parent, std::unique_ptr<CSTNode> child) {
    if (parent && child) {
      parent->add_child(std::move(child));
    }
  }

  /**
   * @brief 批量添加子节点
   * @param parent 父节点
   * @param children 子节点列表
   */
  static void add_children(CSTNode* parent,
                           std::vector<std::unique_ptr<CSTNode>>& children) {
    if (!parent) {
      return;
    }
    for (auto& child : children) {
      if (child) {
        parent->add_child(std::move(child));
      }
    }
  }

  /**
   * @brief 设置节点的 Token
   * @param node 节点
   * @param token Token
   */
  static void set_token(CSTNode* node, const lexer::Token& token) {
    if (node) {
      node->set_token(token);
    }
  }
};

} // namespace czc::cst

#endif // CZC_CST_BUILDER_HPP
