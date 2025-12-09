/**
 * @file scanner_test.cpp
 * @brief ScanContext 单元测试。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-30
 */

#include "czc/lexer/scanner.hpp"
#include "czc/lexer/source_manager.hpp"
#include "czc/lexer/source_reader.hpp"

#include <gtest/gtest.h>

namespace czc::lexer {
namespace {

class ScanContextTest : public ::testing::Test {
protected:
  SourceManager sm_;
  ErrorCollector errors_;

  BufferID addSource(std::string_view source,
                     std::string filename = "test.zero") {
    return sm_.addBuffer(source, std::move(filename));
  }

  std::unique_ptr<SourceReader> createReader(BufferID id) {
    return std::make_unique<SourceReader>(sm_, id);
  }
};

// ============================================================================
// 基本功能测试
// ============================================================================

TEST_F(ScanContextTest, CurrentChar) {
  auto id = addSource("abc");
  auto reader = createReader(id);
  ScanContext ctx(*reader, errors_);

  auto ch = ctx.current();
  ASSERT_TRUE(ch.has_value());
  EXPECT_EQ(ch.value(), 'a');
}

TEST_F(ScanContextTest, PeekChar) {
  auto id = addSource("abc");
  auto reader = createReader(id);
  ScanContext ctx(*reader, errors_);

  EXPECT_EQ(ctx.peek(0).value(), 'a');
  EXPECT_EQ(ctx.peek(1).value(), 'b');
  EXPECT_EQ(ctx.peek(2).value(), 'c');
  EXPECT_FALSE(ctx.peek(3).has_value());
}

TEST_F(ScanContextTest, IsAtEnd) {
  auto id = addSource("a");
  auto reader = createReader(id);
  ScanContext ctx(*reader, errors_);

  EXPECT_FALSE(ctx.isAtEnd());
  ctx.advance();
  EXPECT_TRUE(ctx.isAtEnd());
}

TEST_F(ScanContextTest, Location) {
  auto id = addSource("ab\ncd");
  auto reader = createReader(id);
  ScanContext ctx(*reader, errors_);

  auto loc = ctx.location();
  EXPECT_EQ(loc.line, 1u);
  EXPECT_EQ(loc.column, 1u);

  ctx.advance(); // 'a'
  ctx.advance(); // 'b'
  ctx.advance(); // '\n'

  loc = ctx.location();
  EXPECT_EQ(loc.line, 2u);
  EXPECT_EQ(loc.column, 1u);
}

TEST_F(ScanContextTest, Offset) {
  auto id = addSource("abc");
  auto reader = createReader(id);
  ScanContext ctx(*reader, errors_);

  EXPECT_EQ(ctx.offset(), 0u);
  ctx.advance();
  EXPECT_EQ(ctx.offset(), 1u);
  ctx.advance();
  EXPECT_EQ(ctx.offset(), 2u);
}

TEST_F(ScanContextTest, Buffer) {
  auto id = addSource("abc");
  auto reader = createReader(id);
  ScanContext ctx(*reader, errors_);

  EXPECT_EQ(ctx.buffer().value, id.value);
}

// ============================================================================
// advance 测试
// ============================================================================

TEST_F(ScanContextTest, AdvanceSingleChar) {
  auto id = addSource("abc");
  auto reader = createReader(id);
  ScanContext ctx(*reader, errors_);

  EXPECT_EQ(ctx.current().value(), 'a');
  ctx.advance();
  EXPECT_EQ(ctx.current().value(), 'b');
  ctx.advance();
  EXPECT_EQ(ctx.current().value(), 'c');
}

TEST_F(ScanContextTest, AdvanceMultipleChars) {
  auto id = addSource("abcdef");
  auto reader = createReader(id);
  ScanContext ctx(*reader, errors_);

  ctx.advance(3);
  EXPECT_EQ(ctx.current().value(), 'd');
  EXPECT_EQ(ctx.offset(), 3u);
}

// ============================================================================
// check / match 测试
// ============================================================================

TEST_F(ScanContextTest, CheckChar) {
  auto id = addSource("abc");
  auto reader = createReader(id);
  ScanContext ctx(*reader, errors_);

  EXPECT_TRUE(ctx.check('a'));
  EXPECT_FALSE(ctx.check('b'));
  EXPECT_FALSE(ctx.check('x'));
}

TEST_F(ScanContextTest, MatchCharSuccess) {
  auto id = addSource("abc");
  auto reader = createReader(id);
  ScanContext ctx(*reader, errors_);

  EXPECT_TRUE(ctx.match('a'));
  EXPECT_EQ(ctx.current().value(), 'b');
}

TEST_F(ScanContextTest, MatchCharFailure) {
  auto id = addSource("abc");
  auto reader = createReader(id);
  ScanContext ctx(*reader, errors_);

  EXPECT_FALSE(ctx.match('x'));
  EXPECT_EQ(ctx.current().value(), 'a');
}

TEST_F(ScanContextTest, MatchStringSuccess) {
  auto id = addSource("hello world");
  auto reader = createReader(id);
  ScanContext ctx(*reader, errors_);

  EXPECT_TRUE(ctx.match("hello"));
  EXPECT_EQ(ctx.current().value(), ' ');
}

TEST_F(ScanContextTest, MatchStringFailure) {
  auto id = addSource("hello world");
  auto reader = createReader(id);
  ScanContext ctx(*reader, errors_);

  EXPECT_FALSE(ctx.match("world"));
  EXPECT_EQ(ctx.current().value(), 'h');
}

TEST_F(ScanContextTest, MatchEmptyString) {
  auto id = addSource("abc");
  auto reader = createReader(id);
  ScanContext ctx(*reader, errors_);

  EXPECT_TRUE(ctx.match(""));
  EXPECT_EQ(ctx.current().value(), 'a');
}

TEST_F(ScanContextTest, MatchStringTooLong) {
  auto id = addSource("ab");
  auto reader = createReader(id);
  ScanContext ctx(*reader, errors_);

  EXPECT_FALSE(ctx.match("abcdef"));
  EXPECT_EQ(ctx.current().value(), 'a');
}

// ============================================================================
// slice / text 测试
// ============================================================================

TEST_F(ScanContextTest, SliceFrom) {
  auto id = addSource("hello world");
  auto reader = createReader(id);
  ScanContext ctx(*reader, errors_);

  ctx.advance(5);
  auto slice = ctx.sliceFrom(0);
  EXPECT_EQ(slice.offset, 0u);
  EXPECT_EQ(slice.length, 5u);
}

TEST_F(ScanContextTest, TextFrom) {
  auto id = addSource("hello world");
  auto reader = createReader(id);
  ScanContext ctx(*reader, errors_);

  ctx.advance(5);
  auto text = ctx.textFrom(0);
  EXPECT_EQ(text, "hello");
}

// ============================================================================
// sourceManager 测试
// ============================================================================

TEST_F(ScanContextTest, SourceManagerAccess) {
  auto id = addSource("abc", "test.zero");
  auto reader = createReader(id);
  ScanContext ctx(*reader, errors_);

  EXPECT_EQ(ctx.sourceManager().getFilename(id), "test.zero");

  const ScanContext &constCtx = ctx;
  EXPECT_EQ(constCtx.sourceManager().getFilename(id), "test.zero");
}

// ============================================================================
// 错误报告测试
// ============================================================================

TEST_F(ScanContextTest, ReportError) {
  auto id = addSource("abc");
  auto reader = createReader(id);
  ScanContext ctx(*reader, errors_);

  EXPECT_FALSE(ctx.hasErrors());

  ctx.reportError(LexerError::simple(LexerErrorCode::InvalidCharacter,
                                     ctx.location(), "test error"));

  EXPECT_TRUE(ctx.hasErrors());
  EXPECT_EQ(errors_.count(), 1u);
}

// ============================================================================
// makeToken 测试
// ============================================================================

TEST_F(ScanContextTest, MakeToken) {
  auto id = addSource("hello");
  auto reader = createReader(id);
  ScanContext ctx(*reader, errors_);

  auto startOffset = ctx.offset();
  auto startLoc = ctx.location();
  ctx.advance(5);

  auto token = ctx.makeToken(TokenType::IDENTIFIER, startOffset, startLoc);
  EXPECT_EQ(token.type(), TokenType::IDENTIFIER);
  EXPECT_EQ(token.value(sm_), "hello");
}

TEST_F(ScanContextTest, MakeUnknown) {
  auto id = addSource("@");
  auto reader = createReader(id);
  ScanContext ctx(*reader, errors_);

  auto startOffset = ctx.offset();
  auto startLoc = ctx.location();
  ctx.advance();

  auto token = ctx.makeUnknown(startOffset, startLoc);
  EXPECT_EQ(token.type(), TokenType::TOKEN_UNKNOWN);
}

// ============================================================================
// 空源测试
// ============================================================================

TEST_F(ScanContextTest, EmptySource) {
  auto id = addSource("");
  auto reader = createReader(id);
  ScanContext ctx(*reader, errors_);

  EXPECT_TRUE(ctx.isAtEnd());
  EXPECT_FALSE(ctx.current().has_value());
  EXPECT_FALSE(ctx.check('a'));
  EXPECT_FALSE(ctx.match('a'));
  EXPECT_FALSE(ctx.match("hello"));
}

} // namespace
} // namespace czc::lexer
