/**
 * @file formatter.hpp
 * @brief 输出格式化器接口定义。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-30
 *
 * @details
 *   定义输出格式化的抽象接口，支持 Text 和 JSON 两种格式。
 */

#ifndef CZC_CLI_OUTPUT_FORMATTER_HPP
#define CZC_CLI_OUTPUT_FORMATTER_HPP

#include "czc/common/config.hpp"

#include "czc/cli/context.hpp"
#include "czc/lexer/lexer_error.hpp"
#include "czc/lexer/source_manager.hpp"
#include "czc/lexer/token.hpp"

#include <memory>
#include <span>
#include <string>

namespace czc::cli {

/**
 * @brief 输出格式化器接口。
 *
 * @details
 *   定义格式化输出的抽象接口，具体实现包括：
 *   - TextFormatter: 人类可读的文本格式
 *   - JsonFormatter: JSON 格式（使用 glaze 库）
 */
class OutputFormatter {
public:
  virtual ~OutputFormatter() = default;

  // 不可拷贝
  OutputFormatter(const OutputFormatter &) = delete;
  OutputFormatter &operator=(const OutputFormatter &) = delete;

  // 可移动
  OutputFormatter(OutputFormatter &&) noexcept = default;
  OutputFormatter &operator=(OutputFormatter &&) noexcept = default;

  /**
   * @brief 格式化 Token 列表。
   *
   * @param tokens Token 列表
   * @param sm 源码管理器（用于获取 Token 文本）
   * @return 格式化后的字符串
   */
  [[nodiscard]] virtual std::string
  formatTokens(std::span<const lexer::Token> tokens,
               const lexer::SourceManager &sm) const = 0;

  /**
   * @brief 格式化错误列表。
   *
   * @param errors 错误列表
   * @param sm 源码管理器（用于获取位置信息）
   * @return 格式化后的字符串
   */
  [[nodiscard]] virtual std::string
  formatErrors(std::span<const lexer::LexerError> errors,
               const lexer::SourceManager &sm) const = 0;

protected:
  OutputFormatter() = default;
};

/**
 * @brief 创建格式化器工厂函数。
 *
 * @param format 输出格式
 * @return 对应格式的格式化器实例
 */
[[nodiscard]] std::unique_ptr<OutputFormatter>
createFormatter(OutputFormat format);

} // namespace czc::cli

#endif // CZC_CLI_OUTPUT_FORMATTER_HPP
