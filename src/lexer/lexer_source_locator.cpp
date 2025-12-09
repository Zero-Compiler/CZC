/**
 * @file lexer_source_locator.cpp
 * @brief Lexer 源码定位器适配器实现。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-12-04
 */

#include "czc/lexer/lexer_source_locator.hpp"
#include "czc/diag/i18n.hpp"
#include "czc/lexer/lexer_error_codes.hpp"

namespace czc::lexer {

LexerSourceLocator::LexerSourceLocator(const SourceManager &sm) : sm_(&sm) {}

auto LexerSourceLocator::getFilename(diag::Span span) const
    -> std::string_view {
  BufferID bid{span.fileId};
  return sm_->getFilename(bid);
}

auto LexerSourceLocator::getLineColumn(uint32_t fileId, uint32_t offset) const
    -> diag::LineColumn {
  BufferID bid{fileId};
  auto source = sm_->getSource(bid);

  if (source.empty() || offset > source.size()) {
    return {0, 0};
  }

  uint32_t line = 1;
  uint32_t column = 1;

  for (uint32_t i = 0; i < offset && i < source.size(); ++i) {
    if (source[i] == '\n') {
      ++line;
      column = 1;
    } else {
      ++column;
    }
  }

  return {line, column};
}

auto LexerSourceLocator::getLineContent(uint32_t fileId, uint32_t line) const
    -> std::string_view {
  BufferID bid{fileId};
  return sm_->getLineContent(bid, line);
}

auto LexerSourceLocator::getSourceSlice(diag::Span span) const
    -> std::string_view {
  BufferID bid{span.fileId};
  uint16_t length = static_cast<uint16_t>(
      std::min(static_cast<uint32_t>(UINT16_MAX), span.length()));
  return sm_->slice(bid, span.startOffset, length);
}

// ============================================================================
// 桥接函数实现
// ============================================================================

auto toSpan(const LexerError &err) -> diag::Span {
  // 使用 LexerError 中存储的实际长度
  uint32_t endOffset = err.location.offset + err.length;
  return diag::Span::create(err.location.buffer.value, err.location.offset,
                            endOffset);
}

namespace {

/// 根据错误码获取 i18n 键前缀
auto getI18nKeyPrefix(LexerErrorCode code) -> std::string {
  switch (code) {
  case LexerErrorCode::MissingHexDigits:
    return "lexer.missing_hex_digits";
  case LexerErrorCode::MissingBinaryDigits:
    return "lexer.missing_binary_digits";
  case LexerErrorCode::MissingOctalDigits:
    return "lexer.missing_octal_digits";
  case LexerErrorCode::MissingExponentDigits:
    return "lexer.missing_exponent_digits";
  case LexerErrorCode::InvalidTrailingChar:
    return "lexer.invalid_trailing_char";
  case LexerErrorCode::InvalidNumberSuffix:
    return "lexer.invalid_number_suffix";
  case LexerErrorCode::InvalidEscapeSequence:
    return "lexer.invalid_escape_sequence";
  case LexerErrorCode::UnterminatedString:
    return "lexer.unterminated_string";
  case LexerErrorCode::InvalidHexEscape:
    return "lexer.invalid_hex_escape";
  case LexerErrorCode::InvalidUnicodeEscape:
    return "lexer.invalid_unicode_escape";
  case LexerErrorCode::UnterminatedRawString:
    return "lexer.unterminated_raw_string";
  case LexerErrorCode::InvalidCharacter:
    return "lexer.invalid_character";
  case LexerErrorCode::InvalidUtf8Sequence:
    return "lexer.invalid_utf8_sequence";
  case LexerErrorCode::UnterminatedBlockComment:
    return "lexer.unterminated_block_comment";
  case LexerErrorCode::TokenTooLong:
    return "lexer.token_too_long";
  default:
    return "";
  }
}

} // namespace

auto toDiagnostic(const LexerError &err, const SourceManager & /*sm*/,
                  const diag::i18n::Translator &translator)
    -> diag::Diagnostic {
  // 从 LexerErrorCode 映射到 diag::ErrorCode
  auto diagCode = diag::ErrorCode(diag::ErrorCategory::Lexer,
                                  static_cast<uint16_t>(err.code));

  diag::Diagnostic diag(diag::Level::Error, diag::Message(err.formattedMessage),
                        diagCode);

  // 获取 i18n 键前缀
  auto keyPrefix = getI18nKeyPrefix(err.code);

  // 获取标签
  std::string label;
  if (!keyPrefix.empty()) {
    auto labelKey = keyPrefix + ".label";
    auto labelView = translator.get(labelKey);
    if (!labelView.empty()) {
      label = std::string(labelView);
    }
  }

  // 添加位置信息（带标签）
  diag.spans.addPrimary(toSpan(err), label);

  // 获取帮助信息（如果有）
  if (!keyPrefix.empty()) {
    auto helpKey = keyPrefix + ".help";
    auto helpView = translator.get(helpKey);
    if (!helpView.empty()) {
      diag.children.emplace_back(diag::Level::Help, std::string(helpView));
    }
  }

  return diag;
}

void emitLexerErrors(diag::DiagContext &dcx, std::span<const LexerError> errors,
                     const SourceManager &sm, BufferID /*bufferId*/) {
  // 创建 SourceLocator 适配器
  LexerSourceLocator locator(sm);
  dcx.setLocator(&locator);

  // 获取 DiagContext 中的 Translator
  const auto &translator = dcx.translator();

  // 发射所有错误
  for (const auto &err : errors) {
    dcx.emit(toDiagnostic(err, sm, translator));
  }
}

} // namespace czc::lexer
