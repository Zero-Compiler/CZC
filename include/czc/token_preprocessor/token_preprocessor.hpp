#ifndef CZC_TOKEN_PREPROCESSOR_HPP
#define CZC_TOKEN_PREPROCESSOR_HPP

#include "czc/lexer/token.hpp"
#include <string>
#include <vector>
#include <optional>
#include <cstdint>

enum class InferredNumericType
{
    INT64, // i64
    FLOAT  // f64
};

struct ScientificNotationInfo
{
    std::string original_literal;      // 原始字符串 (如 "1.5e10")
    std::string mantissa;              // 尾数部分 (如 "1.5")
    int64_t exponent;                  // 指数部分 (如 10)
    bool has_decimal_point;            // 是否包含小数点
    size_t decimal_digits;             // 小数点后的有效位数（去除尾随0后）
    InferredNumericType inferred_type; // 推断的类型
    std::string normalized_value;      // 规范化后的值（用于后续计算）
};

// 科学计数法分析器
class ScientificNotationAnalyzer
{
public:
    // 解析科学计数法字面量
    static std::optional<ScientificNotationInfo> analyze(const std::string &literal);

private:
    // 解析尾数和指数
    static bool parse_components(const std::string &literal, std::string &mantissa, int64_t &exponent);

    // 去除小数部分的尾随零
    static std::string trim_trailing_zeros(const std::string &decimal_part);

    // 计算去除尾随零后的小数位数
    static size_t count_decimal_digits(const std::string &mantissa);

    // 根据规则推断类型
    static InferredNumericType infer_type(const ScientificNotationInfo &info);

    // 检查整数是否在INT64范围内
    static bool fits_in_int64(const std::string &mantissa, int64_t exponent);

    // 计算实际数值的位数（用于溢出判断）
    static std::optional<int64_t> calculate_magnitude(const std::string &mantissa, int64_t exponent);
};

// Token预处理器
class TokenPreprocessor
{
public:
    // 处理Token流，对科学计数法Token进行类型推断
    static std::vector<Token> process(const std::vector<Token> &tokens);

    // 处理单个科学计数法Token
    static Token process_scientific_token(const Token &token);

private:
    // 将推断类型转换为TokenType
    static TokenType inferred_type_to_token_type(InferredNumericType type);
};

// 将InferredNumericType转换为字符串
std::string inferred_type_to_string(InferredNumericType type);

#endif // CZC_TOKEN_PREPROCESSOR_HPP
