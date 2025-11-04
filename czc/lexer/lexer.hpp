/**
 * @file lexer.hpp
 * @brief 词法分析器类定义
 * @author BegoniaHe
 * @date 2025-11-04
 */

#ifndef CZC_LEXER_HPP
#define CZC_LEXER_HPP

#include "czc/diagnostics/diagnostic_code.hpp"
#include "czc/utils/source_tracker.hpp"
#include "error_collector.hpp"
#include "token.hpp"
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace czc {
namespace lexer {

/**
 * @brief 负责将源代码文本分解为一系列 Token 的词法分析器。
 * @details
 *   词法分析器（或扫描器）是编译器的第一阶段。它逐字符地读取源代码，
 *   并将其组合成有意义的单元，称为 Token（例如，标识符、数字、运算符）。
 *   此类还负责处理空白、注释和基本的词法错误。
 * @property {线程安全} 非线程安全。此类是有状态的，不可在多线程间共享。
 */
class Lexer {
private:
  // 管理源代码输入流和当前位置（行、列）。
  utils::SourceTracker tracker;
  // 当前正在处理的字符。
  std::optional<char> current_char;
  // 用于收集在词法分析期间遇到的所有错误。
  LexErrorCollector error_collector;

  /**
   * @brief 将 `current_char` 更新为输入流中的下一个字符。
   */
  void advance();

  /**
   * @brief 向前查看输入流中的字符，而不消耗它。
   * @param[in] offset 从当前位置开始的偏移量。
   * @return 如果位置有效，则返回该位置的字符；否则返回 `std::nullopt`。
   */
  std::optional<char> peek(size_t offset) const;

  /**
   * @brief 消耗并忽略所有连续的空白字符（空格、制表符、换行符等）。
   */
  void skip_whitespace();

  /**
   * @brief 消耗并忽略单行或多行注释。
   */
  void skip_comment();

  /**
   * @brief 从当前位置解析一个数字字面量（整数、浮点数或科学计数法）。
   * @return 返回一个表示该数字的 Token。
   */
  Token read_number();

  /**
   * @brief 从当前位置解析一个标识符或关键字。
   * @return 返回一个表示标识符或关键字的 Token。
   */
  Token read_identifier();

  /**
   * @brief 从当前位置解析一个字符串字面量，处理转义序列。
   * @return 返回一个表示该字符串的 Token。
   */
  Token read_string();

  /**
   * @brief 从当前位置解析一个原始字符串字面量。
   * @return 返回一个表示该原始字符串的 Token。
   */
  Token read_raw_string();

  /**
   * @brief 解析一个 `\u` 或 `\U` 形式的 Unicode 转义序列。
   * @param[in] digit_count 期望的十六进制数字位数（4 或 8）。
   * @return 返回转义序列对应的 UTF-8 编码字符串。
   */
  std::string parse_unicode_escape(size_t digit_count);

  /**
   * @brief 解析一个 `\x` 形式的十六进制转义序列。
   * @return 返回转义序列对应的 UTF-8 编码字符串。
   */
  std::string parse_hex_escape();

  /**
   * @brief 读取并验证一个带特定前缀的数字（例如 0x, 0b, 0o）。
   * @param[in] valid_chars 允许出现在数字部分的字符集。
   * @param[in] prefix_str  数字的前缀（例如 "0x"）。
   * @param[in] error_code  如果缺少数字部分时要报告的错误代码。
   * @return 返回一个表示该数字的 Token。
   */
  Token read_prefixed_number(const std::string &valid_chars,
                             const std::string &prefix_str,
                             diagnostics::DiagnosticCode error_code);

  /**
   * @brief 辅助函数，用于在错误收集器中记录一个新错误。
   * @param[in] code         错误的诊断代码。
   * @param[in] error_line   错误发生的行号。
   * @param[in] error_column 错误发生的列号。
   * @param[in] args         (可选) 格式化错误消息所需的参数。
   */
  void report_error(diagnostics::DiagnosticCode code, size_t error_line,
                    size_t error_column,
                    const std::vector<std::string> &args = {});

public:
  /**
   * @brief 构造一个新的词法分析器。
   * @param[in] input_str 要进行词法分析的源代码字符串。
   * @param[in] fname (可选) 源代码的文件名，用于错误报告。
   */
  Lexer(const std::string &input_str, const std::string &fname = "<stdin>");

  /**
   * @brief 从输入流中获取并返回下一个 Token。
   * @return 返回解析出的下一个 Token。当到达输入末尾时，
   *         将持续返回 `TokenType::EndOfFile` 类型的 Token。
   */
  Token next_token();

  /**
   * @brief 对整个输入字符串执行词法分析，并返回所有 Token 的列表。
   * @return 包含所有解析出的 Token 的向量，不包括 `EndOfFile` Token。
   */
  std::vector<Token> tokenize();

  /**
   * @brief 获取对内部错误收集器的只读访问权限。
   * @return 对 LexErrorCollector 对象的常量引用。
   */
  const LexErrorCollector &get_errors() const { return error_collector; }
};

} // namespace lexer
} // namespace czc

#endif // CZC_LEXER_HPP
