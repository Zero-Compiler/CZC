/**
 * @file token_preprocessor.cpp
 * @brief Token 预处理器实现
 * @author BegoniaHe
 */

#include "czc/token_preprocessor/token_preprocessor.hpp"
#include "czc/diagnostics/diagnostic_code.hpp"
#include "czc/diagnostics/diagnostic.hpp"
#include "czc/utils/source_tracker.hpp"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <limits>
#include <sstream>
#include <stdexcept>

/**
 * @brief 分析科学计数法字面量
 * @param literal 字面量字符串
 * @param token 对应的 Token
 * @param context 分析上下文
 * @return 如果分析成功, 返回包含分析信息的 ScientificNotationInfo, 否则返回 std::nullopt
 */
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
/**
 * @brief 解析科学计数法的尾数和指数
 * @param literal 字面量字符串
 * @param mantissa 用于存储尾数的字符串
 * @param exponent 用于存储指数的整数
 * @return 如果解析成功返回 true, 否则返回 false
 */
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

/**
 * @brief 去除小数部分末尾的零
 * @param decimal_part 小数部分的字符串
 * @return 去除末尾零后的字符串
 */
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

/**
 * @brief 计算尾数的小数位数 (去除末尾零后)
 * @param mantissa 尾数字符串
 * @return 小数位数
 */
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

/**
 * @brief 推断科学计数法字面量的数值类型
 * @param info 科学计数法分析信息
 * @param token 对应的 Token
 * @param context 分析上下文
 * @return 推断出的数值类型 (INT64 或 FLOAT)
 */
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
            // 超过 INT64 范围，推为 FLOAT
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
            // 超过 INT64 范围，推为 FLOAT
            return InferredNumericType::FLOAT;
        }
    }
}

/**
 * @brief 检查科学计数法表示的数值是否在 int64 范围内
 * @param mantissa 尾数
 * @param exponent 指数
 * @param token 对应的 Token
 * @param context 分析上下文
 * @return 如果在范围内返回 true, 否则返回 false
 */
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

/**
 * @brief 计算数值的量级 (大致为10的多少次方)
 * @param mantissa 尾数
 * @param exponent 指数
 * @param token 对应的 Token
 * @param context 分析上下文
 * @return 如果计算成功, 返回量级, 否则返回 std::nullopt
 */
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
        // 溢出，返回一个大于 MAX_I64_MAGNITUDE 的值，让上层判断
        return MAX_I64_MAGNITUDE + 1;
    }

    int64_t magnitude = num_significant_digits + actual_exponent - 1;

    // 检查是否超过 float64 的范围
    if (magnitude > MAX_F64_MAGNITUDE)
    {
        // 超过 float64 范围，报告溢出错误
        report_overflow(token, mantissa, exponent, context);
        return std::nullopt;
    }

    return magnitude;
}

/**
 * @brief 报告整数溢出错误
 * @param token 对应的 Token
 * @param mantissa 尾数
 * @param exponent 指数
 * @param context 分析上下文
 */
/**
 * @brief 报告错误到错误收集器
 * @param token Token 指针
 * @param mantissa 尾数字符串
 * @param exponent 指数
 * @param context 分析上下文
 */
void ScientificNotationAnalyzer::report_overflow(
    const Token *token,
    const std::string &mantissa,
    int64_t exponent,
    const AnalysisContext &context)
{
    if (!context.error_collector || !token)
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

    context.error_collector->add(DiagnosticCode::T0002_ScientificFloatOverflow, loc, {literal});
}

/**
 * @brief 报告错误
 * @param code 诊断代码
 * @param token Token 指针
 * @param args 消息参数列表
 */
void TokenPreprocessor::report_error(DiagnosticCode code,
                                     const Token *token,
                                     const std::vector<std::string> &args)
{
    if (!token)
    {
        return;
    }
    auto loc = SourceLocation(
        "<unknown>",
        token->line,
        token->column,
        token->line,
        token->column + token->value.length());
    error_collector.add(code, loc, args);
}

/**
 * @brief 预处理 Token 流, 主要处理科学计数法
 * @param tokens 原始 Token 流
 * @param filename 文件名
 * @param source_content 源码内容
 * @return 处理后的 Token 流
 */
std::vector<Token> TokenPreprocessor::process(const std::vector<Token> &tokens,
                                              const std::string &filename,
                                              const std::string &source_content)
{
    std::vector<Token> processed_tokens;
    processed_tokens.reserve(tokens.size());

    for (const auto &token : tokens)
    {
        if (token.token_type == TokenType::ScientificExponent)
        {
            processed_tokens.push_back(process_scientific_token(token, filename, source_content));
        }
        else
        {
            processed_tokens.push_back(token);
        }
    }

    return processed_tokens;
}

/**
 * @brief 处理单个科学计数法 Token
 * @param token 待处理的 Token
 * @param filename 文件名
 * @param source_content 源码内容
 * @return 处理后的 Token
 */
Token TokenPreprocessor::process_scientific_token(const Token &token,
                                                  const std::string &filename,
                                                  const std::string &source_content)
{
    AnalysisContext context(filename, source_content, &error_collector);
    auto info = ScientificNotationAnalyzer::analyze(token.value, &token, context);

    if (!info.has_value())
    {
        // 分析失败，返回 Unknown token
        return Token(TokenType::Unknown, token.value, token.line, token.column);
    }

    TokenType new_type = inferred_type_to_token_type(info->inferred_type);

    return Token(new_type, token.value, token.line, token.column);
}

/**
 * @brief 将推断出的数值类型转换为 TokenType
 * @param type 推断出的数值类型
 * @return 对应的 TokenType
 */
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

/**
 * @brief 将推断出的数值类型转换为字符串
 * @param type 推断出的数值类型
 * @return 类型的字符串表示
 */
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
