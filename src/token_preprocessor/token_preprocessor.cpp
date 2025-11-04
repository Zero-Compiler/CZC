/**
 * @file token_preprocessor.cpp
 * @brief Token 预处理器实现
 * @author BegoniaHe
 * @date 2025-11-04
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

namespace czc {
namespace token_preprocessor {

using namespace czc::diagnostics;
using namespace czc::lexer;
using namespace czc::utils;

std::optional<ScientificNotationInfo>
ScientificNotationAnalyzer::analyze(const std::string &literal,
                                    const Token *token,
                                    const AnalysisContext &context) {
  ScientificNotationInfo info;
  info.original_literal = literal;

  // 1. 将字面量分解为尾数和指数部分。
  //    如果基本结构（如缺少'e'）都不满足，则分析失败。
  if (!parse_components(literal, info.mantissa, info.exponent)) {
    return std::nullopt;
  }

  // 2. 记录尾数是否包含小数点，这是类型推断的关键信息之一。
  info.has_decimal_point = (info.mantissa.find('.') != std::string::npos);

  // 3. 计算有效小数位数（去除尾随零后），用于判断是否能无损转换为整数。
  info.decimal_digits = count_decimal_digits(info.mantissa);

  // 4. 根据尾数、指数和小数点信息，推断其最合适的类型（INT64 或 FLOAT）。
  info.inferred_type = infer_type(info, token, context);

  // 5.
  // 创建一个规范化的表示，虽然当前未使用，但可能对未来的代码生成或常量折叠有用。
  info.normalized_value = info.mantissa + "e" + std::to_string(info.exponent);

  return info;
}

bool ScientificNotationAnalyzer::parse_components(const std::string &literal,
                                                  std::string &mantissa,
                                                  int64_t &exponent) {
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

  // 使用 std::stoll 将指数字符串转换为64位整数。
  // 使用 try-catch 块来处理转换失败的情况（例如，"1e_abc" 或指数超出 int64
  // 范围）。
  try {
    exponent = std::stoll(exp_str);
  } catch (const std::invalid_argument &) {
    return false; // 指数部分不是有效的数字。
  } catch (const std::out_of_range &) {
    return false; // 指数值超出了 int64 的表示范围。
  }

  return true;
}

std::string ScientificNotationAnalyzer::trim_trailing_zeros(
    const std::string &decimal_part) {
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
ScientificNotationAnalyzer::count_decimal_digits(const std::string &mantissa) {
  size_t dot_pos = mantissa.find('.');
  if (dot_pos == std::string::npos) {
    return 0;
  }

  std::string decimal_part = mantissa.substr(dot_pos + 1);

  // 去除尾随的零，因为它们不影响数值，但会影响小数位数的判断。
  // 例如，1.20e2 和 1.2e2 的值相同，都应该能推断为整数 120。
  return trim_trailing_zeros(decimal_part).length();
}

InferredNumericType
ScientificNotationAnalyzer::infer_type(const ScientificNotationInfo &info,
                                       const Token *token,
                                       const AnalysisContext &context) {
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

bool ScientificNotationAnalyzer::fits_in_int64(const std::string &mantissa,
                                               int64_t exponent,
                                               const Token *token,
                                               const AnalysisContext &context) {
  // 通过计算数值的量级（大致是10的多少次方）来快速判断是否可能溢出。
  // 这是一个近似检查，比直接进行高精度计算要快得多。
  auto magnitude = calculate_magnitude(mantissa, exponent, token, context);
  if (!magnitude.has_value()) {
    // 如果量级计算失败（通常意味着超过了 float64 的范围），那它肯定也超出了
    // int64。
    return false;
  }

  // 如果量级小于或等于 int64 能表示的最大量级，我们假设它能被装下。
  return magnitude.value() <= MAX_I64_MAGNITUDE;
}

std::optional<int64_t> ScientificNotationAnalyzer::calculate_magnitude(
    const std::string &mantissa, int64_t exponent, const Token *token,
    const AnalysisContext &context) {

  // --- 通过估算最终数值的位数来判断其量级 ---
  // 例如，1.23e10 的量级大约是 2 + 10 - 1 = 11 (因为 123 * 10^8)。

  // 1. 从尾数中提取所有数字，忽略小数点。 "1.23" -> "123"
  std::string significant_digits_str;
  size_t dot_pos = mantissa.find('.');
  bool has_dot = (dot_pos != std::string::npos);

  for (char ch : mantissa) {
    if (std::isdigit(ch)) {
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
    const Token *token, const std::string &mantissa, int64_t exponent,
    const AnalysisContext &context) {
  if (!context.error_collector || !token) {
    return;
  }

  std::string literal = mantissa + "e" + std::to_string(exponent);
  auto loc = SourceLocation(context.filename, token->line, token->column,
                            token->line, token->column + token->value.length());

  context.error_collector->add(DiagnosticCode::T0002_ScientificFloatOverflow,
                               loc, {literal});
}

void TokenPreprocessor::report_error(DiagnosticCode code, const Token *token,
                                     const std::vector<std::string> &args) {
  if (!token) {
    return;
  }
  auto loc = SourceLocation("<unknown>", token->line, token->column,
                            token->line, token->column + token->value.length());
  error_collector.add(code, loc, args);
}

std::vector<Token>
TokenPreprocessor::process(const std::vector<Token> &tokens,
                           const std::string &filename,
                           const std::string &source_content) {
  std::vector<Token> processed_tokens;
  processed_tokens.reserve(tokens.size());

  // 遍历词法分析器生成的 Token 流。
  for (const auto &token : tokens) {
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
    const Token &token, const std::string &filename,
    const std::string &source_content) {
  AnalysisContext context(filename, source_content, &error_collector);
  auto info = ScientificNotationAnalyzer::analyze(token.value, &token, context);

  // 如果分析器无法处理该字面量（例如，因为它溢出了 float64），
  // 则将其标记为 Unknown 类型的 Token，以便后续阶段可以报告错误。
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

} // namespace token_preprocessor
} // namespace czc
