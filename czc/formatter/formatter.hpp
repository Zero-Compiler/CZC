/**
 * @file formatter.hpp
 * @brief 定义了 `Formatter` 类，用于格式化 CST 生成美化的源代码。
 * @author BegoniaHe
 * @date 2025-11-11
 */

#ifndef CZC_FORMATTER_HPP
#define CZC_FORMATTER_HPP

#include "czc/cst/cst_node.hpp"
#include "czc/formatter/error_collector.hpp"
#include "czc/formatter/format_options.hpp"
#include "czc/formatter/format_visitor.hpp"

#include <memory>
#include <string>

namespace czc {
namespace formatter {

const std::string ONE_WIDTH_SPACE_STRING = " ";     // 单个空格字符串常量
const std::string TWO_WIDTH_SPACE_STRING = "  ";    // 两个空格字符串常量
const std::string THREE_WIDTH_SPACE_STRING = "   "; // 三个空格字符串常量
const std::string FOUR_WIDTH_SPACE_STRING = "    "; // 四个空格字符串常量

const std::string TAB_STRING = "\t"; // 制表符字符串常量

/**
 * @brief 将 CST 格式化为美化的源代码。
 * @details 此类通过访问者模式遍历
 * CST（具体语法树），根据预设的格式化选项（`FormatOptions`），
 *          生成符合编码规范的可读源代码。它是实现 `zero format` 命令的核心。
 * @property {线程安全} 非线程安全。格式化过程是有状态的（例如，跟踪缩进级别）。
 */
class Formatter : public FormatVisitor {
public:
  /**
   * @brief 构造一个新的 Formatter 实例。
   * @param[in] options 格式化选项，用于控制缩进、空格等。
   */
  explicit Formatter(const FormatOptions& options = FormatOptions());

  /**
   * @brief 格式化给定的 CST 树。
   * @details 此方法会重置内部状态（如缩进级别和错误收集器），然后从根节点开始
   *          递归地格式化整个树。
   * @param[in] root 指向 CST 根节点的指针。
   * @return 格式化后的源代码字符串。如果根节点为空，则返回空字符串。
   */
  std::string format(const cst::CSTNode* root);

  /**
   * @brief 获取对内部错误收集器的访问权限。
   * @return 对 FormatterErrorCollector 对象的引用。
   */
  FormatterErrorCollector& get_error_collector() {
    return error_collector;
  }

  /**
   * @brief 获取对内部错误收集器的只读访问权限。
   * @return 对 FormatterErrorCollector 对象的常量引用。
   */
  const FormatterErrorCollector& get_error_collector() const {
    return error_collector;
  }

  // --- FormatVisitor 接口实现 ---
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
  // 格式化选项
  FormatOptions options;
  // 错误收集器
  FormatterErrorCollector error_collector;
  // 当前缩进级别
  int indent_level;

  /**
   * @brief 递归地格式化单个 CST 节点。
   * @details 这是格式化逻辑的核心，它根据节点的类型（`CSTNodeType`）
   *          应用不同的格式化规则。
   * @param[in] node 要格式化的节点。
   * @return 该节点及其子节点格式化后的字符串。
   */
  std::string format_node(const cst::CSTNode* node);

  /**
   * @brief 根据当前缩进级别和选项生成缩进字符串。
   * @return 由空格或制表符组成的缩进字符串。
   */
  std::string get_indent() const;

  /**
   * @brief 增加缩进级别。
   */
  void increase_indent() {
    ++indent_level;
  }

  /**
   * @brief 减少缩进级别，确保不小于0。
   */
  void decrease_indent() {
    if (indent_level > 0) {
      --indent_level;
    }
  }

  /**
   * @brief 格式化行内注释（在语句后）。
   * @details 在注释前添加固定的两个空格。
   * @param[in] comment 注释节点。
   * @return 格式化后的注释字符串，前置两个空格。
   */
  std::string format_inline_comment(const cst::CSTNode* comment);

  /**
   * @brief 格式化独立行注释。
   * @details 添加当前缩进并在末尾换行。
   * @param[in] comment 注释节点。
   * @return 格式化后的注释字符串，带缩进和换行。
   */
  std::string format_standalone_comment(const cst::CSTNode* comment);
};

} // namespace formatter
} // namespace czc

#endif // CZC_FORMATTER_HPP