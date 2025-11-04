/**
 * @file token_preprocessor.cpp
 * @brief Token 预处理器实现
 * @author BegoniaHe
 */

#include "czc/token_preprocessor/token_preprocessor.hpp"
#include "czc/diagnostics/diagnostic_code.hpp"
#include "czc/diagnostics/diagnostic.hpp"
#include "czc/lexer/source_tracker.hpp"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <limits>
#include <sstream>
#include <stdexcept>

std::optional<ScientificNotationInfo> ScientificNotationAnalyzer::analyze(
    const std::string &literal,
    const Token *token,
    const AnalysisContext &context)
{
    ScientificNotationInfo info;
    info.original_literal = literal;

    if (!parse_components(literal, info.mantissa, info.exponent))
    {
        return std::nullopt;
    }

    info.has_decimal_point = (info.mantissa.find('.') != std::string::npos);

    info.decimal_digits = count_decimal_digits(info.mantissa);

    info.inferred_type = infer_type(info, token, context);

    info.normalized_value = info.mantissa + "e" + std::to_string(info.exponent);

    return info;
}
bool ScientificNotationAnalyzer::parse_components(const std::string &literal, std::string &mantissa, int64_t &exponent)
{
    // 查找 'e' 或 'E'
    size_t e_pos = literal.find_first_of("eE");
    if (e_pos == std::string::npos)
    {
        return false;
    }

    // 提取尾数部分
    mantissa = literal.substr(0, e_pos);
    if (mantissa.empty())
    {
        return false;
    }

    // 提取指数部分
    std::string exp_str = literal.substr(e_pos + 1);
    if (exp_str.empty())
    {
        return false;
    }

    // 解析指数
    try
    {
        exponent = std::stoll(exp_str);
    }
    catch (const std::invalid_argument &)
    {
        return false;
    }
    catch (const std::out_of_range &)
    {
        return false;
    }

    return true;
}

std::string ScientificNotationAnalyzer::trim_trailing_zeros(const std::string &decimal_part)
{
    if (decimal_part.empty())
    {
        return decimal_part;
    }

    size_t end = decimal_part.length();
    while (end > 0 && decimal_part[end - 1] == '0')
    {
        --end;
    }

    return decimal_part.substr(0, end);
}

size_t ScientificNotationAnalyzer::count_decimal_digits(const std::string &mantissa)
{
    size_t dot_pos = mantissa.find('.');
    if (dot_pos == std::string::npos)
    {
        return 0;
    }

    std::string decimal_part = mantissa.substr(dot_pos + 1);

    std::string trimmed = trim_trailing_zeros(decimal_part);

    return trimmed.length();
}

InferredNumericType ScientificNotationAnalyzer::infer_type(
    const ScientificNotationInfo &info,
    const Token *token,
    const AnalysisContext &context)
{
    // 如果指数为负数，直接推为 FLOAT
    if (info.exponent < 0)
    {
        return InferredNumericType::FLOAT;
    }

    // 如果没有小数点
    if (!info.has_decimal_point)
    {
        // 根据指数范围决定是否能表示为 INT64
        if (fits_in_int64(info.mantissa, info.exponent, token, context))
        {
            return InferredNumericType::INT64;
        }
        else
        {
            return InferredNumericType::FLOAT;
        }
    }

    // 如果有小数点，比较小数位数与指数的大小
    if (info.decimal_digits > static_cast<size_t>(info.exponent))
    {
        // 小数位数 > 指数，推为 FLOAT
        return InferredNumericType::FLOAT;
    }
    else
    {
        // 小数位数 <= 指数，可能转为整数
        // 检查是否在 INT64 范围内
        if (fits_in_int64(info.mantissa, info.exponent, token, context))
        {
            return InferredNumericType::INT64;
        }
        else
        {
            return InferredNumericType::FLOAT;
        }
    }
}

bool ScientificNotationAnalyzer::fits_in_int64(
    const std::string &mantissa,
    int64_t exponent,
    const Token *token,
    const AnalysisContext &context)
{
    auto magnitude = calculate_magnitude(mantissa, exponent, token, context);
    if (!magnitude.has_value())
    {
        return false;
    }

    return magnitude.value() <= MAX_I64_MAGNITUDE;
}

