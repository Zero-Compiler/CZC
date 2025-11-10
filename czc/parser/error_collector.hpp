/**
 * @file error_collector.hpp
 * @brief 定义了用于收集语法分析错误的 `ParserError` 和 `ParserErrorCollector`。
 * @author BegoniaHe
 * @date 2025-11-05
 */

#ifndef CZC_PARSER_ERROR_COLLECTOR_HPP
#define CZC_PARSER_ERROR_COLLECTOR_HPP

#include "czc/diagnostics/diagnostic_code.hpp"
#include "czc/utils/error_collector.hpp"
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
  using LocationType = utils::SourceLocation;

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
 *   使用统一的 ErrorCollector 模板实现错误收集。支持延迟错误报告和错误恢复，
 *   使解析器能够从错误中恢复并一次性报告多个独立的语法错误。
 *
 * @property {线程安全} 非线程安全。应在单个解析线程中使用。
 */
using ParserErrorCollector = utils::ErrorCollector<ParserError>;

} // namespace parser
} // namespace czc

#endif // CZC_PARSER_ERROR_COLLECTOR_HPP
