/**
 * @file scanner.cpp
 * @brief ScanContext 的实现。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-29
 */

#include "czc/lexer/scanner.hpp"

namespace czc::lexer {

ScanContext::ScanContext(SourceReader &reader, ErrorCollector &errors)
    : reader_(reader), errors_(errors) {}

std::optional<char> ScanContext::current() const noexcept {
  return reader_.current();
}

std::optional<char> ScanContext::peek(std::size_t offset) const noexcept {
  return reader_.peek(offset);
}

bool ScanContext::isAtEnd() const noexcept { return reader_.isAtEnd(); }

SourceLocation ScanContext::location() const noexcept {
  return reader_.location();
}

std::size_t ScanContext::offset() const noexcept { return reader_.offset(); }

BufferID ScanContext::buffer() const noexcept { return reader_.buffer(); }

void ScanContext::advance() { reader_.advance(); }

void ScanContext::advance(std::size_t count) { reader_.advance(count); }

bool ScanContext::check(char expected) const noexcept {
  auto ch = current();
  return ch.has_value() && ch.value() == expected;
}

bool ScanContext::match(char expected) {
  if (check(expected)) {
    advance();
    return true;
  }
  return false;
}

bool ScanContext::match(std::string_view expected) {
  if (expected.empty()) {
    return true;
  }

  // 检查是否有足够的字符
  for (std::size_t i = 0; i < expected.size(); ++i) {
    auto ch = peek(i);
    if (!ch.has_value() || ch.value() != expected[i]) {
      return false;
    }
  }

  // 匹配成功，前进
  advance(expected.size());
  return true;
}

SourceReader::Slice ScanContext::sliceFrom(std::size_t startOffset) const {
  return reader_.sliceFrom(startOffset);
}

std::string_view ScanContext::textFrom(std::size_t startOffset) const {
  return reader_.textFrom(startOffset);
}

SourceManager &ScanContext::sourceManager() noexcept {
  return reader_.sourceManager();
}

const SourceManager &ScanContext::sourceManager() const noexcept {
  return reader_.sourceManager();
}

void ScanContext::reportError(LexerError error) {
  errors_.add(std::move(error));
}

bool ScanContext::hasErrors() const noexcept { return errors_.hasErrors(); }

Token ScanContext::makeToken(TokenType type, std::size_t startOffset,
                             SourceLocation startLoc) const {
  auto slice = reader_.sliceFrom(startOffset);

  // 检测超长 Token（超过 uint16_t 最大值 65535 字节）
  constexpr std::size_t kMaxTokenLength = 0xFFFF;
  std::size_t actualLength = reader_.offset() - startOffset;
  if (actualLength > kMaxTokenLength) {
    // 报告错误，但仍然创建一个截断的 Token 以便继续解析
    // 使用实际长度作为 span 长度（截断到 uint32_t 范围）
    uint32_t spanLength = static_cast<uint32_t>(
        std::min(actualLength, static_cast<std::size_t>(UINT32_MAX)));
    const_cast<ScanContext *>(this)->reportError(
        LexerError::make(LexerErrorCode::TokenTooLong, startLoc, spanLength,
                         "token length {} exceeds maximum allowed length {}",
                         actualLength, kMaxTokenLength));
  }

  return Token(type, buffer(), slice.offset, slice.length, startLoc);
}

Token ScanContext::makeUnknown(std::size_t startOffset,
                               SourceLocation startLoc) const {
  return makeToken(TokenType::TOKEN_UNKNOWN, startOffset, startLoc);
}

} // namespace czc::lexer
