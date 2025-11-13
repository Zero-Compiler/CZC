/**
 * @file test_helpers.hpp
 * @brief CST 测试辅助函数，提供结构化验证工具。
 * @author BegoniaHe
 * @date 2025-11-13
 */

#ifndef CZC_TEST_HELPERS_HPP
#define CZC_TEST_HELPERS_HPP

#include "czc/cst/cst_node.hpp"
#include "czc/lexer/token.hpp"

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

namespace czc {
namespace test {

/**
 * @brief 验证 CST 节点的基本属性。
 * @param node 要验证的节点
 * @param expected_type 期望的节点类型
 * @param expected_child_count 期望的子节点数量（-1 表示不检查）
 */
inline void verify_node(const cst::CSTNode* node,
                        cst::CSTNodeType expected_type,
                        int expected_child_count = -1) {
  ASSERT_NE(node, nullptr) << "CST node should not be null";
  EXPECT_EQ(node->get_type(), expected_type)
      << "Expected node type: " << static_cast<int>(expected_type)
      << ", got: " << static_cast<int>(node->get_type());

  if (expected_child_count >= 0) {
    EXPECT_EQ(node->get_children().size(),
              static_cast<size_t>(expected_child_count))
        << "Expected " << expected_child_count << " children, got "
        << node->get_children().size();
  }
}

/**
 * @brief 验证节点是否具有特定的 Token 值。
 * @param node 要验证的节点
 * @param expected_value 期望的 Token 值
 */
inline void verify_token_value(const cst::CSTNode* node,
                                const std::string& expected_value) {
  ASSERT_NE(node, nullptr) << "CST node should not be null";
  const auto& token = node->get_token();
  ASSERT_TRUE(token.has_value())
      << "Node should have a token but doesn't";
  EXPECT_EQ(token->value, expected_value)
      << "Expected token value '" << expected_value << "', got '"
      << token->value << "'";
}

/**
 * @brief 验证节点的子节点类型序列。
 * @param node 父节点
 * @param expected_types 期望的子节点类型序列
 */
inline void verify_child_types(
    const cst::CSTNode* node,
    const std::vector<cst::CSTNodeType>& expected_types) {
  ASSERT_NE(node, nullptr) << "CST node should not be null";
  const auto& children = node->get_children();
  ASSERT_EQ(children.size(), expected_types.size())
      << "Expected " << expected_types.size() << " children, got "
      << children.size();

  for (size_t i = 0; i < expected_types.size(); i++) {
    EXPECT_EQ(children[i]->get_type(), expected_types[i])
        << "Child " << i << ": expected type "
        << static_cast<int>(expected_types[i]) << ", got "
        << static_cast<int>(children[i]->get_type());
  }
}

/**
 * @brief 获取指定索引的子节点。
 * @param node 父节点
 * @param index 子节点索引
 * @return 子节点指针，如果索引越界则返回 nullptr
 */
inline const cst::CSTNode* get_child(const cst::CSTNode* node, size_t index) {
  if (node == nullptr) {
    return nullptr;
  }
  const auto& children = node->get_children();
  if (index >= children.size()) {
    return nullptr;
  }
  return children[index].get();
}

/**
 * @brief 查找第一个指定类型的子节点。
 * @param node 父节点
 * @param type 要查找的节点类型
 * @return 找到的节点指针，未找到返回 nullptr
 */
inline const cst::CSTNode* find_child_by_type(const cst::CSTNode* node,
                                               cst::CSTNodeType type) {
  if (node == nullptr) {
    return nullptr;
  }

  for (const auto& child : node->get_children()) {
    if (child->get_type() == type) {
      return child.get();
    }
  }
  return nullptr;
}

/**
 * @brief 递归查找第一个指定类型的节点（深度优先）。
 * @param node 根节点
 * @param type 要查找的节点类型
 * @return 找到的节点指针，未找到返回 nullptr
 */
inline const cst::CSTNode* find_node_recursive(const cst::CSTNode* node,
                                                cst::CSTNodeType type) {
  if (node == nullptr) {
    return nullptr;
  }

  if (node->get_type() == type) {
    return node;
  }

  for (const auto& child : node->get_children()) {
    auto result = find_node_recursive(child.get(), type);
    if (result != nullptr) {
      return result;
    }
  }

  return nullptr;
}

/**
 * @brief 统计特定类型节点的数量（递归）。
 * @param node 根节点
 * @param type 要统计的节点类型
 * @return 节点数量
 */
inline size_t count_nodes(const cst::CSTNode* node, cst::CSTNodeType type) {
  if (node == nullptr) {
    return 0;
  }

  size_t count = (node->get_type() == type) ? 1 : 0;

  for (const auto& child : node->get_children()) {
    count += count_nodes(child.get(), type);
  }

  return count;
}

/**
 * @brief 验证标识符节点。
 * @param node 要验证的节点
 * @param expected_name 期望的标识符名称
 */
inline void verify_identifier(const cst::CSTNode* node,
                               const std::string& expected_name) {
  verify_node(node, cst::CSTNodeType::Identifier);
  verify_token_value(node, expected_name);
}

/**
 * @brief 验证二元运算表达式。
 * @param node 表达式节点
 * @param expected_operator 期望的运算符
 */
inline void verify_binary_expr(const cst::CSTNode* node,
                                const std::string& expected_operator) {
  ASSERT_NE(node, nullptr) << "Binary expression node should not be null";
  const auto& children = node->get_children();
  ASSERT_GE(children.size(), 3u)
      << "Binary expression should have at least 3 children (left, op, right)";

  // 验证运算符（通常在中间）
  bool found_operator = false;
  for (const auto& child : children) {
    const auto& token = child->get_token();
    if (token.has_value() && token->value == expected_operator) {
      found_operator = true;
      break;
    }
  }
  EXPECT_TRUE(found_operator) << "Operator '" << expected_operator
                              << "' not found in binary expression";
}

/**
 * @brief 验证数组类型节点（动态或固定）。
 * @param node 类型节点
 * @param is_sized 是否为固定大小数组
 * @param expected_base_type 期望的基础类型名称（可选）
 */
inline void verify_array_type(const cst::CSTNode* node, bool is_sized,
                               const std::string& expected_base_type = "") {
  if (is_sized) {
    verify_node(node, cst::CSTNodeType::SizedArrayType);
  } else {
    verify_node(node, cst::CSTNodeType::ArrayType);
  }

  // 验证基础类型（第一个子节点）
  const auto& children = node->get_children();
  if (!expected_base_type.empty() && children.size() > 0) {
    auto base = get_child(node, 0);
    if (base->get_type() == cst::CSTNodeType::TypeAnnotation) {
      verify_token_value(base, expected_base_type);
    }
  }
}

/**
 * @brief 验证结构体声明。
 * @param node 结构体声明节点
 * @param expected_name 期望的结构体名称
 * @param expected_field_count 期望的字段数量
 */
inline void verify_struct_declaration(const cst::CSTNode* node,
                                       const std::string& expected_name,
                                       size_t expected_field_count) {
  verify_node(node, cst::CSTNodeType::StructDeclaration);

  // 查找结构体名称标识符
  auto name_node = find_child_by_type(node, cst::CSTNodeType::Identifier);
  ASSERT_NE(name_node, nullptr) << "Struct declaration should have a name";
  verify_token_value(name_node, expected_name);

  // 统计字段数量
  size_t field_count = count_nodes(node, cst::CSTNodeType::StructField);
  EXPECT_EQ(field_count, expected_field_count)
      << "Expected " << expected_field_count << " fields, got " << field_count;
}

/**
 * @brief 验证函数声明。
 * @param node 函数声明节点
 * @param expected_name 期望的函数名称
 * @param expected_param_count 期望的参数数量
 */
inline void verify_function_declaration(const cst::CSTNode* node,
                                        const std::string& expected_name,
                                        size_t expected_param_count) {
  verify_node(node, cst::CSTNodeType::FnDeclaration);

  // 查找函数名称标识符（通常是第二个子节点）
  auto name_node = find_child_by_type(node, cst::CSTNodeType::Identifier);
  ASSERT_NE(name_node, nullptr) << "Function declaration should have a name";
  verify_token_value(name_node, expected_name);

  // 统计参数数量
  size_t param_count = count_nodes(node, cst::CSTNodeType::Parameter);
  EXPECT_EQ(param_count, expected_param_count)
      << "Expected " << expected_param_count << " parameters, got "
      << param_count;
}

/**
 * @brief 打印 CST 树结构（用于调试）。
 * @param node 根节点
 * @param indent 缩进级别
 */
inline void print_cst_tree(const cst::CSTNode* node, int indent = 0) {
  if (node == nullptr) {
    std::cout << std::string(indent * 2, ' ') << "(null)" << std::endl;
    return;
  }

  std::cout << std::string(indent * 2, ' ')
            << static_cast<int>(node->get_type());
  const auto& token = node->get_token();
  if (token.has_value()) {
    std::cout << " [" << token->value << "]";
  }
  std::cout << " (" << node->get_children().size() << " children)" << std::endl;

  for (const auto& child : node->get_children()) {
    print_cst_tree(child.get(), indent + 1);
  }
}

} // namespace test
} // namespace czc

#endif // CZC_TEST_HELPERS_HPP
