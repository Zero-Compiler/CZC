/**
 * @file string_scanner.cpp
 * @brief 字符串字面量扫描器的实现。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-29
 */

#include "czc/lexer/string_scanner.hpp"

namespace czc::lexer {

namespace {

/**
 * @brief 跳过指定数量的十六进制数字。
 * @param ctx 扫描上下文
 * @param count 要跳过的最大数字数量
 */
void skipHexDigits(ScanContext &ctx, std::size_t count) {
  for (std::size_t i = 0; i < count; ++i) {
    auto ch = ctx.current();
    if (!ch.has_value()) {
      break;
    }
    char c = ch.value();
    if (std::isxdigit(static_cast<unsigned char>(c))) {
      ctx.advance();
    } else {
      break;
    }
  }
}

/**
 * @brief 跳过 Unicode 转义序列（直到遇到 '}'）。
 * @param ctx 扫描上下文
 */
void skipUnicodeEscape(ScanContext &ctx) {
  while (true) {
    auto ch = ctx.current();
    if (!ch.has_value()) {
      break;
    }
    char c = ch.value();
    if (c == '}') {
      ctx.advance();
      break;
    }
    if (std::isxdigit(static_cast<unsigned char>(c))) {
      ctx.advance();
    } else {
      break;
    }
  }
}

} // namespace

bool StringScanner::canScan(const ScanContext &ctx) const noexcept {
  auto ch = ctx.current();
  if (!ch.has_value()) {
    return false;
  }

  char c = ch.value();

  // 普通字符串: "..."
  if (c == '"') {
    return true;
  }

  // 原始字符串: r"..." 或 r#"..."#
  if (c == 'r') {
    auto next = ctx.peek(1);
    if (next.has_value()) {
      char n = next.value();
      return n == '"' || n == '#';
    }
  }

  // TeX 字符串: t"..."
  if (c == 't') {
    auto next = ctx.peek(1);
    return next.has_value() && next.value() == '"';
  }

  return false;
}

Token StringScanner::scan(ScanContext &ctx) const {
  std::size_t startOffset = ctx.offset();
  SourceLocation startLoc = ctx.location();

  auto ch = ctx.current();
  if (!ch.has_value()) {
    return ctx.makeUnknown(startOffset, startLoc);
  }

  char c = ch.value();

  // 原始字符串
  if (c == 'r') {
    return scanRawString(ctx, startOffset, startLoc);
  }

  // TeX 字符串
  if (c == 't') {
    return scanTexString(ctx, startOffset, startLoc);
  }

  // 普通字符串
  return scanNormalString(ctx, startOffset, startLoc);
}

Token StringScanner::scanNormalString(ScanContext &ctx, std::size_t startOffset,
                                      SourceLocation startLoc) const {
  // 消费开始的引号
  ctx.advance();

  EscapeFlags escapeFlags{};

  while (true) {
    auto ch = ctx.current();
    if (!ch.has_value()) {
      // 未闭合的字符串 - 到达文件末尾
      // 计算从字符串开始到当前位置的长度
      uint32_t spanLength = static_cast<uint32_t>(ctx.offset() - startOffset);
      ctx.reportError(LexerError::make(LexerErrorCode::UnterminatedString,
                                       startLoc, spanLength,
                                       "unterminated string literal"));
      break;
    }

    char c = ch.value();

    // 字符串结束
    if (c == '"') {
      ctx.advance();
      break;
    }

    // 转义序列
    if (c == '\\') {
      ctx.advance();
      auto escaped = ctx.current();
      if (escaped.has_value()) {
        char e = escaped.value();
        switch (e) {
        case 'n':
        case 'r':
        case 't':
        case '\\':
        case '"':
        case '\'':
        case '0':
          escapeFlags.set(kHasNamed);
          ctx.advance();
          break;
        case 'x':
          escapeFlags.set(kHasHex);
          ctx.advance();
          // 消费两位十六进制数
          skipHexDigits(ctx, 2);
          break;
        case 'u':
          escapeFlags.set(kHasUnicode);
          ctx.advance();
          // Unicode 转义 \u{XXXX}
          if (ctx.current().has_value() && ctx.current().value() == '{') {
            ctx.advance();
            skipUnicodeEscape(ctx);
          }
          break;
        default:
          // 未知转义，继续
          ctx.advance();
          break;
        }
      }
      continue;
    }

    // 允许多行字符串，直接嵌入换行符
    ctx.advance();
  }

  Token token = ctx.makeToken(TokenType::LIT_STRING, startOffset, startLoc);
  token.setEscapeFlags(escapeFlags);
  return token;
}

Token StringScanner::scanRawString(ScanContext &ctx, std::size_t startOffset,
                                   SourceLocation startLoc) const {
  // 消费 'r'
  ctx.advance();

  // 计算 # 的数量
  std::size_t hashCount = 0;
  while (ctx.current().has_value() && ctx.current().value() == '#') {
    hashCount++;
    ctx.advance();
  }

  // 消费开始的引号
  if (!ctx.match('"')) {
    return ctx.makeUnknown(startOffset, startLoc);
  }

  // 读取内容直到找到 "###...（相同数量的 #）
  while (true) {
    auto ch = ctx.current();
    if (!ch.has_value()) {
      // 未闭合
      break;
    }

    char c = ch.value();

    // 检查是否是结束序列
    if (c == '"') {
      ctx.advance();

      // 检查是否有足够的 #
      std::size_t endHashCount = 0;
      while (endHashCount < hashCount && ctx.current().has_value() &&
             ctx.current().value() == '#') {
        endHashCount++;
        ctx.advance();
      }

      if (endHashCount == hashCount) {
        // 找到正确的结束序列
        break;
      }
      // 否则继续，这不是结束
      continue;
    }

    ctx.advance();
  }

  Token token = ctx.makeToken(TokenType::LIT_RAW_STRING, startOffset, startLoc);
  return token;
}

Token StringScanner::scanTexString(ScanContext &ctx, std::size_t startOffset,
                                   SourceLocation startLoc) const {
  // 消费 't'
  ctx.advance();

  // 消费开始的引号
  if (!ctx.match('"')) {
    return ctx.makeUnknown(startOffset, startLoc);
  }

  // TeX 字符串，只处理 $...$ 数学环境，其他内容原样保留
  EscapeFlags escapeFlags{};

  while (true) {
    auto ch = ctx.current();
    if (!ch.has_value()) {
      // 未闭合
      break;
    }

    char c = ch.value();

    // 字符串结束
    if (c == '"') {
      ctx.advance();
      break;
    }

    // 处理转义的引号
    if (c == '\\') {
      ctx.advance();
      auto next = ctx.current();
      if (next.has_value() && next.value() == '"') {
        escapeFlags.set(kHasNamed);
        ctx.advance();
      }
      continue;
    }

    ctx.advance();
  }

  Token token = ctx.makeToken(TokenType::LIT_TEX_STRING, startOffset, startLoc);
  token.setEscapeFlags(escapeFlags);
  return token;
}

bool StringScanner::parseHexEscape([[maybe_unused]] ScanContext &ctx,
                                   [[maybe_unused]] std::string &result) const {
  // 解析 \xHH
  for (std::size_t i = 0; i < 2; ++i) {
    auto ch = ctx.current();
    if (!ch.has_value()) {
      return false;
    }
    char c = ch.value();
    if (std::isxdigit(static_cast<unsigned char>(c))) {
      ctx.advance();
    } else {
      return false;
    }
  }
  return true;
}

bool StringScanner::parseUnicodeEscape(
    [[maybe_unused]] ScanContext &ctx,
    [[maybe_unused]] std::string &result) const {
  // 解析 \u{XXXX} 或 \u{XXXXXX}
  if (!ctx.current().has_value() || ctx.current().value() != '{') {
    return false;
  }
  ctx.advance();

  while (true) {
    auto ch = ctx.current();
    if (!ch.has_value()) {
      return false;
    }
    char c = ch.value();
    if (c == '}') {
      ctx.advance();
      return true;
    }
    if (std::isxdigit(static_cast<unsigned char>(c))) {
      ctx.advance();
    } else {
      return false;
    }
  }
}

std::size_t StringScanner::countHashes(ScanContext &ctx) const {
  std::size_t count = 0;
  while (ctx.current().has_value() && ctx.current().value() == '#') {
    count++;
    ctx.advance();
  }
  return count;
}

} // namespace czc::lexer
