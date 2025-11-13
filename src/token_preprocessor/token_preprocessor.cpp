/**
 * @file token_preprocessor.cpp
 * @brief `TokenPreprocessor` 类的功能实现。
 * @author BegoniaHe
 * @date 2025-11-11
 */

#include "czc/token_preprocessor/token_preprocessor.hpp"

#include "czc/diagnostics/diagnostic.hpp"
#include "czc/diagnostics/diagnostic_code.hpp"
#include "czc/utils/source_tracker.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <limits>
#include <sstream>
#include <stdexcept>

namespace czc::token_preprocessor {

using namespace czc::diagnostics;
using namespace czc::lexer;
using namespace czc::utils;

std::optional<ScientificNotationInfo>
ScientificNotationAnalyzer::analyze(const std::string& literal,
                                    const Token* token,
                                    const AnalysisContext& context) {
  ScientificNotationInfo info;
  info.original_literal = literal;

  // --- 分析流程 ---
  // 1. 将字面量分解为尾数和指数部分。
  //    这是最基础的结构验证，如果连 'e'/'E' 都没有，则直接失败。
  if (!parse_components(literal, info.mantissa, info.exponent)) {
    return std::nullopt;
  }

  // 2. 记录尾数是否包含小数点，这是类型推断的关键信息之一。
  info.has_decimal_point = (info.mantissa.find('.') != std::string::npos);

  // 3. 计算有效小数位数（去除尾随零后），用于判断是否能无损转换为整数。
  info.decimal_digits = count_decimal_digits(info.mantissa);

  // 4. 根据尾数、指数和小数点信息，推断其最合适的类型（INT64 或 FLOAT）。
  info.inferred_type = infer_type(info, token, context);

  // 5. 创建一个规范化的字符串表示。
  //    NOTE: 当前此值未被使用，但保留它是为了将来可能的扩展，例如
  //          需要进行更高精度的常量折叠或代码生成时，有一个统一的
  //          中间表示会很有用。
  info.normalized_value = info.mantissa + "e" + std::to_string(info.exponent);

  return info;
}

bool ScientificNotationAnalyzer::parse_components(const std::string& literal,
                                                  std::string& mantissa,
                                                  int64_t& exponent) {
  // 科学计数法的核心是 'e' 或 'E' 分隔符。
  size_t e_pos = literal.find_first_of("eE");
  if (e_pos == std::string::npos) {
    return false; // 如果没有 'e' 或 'E'，则不是有效的科学计数法表示。
  }

  mantissa = literal.substr(0, e_pos);
  if (mantissa.empty()) {
    return false; // 尾数不能为空。
  }

  std::string exp_str = literal.substr(e_pos + 1);
  if (exp_str.empty()) {
    return false; // 指数不能为空。
  }

  // NOTE: 使用 std::stoll 将指数字符串转换为64位整数。stoll 提供了内置的
  //       错误处理机制：
  //       - `std::invalid_argument`: 如果指数部分包含非数字字符（除了
  //         可选的前导 `+` 或 `-`），例如 "1e_abc"。
  //       - `std::out_of_range`: 如果指数的值超出了 `int64_t` 的表示范围。
  //       通过捕获这些异常，我们可以稳健地处理格式错误的科学计数法。
  try {
    exponent = std::stoll(exp_str);
  } catch (const std::invalid_argument&) {
    return false;
  } catch (const std::out_of_range&) {
    return false;
  }

  return true;
}

std::string ScientificNotationAnalyzer::trim_trailing_zeros(
    const std::string& decimal_part) {
  if (decimal_part.empty()) {
    return decimal_part;
  }
  size_t end = decimal_part.find_last_not_of('0');
  if (end == std::string::npos) {
    return ""; // 字符串全为 '0'
  }
  return decimal_part.substr(0, end + 1);
}

size_t
ScientificNotationAnalyzer::count_decimal_digits(const std::string& mantissa) {
  size_t dot_pos = mantissa.find('.');
  if (dot_pos == std::string::npos) {
    return 0;
  }

  std::string decimal_part = mantissa.substr(dot_pos + 1);

  // NOTE: 在计算小数位数之前，必须去除尾随的零。这是因为它们不影响数值，
  //       但会影响类型推断。例如，`1.20e2` 和 `1.2e2` 的值相同（都是 120），
  //       我们希望两者都能被正确地推断为整数。如果不去除尾随零，`1.20e2`
  //       的小数位数会被误判为 2，导致 `exponent (2)` 不大于 `decimal_digits
  //       (2)`， 从而错误地进入 `fits_in_int64` 检查。
  return trim_trailing_zeros(decimal_part).length();
}

InferredNumericType
ScientificNotationAnalyzer::infer_type(const ScientificNotationInfo& info,
                                       const Token* token,
                                       const AnalysisContext& context) {
  // --- 类型推断的核心逻辑 ---
  // 目标是尽可能将数值表示为整数（INT64），只有在必要时才使用浮点数（FLOAT）。

  // 1. 如果指数为负，数值必然是小数（除非尾数为0），因此推断为 FLOAT。
  //    例如：1e-2 -> 0.01
  if (info.exponent < 0) {
    return InferredNumericType::FLOAT;
  }

  // 2. 如果尾数没有小数点（例如 123e4），则其是否为整数仅取决于其最终值是否在
  // INT64 范围内。
  if (!info.has_decimal_point) {
    return fits_in_int64(info.mantissa, info.exponent, token, context)
               ? InferredNumericType::INT64
               : InferredNumericType::FLOAT;
  }

  // 3. 如果尾数有小数点（例如 1.23e5），需要判断指数是否足以“消除”所有小数位。
  //    如果指数小于小数位数，则最终结果必然是小数。
  if (info.decimal_digits > static_cast<size_t>(info.exponent)) {
    // 例如：1.23e1 -> 12.3，仍然是小数。
    return InferredNumericType::FLOAT;
  }

  // 4. 如果指数足以或超过小数位数（例如 1.23e2 -> 123），
  //    则该数值在数学上是整数。接下来只需检查这个整数是否在 INT64 范围内。
  return fits_in_int64(info.mantissa, info.exponent, token, context)
             ? InferredNumericType::INT64
             : InferredNumericType::FLOAT;
}

bool ScientificNotationAnalyzer::fits_in_int64(const std::string& mantissa,
                                               int64_t exponent,
                                               const Token* token,
                                               const AnalysisContext& context) {
  // NOTE: 这是一个关键的优化。我们不进行实际的高精度数学计算来判断溢出，
  //       因为这会非常慢且复杂。相反，我们通过 `calculate_magnitude`
  //       来估算数值的数量级（即它大约是 10 的多少次方）。
  //       这是一个非常快速的近似检查。
  auto magnitude = calculate_magnitude(mantissa, exponent, token, context);
  if (!magnitude.has_value()) {
    // 如果量级计算本身就失败了（通常意味着该数值甚至超出了 float64 的
    // 表示范围），那么它肯定也无法放入 int64。
    return false;
  }

  // 如果量级超过 int64 能表示的最大量级，报告整数溢出错误
  if (magnitude.value() > MAX_I64_MAGNITUDE) {
    if (context.error_collector && token) {
      std::string literal = mantissa + "e" + std::to_string(exponent);
      auto loc =
          SourceLocation(context.filename, token->line, token->column,
                         token->line, token->column + token->value.length());
      TPError error(DiagnosticCode::T0001_ScientificIntOverflow, loc,
                    {literal});
      context.error_collector->add(error);
    }
    return false;
  }

  return true;
}

std::optional<int64_t> ScientificNotationAnalyzer::calculate_magnitude(
    const std::string& mantissa, int64_t exponent, const Token* token,
    const AnalysisContext& context) {
  // --- 通过估算最终数值的位数来判断其量级 ---
  // NOTE: 这个算法的目的是在不执行实际浮点运算的情况下，估算出一个科学
  //       计数法字面量的数量级（magnitude），即它约等于 10 的多少次方。
  //       例如，对于 `1.23e10`，我们将其转换为 `123 * 10^8`。
  //       `123` 有 3 位有效数字，所以它的值在 `10^2` 和 `10^3` 之间。
  //       因此，`123 * 10^8` 的值在 `10^10` 和 `10^11` 之间，其量级可以
  //       估算为 `(3 - 1) + 8 = 10`。

  // 1. 从尾数中提取所有数字，忽略小数点。 "1.23" -> "123"
  std::string significant_digits_str;
  size_t dot_pos = mantissa.find('.');
  bool has_dot = (dot_pos != std::string::npos);

  for (char ch : mantissa) {
    if (std::isdigit(static_cast<unsigned char>(ch))) {
      significant_digits_str += ch;
    }
  }

  // 2. 去除前导零，因为它们不影响有效数字的位数。 "00123" -> "123"
  size_t first_nonzero = significant_digits_str.find_first_not_of('0');
  if (first_nonzero == std::string::npos) {
    return 0; // 如果尾数是0（例如 0.0e5），则量级为0。
  }
  significant_digits_str = significant_digits_str.substr(first_nonzero);
  int64_t num_significant_digits =
      static_cast<int64_t>(significant_digits_str.length());

  // 3. 调整指数，以反映小数点的位置。
  //    例如，对于 "1.23e10"，我们将 "123" 视为基数，
  //    原来的指数 10 需要减去小数位数 2，得到实际指数 8。
  //    所以数值等价于 123 * 10^8。
  int64_t actual_exponent = exponent;
  if (has_dot) {
    size_t decimal_places = mantissa.length() - dot_pos - 1;
    actual_exponent -= static_cast<int64_t>(decimal_places);
  }

  // 4. 计算最终的量级。
  //    量级 = (有效数字位数 - 1) + 实际指数。
  //    例如，123 * 10^8，有效数字是3位，所以量级是 (3-1) + 8 = 10。
  //    这表示该数在 10^10 到 10^11 之间。
  int64_t magnitude = num_significant_digits + actual_exponent - 1;

  // 5. 检查计算出的量级是否超出了 float64 的表示范围。
  if (magnitude > MAX_F64_MAGNITUDE) {
    // 如果是，则报告一个硬溢出错误，并认为分析失败。
    report_overflow(token, mantissa, exponent, context);
    return std::nullopt;
  }

  return magnitude;
}

void ScientificNotationAnalyzer::report_overflow(
    const Token* token, const std::string& mantissa, int64_t exponent,
    const AnalysisContext& context) {
  if (!context.error_collector || !token) {
    return;
  }

  std::string literal = mantissa + "e" + std::to_string(exponent);
  auto loc = SourceLocation(context.filename, token->line, token->column,
                            token->line, token->column + token->value.length());

  TPError error(DiagnosticCode::T0002_ScientificFloatOverflow, loc, {literal});
  context.error_collector->add(error);
}

void TokenPreprocessor::report_error(DiagnosticCode code, const Token* token,
                                     const std::vector<std::string>& args) {
  if (!token) {
    return;
  }
  auto loc = SourceLocation("<unknown>", token->line, token->column,
                            token->line, token->column + token->value.length());
  TPError error(code, loc, args);
  error_collector.add(error);
}

std::vector<Token>
TokenPreprocessor::process(const std::vector<Token>& tokens,
                           const std::string& filename,
                           const std::string& source_content) {
  std::vector<Token> processed_tokens;
  processed_tokens.reserve(tokens.size());

  // 遍历词法分析器生成的 Token 流。
  for (const auto& token : tokens) {
    // 只对 `ScientificExponent` 类型的 Token 进行特殊处理。
    if (token.token_type == TokenType::ScientificExponent) {
      // 调用辅助函数处理该 Token，并将结果放入新的列表中。
      processed_tokens.push_back(
          process_scientific_token(token, filename, source_content));
    } else {
      // 其他类型的 Token 直接复制到新列表中。
      processed_tokens.push_back(token);
    }
  }

  return processed_tokens;
}

Token TokenPreprocessor::process_scientific_token(
    const Token& token, const std::string& filename,
    const std::string& source_content) {
  AnalysisContext context(filename, source_content, &error_collector);
  auto info = ScientificNotationAnalyzer::analyze(token.value, &token, context);

  // NOTE: 如果 `analyze` 返回 `std::nullopt`，通常意味着该字面量的值
  //       过大，甚至超出了 `double` 的表示范围（在 `calculate_magnitude`
  //       中检测到）。在这种情况下，错误已经被报告，我们只需将此 Token
  //       标记为 `Unknown`，以防止后续阶段（如语法分析）尝试处理这个无效值。
  if (!info.has_value()) {
    return Token(TokenType::Unknown, token.value, token.line, token.column);
  }

  // 根据分析结果，将 Token 类型从 `ScientificExponent` 转换为更具体的 `Integer`
  // 或 `Float`。
  TokenType new_type = inferred_type_to_token_type(info->inferred_type);

  // 返回一个新的 Token，其类型已更新，但值和位置信息保持不变。
  return Token(new_type, token.value, token.line, token.column);
}

TokenType
TokenPreprocessor::inferred_type_to_token_type(InferredNumericType type) {
  switch (type) {
  case InferredNumericType::INT64:
    return TokenType::Integer;
  case InferredNumericType::FLOAT:
    return TokenType::Float;
  default:
    return TokenType::Unknown;
  }
}

std::string inferred_type_to_string(InferredNumericType type) {
  switch (type) {
  case InferredNumericType::INT64:
    return "INT64";
  case InferredNumericType::FLOAT:
    return "FLOAT";
  default:
    return "Unknown";
  }
}

} // namespace czc::token_preprocessor
