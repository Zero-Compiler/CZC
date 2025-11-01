#include "czc/token_preprocessor/token_preprocessor.hpp"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <limits>
#include <sstream>

std::optional<ScientificNotationInfo> ScientificNotationAnalyzer::analyze(const std::string &literal)
{
    ScientificNotationInfo info;
    info.original_literal = literal;

    // 解析尾数和指数
    if (!parse_components(literal, info.mantissa, info.exponent))
    {
        return std::nullopt;
    }

    // 检查是否有小数点
    info.has_decimal_point = (info.mantissa.find('.') != std::string::npos);

    // 计算小数位数（去除尾随零后）
    info.decimal_digits = count_decimal_digits(info.mantissa);

    // 推断类型
    info.inferred_type = infer_type(info);

    // 生成规范化值
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
    catch (...)
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

InferredNumericType ScientificNotationAnalyzer::infer_type(const ScientificNotationInfo &info)
{
    // 规则1: 如果指数为负数，直接推为 FLOAT
    if (info.exponent < 0)
    {
        return InferredNumericType::FLOAT;
    }

    // 规则2: 如果没有小数点（纯整数形式）
    if (!info.has_decimal_point)
    {
        // 根据指数范围决定是否能表示为 INT64
        if (fits_in_int64(info.mantissa, info.exponent))
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
        if (fits_in_int64(info.mantissa, info.exponent))
        {
            return InferredNumericType::INT64;
        }
        else
        {
            return InferredNumericType::FLOAT;
        }
    }
}

bool ScientificNotationAnalyzer::fits_in_int64(const std::string &mantissa, int64_t exponent)
{
    // 计算实际数值的数量级
    auto magnitude = calculate_magnitude(mantissa, exponent);
    if (!magnitude.has_value())
    {
        return false; // 无法计算，保守地返回 false
    }

    // INT64 的最大数量级为 18
    return magnitude.value() <= 18;
}

std::optional<int64_t> ScientificNotationAnalyzer::calculate_magnitude(const std::string &mantissa, int64_t exponent)
{
    std::string digits_only;
    size_t dot_pos = mantissa.find('.');
    bool has_dot = (dot_pos != std::string::npos);

    for (char ch : mantissa)
    {
        if (std::isdigit(ch))
        {
            digits_only += ch;
        }
    }

    // 去除前导零
    size_t first_nonzero = digits_only.find_first_not_of('0');
    if (first_nonzero == std::string::npos)
    {
        // 全是零，数量级为 0
        return 0;
    }

    digits_only = digits_only.substr(first_nonzero);
    int64_t significant_digits = static_cast<int64_t>(digits_only.length());

    // 如果有小数点，需要调整指数
    int64_t actual_exponent = exponent;
    if (has_dot)
    {
        // 小数点后的位数
        size_t decimal_places = mantissa.length() - dot_pos - 1;
        actual_exponent -= static_cast<int64_t>(decimal_places);
    }

    // 总的数量级 = 有效数字位数 + 实际指数 - 1
    int64_t magnitude = significant_digits + actual_exponent - 1;

    return magnitude;
}

// ==================== TokenPreprocessor 实现 ====================

std::vector<Token> TokenPreprocessor::process(const std::vector<Token> &tokens)
{
    std::vector<Token> processed_tokens;
    processed_tokens.reserve(tokens.size());

    for (const auto &token : tokens)
    {
        if (token.token_type == TokenType::ScientificExponent)
        {
            processed_tokens.push_back(process_scientific_token(token));
        }
        else
        {
            processed_tokens.push_back(token);
        }
    }

    return processed_tokens;
}

Token TokenPreprocessor::process_scientific_token(const Token &token)
{
    auto info = ScientificNotationAnalyzer::analyze(token.value);

    if (!info.has_value())
    {
        return token;
    }

    TokenType new_type = inferred_type_to_token_type(info->inferred_type);

    return Token(new_type, token.value, token.line, token.column);
}

TokenType TokenPreprocessor::inferred_type_to_token_type(InferredNumericType type)
{
    switch (type)
    {
    case InferredNumericType::INT64:
        return TokenType::Integer;
    case InferredNumericType::FLOAT:
        return TokenType::Float;
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
