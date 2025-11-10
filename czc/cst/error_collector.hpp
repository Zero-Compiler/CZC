/**
 * @file error_collector.hpp
 * @brief 定义了用于收集 CST 构建阶段错误的 `CSTError` 和 `CSTErrorCollector`。
 * @author BegoniaHe
 * @date 2025-11-05
 */

#ifndef CZC_CST_ERROR_COLLECTOR_HPP
#define CZC_CST_ERROR_COLLECTOR_HPP

#include "czc/diagnostics/diagnostic_code.hpp"
#include "czc/utils/error_collector.hpp"
#include "czc/utils/source_location.hpp"
#include <string>
#include <vector>

namespace czc {
namespace cst {

/**
 * @brief 代表一个在 CST 构建阶段捕获到的语法错误。
 * @details
 *   此结构体是一个数据容器,用于封装 CST
 * 构建过程中检测到的单个错误的所有关键信息。
 *   它包括了错误的唯一标识码、精确的源码位置以及用于生成用户友好错误消息的动态参数。
 */
struct CSTError {
  using LocationType = utils::SourceLocation;

  // 标识错误类型的唯一代码。
  diagnostics::DiagnosticCode code;

  // 错误在源代码中的精确位置(文件、行、列)。
  utils::SourceLocation location;

  // 用于填充错误消息模板的参数列表。
  std::vector<std::string> args;

  /**
   * @brief 构造一个新的 CST 错误记录。
   * @param[in] c         诊断代码。
   * @param[in] loc       源码位置。
   * @param[in] arguments (可选) 消息参数列表。
   */
  CSTError(diagnostics::DiagnosticCode c, const utils::SourceLocation &loc,
           const std::vector<std::string> &arguments = {})
      : code(c), location(loc), args(arguments) {}
};

/**
 * @brief 收集并管理 CST 构建过程中产生的所有错误。
 * @details
 *   使用统一的 ErrorCollector 模板实现错误收集。该类的核心设计思想是
 *   **延迟错误报告**。在 CST 构建过程中,当遇到错误时,并不会立即中断流程,
 *   而是通过 `add`
 * 方法将错误记录下来。这种机制使得编译器能够一次性分析完整个文件,
 *   并向用户报告所有检测到的语法问题,极大地提升了开发效率。
 *
 * @property {线程安全} 非线程安全。应在单个解析线程中使用。
 */
using CSTErrorCollector = utils::ErrorCollector<CSTError>;

} // namespace cst
} // namespace czc

#endif // CZC_CST_ERROR_COLLECTOR_HPP
