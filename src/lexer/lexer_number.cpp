/**
 * @file lexer_number.cpp
 * @brief 数字字面量解析实现 (整数、浮点数、科学计数法、进制前缀)
 * @details 实现各种数字字面量的词法分析，支持十进制、十六进制、
 *          八进制、二进制，以及浮点数和科学计数法。
 * @author BegoniaHe
 * @date 2025-11-14
 */

#include "czc/diagnostics/diagnostic_code.hpp"
#include "czc/lexer/lexer.hpp"

namespace czc::lexer {

using diagnostics::DiagnosticCode;

Token Lexer::read_prefixed_number(const std::string& valid_chars,
                                  const std::string& prefix_str,
                                  DiagnosticCode error_code) {
  size_t start = tracker.get_position();
  size_t token_line = tracker.get_line();
  size_t token_column = tracker.get_column();

  // 消耗掉数字前缀，例如 "0x"。
  advance(); // '0'
  advance(); // 'x'/'b'/'o'/'X'/'B'/'O'

  // 记录数字部分的起始位置，用于后续检查是否为空。
  size_t digit_start = tracker.get_position();

  // --- 读取有效数字字符 ---
  // 循环读取所有属于该数字进制的有效字符。
  while (current_char.has_value()) {
    char ch = current_char.value();
    if (valid_chars.find(ch) != std::string::npos) {
      advance();
    } else {
      // 遇到无效字符，数字部分结束。
      break;
    }
  }

  // --- 验证前缀后是否有数字 ---
  // 例如，"0x" 是无效的，必须是 "0x1" 这样的形式。
  size_t current_pos = tracker.get_position();
  if (current_pos == digit_start) {
    report_error(error_code, token_line, token_column, {prefix_str});
    // --- 错误恢复 ---
    // NOTE: 即使在报告错误后，我们仍然返回一个 `Unknown` 类型的 Token，
    //       其中包含了被识别为错误的文本。这是一种简单的错误恢复策略，
    //       它允许解析器（Parser）看到这个错误并可能尝试从中恢复，而不是
    //       让词法分析器完全卡住。
    const auto& input = tracker.get_input();
    return Token(TokenType::Unknown,
                 std::string(input.data() + start, current_pos - start),
                 token_line, token_column);
  }

  const auto& input = tracker.get_input();
  return Token(TokenType::Integer,
               std::string(input.data() + start, current_pos - start),
               token_line, token_column);
}

Token Lexer::read_number() {
  size_t start = tracker.get_position();
  size_t token_line = tracker.get_line();
  size_t token_column = tracker.get_column();
  bool is_float = false;
  bool is_scientific = false;

  // --- 处理特殊数字前缀 (0x, 0b, 0o) ---
  if (current_char == '0' && peek(1).has_value()) {
    char next_ch = peek(1).value();
    if (next_ch == 'x' || next_ch == 'X') {
      return read_prefixed_number("0123456789abcdefABCDEF", "0x",
                                  DiagnosticCode::L0001_MissingHexDigits);
    }
    if (next_ch == 'b' || next_ch == 'B') {
      return read_prefixed_number("01", "0b",
                                  DiagnosticCode::L0002_MissingBinaryDigits);
    }
    if (next_ch == 'o' || next_ch == 'O') {
      return read_prefixed_number("01234567", "0o",
                                  DiagnosticCode::L0003_MissingOctalDigits);
    }
  }

  // --- 读取十进制数字的整数和可选的小数部分 ---
  while (current_char.has_value()) {
    char ch = current_char.value();
    if (std::isdigit(static_cast<unsigned char>(ch))) {
      advance();
    }
    // 处理小数点。只允许一个小数点。
    else if (ch == '.' && !is_float) {
      // NOTE: 小数点后面必须紧跟一个数字才被认为是浮点数的一部分。
      //       像 `1.` 这样的写法将被解析为整数 `1` 和一个单独的点号 `.` Token。
      //       这是一种设计选择，旨在简化语言的语法，避免范围操作符（`..`）
      //       或方法调用（`.`）与浮点数产生歧义。
      auto next = peek(1);
      if (next.has_value() &&
          std::isdigit(static_cast<unsigned char>(next.value()))) {
        is_float = true;
        advance();
      } else {
        // 小数点后不是数字，说明数字字面量到此结束。
        break;
      }
    } else {
      break;
    }
  }

  // --- 处理科学计数法部分 (e.g., e+10, E-5) ---
  if (current_char.has_value() &&
      (current_char.value() == 'e' || current_char.value() == 'E')) {
    is_scientific = true;
    advance(); // 消耗 'e' 或 'E'

    // 消耗可选的指数符号 '+' 或 '-'。
    if (current_char.has_value() &&
        (current_char.value() == '+' || current_char.value() == '-')) {
      advance();
    }

    // 指数部分必须至少包含一个数字。
    size_t exp_start = tracker.get_position();
    while (current_char.has_value() &&
           std::isdigit(static_cast<unsigned char>(current_char.value()))) {
      advance();
    }

    // 如果在 'e' 或 'e-' 之后没有数字，这是一个词法错误。
    size_t current_pos = tracker.get_position();
    if (current_pos == exp_start) {
      const auto& input = tracker.get_input();
      report_error(DiagnosticCode::L0004_MissingExponentDigits, token_line,
                   token_column,
                   {std::string(input.data() + start, current_pos - start)});
      // --- 错误恢复 ---
      // 返回一个 `Unknown` 类型的 Token，包含错误的文本。
      return Token(TokenType::Unknown,
                   std::string(input.data() + start, current_pos - start),
                   token_line, token_column);
    }
  }

  // --- 验证数字字面量后的字符 ---
  // NOTE: 根据语言规范，数字字面量后面不能直接跟标识符字符（字母或下划线）。
  //       例如 `123a` 是无效的。这个规则是为了消除歧义，例如区分
  //       `123` 和变量 `a`，而不是一个名为 `123a` 的无效标识符。
  if (current_char.has_value() &&
      (std::isalpha(static_cast<unsigned char>(current_char.value())) ||
       current_char.value() == '_')) {
    std::string bad_suffix(1, current_char.value());
    report_error(DiagnosticCode::L0005_InvalidTrailingChar, token_line,
                 token_column, {bad_suffix});
    // --- 错误恢复 ---
    // 消耗掉这个无效字符，以避免词法分析器在下一次调用时再次报告同一个错误。
    advance();
    size_t current_pos = tracker.get_position();
    const auto& input = tracker.get_input();
    return Token(TokenType::Unknown,
                 std::string(input.data() + start, current_pos - start),
                 token_line, token_column);
  }

  size_t current_pos = tracker.get_position();
  const auto& input = tracker.get_input();
  std::string value(input.data() + start, current_pos - start);

  // --- 根据解析过程中设置的标志，确定最终的 Token 类型 ---
  if (is_scientific) {
    // NOTE: 所有科学计数法字面量都被暂时标记为 `ScientificExponent`。
    //       这是因为在词法分析阶段，我们只关心其语法形式，而不关心其
    //       具体的值。其最终类型（整数或浮点数）的推断和溢出检查将在
    //       后续的 TokenPreprocessor 阶段完成。
    return Token(TokenType::ScientificExponent, value, token_line,
                 token_column);
  }
  if (is_float) {
    return Token(TokenType::Float, value, token_line, token_column);
  }
  return Token(TokenType::Integer, value, token_line, token_column);
}

} // namespace czc::lexer