std::optional<int64_t> ScientificNotationAnalyzer::calculate_magnitude(
    const std::string &mantissa,
    int64_t exponent,
    const Token *token,
    const AnalysisContext &context)
{

    // 提取有效数字
    std::string significant_digits_str;
    size_t dot_pos = mantissa.find('.');
    bool has_dot = (dot_pos != std::string::npos);

    for (char ch : mantissa)
    {
        if (std::isdigit(ch))
        {
            significant_digits_str += ch;
        }
    }

    // 去除前导零
    size_t first_nonzero = significant_digits_str.find_first_not_of('0');
    if (first_nonzero == std::string::npos)
    {
        return 0; // 零的特殊情况
    }

    significant_digits_str = significant_digits_str.substr(first_nonzero);
    int64_t num_significant_digits = static_cast<int64_t>(significant_digits_str.length());

    // 计算实际指数
    int64_t actual_exponent = exponent;
    if (has_dot)
    {
        size_t decimal_places = mantissa.length() - dot_pos - 1;
        actual_exponent -= static_cast<int64_t>(decimal_places);
    }

    // 检查加法溢出
    if (actual_exponent > 0 &&
        num_significant_digits > std::numeric_limits<int64_t>::max() - actual_exponent)
    {
        report_overflow(token, mantissa, exponent, context);
        return std::nullopt;
    }

    int64_t magnitude = num_significant_digits + actual_exponent - 1;

    // 只需要检查是否超过 int64 范围
    if (magnitude > MAX_I64_MAGNITUDE)
    {
        report_overflow(token, mantissa, exponent, context);
        return std::nullopt;
    }

    return magnitude;
}

void ScientificNotationAnalyzer::report_overflow(
    const Token *token,
    const std::string &mantissa,
    int64_t exponent,
    const AnalysisContext &context)
{
    if (!context.reporter || !token)
    {
        return;
    }

    std::string literal = mantissa + "e" + std::to_string(exponent);
    auto loc = SourceLocation(
        context.filename,
        token->line,
        token->column,
        token->line,
        token->column + token->value.length());

    auto diag = std::make_shared<Diagnostic>(
        DiagnosticLevel::Error,
        DiagnosticCode::T0001_ScientificIntOverflow,
        loc,
        std::vector<std::string>{literal});

    // 使用 SourceTracker 提取源码行
    SourceTracker temp_tracker(context.source_content, context.filename);
    diag->set_source_line(temp_tracker.get_source_line(token->line));
    context.reporter->report(diag);
}

std::vector<Token> TokenPreprocessor::process(const std::vector<Token> &tokens,
                                              const std::string &filename,
                                              const std::string &source_content,
                                              IDiagnosticReporter *reporter)
{
    std::vector<Token> processed_tokens;
    processed_tokens.reserve(tokens.size());

    for (const auto &token : tokens)
    {
        if (token.token_type == TokenType::ScientificExponent)
        {
            processed_tokens.push_back(process_scientific_token(token, filename, source_content, reporter));
        }
        else
        {
            processed_tokens.push_back(token);
        }
    }

    return processed_tokens;
}

Token TokenPreprocessor::process_scientific_token(const Token &token,
                                                  const std::string &filename,
                                                  const std::string &source_content,
                                                  IDiagnosticReporter *reporter)
{
    AnalysisContext context(filename, source_content, reporter);
    auto info = ScientificNotationAnalyzer::analyze(token.value, &token, context);

    if (!info.has_value())
    {
        // 分析失败，返回 Unknown token
        return Token(TokenType::Unknown, token.value, token.line, token.column);
    }

    TokenType new_type = inferred_type_to_token_type(info->inferred_type);

    return Token(new_type, token.value, token.line, token.column);
}

TokenType TokenPreprocessor::inferred_type_to_token_type(InferredNumericType type)
{
    switch (type)
    {
    case InferredNumericType::INT64:
        return TokenType::Integer; // int64
    case InferredNumericType::FLOAT:
        return TokenType::Float; // float64
    default:
        return TokenType::Unknown;
    }
}

std::string inferred_type_to_string(InferredNumericType type)
{
    switch (type)
    {
    case InferredNumericType::INT64:
        return "INT64";
    case InferredNumericType::FLOAT:
        return "FLOAT";
    default:
        return "Unknown";
    }
}
