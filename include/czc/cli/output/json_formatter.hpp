/**
 * @file json_formatter.hpp
 * @brief JSON 格式化器定义。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-30
 *
 * @details
 *   使用 glaze 库实现 JSON 输出格式。
 */

#ifndef CZC_CLI_OUTPUT_JSON_FORMATTER_HPP
#define CZC_CLI_OUTPUT_JSON_FORMATTER_HPP

#include "czc/common/config.hpp"

#include "czc/cli/output/formatter.hpp"

namespace czc::cli {

/**
 * @brief JSON 格式化器。
 *
 * @details
 *   使用 glaze 库将 Token 和错误信息格式化为 JSON 格式。
 */
class JsonFormatter : public OutputFormatter {
public:
  JsonFormatter() = default;
  ~JsonFormatter() override = default;

  /**
   * @brief 格式化 Token 列表为 JSON。
   *
   * @param tokens Token 列表
   * @param sm 源码管理器
   * @return 格式化后的 JSON 字符串
   */
  [[nodiscard]] std::string
  formatTokens(std::span<const lexer::Token> tokens,
               const lexer::SourceManager &sm) const override;

  /**
   * @brief 格式化错误列表为 JSON。
   *
   * @param errors 错误列表
   * @param sm 源码管理器
   * @return 格式化后的 JSON 字符串
   */
  [[nodiscard]] std::string
  formatErrors(std::span<const lexer::LexerError> errors,
               const lexer::SourceManager &sm) const override;
};

} // namespace czc::cli

#endif // CZC_CLI_OUTPUT_JSON_FORMATTER_HPP
