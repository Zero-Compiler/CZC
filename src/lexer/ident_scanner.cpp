/**
 * @file ident_scanner.cpp
 * @brief 标识符扫描器的实现。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-29
 */

#include "czc/lexer/ident_scanner.hpp"
#include "czc/lexer/utf8.hpp"

namespace czc::lexer {

bool IdentScanner::canScan(const ScanContext &ctx) const noexcept {
  auto ch = ctx.current();
  if (!ch.has_value()) {
    return false;
  }

  char c = ch.value();
  auto uc = static_cast<unsigned char>(c);

  // ASCII 标识符起始：字母或下划线
  if (isAsciiIdentStart(c)) {
    return true;
  }

  // UTF-8 多字节字符起始：可作为标识符
  if (isUtf8Start(uc)) {
    return true;
  }

  return false;
}

Token IdentScanner::scan(ScanContext &ctx) const {
  std::size_t startOffset = ctx.offset();
  SourceLocation startLoc = ctx.location();

  // 处理第一个字符
  auto firstCh = ctx.current();
  if (!firstCh.has_value()) {
    return ctx.makeUnknown(startOffset, startLoc);
  }

  auto firstUc = static_cast<unsigned char>(firstCh.value());

  if (isUtf8Start(firstUc)) {
    // UTF-8 多字节字符
    if (!consumeUtf8Char(ctx)) {
      // 无效的 UTF-8 序列
      ctx.advance(); // 跳过一个字节
      return ctx.makeUnknown(startOffset, startLoc);
    }
  } else {
    // ASCII 字符
    ctx.advance();
  }

  // 继续读取后续字符
  while (true) {
    auto ch = ctx.current();
    if (!ch.has_value()) {
      break;
    }

    char c = ch.value();
    auto uc = static_cast<unsigned char>(c);

    if (isAsciiIdentContinue(c)) {
      // ASCII 标识符后续字符
      ctx.advance();
    } else if (isUtf8Start(uc)) {
      // UTF-8 多字节字符
      if (!consumeUtf8Char(ctx)) {
        // 无效的 UTF-8 序列，标识符在此结束
        break;
      }
    } else {
      // 非标识符字符，结束
      break;
    }
  }

  // 获取标识符文本
  std::string_view text = ctx.textFrom(startOffset);

  // 查找关键字
  auto keyword = lookupKeyword(text);
  TokenType type = keyword.value_or(TokenType::IDENTIFIER);

  return ctx.makeToken(type, startOffset, startLoc);
}

bool IdentScanner::isAsciiIdentStart(char ch) noexcept {
  return utf8::isAsciiIdentStart(ch);
}

bool IdentScanner::isAsciiIdentContinue(char ch) noexcept {
  return utf8::isAsciiIdentContinue(ch);
}

bool IdentScanner::isUtf8Start(unsigned char ch) noexcept {
  // UTF-8 多字节字符的起始字节 >= 0xC0
  // 0x80-0xBF 是续字节
  // 0xC0-0xC1 是无效的起始字节（过长编码）
  // 0xC2-0xF4 是有效的起始字节
  return ch >= 0xC2 && ch <= 0xF4;
}

bool IdentScanner::consumeUtf8Char(ScanContext &ctx) const {
  auto ch = ctx.current();
  if (!ch.has_value()) {
    return false;
  }

  auto firstByte = static_cast<unsigned char>(ch.value());
  std::size_t len = utf8::charLength(firstByte);

  if (len == 0) {
    return false;
  }

  // 检查续字节
  for (std::size_t i = 1; i < len; ++i) {
    auto nextCh = ctx.peek(i);
    if (!nextCh.has_value()) {
      return false;
    }
    if (!utf8::isContinuationByte(static_cast<unsigned char>(nextCh.value()))) {
      return false;
    }
  }

  // 消费所有字节
  ctx.advance(len);
  return true;
}

} // namespace czc::lexer
