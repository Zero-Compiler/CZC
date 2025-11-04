/**
 * @file token_preprocessor.hpp
 * @brief Token 预处理器类定义
 * @author BegoniaHe
 * @date 2025-11-04
 */

#ifndef CZC_TOKEN_PREPROCESSOR_HPP
#define CZC_TOKEN_PREPROCESSOR_HPP

#include "czc/lexer/token.hpp"
#include "error_collector.hpp"
#include "czc/diagnostics/diagnostic_reporter.hpp"
#include <string>
#include <vector>
#include <optional>
#include <cstdint>

constexpr int MAX_I64_MAGNITUDE = 18;  // int64 最大位数 (约 10^18)
constexpr int MAX_F64_MAGNITUDE = 308; // float64 最大位数 (约 10^308)

/**
 * @brief 推断的数值类型枚举
 */
enum class InferredNumericType
{
    INT64, // 64位整数
    FLOAT  // 64位浮点数
};

/**
 * @brief 科学计数法信息结构
 */
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

/**
 * @brief 分析上下文结构
 */
struct AnalysisContext
{
    const std::string &filename;       // 文件名
    const std::string &source_content; // 源码内容
    TPErrorCollector *error_collector; // 错误收集器

    /**
     * @brief 构造函数
     * @param fname 文件名
     * @param source 源码内容
     * @param collector 错误收集器指针，默认为 nullptr
     */
    AnalysisContext(const std::string &fname,
                    const std::string &source,
                    TPErrorCollector *collector = nullptr)
        : filename(fname), source_content(source), error_collector(collector) {}
};

/**
 * @brief 科学计数法分析器类
 */
class ScientificNotationAnalyzer
{
public:
    /**
     * @brief 解析科学计数法字面量
     * @param literal 字面量字符串
     * @param token Token 指针
     * @param context 分析上下文
     * @return 科学计数法信息的可选值
     */
    static std::optional<ScientificNotationInfo> analyze(
        const std::string &literal,
        const Token *token,
        const AnalysisContext &context);

private:
    /**
     * @brief 解析尾数和指数
     * @param literal 字面量字符串
     * @param mantissa 尾数（输出参数）
     * @param exponent 指数（输出参数）
     * @return 解析是否成功
     */
    static bool parse_components(const std::string &literal, std::string &mantissa, int64_t &exponent);

    /**
     * @brief 去除小数部分的尾随零
     * @param decimal_part 小数部分字符串
     * @return 去除尾随零后的字符串
     */
    static std::string trim_trailing_zeros(const std::string &decimal_part);

    /**
     * @brief 计算去除尾随零后的小数位数
     * @param mantissa 尾数字符串
     * @return 小数位数
     */
    static size_t count_decimal_digits(const std::string &mantissa);

    /**
     * @brief 根据规则推断类型
     * @param info 科学计数法信息
     * @param token Token 指针
     * @param context 分析上下文
     * @return 推断的数值类型
     */
    static InferredNumericType infer_type(
        const ScientificNotationInfo &info,
        const Token *token,
        const AnalysisContext &context);

    /**
     * @brief 检查整数是否在 INT64 范围内
     * @param mantissa 尾数字符串
     * @param exponent 指数
     * @param token Token 指针
     * @param context 分析上下文
     * @return 是否在范围内
     */
    static bool fits_in_int64(
        const std::string &mantissa,
        int64_t exponent,
        const Token *token,
        const AnalysisContext &context);

    /**
     * @brief 计算实际数值的位数
     * @param mantissa 尾数字符串
     * @param exponent 指数
     * @param token Token 指针
     * @param context 分析上下文
     * @return 位数的可选值
     */
    static std::optional<int64_t> calculate_magnitude(
        const std::string &mantissa,
        int64_t exponent,
        const Token *token,
        const AnalysisContext &context);

    /**
     * @brief 报告溢出错误
     * @param token Token 指针
     * @param mantissa 尾数字符串
     * @param exponent 指数
     * @param context 分析上下文
     */
    static void report_overflow(
        const Token *token,
        const std::string &mantissa,
        int64_t exponent,
        const AnalysisContext &context);
};

/**
 * @brief 在语法分析前对 Token 流进行分析和转换。
 * @details
 *   此预处理器目前专注于处理科学计数法字面量。词法分析器会将所有
 *   科学计数法形式的数字（例如 `1.23e10`）识别为 `ScientificExponent` 类型。
 *   TokenPreprocessor 则会进一步分析这些字面量，根据其值和形式推断
 *   它们应该是整数（`INT64`）还是浮点数（`FLOAT`），并相应地更新 Token 的类型。
 *   它还会检查数值溢出等问题。
 * @note 此类不是线程安全的。
 */
class TokenPreprocessor
{
private:
    /// @brief 用于收集在预处理期间遇到的所有错误。
    TPErrorCollector error_collector;

    /**
     * @brief 辅助函数，用于在错误收集器中记录一个新错误。
     * @param[in] code 错误的诊断代码。
     * @param[in] token 发生错误的 Token。
     * @param[in] args (可选) 格式化错误消息所需的参数。
     */
    void report_error(DiagnosticCode code,
                      const Token *token,
                      const std::vector<std::string> &args = {});

public:
    /**
     * @brief 默认构造函数。
     */
    TokenPreprocessor() = default;

    /**
     * @brief 处理一个完整的 Token 列表。
     * @details
     *   遍历输入的 Token 向量，当遇到 `ScientificExponent` 类型的 Token 时，
     *   调用 `process_scientific_token` 对其进行处理和转换，然后返回新的 Token 列表。
     * @param[in] tokens 从词法分析器获得的原始 Token 列表。
     * @param[in] filename 源代码的文件名。
     * @param[in] source_content 完整的源代码内容。
     * @return 返回经过处理和类型调整的 Token 列表。
     */
    std::vector<Token> process(const std::vector<Token> &tokens,
                               const std::string &filename,
                               const std::string &source_content);

    /**
     * @brief 分析并转换单个科学计数法 Token。
     * @details
     *   使用 ScientificNotationAnalyzer 对 Token 的值进行分析，推断其类型
     *   （INT64 或 FLOAT），并检查是否存在溢出。
     *   返回一个更新了类型和值的新 Token。
     * @param[in] token 一个类型为 `ScientificExponent` 的 Token。
     * @param[in] filename 源代码的文件名。
     * @param[in] source_content 完整的源代码内容。
     * @return 返回类型被更新为 `Integer` 或 `Float` 的新 Token。
     */
    Token process_scientific_token(const Token &token,
                                   const std::string &filename,
                                   const std::string &source_content);

    /**
     * @brief 获取对内部错误收集器的访问权限。
     * @return 对 TPErrorCollector 对象的常量引用。
     */
    const TPErrorCollector &get_errors() const { return error_collector; }

private:
    /**
     * @brief 将内部的 InferredNumericType 映射到词法分析器的 TokenType。
     * @param[in] type 推断出的数值类型。
     * @return 对应的 TokenType (`Integer` 或 `Float`)。
     */
    static TokenType inferred_type_to_token_type(InferredNumericType type);
};

/**
 * @brief 将 InferredNumericType 转换为字符串
 * @param type 推断的数值类型
 * @return 类型的字符串表示
 */
std::string inferred_type_to_string(InferredNumericType type);

#endif // CZC_TOKEN_PREPROCESSOR_HPP
