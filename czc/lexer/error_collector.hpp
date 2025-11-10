/**
 * @file error_collector.hpp
 * @brief 定义了用于收集词法分析错误的 `LexerError` 和 `LexErrorCollector`。
 * @author BegoniaHe
 * @date 2025-11-05
 */

#ifndef CZC_LEX_ERROR_COLLECTOR_HPP
#define CZC_LEX_ERROR_COLLECTOR_HPP

#include "czc/diagnostics/diagnostic_code.hpp"
#include "czc/utils/error_collector.hpp"
#include "czc/utils/source_location.hpp"
#include <string>
#include <vector>

namespace czc {
namespace lexer {

/**
 * @brief 代表一个在词法分析阶段捕获到的词法错误。
 * @details
 *   此结构体是一个数据容器，用于封装词法分析器检测到的单个错误的所有关键信息。
 *   它包括了错误的唯一标识码、精确的源码位置以及用于生成用户友好错误消息的动态参数。
 */
struct LexerError {
  using LocationType = utils::SourceLocation;

  // 标识错误类型的唯一代码，例如 `L0007_UnterminatedString`。
  diagnostics::DiagnosticCode code;

  // 错误在源代码中的精确位置（文件、行、列）。
  utils::SourceLocation location;

  // 用于填充错误消息模板的参数列表，例如无效的字符或缺失的符号。
  std::vector<std::string> args;

  /**
   * @brief 构造一个新的词法错误记录。
   * @param[in] c         诊断代码。
   * @param[in] loc       源码位置。
   * @param[in] arguments (可选) 消息参数列表。
   */
  LexerError(diagnostics::DiagnosticCode c, const utils::SourceLocation &loc,
             const std::vector<std::string> &arguments = {})
      : code(c), location(loc), args(arguments) {}
};

/**
 * @brief 收集并管理词法分析过程中产生的所有错误。
 * @details
 *   使用统一的 ErrorCollector 模板实现错误收集。延迟错误报告机制使得
 *   编译器能够一次性分析完整个文件，并向用户报告所有检测到的词法问题。
 *
 * @property {线程安全} 非线程安全。应在单个词法分析线程中使用。
 */
using LexErrorCollector = utils::ErrorCollector<LexerError>;

} // namespace lexer
} // namespace czc

#endif // CZC_LEX_ERROR_COLLECTOR_HPP
