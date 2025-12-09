/**
 * @file lexer_error_test.cpp
 * @brief 词法分析错误处理单元测试。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-30
 */

#include "czc/lexer/lexer_error.hpp"
#include "czc/lexer/source_manager.hpp"

#include <gtest/gtest.h>

namespace czc::lexer {
namespace {

class LexerErrorTest : public ::testing::Test {
protected:
  SourceManager sm_;

  BufferID addSource(std::string_view source, std::string filename) {
    return sm_.addBuffer(source, std::move(filename));
  }
};

// ============================================================================
// LexerError 构造测试
// ============================================================================

TEST_F(LexerErrorTest, MakeError) {
  SourceLocation loc(BufferID{1}, 5, 3, 10);
  auto error = LexerError::make(LexerErrorCode::InvalidCharacter, loc, 1,
                                "invalid character '@'");

  EXPECT_EQ(error.code, LexerErrorCode::InvalidCharacter);
  EXPECT_EQ(error.location.buffer.value, 1u);
  EXPECT_EQ(error.location.offset, 10u);
  EXPECT_EQ(error.location.line, 5u);
  EXPECT_EQ(error.location.column, 3u);
  EXPECT_EQ(error.length, 1u);
  EXPECT_EQ(error.formattedMessage, "invalid character '@'");
}

TEST_F(LexerErrorTest, MakeErrorWithLength) {
  SourceLocation loc(BufferID{1}, 1, 1, 0);
  auto error = LexerError::make(LexerErrorCode::UnterminatedString, loc, 14,
                                "unterminated string literal");

  EXPECT_EQ(error.code, LexerErrorCode::UnterminatedString);
  EXPECT_EQ(error.length, 14u);
  EXPECT_EQ(error.formattedMessage, "unterminated string literal");
}

TEST_F(LexerErrorTest, ErrorCodeString) {
  SourceLocation loc(BufferID{1}, 1, 1, 0);

  auto error1 =
      LexerError::simple(LexerErrorCode::InvalidCharacter, loc, "test");
  EXPECT_EQ(error1.codeString(), "L1021");

  auto error2 =
      LexerError::simple(LexerErrorCode::InvalidNumberSuffix, loc, "test");
  EXPECT_EQ(error2.codeString(), "L1006");

  auto error3 =
      LexerError::simple(LexerErrorCode::UnterminatedString, loc, "test");
  EXPECT_EQ(error3.codeString(), "L1012");

  auto error4 =
      LexerError::simple(LexerErrorCode::UnterminatedBlockComment, loc, "test");
  EXPECT_EQ(error4.codeString(), "L1031");

  auto error5 =
      LexerError::simple(LexerErrorCode::InvalidEscapeSequence, loc, "test");
  EXPECT_EQ(error5.codeString(), "L1011");

  auto error6 =
      LexerError::simple(LexerErrorCode::InvalidUnicodeEscape, loc, "test");
  EXPECT_EQ(error6.codeString(), "L1014");

  auto error7 =
      LexerError::simple(LexerErrorCode::InvalidUtf8Sequence, loc, "test");
  EXPECT_EQ(error7.codeString(), "L1022");

  auto error8 =
      LexerError::simple(LexerErrorCode::MissingHexDigits, loc, "test");
  EXPECT_EQ(error8.codeString(), "L1001");

  auto error9 =
      LexerError::simple(LexerErrorCode::MissingBinaryDigits, loc, "test");
  EXPECT_EQ(error9.codeString(), "L1002");

  auto error10 =
      LexerError::simple(LexerErrorCode::MissingOctalDigits, loc, "test");
  EXPECT_EQ(error10.codeString(), "L1003");
}

TEST_F(LexerErrorTest, UnknownErrorCode) {
  SourceLocation loc(BufferID{1}, 1, 1, 0);
  auto error =
      LexerError::simple(static_cast<LexerErrorCode>(9999), loc, "test");
  // 实现直接使用错误码数值
  EXPECT_EQ(error.codeString(), "L9999");
}

// ============================================================================
// formatError 测试
// ============================================================================

TEST_F(LexerErrorTest, FormatErrorWithValidBuffer) {
  auto id = addSource("let x = 1;", "main.czc");
  SourceLocation loc(id, 1, 5, 4);
  auto error = LexerError::make(LexerErrorCode::InvalidCharacter, loc, 1,
                                "unexpected character");

  std::string formatted = formatError(error, sm_);
  EXPECT_TRUE(formatted.find("main.czc") != std::string::npos);
  EXPECT_TRUE(formatted.find("1:5") != std::string::npos);
  EXPECT_TRUE(formatted.find("L1021") !=
              std::string::npos); // InvalidCharacter = 1021
  EXPECT_TRUE(formatted.find("unexpected character") != std::string::npos);
}

TEST_F(LexerErrorTest, FormatErrorWithInvalidBuffer) {
  SourceLocation loc(BufferID{999}, 1, 1, 0);
  auto error =
      LexerError::simple(LexerErrorCode::InvalidCharacter, loc, "test");

  std::string formatted = formatError(error, sm_);
  EXPECT_TRUE(formatted.find("<unknown>") != std::string::npos);
}

// ============================================================================
// ErrorCollector 测试
// ============================================================================

TEST_F(LexerErrorTest, ErrorCollectorEmpty) {
  ErrorCollector collector;
  EXPECT_FALSE(collector.hasErrors());
  EXPECT_EQ(collector.count(), 0u);
  EXPECT_TRUE(collector.errors().empty());
}

TEST_F(LexerErrorTest, ErrorCollectorAddError) {
  ErrorCollector collector;
  SourceLocation loc(BufferID{1}, 1, 1, 0);

  collector.add(
      LexerError::simple(LexerErrorCode::InvalidCharacter, loc, "error1"));
  EXPECT_TRUE(collector.hasErrors());
  EXPECT_EQ(collector.count(), 1u);
}

TEST_F(LexerErrorTest, ErrorCollectorAddMultipleErrors) {
  ErrorCollector collector;
  SourceLocation loc(BufferID{1}, 1, 1, 0);

  collector.add(
      LexerError::simple(LexerErrorCode::InvalidCharacter, loc, "error1"));
  collector.add(
      LexerError::simple(LexerErrorCode::InvalidNumberSuffix, loc, "error2"));
  collector.add(
      LexerError::simple(LexerErrorCode::UnterminatedString, loc, "error3"));

  EXPECT_EQ(collector.count(), 3u);

  const auto &errors = collector.errors();
  EXPECT_EQ(errors[0].code, LexerErrorCode::InvalidCharacter);
  EXPECT_EQ(errors[1].code, LexerErrorCode::InvalidNumberSuffix);
  EXPECT_EQ(errors[2].code, LexerErrorCode::UnterminatedString);
}

TEST_F(LexerErrorTest, ErrorCollectorClear) {
  ErrorCollector collector;
  SourceLocation loc(BufferID{1}, 1, 1, 0);

  collector.add(
      LexerError::simple(LexerErrorCode::InvalidCharacter, loc, "error1"));
  collector.add(
      LexerError::simple(LexerErrorCode::InvalidNumberSuffix, loc, "error2"));

  EXPECT_EQ(collector.count(), 2u);

  collector.clear();
  EXPECT_FALSE(collector.hasErrors());
  EXPECT_EQ(collector.count(), 0u);
}

// ============================================================================
// getExpansionChain 测试
// ============================================================================

TEST_F(LexerErrorTest, GetExpansionChainReturnsEmpty) {
  SourceLocation loc(BufferID{1}, 1, 1, 0);
  auto error =
      LexerError::simple(LexerErrorCode::InvalidCharacter, loc, "test");

  auto chain = getExpansionChain(error, sm_);
  EXPECT_TRUE(chain.empty());
}

} // namespace
} // namespace czc::lexer
