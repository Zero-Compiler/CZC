/**
 * @file text_formatter.hpp
 * @brief 文本格式化器定义。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-30
 *
 * @details
 *   实现人类可读的文本输出格式。
 */

#ifndef CZC_CLI_OUTPUT_TEXT_FORMATTER_HPP
#define CZC_CLI_OUTPUT_TEXT_FORMATTER_HPP

#include "czc/common/config.hpp"

#include "czc/cli/output/formatter.hpp"

namespace czc::cli {

/**
 * @brief 文本格式化器。
 *
 * @details
 *   将 Token 和错误信息格式化为人类可读的文本格式。
 */
class TextFormatter : public OutputFormatter {
public:
  TextFormatter() = default;
  ~TextFormatter() override = default;

  /**
   * @brief 格式化 Token 列表为文本。
   *
   * @param tokens Token 列表
   * @param sm 源码管理器
   * @return 格式化后的文本
   */
  [[nodiscard]] std::string
  formatTokens(std::span<const lexer::Token> tokens,
               const lexer::SourceManager &sm) const override;

  /**
   * @brief 格式化错误列表为文本。
   *
   * @param errors 错误列表
   * @param sm 源码管理器
   * @return 格式化后的文本
   */
  [[nodiscard]] std::string
  formatErrors(std::span<const lexer::LexerError> errors,
               const lexer::SourceManager &sm) const override;
};

} // namespace czc::cli

#endif // CZC_CLI_OUTPUT_TEXT_FORMATTER_HPP
