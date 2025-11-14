/**
 * @file lexer_string.cpp
 * @brief 字符串字面量解析实现 (普通字符串、原始字符串、转义序列)
 * @details 实现字符串字面量的词法分析，支持转义序列（包括 Unicode 和
 *          十六进制转义）、多行字符串和原始字符串。
 * @author BegoniaHe
 * @date 2025-11-14
 */

#include "czc/diagnostics/diagnostic_code.hpp"
#include "czc/lexer/lexer.hpp"
#include "czc/lexer/utf8_handler.hpp"

#include <sstream>

namespace czc::lexer {

using diagnostics::DiagnosticCode;

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

} // namespace czc::lexer