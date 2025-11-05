/**
 * @file error_collector.hpp
 * @brief 定义了用于收集语法分析错误的 `ParserError` 和 `ParserErrorCollector`。
 * @author BegoniaHe
 * @date 2025-11-05
 */

#ifndef CZC_PARSER_ERROR_COLLECTOR_HPP
#define CZC_PARSER_ERROR_COLLECTOR_HPP

#include "czc/diagnostics/diagnostic_code.hpp"
#include "czc/utils/source_location.hpp"
#include <string>
#include <vector>

namespace czc {
namespace parser {

/**
 * @brief 代表一个在语法分析阶段捕获到的语法错误。
 * @details
 *   此结构体是一个数据容器，用于封装语法分析器检测到的单个错误的所有关键信息。
 *   它包括了错误的唯一标识码、精确的源码位置以及用于生成用户友好错误消息的动态参数。
 */
struct ParserError {
  // 标识错误类型的唯一代码，例如 `P0001_UnexpectedToken`。
  diagnostics::DiagnosticCode code;

  // 错误在源代码中的精确位置（文件、行、列）。
  utils::SourceLocation location;

  // 用于填充错误消息模板的参数列表，例如期望的 Token 和实际遇到的 Token。
  std::vector<std::string> args;

  /**
   * @brief 构造一个新的语法分析错误记录。
   * @param[in] c         诊断代码。
   * @param[in] loc       源码位置。
   * @param[in] arguments (可选) 消息参数列表。
   */
  ParserError(diagnostics::DiagnosticCode c, const utils::SourceLocation &loc,
              const std::vector<std::string> &arguments = {})
      : code(c), location(loc), args(arguments) {}
};

/**
 * @brief 收集并管理语法分析过程中产生的所有错误。
 * @details
 *   该类的核心设计思想是 **延迟错误报告** 和 **错误恢复**。当语法分析器
 *   遇到不符合语法规则的 Token 时，它会通过 `add` 方法记录一个错误，
 *   然后尝试通过同步（synchronization）等技术跳过一些 Token，直到找到一个
 *   可以安全恢复解析的位置。这使得解析器能够从错误中恢复并继续检查文件的
 *   其余部分，从而一次性报告多个独立的语法错误。
 *
 * @property {线程安全} 非线程安全。应在单个解析线程中使用。
 */
class ParserErrorCollector {
private:
  /// 存储所有已报告的语法分析错误的列表。
  std::vector<ParserError> errors;

public:
  /**
   * @brief 向收集器添加一个新的语法分析错误。
   * @param[in] code 错误的诊断代码。
   * @param[in] loc  错误在源代码中的位置。
   * @param[in] args (可选) 用于格式化错误消息的参数。
   */
  void add(diagnostics::DiagnosticCode code, const utils::SourceLocation &loc,
           const std::vector<std::string> &args = {});

  /**
   * @brief 获取所有已收集的错误。
   * @return 返回对内部错误列表的常量引用。
   */
  const std::vector<ParserError> &get_errors() const { return errors; }

  /**
   * @brief 检查是否收集到了任何错误。
   * @return 如果错误列表不为空，则返回 `true`。
   */
  bool has_errors() const { return !errors.empty(); }

  /**
   * @brief 清空所有已收集的错误。
   */
  void clear() { errors.clear(); }

  /**
   * @brief 获取当前收集到的错误总数。
   * @return 错误列表的大小。
   */
  size_t count() const { return errors.size(); }
};

} // namespace parser
} // namespace czc

#endif // CZC_PARSER_ERROR_COLLECTOR_HPP
