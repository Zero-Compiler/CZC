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
#include "czc/utils/error_collector.hpp"
#include "czc/utils/source_location.hpp"
#include <string>
#include <vector>

namespace czc {
namespace token_preprocessor {

/**
 * @brief 代表一个在 Token 预处理阶段捕获到的错误。
 * @details
 *   此结构体是一个数据容器,用于封装预处理阶段检测到的单个错误的所有关键信息,
 *   例如在分析科学计数法字面量时发生的数值溢出。
 */
struct TPError {
  using LocationType = utils::SourceLocation;

  // 标识错误类型的唯一代码,例如 `T0002_ScientificFloatOverflow`。
  diagnostics::DiagnosticCode code;

  // 错误在源代码中的精确位置(文件、行、列)。
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
 *   使用统一的 ErrorCollector 模板实现错误收集。该类为 `TokenPreprocessor`
 *   提供了一个统一的错误记录机制。当检测到如数值溢出等问题时,预处理器会
 *   通过 `add` 方法记录错误,然后继续处理下一个 Token。这种 **延迟错误报告**
 *   的设计允许一次性向用户展示所有在预处理阶段发现的问题。
 *
 * @property {线程安全} 非线程安全。
 */
using TPErrorCollector = utils::ErrorCollector<TPError>;

} // namespace token_preprocessor
} // namespace czc

#endif // CZC_TP_ERROR_COLLECTOR_HPP
