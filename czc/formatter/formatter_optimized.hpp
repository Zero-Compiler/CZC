/**
 * @file formatter.hpp
 * @brief 定义了 `Formatter` 类，用于格式化 CST 生成美化的源代码 - 优化版本
 * @details 通过前向声明减少依赖，提高编译效率
 * @author BegoniaHe
 * @date 2025-11-13
 */

#ifndef CZC_FORMATTER_HPP_OPTIMIZED
#define CZC_FORMATTER_HPP_OPTIMIZED

#include "czc/formatter/error_collector.hpp"
#include "czc/formatter/format_options.hpp"
#include "czc/formatter/format_visitor.hpp"

#include <memory>
#include <string>

namespace czc::cst {
// 前向声明 - 避免包含完整头文件
class CSTNode;
} // namespace czc::cst

namespace czc::formatter {

// 常量定义保持不变
const std::string ONE_WIDTH_SPACE_STRING = " ";
const std::string TWO_WIDTH_SPACE_STRING = "  ";
const std::string THREE_WIDTH_SPACE_STRING = "   ";
const std::string FOUR_WIDTH_SPACE_STRING = "    ";
const std::string TAB_STRING = "\t";

/**
 * @brief 将 CST 格式化为美化的源代码 - 优化版本
 * @details 通过前向声明减少编译依赖，提高编译速度
 * @property {线程安全} 非线程安全。格式化过程是有状态的
 */
class Formatter : public FormatVisitor {
public:
  /**
   * @brief 构造一个新的 Formatter 实例
   * @param[in] options 格式化选项，用于控制缩进、空格等
   */
  explicit Formatter(const FormatOptions& options = FormatOptions());

  /**
   * @brief 格式化给定的 CST 树
   * @param[in] root 指向 CST 根节点的指针
   * @return 格式化后的源代码字符串。如果根节点为空，则返回空字符串
   * @note 使用指针参数避免需要CSTNode的完整定义
   */
  std::string format(const cst::CSTNode* root);

  /**
   * @brief 获取对内部错误收集器的访问权限
   * @return 对 FormatterErrorCollector 对象的引用
   */
  FormatterErrorCollector& get_error_collector() {
    return error_collector;
  }

  /**
   * @brief 获取对内部错误收集器的只读访问权限
   * @return 对 FormatterErrorCollector 对象的常量引用
   */
  const FormatterErrorCollector& get_error_collector() const {
    return error_collector;
  }

  // --- FormatVisitor 接口实现 ---
  // 所有方法都使用指针参数，避免完整类型定义
  std::string visit_program(const cst::CSTNode* node) override;
  std::string visit_var_declaration(const cst::CSTNode* node) override;
  std::string visit_fn_declaration(const cst::CSTNode* node) override;
  std::string visit_struct_declaration(const cst::CSTNode* node) override;
  std::string visit_type_alias_declaration(const cst::CSTNode* node) override;
  std::string visit_return_stmt(const cst::CSTNode* node) override;
  std::string visit_if_stmt(const cst::CSTNode* node) override;
  std::string visit_while_stmt(const cst::CSTNode* node) override;
  std::string visit_block_stmt(const cst::CSTNode* node) override;
  std::string visit_expr_stmt(const cst::CSTNode* node) override;
  std::string visit_binary_expr(const cst::CSTNode* node) override;
  std::string visit_unary_expr(const cst::CSTNode* node) override;
  std::string visit_call_expr(const cst::CSTNode* node) override;
  std::string visit_index_expr(const cst::CSTNode* node) override;
  std::string visit_member_expr(const cst::CSTNode* node) override;
  std::string visit_assign_expr(const cst::CSTNode* node) override;
  std::string visit_index_assign_expr(const cst::CSTNode* node) override;
  std::string visit_member_assign_expr(const cst::CSTNode* node) override;
  std::string visit_array_literal(const cst::CSTNode* node) override;
  std::string visit_struct_literal(const cst::CSTNode* node) override;
  std::string visit_paren_expr(const cst::CSTNode* node) override;
  std::string visit_integer_literal(const cst::CSTNode* node) override;
  std::string visit_float_literal(const cst::CSTNode* node) override;
  std::string visit_string_literal(const cst::CSTNode* node) override;
  std::string visit_boolean_literal(const cst::CSTNode* node) override;
  std::string visit_identifier(const cst::CSTNode* node) override;
  std::string visit_type_annotation(const cst::CSTNode* node) override;
  std::string visit_array_type(const cst::CSTNode* node) override;
  std::string visit_sized_array_type(const cst::CSTNode* node) override;
  std::string visit_tuple_literal(const cst::CSTNode* node) override;
  std::string visit_function_literal(const cst::CSTNode* node) override;
  std::string visit_union_type(const cst::CSTNode* node) override;
  std::string visit_intersection_type(const cst::CSTNode* node) override;
  std::string visit_negation_type(const cst::CSTNode* node) override;
  std::string visit_tuple_type(const cst::CSTNode* node) override;
  std::string visit_function_signature_type(const cst::CSTNode* node) override;
  std::string visit_anonymous_struct_type(const cst::CSTNode* node) override;
  std::string visit_struct_field(const cst::CSTNode* node) override;
  std::string visit_parameter(const cst::CSTNode* node) override;
  std::string visit_parameter_list(const cst::CSTNode* node) override;
  std::string visit_argument_list(const cst::CSTNode* node) override;
  std::string visit_statement_list(const cst::CSTNode* node) override;
  std::string visit_operator(const cst::CSTNode* node) override;
  std::string visit_delimiter(const cst::CSTNode* node) override;
  std::string visit_comment(const cst::CSTNode* node) override;

private:
  /**
   * @brief 递归地格式化单个 CST 节点
   * @details 这是格式化逻辑的核心，在实现文件中定义
   * @param[in] node 要格式化的节点
   * @return 该节点及其子节点格式化后的字符串
   */
  std::string format_node(const cst::CSTNode* node);

  /**
   * @brief 根据当前缩进级别和选项生成缩进字符串
   * @return 由空格或制表符组成的缩进字符串
   */
  std::string get_indent() const;

  /**
   * @brief 增加缩进级别
   */
  void increase_indent() {
    ++indent_level;
  }

  /**
   * @brief 减少缩进级别，确保不小于0
   */
  void decrease_indent() {
    if (indent_level > 0) {
      --indent_level;
    }
  }

  /**
   * @brief 格式化行内注释（在语句后）
   * @param[in] comment 注释节点
   * @return 格式化后的注释字符串，前置两个空格
   */
  std::string format_inline_comment(const cst::CSTNode* comment);

  /**
   * @brief 格式化独立行注释
   * @param[in] comment 注释节点
   * @return 格式化后的注释字符串，带缩进和换行
   */
  std::string format_standalone_comment(const cst::CSTNode* comment);

  // 成员变量
  FormatOptions options;                   ///< 格式化选项
  FormatterErrorCollector error_collector; ///< 错误收集器
  int indent_level;                        ///< 当前缩进级别
};

/**
 * @brief 创建默认格式化器
 * @return 默认配置的Formatter实例
 */
inline Formatter make_default_formatter() {
  return Formatter(FormatOptions());
}

/**
 * @brief 创建自定义格式化器
 * @param options 格式化选项
 * @return 自定义配置的Formatter实例
 */
inline Formatter make_formatter(const FormatOptions& options) {
  return Formatter(options);
}

} // namespace czc::formatter

#endif // CZC_FORMATTER_HPP_OPTIMIZED