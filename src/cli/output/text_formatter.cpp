/**
 * @file text_formatter.cpp
 * @brief 文本格式化器实现。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-30
 */

#include "czc/cli/output/text_formatter.hpp"
#include "czc/lexer/token.hpp"

#include <sstream>

namespace czc::cli {

std::string TextFormatter::formatTokens(std::span<const lexer::Token> tokens,
                                        const lexer::SourceManager &sm) const {
  std::ostringstream oss;

  oss << "Total tokens: " << tokens.size() << "\n\n";

  for (const auto &token : tokens) {
    const auto &loc = token.location();
    auto type_name = lexer::tokenTypeName(token.type());
    auto value = token.value(sm);

    // 格式: [行:列] 类型 "值"
    oss << "[" << loc.line << ":" << loc.column << "] ";
    oss << type_name;

    // 对于非空值，显示实际内容
    if (!value.empty() && token.type() != lexer::TokenType::TOKEN_EOF) {
      oss << " \"";
      // 转义特殊字符以便显示
      for (char c : value) {
        switch (c) {
        case '\n':
          oss << "\\n";
          break;
        case '\r':
          oss << "\\r";
          break;
        case '\t':
          oss << "\\t";
          break;
        case '\\':
          oss << "\\\\";
          break;
        case '"':
          oss << "\\\"";
          break;
        default:
          if (static_cast<unsigned char>(c) < 32) {
            oss << "\\x" << std::hex << static_cast<int>(c) << std::dec;
          } else {
            oss << c;
          }
          break;
        }
      }
      oss << "\"";
    }

    oss << "\n";

    // 显示 Trivia（如果有）
    if (token.hasTrivia()) {
      for (const auto &trivia : token.leadingTrivia()) {
        oss << "  (leading trivia: ";
        switch (trivia.kind) {
        case lexer::Trivia::Kind::kWhitespace:
          oss << "whitespace";
          break;
        case lexer::Trivia::Kind::kNewline:
          oss << "newline";
          break;
        case lexer::Trivia::Kind::kComment:
          oss << "comment";
          break;
        }
        oss << ")\n";
      }
      for (const auto &trivia : token.trailingTrivia()) {
        oss << "  (trailing trivia: ";
        switch (trivia.kind) {
        case lexer::Trivia::Kind::kWhitespace:
          oss << "whitespace";
          break;
        case lexer::Trivia::Kind::kNewline:
          oss << "newline";
          break;
        case lexer::Trivia::Kind::kComment:
          oss << "comment";
          break;
        }
        oss << ")\n";
      }
    }
  }

  return oss.str();
}

std::string
TextFormatter::formatErrors(std::span<const lexer::LexerError> errors,
                            const lexer::SourceManager &sm) const {
  std::ostringstream oss;

  oss << "=== Lexical Errors ===\n";
  oss << "Total errors: " << errors.size() << "\n\n";

  for (const auto &error : errors) {
    const auto &loc = error.location;

    // 获取文件名
    auto filename = sm.getFilename(loc.buffer);

    // 格式: 文件:行:列: error[E####]: 消息
    oss << filename << ":" << loc.line << ":" << loc.column << ": ";
    oss << "error[" << error.codeString() << "]: ";
    oss << error.formattedMessage << "\n";

    // 显示源码上下文（如果可用）
    // TODO: 添加源码片段显示
  }

  return oss.str();
}

} // namespace czc::cli
