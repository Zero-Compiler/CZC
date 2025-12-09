/**
 * @file source_reader_test.cpp
 * @brief SourceReader 单元测试。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-30
 */

#include "czc/lexer/source_reader.hpp"

#include <gtest/gtest.h>

namespace czc::lexer {
namespace {

class SourceReaderTest : public ::testing::Test {
protected:
  SourceManager sm_;

  BufferID addSource(std::string_view source) {
    return sm_.addBuffer(source, "test.zero");
  }
};

// ============================================================================
// 基本功能测试
// ============================================================================

TEST_F(SourceReaderTest, InitialPositionIsAtStart) {
  auto id = addSource("hello");
  SourceReader reader(sm_, id);

  EXPECT_EQ(reader.offset(), 0u);
  EXPECT_EQ(reader.line(), 1u);
  EXPECT_EQ(reader.column(), 1u);
  EXPECT_FALSE(reader.isAtEnd());
}

TEST_F(SourceReaderTest, EmptySourceIsAtEnd) {
  auto id = addSource("");
  SourceReader reader(sm_, id);

  EXPECT_TRUE(reader.isAtEnd());
  EXPECT_EQ(reader.current(), std::nullopt);
}

TEST_F(SourceReaderTest, CurrentReturnsFirstChar) {
  auto id = addSource("abc");
  SourceReader reader(sm_, id);

  auto ch = reader.current();
  ASSERT_TRUE(ch.has_value());
  EXPECT_EQ(ch.value(), 'a');
}

TEST_F(SourceReaderTest, PeekReturnsCharAtOffset) {
  auto id = addSource("abcdef");
  SourceReader reader(sm_, id);

  EXPECT_EQ(reader.peek(0), 'a');
  EXPECT_EQ(reader.peek(1), 'b');
  EXPECT_EQ(reader.peek(2), 'c');
  EXPECT_EQ(reader.peek(5), 'f');
}

TEST_F(SourceReaderTest, PeekBeyondEndReturnsNullopt) {
  auto id = addSource("ab");
  SourceReader reader(sm_, id);

  EXPECT_EQ(reader.peek(0), 'a');
  EXPECT_EQ(reader.peek(1), 'b');
  EXPECT_EQ(reader.peek(2), std::nullopt);
  EXPECT_EQ(reader.peek(100), std::nullopt);
}

// ============================================================================
// Advance 测试
// ============================================================================

TEST_F(SourceReaderTest, AdvanceMovesPosition) {
  auto id = addSource("abc");
  SourceReader reader(sm_, id);

  reader.advance();
  EXPECT_EQ(reader.offset(), 1u);
  EXPECT_EQ(reader.current(), 'b');

  reader.advance();
  EXPECT_EQ(reader.offset(), 2u);
  EXPECT_EQ(reader.current(), 'c');

  reader.advance();
  EXPECT_TRUE(reader.isAtEnd());
}

TEST_F(SourceReaderTest, AdvanceUpdatesColumn) {
  auto id = addSource("hello");
  SourceReader reader(sm_, id);

  EXPECT_EQ(reader.column(), 1u);
  reader.advance();
  EXPECT_EQ(reader.column(), 2u);
  reader.advance();
  EXPECT_EQ(reader.column(), 3u);
}

TEST_F(SourceReaderTest, AdvanceWithCountMovesMultiplePositions) {
  auto id = addSource("abcdef");
  SourceReader reader(sm_, id);

  reader.advance(3);
  EXPECT_EQ(reader.offset(), 3u);
  EXPECT_EQ(reader.current(), 'd');
}

TEST_F(SourceReaderTest, NewlineUpdatesLineAndColumn) {
  auto id = addSource("ab\ncd");
  SourceReader reader(sm_, id);

  reader.advance(); // 'a'
  reader.advance(); // 'b'
  EXPECT_EQ(reader.line(), 1u);

  reader.advance(); // '\n'
  EXPECT_EQ(reader.line(), 2u);
  EXPECT_EQ(reader.column(), 1u);
}

TEST_F(SourceReaderTest, WindowsNewlineHandledAsSingleNewline) {
  auto id = addSource("a\r\nb");
  SourceReader reader(sm_, id);

  reader.advance(); // 'a'
  EXPECT_EQ(reader.line(), 1u);

  // 当前实现: \r\n 序列需要两次 advance
  // \r 不单独更新行号，\n 才更新
  reader.advance(); // '\r' - 不更新行号
  reader.advance(); // '\n' - 更新行号
  EXPECT_EQ(reader.line(), 2u);
  EXPECT_EQ(reader.column(), 1u);
}

// ============================================================================
// Location 测试
// ============================================================================

TEST_F(SourceReaderTest, LocationReturnsCorrectPosition) {
  auto id = addSource("abc\ndef");
  SourceReader reader(sm_, id);

  auto loc = reader.location();
  EXPECT_EQ(loc.buffer, id);
  EXPECT_EQ(loc.line, 1u);
  EXPECT_EQ(loc.column, 1u);
  EXPECT_EQ(loc.offset, 0u);

  reader.advance(4); // 到第二行
  loc = reader.location();
  EXPECT_EQ(loc.line, 2u);
  EXPECT_EQ(loc.column, 1u);
}

// ============================================================================
// Slice 测试
// ============================================================================

TEST_F(SourceReaderTest, SliceFromReturnsCorrectSlice) {
  auto id = addSource("hello world");
  SourceReader reader(sm_, id);

  reader.advance(5);
  auto slice = reader.sliceFrom(0);
  EXPECT_EQ(slice.offset, 0u);
  EXPECT_EQ(slice.length, 5u);
}

// ============================================================================
// Unicode 测试
// ============================================================================

TEST_F(SourceReaderTest, UnicodeSourceHandledCorrectly) {
  auto id = addSource("变量");
  SourceReader reader(sm_, id);

  // UTF-8: 变 = E5 8F 98, 量 = E9 87 8F
  // 每个中文字符占3个字节
  EXPECT_FALSE(reader.isAtEnd());

  // 逐字节读取
  auto ch = reader.current();
  ASSERT_TRUE(ch.has_value());
  // 第一个字节是 0xE5 (负数表示)
  EXPECT_EQ(static_cast<unsigned char>(ch.value()), 0xE5);
}

} // namespace
} // namespace czc::lexer
