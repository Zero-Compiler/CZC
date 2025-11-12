/**
 * @file lexer.cpp
 * @brief `Lexer` 类的功能实现。
 * @author BegoniaHe
 * @date 2025-11-11
 */

#include "czc/lexer/lexer.hpp"

#include "czc/lexer/utf8_handler.hpp"

#include <cctype>
#include <iomanip>
#include <sstream>

namespace czc {
namespace lexer {

using namespace czc::diagnostics;
using namespace czc::utils;

void Lexer::report_error(DiagnosticCode code, size_t error_line,
                         size_t error_column,
                         const std::vector<std::string>& args) {
  // NOTE: 创建一个只包含错误发生点的 SourceLocation。对于词法错误，
  //       通常我们只关心单个字符或符号的位置，因此起始和结束位置是相同的。
  auto loc = SourceLocation(tracker.get_filename(), error_line, error_column,
                            error_line, error_column);
  error_collector.add(code, loc, args);
}

void Lexer::advance() {
  // --- 防御性检查 ---
  // 如果已经到达源码末尾，则无需再前进，防止 tracker 越界。
  if (!current_char.has_value()) {
    return;
  }

  // NOTE: 将当前字符传递给 tracker 是 advance 逻辑的核心。tracker 内部会
  //       检查该字符是否为换行符，并据此更新其内部的行号和列号状态。
  //       这是将位置跟踪逻辑与词法分析规则逻辑解耦的关键。
  char ch = current_char.value();
  tracker.advance(ch);

  size_t pos = tracker.get_position();
  const auto& input = tracker.get_input();

  // --- 更新当前字符 ---
  // 更新 current_char 为 tracker 前进后的新位置上的字符。
  // 如果到达末尾，则设置为 std::nullopt，这将作为后续所有处理的终止信号。
  if (pos < input.size()) {
    current_char = input[pos];
  } else {
    current_char = std::nullopt;
  }
}

std::optional<char> Lexer::peek(size_t offset) const {
  size_t peek_pos = tracker.get_position() + offset;
  const auto& input = tracker.get_input();
  if (peek_pos < input.size()) {
    return input[peek_pos];
  }
  return std::nullopt;
}

void Lexer::skip_whitespace() {
  // NOTE: 持续消耗字符，直到遇到非空白字符或源码结束。
  //       `isspace` 可以正确处理空格、制表符、换行符等多种空白字符。
  //       转换为 unsigned char 以避免在 char 为有符号类型的平台上出现 UB。
  while (current_char.has_value() &&
         std::isspace(static_cast<unsigned char>(current_char.value()))) {
    advance();
  }
}

Token Lexer::read_comment() {
  // --- 单行注释处理 ---
  // 检查是否是 `//` 开头的单行注释。
  size_t token_line = tracker.get_line();
  size_t token_column = tracker.get_column();
  std::string comment_text;

  if (current_char == '/' && peek(1) == '/') {
    // 记录 "//"
    comment_text += "//";
    advance(); // 跳过第一个 '/'
    advance(); // 跳过第二个 '/'

    // 读取注释内容直到行尾（`\n`）或文件末尾。
    while (current_char.has_value() && current_char != '\n') {
      comment_text += current_char.value();
      advance();
    }

    // NOTE: 如果是因为换行符而停止，则需要额外调用一次 advance() 来消耗掉
    //       这个换行符本身，以便下一次 next_token() 从新的一行开始。
    if (current_char == '\n') {
      advance();
    }

    return Token(TokenType::Comment, comment_text, token_line, token_column);
  }

  // 如果不是注释，返回一个 Unknown token。
  return Token(TokenType::Unknown, "", token_line, token_column);
}

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

Token Lexer::read_identifier() {
  size_t start = tracker.get_position();
  size_t token_line = tracker.get_line();
  size_t token_column = tracker.get_column();

  // 处理标识符的第一个字符
  if (current_char.has_value()) {
    unsigned char uch = static_cast<unsigned char>(current_char.value());

    // 如果第一个字符是UTF-8起始字节,使用Utf8Handler读取完整字符
    if (uch >= 0x80) {
      size_t current_pos = tracker.get_position();
      std::string utf8_char;
      const auto& input = tracker.get_input();

      if (Utf8Handler::read_char(input, current_pos, utf8_char)) {
        // UTF-8 字符有效，同步 tracker 的位置
        while (tracker.get_position() < current_pos) {
          advance();
        }
      } else {
        // 第一个字符就是无效的UTF-8序列,返回错误token
        advance();
        return Token(TokenType::Unknown, std::string(input.data() + start, 1),
                     token_line, token_column);
      }
    } else {
      // ASCII字符,直接消耗
      advance();
    }
  }

  // 继续读取后续字符
  while (current_char.has_value()) {
    char ch = current_char.value();
    unsigned char uch = static_cast<unsigned char>(ch);

    // 标识符可以包含字母、数字、下划线
    if (std::isalnum(static_cast<unsigned char>(ch)) || ch == '_') {
      advance();
    }
    // NOTE: 对于非 ASCII 字符（UTF-8 起始字节），使用 Utf8Handler
    //       来正确读取完整的多字节字符序列，并验证其有效性。
    else if (uch >= 0x80) {
      size_t current_pos = tracker.get_position();
      std::string utf8_char;
      const auto& input = tracker.get_input();

      if (Utf8Handler::read_char(input, current_pos, utf8_char)) {
        // UTF-8 字符有效，同步 tracker 的位置
        while (tracker.get_position() < current_pos) {
          advance();
        }
      } else {
        // 遇到无效的 UTF-8 序列，标识符在此处结束
        break;
      }
    } else {
      break;
    }
  }

  size_t current_pos = tracker.get_position();
  const auto& input = tracker.get_input();
  std::string value(input.data() + start, current_pos - start);

  // 检查解析出的字符串是否是语言的关键字。
  auto keyword_type = get_keyword(value);
  // 如果是关键字，则使用关键字的 Token 类型；否则，它是一个普通的标识符。
  TokenType token_type = keyword_type.value_or(TokenType::Identifier);

  return Token(token_type, value, token_line, token_column);
}

std::string Lexer::parse_unicode_escape(size_t digit_count) {
  std::string hex_digits;

  for (size_t i = 0; i < digit_count; ++i) {
    if (!current_char.has_value() ||
        !std::isxdigit(static_cast<unsigned char>(current_char.value()))) {
      report_error(DiagnosticCode::L0009_InvalidUnicodeEscape,
                   tracker.get_line(), tracker.get_column(), {"u"});
      return "";
    }
    hex_digits += current_char.value();
    advance();
  }

  unsigned int codepoint;
  std::stringstream ss;
  ss << std::hex << hex_digits;
  ss >> codepoint;

  // 将解析出的 Unicode 码点转换为 UTF-8 编码的字符串。
  return Utf8Handler::codepoint_to_utf8(codepoint);
}

std::string Lexer::parse_hex_escape() {
  std::string hex_digits;

  // 读取最多 2 个十六进制数字
  for (size_t i = 0; i < 2; ++i) {
    if (!current_char.has_value() ||
        !std::isxdigit(static_cast<unsigned char>(current_char.value()))) {
      break;
    }
    hex_digits += current_char.value();
    advance();
  }

  if (hex_digits.empty()) {
    report_error(DiagnosticCode::L0008_InvalidHexEscape, tracker.get_line(),
                 tracker.get_column(), {"x"});
    return "";
  }

  // 将十六进制字符串（例如 "FF"）转换为对应的字节值。
  unsigned int byte_value;
  std::stringstream ss;
  ss << std::hex << hex_digits;
  ss >> byte_value;

  return std::string(1, static_cast<char>(byte_value));
}

Token Lexer::read_string() {
  size_t token_line = tracker.get_line();
  size_t token_column = tracker.get_column();
  size_t start_pos = tracker.get_position(); // 记录起始位置（包括开头的 "）
  advance();                                 // 跳过开头的 "

  std::string value;
  value.reserve(64); // 预分配以提高性能
  bool terminated = false;

  while (current_char.has_value()) {
    char ch = current_char.value();

    if (ch == '"') {
      terminated = true;
      break;
    }

    if (ch == '\n') { // 允许字符串跨行
      value += ch;
      advance();
      continue;
    }

    if (ch == '\\') {
      // --- 处理转义序列 ---
      advance(); // 消耗反斜杠 '\'
      if (!current_char.has_value()) {
        // 如果反斜杠是文件的最后一个字符，则字符串未闭合。
        break;
      }

      char escaped = current_char.value();
      switch (escaped) {
      case 'n':
        value += '\n';
        advance();
        break;
      case 't':
        value += '\t';
        advance();
        break;
      case 'r':
        value += '\r';
        advance();
        break;
      case '\\':
        value += '\\';
        advance();
        break;
      case '"':
        value += '"';
        advance();
        break;
      case '\'':
        value += '\'';
        advance();
        break;
      case '0':
        value += '\0';
        advance();
        break;
      case 'x': // 十六进制转义 \xHH
        advance();
        value += parse_hex_escape();
        break;
      case 'u': // Unicode 转义 \uXXXX or \u{...}
        advance();
        if (current_char.has_value() && current_char.value() == '{') {
          // \u{XXXXXX} 格式
          advance(); // 跳过 '{'
          std::string hex_digits;
          while (current_char.has_value() && current_char.value() != '}') {
            if (!std::isxdigit(
                    static_cast<unsigned char>(current_char.value()))) {
              report_error(DiagnosticCode::L0009_InvalidUnicodeEscape,
                           tracker.get_line(), tracker.get_column(), {"u"});
              // 尝试恢复：跳过无效内容直到 '}' 或字符串结束
              while (current_char.has_value() && current_char.value() != '}' &&
                     current_char.value() != '"') {
                advance();
              }
              hex_digits.clear(); // 标记为失败
              break;
            }
            hex_digits += current_char.value();
            advance();
          }

          if (!current_char.has_value() || current_char.value() != '}') {
            report_error(DiagnosticCode::L0009_InvalidUnicodeEscape,
                         tracker.get_line(), tracker.get_column(), {"u"});
          } else {
            advance(); // 跳过 '}'
            if (!hex_digits.empty() && hex_digits.length() <= 6) {
              unsigned int codepoint = std::stoul(hex_digits, nullptr, 16);
              value += Utf8Handler::codepoint_to_utf8(codepoint);
            } else {
              report_error(DiagnosticCode::L0009_InvalidUnicodeEscape,
                           tracker.get_line(), tracker.get_column(), {"u"});
            }
          }
        } else {
          // \uXXXX 格式
          value += parse_unicode_escape(4);
        }
        break;
      default:
        report_error(DiagnosticCode::L0006_InvalidEscapeSequence,
                     tracker.get_line(), tracker.get_column(),
                     {std::string(1, escaped)});
        value += escaped; // 将无效的转义字符本身添加到值中
        advance();
        break;
      }
    } else {
      // --- 处理非转义的普通字符（可能为多字节 UTF-8） ---
      // NOTE: 使用 Utf8Handler::read_char 来确保正确处理多字节字符。
      //       此函数会完整读取一个 UTF-8 字符（可能是1-4字节），
      //       并相应地推进 tracker 的位置。
      size_t current_pos = tracker.get_position();
      std::string utf8_char;
      const auto& input = tracker.get_input();

      if (Utf8Handler::read_char(input, current_pos, utf8_char)) {
        value += utf8_char;
        // 同步 tracker 和 lexer 的状态
        while (tracker.get_position() < current_pos) {
          advance();
        }
      } else {
        // 如果 read_char 失败，说明遇到了无效的 UTF-8 序列。
        // 报告错误并消耗掉这个无效字节，以避免无限循环。
        report_error(DiagnosticCode::L0011_InvalidUtf8Sequence,
                     tracker.get_line(), tracker.get_column(), {});
        advance();
      }
    }
  }

  if (!terminated) {
    report_error(DiagnosticCode::L0007_UnterminatedString, token_line,
                 token_column, {});
    Token token(TokenType::String, value, token_line, token_column);
    // 提取原始字符串字面量文本（从起始位置到当前位置）
    size_t end_pos = tracker.get_position();
    const auto& input = tracker.get_input();
    token.raw_literal =
        std::string(input.begin() + start_pos, input.begin() + end_pos);
    return token;
  }

  advance(); // 跳过结尾的 "
  Token token(TokenType::String, value, token_line, token_column);
  // 提取原始字符串字面量文本（包括两端的引号）
  size_t end_pos = tracker.get_position();
  const auto& input = tracker.get_input();
  token.raw_literal =
      std::string(input.begin() + start_pos, input.begin() + end_pos);
  return token;
}

Token Lexer::read_raw_string() {
  size_t token_line = tracker.get_line();
  size_t token_column = tracker.get_column();
  size_t start_pos = tracker.get_position(); // 记录起始位置（包括 'r'）

  advance(); // 跳过 'r'

  if (!current_char.has_value() || current_char.value() != '"') {
    report_error(DiagnosticCode::L0010_InvalidCharacter, token_line,
                 token_column, {"r"});
    return Token(TokenType::Unknown, "r", token_line, token_column);
  }

  advance(); // 跳过 '"'

  std::string value;
  value.reserve(64);
  bool terminated = false;

  while (current_char.has_value()) {
    char ch = current_char.value();

    if (ch == '"') {
      terminated = true;
      break;
    }

    // NOTE: 在原始字符串中，所有字符（包括反斜杠 `\` 和换行符 `\n`）
    //       都按其字面意义处理，不进行任何转义。
    //       同样使用 Utf8Handler::read_char 来确保正确处理多字节字符。
    size_t current_pos = tracker.get_position();
    std::string utf8_char;
    const auto& input = tracker.get_input();

    if (Utf8Handler::read_char(input, current_pos, utf8_char)) {
      value += utf8_char;
      while (tracker.get_position() < current_pos) {
        advance();
      }
    } else {
      report_error(DiagnosticCode::L0011_InvalidUtf8Sequence,
                   tracker.get_line(), tracker.get_column(), {});
      advance();
    }
  }

  if (!terminated) {
    report_error(DiagnosticCode::L0007_UnterminatedString, token_line,
                 token_column, {});
  } else {
    advance(); // 跳过结尾的 "
  }

  Token token(TokenType::String, value, token_line, token_column);
  token.is_raw_string = true; // 标记为原始字符串
  // 提取原始字符串字面量文本（包括 r"..."）
  size_t end_pos = tracker.get_position();
  const auto& input = tracker.get_input();
  token.raw_literal =
      std::string(input.begin() + start_pos, input.begin() + end_pos);
  return token;
}

std::optional<Token> Lexer::try_read_two_char_operator(char first_char,
                                                       size_t token_line,
                                                       size_t token_column) {
  std::optional<char> next = peek(1);
  if (!next.has_value()) {
    return std::nullopt;
  }

  char second_char = next.value();

  // Helper lambda to advance and create token
  auto make_two_char_token = [&](TokenType type,
                                 const std::string& value) -> Token {
    advance();
    return Token(type, value, token_line, token_column);
  };

  switch (first_char) {
  case '+':
    if (second_char == '=')
      return make_two_char_token(TokenType::PlusEqual, "+=");
    break;
  case '-':
    if (second_char == '=')
      return make_two_char_token(TokenType::MinusEqual, "-=");
    if (second_char == '>')
      return make_two_char_token(TokenType::Arrow, "->");
    break;
  case '*':
    if (second_char == '=')
      return make_two_char_token(TokenType::StarEqual, "*=");
    break;
  case '/':
    if (second_char == '=')
      return make_two_char_token(TokenType::SlashEqual, "/=");
    break;
  case '%':
    if (second_char == '=')
      return make_two_char_token(TokenType::PercentEqual, "%=");
    break;
  case '=':
    if (second_char == '=')
      return make_two_char_token(TokenType::EqualEqual, "==");
    break;
  case '!':
    if (second_char == '=')
      return make_two_char_token(TokenType::BangEqual, "!=");
    break;
  case '<':
    if (second_char == '=')
      return make_two_char_token(TokenType::LessEqual, "<=");
    break;
  case '>':
    if (second_char == '=')
      return make_two_char_token(TokenType::GreaterEqual, ">=");
    break;
  case '&':
    if (second_char == '&')
      return make_two_char_token(TokenType::And, "&&");
    break;
  case '|':
    if (second_char == '|')
      return make_two_char_token(TokenType::Or, "||");
    break;
  case '.':
    if (second_char == '.')
      return make_two_char_token(TokenType::DotDot, "..");
    break;
  default:
    break;
  }

  return std::nullopt;
}

Token Lexer::read_single_char_token(char ch, size_t token_line,
                                    size_t token_column) {
  switch (ch) {
  case '+':
    return Token(TokenType::Plus, "+", token_line, token_column);
  case '-':
    return Token(TokenType::Minus, "-", token_line, token_column);
  case '*':
    return Token(TokenType::Star, "*", token_line, token_column);
  case '/':
    return Token(TokenType::Slash, "/", token_line, token_column);
  case '%':
    return Token(TokenType::Percent, "%", token_line, token_column);
  case '=':
    return Token(TokenType::Equal, "=", token_line, token_column);
  case '!':
    return Token(TokenType::Bang, "!", token_line, token_column);
  case '<':
    return Token(TokenType::Less, "<", token_line, token_column);
  case '>':
    return Token(TokenType::Greater, ">", token_line, token_column);
  case '&':
    return Token(TokenType::Unknown, "&", token_line, token_column);
  case '|':
    return Token(TokenType::Unknown, "|", token_line, token_column);
  case '(':
    return Token(TokenType::LeftParen, "(", token_line, token_column);
  case ')':
    return Token(TokenType::RightParen, ")", token_line, token_column);
  case '{':
    return Token(TokenType::LeftBrace, "{", token_line, token_column);
  case '}':
    return Token(TokenType::RightBrace, "}", token_line, token_column);
  case '[':
    return Token(TokenType::LeftBracket, "[", token_line, token_column);
  case ']':
    return Token(TokenType::RightBracket, "]", token_line, token_column);
  case ',':
    return Token(TokenType::Comma, ",", token_line, token_column);
  case ';':
    return Token(TokenType::Semicolon, ";", token_line, token_column);
  case ':':
    return Token(TokenType::Colon, ":", token_line, token_column);
  case '.':
    return Token(TokenType::Dot, ".", token_line, token_column);
  default:
    return Token(TokenType::Unknown, std::string(1, ch), token_line,
                 token_column);
  }
}

Lexer::Lexer(const std::string& input_str, const std::string& fname)
    : tracker(input_str, fname) {
  const auto& input = tracker.get_input();
  if (!input.empty()) {
    current_char = input[0];
  } else {
    current_char = std::nullopt;
  }
}

Token Lexer::next_token() {
  // --- 主循环：跳过空白 ---
  // NOTE: 这个循环是词法分析器的"空转"阶段。它的任务是不断地跳过
  //       所有空白字符，直到 `current_char` 指向一个潜在 Token 的起始字符，
  //       或者到达文件末尾。注释现在被视为有效的 Token。
  skip_whitespace();

  // 如果在跳过空白后到达了文件末尾，则返回 EOF Token。
  if (!current_char.has_value()) {
    return Token(TokenType::EndOfFile, "", tracker.get_line(),
                 tracker.get_column());
  }

  // 检查是否是注释
  if (current_char == '/' && peek(1) == '/') {
    return read_comment();
  }

  char ch = current_char.value();
  unsigned char uch = static_cast<unsigned char>(ch);
  size_t token_line = tracker.get_line();
  size_t token_column = tracker.get_column();

  // --- Token 解析分派 ---
  // 这是一个典型的词法分析器状态机分派逻辑。

  if (std::isdigit(static_cast<unsigned char>(ch))) {
    return read_number();
  }

  if (std::isalpha(static_cast<unsigned char>(ch)) || ch == '_' ||
      uch >= 0x80) {
    // 特殊情况：`r"` 前缀表示原始字符串。
    if (ch == 'r' && peek(1) == '"') {
      return read_raw_string();
    }
    return read_identifier();
  }

  if (ch == '"') {
    return read_string();
  }

  // 当前语言不支持单引号字符字面量，因此将其视为未知 Token。
  if (ch == '\'') {
    Token token(TokenType::Unknown, "'", token_line, token_column);
    advance();
    return token;
  }

  Token token(TokenType::Unknown, "", token_line, token_column);

  // --- 处理单字符和双字符运算符及分隔符 ---
  // NOTE: 对于可能构成双字符运算符（如 `+=`, `==`）的字符，词法分析器
  //       必须向前“看”一个字符（peek）来做出决定。如果匹配，则消耗两个
  //       字符并返回双字符 Token；否则，只消耗当前字符并返回单字符 Token。
  //       这种策略被称为“最大匹配原则”（Maximal Munch Principle）。
  switch (ch) {
  case '+':
    token = peek(1) == '='
                ? (advance(),
                   Token(TokenType::PlusEqual, "+=", token_line, token_column))
                : Token(TokenType::Plus, "+", token_line, token_column);
    break;
  case '-':
    if (peek(1) == '=') {
      advance();
      token = Token(TokenType::MinusEqual, "-=", token_line, token_column);
    } else if (peek(1) == '>') {
      advance();
      token = Token(TokenType::Arrow, "->", token_line, token_column);
    } else {
      token = Token(TokenType::Minus, "-", token_line, token_column);
    }
    break;
  case '*':
    token = peek(1) == '='
                ? (advance(),
                   Token(TokenType::StarEqual, "*=", token_line, token_column))
                : Token(TokenType::Star, "*", token_line, token_column);
    break;
  case '/':
    token = peek(1) == '='
                ? (advance(),
                   Token(TokenType::SlashEqual, "/=", token_line, token_column))
                : Token(TokenType::Slash, "/", token_line, token_column);
    break;
  case '%':
    token = peek(1) == '='
                ? (advance(), Token(TokenType::PercentEqual, "%=", token_line,
                                    token_column))
                : Token(TokenType::Percent, "%", token_line, token_column);
    break;
  case '=':
    token = peek(1) == '='
                ? (advance(),
                   Token(TokenType::EqualEqual, "==", token_line, token_column))
                : Token(TokenType::Equal, "=", token_line, token_column);
    break;
  case '!':
    token = peek(1) == '='
                ? (advance(),
                   Token(TokenType::BangEqual, "!=", token_line, token_column))
                : Token(TokenType::Bang, "!", token_line, token_column);
    break;
  case '<':
    token = peek(1) == '='
                ? (advance(),
                   Token(TokenType::LessEqual, "<=", token_line, token_column))
                : Token(TokenType::Less, "<", token_line, token_column);
    break;
  case '>':
    token = peek(1) == '='
                ? (advance(), Token(TokenType::GreaterEqual, ">=", token_line,
                                    token_column))
                : Token(TokenType::Greater, ">", token_line, token_column);
    break;
  case '&':
    if (peek(1) == '&') {
      advance();
      token = Token(TokenType::And, "&&", token_line, token_column);
    } else {
      token = Token(TokenType::Unknown, "&", token_line, token_column);
    }
    break;
  case '|':
    if (peek(1) == '|') {
      advance();
      token = Token(TokenType::Or, "||", token_line, token_column);
    } else {
      token = Token(TokenType::Unknown, "|", token_line, token_column);
    }
    break;
  case '(':
    token = Token(TokenType::LeftParen, "(", token_line, token_column);
    break;
  case ')':
    token = Token(TokenType::RightParen, ")", token_line, token_column);
    break;
  case '{':
    token = Token(TokenType::LeftBrace, "{", token_line, token_column);
    break;
  case '}':
    token = Token(TokenType::RightBrace, "}", token_line, token_column);
    break;
  case '[':
    token = Token(TokenType::LeftBracket, "[", token_line, token_column);
    break;
  case ']':
    token = Token(TokenType::RightBracket, "]", token_line, token_column);
    break;
  case ',':
    token = Token(TokenType::Comma, ",", token_line, token_column);
    break;
  case ';':
    token = Token(TokenType::Semicolon, ";", token_line, token_column);
    break;
  case ':':
    token = Token(TokenType::Colon, ":", token_line, token_column);
    break;
  case '.':
    if (peek(1) == '.') {
      advance();
      token = Token(TokenType::DotDot, "..", token_line, token_column);
    } else {
      token = Token(TokenType::Dot, ".", token_line, token_column);
    }
    break;
  default:
    // 如果字符不匹配任何已知的 Token 模式，则将其标记为 Unknown。
    token =
        Token(TokenType::Unknown, std::string(1, ch), token_line, token_column);
    break;
  }

  // 消耗当前字符，为下一次调用 `next_token` 做准备。
  advance();
  return token;
}

std::vector<Token> Lexer::tokenize() {
  std::vector<Token> tokens;
  tokens.reserve(256); // 预分配以提高性能

  while (true) {
    Token token = next_token();
    bool is_eof = (token.token_type == TokenType::EndOfFile);
    tokens.push_back(token);

    if (is_eof) {
      break;
    }
  }

  return tokens;
}

} // namespace lexer
} // namespace czc
