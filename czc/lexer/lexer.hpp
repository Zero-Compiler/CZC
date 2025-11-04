/**
 * @file lexer.hpp
 * @brief 词法分析器类定义
 * @author BegoniaHe
 */

#ifndef CZC_LEXER_HPP
#define CZC_LEXER_HPP

#include "token.hpp"
#include "source_tracker.hpp"
#include "error_collector.hpp"
#include "czc/diagnostics/diagnostic_code.hpp"
#include <vector>
#include <optional>
#include <string>
#include <memory>

/**
 * @brief 词法分析器类
 */
class Lexer
{
private:
    SourceTracker tracker;            ///< 源码跟踪器
    std::optional<char> current_char; ///< 当前字符
    ErrorCollector error_collector;   ///< 错误收集器

    /**
     * @brief 前进到下一个字符
     */
    void advance();

    /**
     * @brief 向前查看指定偏移量的字符
     * @param offset 偏移量
     * @return 字符的可选值
     */
    std::optional<char> peek(size_t offset) const;

    /**
     * @brief 跳过空白字符
     */
    void skip_whitespace();

    /**
     * @brief 跳过注释
     */
    void skip_comment();

    /**
     * @brief 读取数字字面量
     * @return Token 对象
     */
    Token read_number();

    /**
     * @brief 读取标识符
     * @return Token 对象
     */
    Token read_identifier();

    /**
     * @brief 读取字符串字面量
     * @return Token 对象
     */
    Token read_string();

    /**
     * @brief 读取原始字符串字面量
     * @return Token 对象
     */
    Token read_raw_string();

    /**
     * @brief 解析 Unicode 转义序列
     * @param digit_count 十六进制数字的数量
     * @return UTF-8 编码的字符串
     */
    std::string parse_unicode_escape(size_t digit_count);

    /**
     * @brief 解析十六进制转义序列
     * @return UTF-8 编码的字符串
     */
    std::string parse_hex_escape();

    /**
     * @brief 读取带前缀的数字（十六进制、二进制、八进制）
     * @param valid_chars 有效字符集合
     * @param prefix_str 前缀字符串
     * @param error_code 错误代码
     * @return Token 对象
     */
    Token read_prefixed_number(const std::string &valid_chars,
                               const std::string &prefix_str,
                               DiagnosticCode error_code);

    /**
     * @brief 报告错误
     * @param code 诊断代码
     * @param error_line 错误行号
     * @param error_column 错误列号
     * @param args 消息参数列表
     */
    void report_error(DiagnosticCode code,
                      size_t error_line,
                      size_t error_column,
                      const std::vector<std::string> &args = {});

public:
    /**
     * @brief 构造函数
     * @param input_str 输入源码字符串
     * @param fname 文件名，默认为 "<stdin>"
     */
    Lexer(const std::string &input_str,
          const std::string &fname = "<stdin>");

    /**
     * @brief 获取下一个 Token
     * @return Token 对象
     */
    Token next_token();

    /**
     * @brief 对整个输入进行词法分析
     * @return Token 列表
     */
    std::vector<Token> tokenize();

    /**
     * @brief 获取错误收集器
     * @return 错误收集器的常量引用
     */
    const ErrorCollector &get_errors() const { return error_collector; }
};

#endif // CZC_LEXER_HPP
