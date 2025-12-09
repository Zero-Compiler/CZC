/**
 * @file comment_scanner.cpp
 * @brief 注释扫描器的实现。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-29
 */

#include "czc/lexer/comment_scanner.hpp"

namespace czc::lexer {

bool CommentScanner::canScan(const ScanContext &ctx) const noexcept {
  auto ch = ctx.current();
  if (!ch.has_value() || ch.value() != '/') {
    return false;
  }

  auto next = ctx.peek(1);
  if (!next.has_value()) {
    return false;
  }

  char n = next.value();
  return n == '/' || n == '*';
}

Token CommentScanner::scan(ScanContext &ctx) const {
  std::size_t startOffset = ctx.offset();
  SourceLocation startLoc = ctx.location();

  auto next = ctx.peek(1);
  if (!next.has_value()) {
    return ctx.makeUnknown(startOffset, startLoc);
  }

  char n = next.value();

  if (n == '/') {
    return scanLineComment(ctx, startOffset, startLoc);
  } else if (n == '*') {
    return scanBlockComment(ctx, startOffset, startLoc);
  }

  return ctx.makeUnknown(startOffset, startLoc);
}

Token CommentScanner::scanLineComment(ScanContext &ctx, std::size_t startOffset,
                                      SourceLocation startLoc) const {
  // 消费 "//"
  ctx.advance(2);

  // 检查是否是文档注释 "///"
  bool isDoc = false;
  auto ch = ctx.current();
  if (ch.has_value() && ch.value() == '/') {
    isDoc = true;
    ctx.advance();
  }

  // 消费直到行尾
  while (true) {
    auto current = ctx.current();
    if (!current.has_value()) {
      break;
    }

    char c = current.value();
    if (c == '\n' || c == '\r') {
      // 不消费换行符，留给空白处理
      break;
    }

    ctx.advance();
  }

  TokenType type = isDoc ? TokenType::COMMENT_DOC : TokenType::COMMENT_LINE;
  return ctx.makeToken(type, startOffset, startLoc);
}

Token CommentScanner::scanBlockComment(ScanContext &ctx,
                                       std::size_t startOffset,
                                       SourceLocation startLoc) const {
  // 消费 "/*"
  ctx.advance(2);

  // 检查是否是文档注释 "/**"
  bool isDoc = false;
  auto ch = ctx.current();
  if (ch.has_value() && ch.value() == '*') {
    // 但是 "/**/" 不算文档注释
    auto afterStar = ctx.peek(1);
    if (afterStar.has_value() && afterStar.value() != '/') {
      isDoc = true;
      ctx.advance();
    }
  }

  // 块注释不支持嵌套，扫描直到遇到第一个 "*/"
  while (true) {
    auto current = ctx.current();
    if (!current.has_value()) {
      // 未闭合的块注释 - 计算从注释开始到当前位置的长度
      uint32_t spanLength = static_cast<uint32_t>(ctx.offset() - startOffset);
      ctx.reportError(LexerError::make(LexerErrorCode::UnterminatedBlockComment,
                                       startLoc, spanLength,
                                       "unterminated block comment"));
      break;
    }

    char c = current.value();

    // 检查注释结束 "*/"
    if (c == '*') {
      auto next = ctx.peek(1);
      if (next.has_value() && next.value() == '/') {
        ctx.advance(2);
        break;
      }
    }

    ctx.advance();
  }

  TokenType type = isDoc ? TokenType::COMMENT_DOC : TokenType::COMMENT_BLOCK;
  return ctx.makeToken(type, startOffset, startLoc);
}

} // namespace czc::lexer
