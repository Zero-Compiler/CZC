/**
 * @file error_collector.hpp
 * @brief 定义了用于收集 Token 预处理阶段错误的 `TPError` 和
 * `TPErrorCollector`。
 * @author BegoniaHe
 * @date 2025-11-05
 */

#ifndef CZC_TP_ERROR_COLLECTOR_HPP
#define CZC_TP_ERROR_COLLECTOR_HPP

#include "czc/diagnostics/diagnostic_code.hpp"
#include "czc/utils/source_location.hpp"
#include <string>
#include <vector>

namespace czc {
namespace token_preprocessor {

/**
 * @brief 代表一个在 Token 预处理阶段捕获到的错误。
 * @details
 *   此结构体是一个数据容器，用于封装预处理阶段检测到的单个错误的所有关键信息，
 *   例如在分析科学计数法字面量时发生的数值溢出。
 */
struct TPError {
  // 标识错误类型的唯一代码，例如 `T0002_ScientificFloatOverflow`。
  diagnostics::DiagnosticCode code;

  // 错误在源代码中的精确位置（文件、行、列）。
  utils::SourceLocation location;

  // 用于填充错误消息模板的参数列表。
  std::vector<std::string> args;

  /**
   * @brief 构造一个新的预处理错误记录。
   * @param[in] c         诊断代码。
   * @param[in] loc       源码位置。
   * @param[in] arguments (可选) 消息参数列表。
   */
  TPError(diagnostics::DiagnosticCode c, const utils::SourceLocation &loc,
          const std::vector<std::string> &arguments = {})
      : code(c), location(loc), args(arguments) {}
};

/**
 * @brief 收集并管理 Token 预处理过程中产生的所有错误。
 * @details
 *   该类为 `TokenPreprocessor` 提供了一个统一的错误记录机制。当检测到
 *   如数值溢出等问题时，预处理器会通过 `add` 方法记录错误，然后继续处理
 *   下一个 Token。这种 **延迟错误报告** 的设计允许一次性向用户展示所有
 *   在预处理阶段发现的问题。
 *
 * @property {线程安全} 非线程安全。
 */
class TPErrorCollector {
private:
  /// 存储所有已报告的预处理错误的列表。
  std::vector<TPError> errors;

public:
  /**
   * @brief 向收集中添加一个新的预处理错误。
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
  const std::vector<TPError> &get_errors() const { return errors; }

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

} // namespace token_preprocessor
} // namespace czc

#endif // CZC_TP_ERROR_COLLECTOR_HPP
