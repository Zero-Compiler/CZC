/**
 * @file source_manager_test.cpp
 * @brief SourceManager 单元测试。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-30
 */

#include "czc/lexer/source_manager.hpp"

#include <gtest/gtest.h>

namespace czc::lexer {
namespace {

// ============================================================================
// BufferID 测试
// ============================================================================

TEST(BufferIDTest, DefaultConstructorCreatesInvalidID) {
  BufferID id;
  EXPECT_FALSE(id.isValid());
  EXPECT_EQ(id.value, 0u);
}

TEST(BufferIDTest, InvalidFactoryMethod) {
  auto id = BufferID::invalid();
  EXPECT_FALSE(id.isValid());
  EXPECT_EQ(id.value, 0u);
}

TEST(BufferIDTest, ValidIDHasNonZeroValue) {
  BufferID id{1};
  EXPECT_TRUE(id.isValid());
  EXPECT_EQ(id.value, 1u);
}

TEST(BufferIDTest, Equality) {
  BufferID id1{1};
  BufferID id2{1};
  BufferID id3{2};

  EXPECT_EQ(id1, id2);
  EXPECT_NE(id1, id3);
}

// ============================================================================
// ExpansionID 测试
// ============================================================================

TEST(ExpansionIDTest, DefaultConstructorCreatesInvalidID) {
  ExpansionID id;
  EXPECT_FALSE(id.isValid());
  EXPECT_EQ(id.value, 0u);
}

TEST(ExpansionIDTest, InvalidFactoryMethod) {
  auto id = ExpansionID::invalid();
  EXPECT_FALSE(id.isValid());
}

// ============================================================================
// SourceManager 测试
// ============================================================================

class SourceManagerTest : public ::testing::Test {
protected:
  SourceManager sm_;

  /**
   * @brief 辅助方法：添加源码缓冲区（使用 string_view）。
   */
  BufferID addSource(std::string_view source, std::string filename) {
    return sm_.addBuffer(source, std::move(filename));
  }
};

TEST_F(SourceManagerTest, InitiallyHasNoBuffers) {
  EXPECT_EQ(sm_.bufferCount(), 0u);
}

TEST_F(SourceManagerTest, AddBufferReturnsValidID) {
  auto id = addSource("let x = 1;", "test.zero");
  EXPECT_TRUE(id.isValid());
  EXPECT_EQ(sm_.bufferCount(), 1u);
}

TEST_F(SourceManagerTest, AddBufferWithMoveSemantics) {
  std::string source = "fn main() {}";
  auto id = sm_.addBuffer(std::move(source), "main.zero");
  EXPECT_TRUE(id.isValid());
  EXPECT_EQ(sm_.getSource(id), "fn main() {}");
}

TEST_F(SourceManagerTest, AddBufferWithStringView) {
  std::string_view source = "var y = 2;";
  auto id = sm_.addBuffer(source, "view.zero");
  EXPECT_TRUE(id.isValid());
  EXPECT_EQ(sm_.getSource(id), source);
}

TEST_F(SourceManagerTest, MultipleBuffersGetUniqueIDs) {
  auto id1 = addSource("source1", "file1.zero");
  auto id2 = addSource("source2", "file2.zero");
  auto id3 = addSource("source3", "file3.zero");

  EXPECT_NE(id1, id2);
  EXPECT_NE(id2, id3);
  EXPECT_NE(id1, id3);
  EXPECT_EQ(sm_.bufferCount(), 3u);
}

TEST_F(SourceManagerTest, GetSourceReturnsCorrectContent) {
  auto id = addSource("hello world", "test.zero");
  EXPECT_EQ(sm_.getSource(id), "hello world");
}

TEST_F(SourceManagerTest, GetSourceWithInvalidIDReturnsEmpty) {
  auto result = sm_.getSource(BufferID::invalid());
  EXPECT_TRUE(result.empty());
}

TEST_F(SourceManagerTest, SliceReturnsCorrectSubstring) {
  auto id = addSource("hello world", "test.zero");
  auto slice = sm_.slice(id, 0, 5);
  EXPECT_EQ(slice, "hello");

  slice = sm_.slice(id, 6, 5);
  EXPECT_EQ(slice, "world");
}

TEST_F(SourceManagerTest, SliceWithInvalidIDReturnsEmpty) {
  auto result = sm_.slice(BufferID::invalid(), 0, 5);
  EXPECT_TRUE(result.empty());
}

TEST_F(SourceManagerTest, GetFilenameReturnsCorrectName) {
  auto id = addSource("content", "my_file.zero");
  EXPECT_EQ(sm_.getFilename(id), "my_file.zero");
}

TEST_F(SourceManagerTest, GetFilenameWithInvalidIDReturnsEmpty) {
  auto result = sm_.getFilename(BufferID::invalid());
  EXPECT_TRUE(result.empty());
}

TEST_F(SourceManagerTest, GetLineContentReturnsCorrectLine) {
  auto id = addSource("line1\nline2\nline3", "test.zero");

  EXPECT_EQ(sm_.getLineContent(id, 1), "line1");
  EXPECT_EQ(sm_.getLineContent(id, 2), "line2");
  EXPECT_EQ(sm_.getLineContent(id, 3), "line3");
}

TEST_F(SourceManagerTest, GetLineContentWithInvalidLineReturnsEmpty) {
  auto id = addSource("line1\nline2", "test.zero");
  auto result = sm_.getLineContent(id, 100);
  EXPECT_TRUE(result.empty());
}

TEST_F(SourceManagerTest, SyntheticBufferIsMarkedAsSynthetic) {
  auto realId = addSource("real source", "real.zero");
  auto synthId = sm_.addSyntheticBuffer("synthetic code", "<macro>", realId);

  EXPECT_FALSE(sm_.isSynthetic(realId));
  EXPECT_TRUE(sm_.isSynthetic(synthId));
}

TEST_F(SourceManagerTest, EmptySourceHandledCorrectly) {
  auto id = addSource("", "empty.zero");
  EXPECT_TRUE(id.isValid());
  EXPECT_TRUE(sm_.getSource(id).empty());
}

TEST_F(SourceManagerTest, UnicodeSourceHandledCorrectly) {
  auto id = addSource("let 变量 = \"你好世界\";", "unicode.zero");
  EXPECT_EQ(sm_.getSource(id), "let 变量 = \"你好世界\";");
}

// ============================================================================
// slice 边界测试
// ============================================================================

TEST_F(SourceManagerTest, SliceWithOutOfBoundsOffsetReturnsEmpty) {
  auto id = addSource("hello", "test.zero");
  auto result = sm_.slice(id, 100, 5);
  EXPECT_TRUE(result.empty());
}

TEST_F(SourceManagerTest, SliceWithExcessLengthIsTruncated) {
  auto id = addSource("hello", "test.zero");
  auto result = sm_.slice(id, 2, 100);
  EXPECT_EQ(result, "llo");
}

TEST_F(SourceManagerTest, SliceWithOversizedBufferID) {
  auto result = sm_.slice(BufferID{999}, 0, 5);
  EXPECT_TRUE(result.empty());
}

// ============================================================================
// getSource 边界测试
// ============================================================================

TEST_F(SourceManagerTest, GetSourceWithOversizedBufferID) {
  auto result = sm_.getSource(BufferID{999});
  EXPECT_TRUE(result.empty());
}

// ============================================================================
// getFilename 边界测试
// ============================================================================

TEST_F(SourceManagerTest, GetFilenameWithOversizedBufferID) {
  auto result = sm_.getFilename(BufferID{999});
  EXPECT_TRUE(result.empty());
}

// ============================================================================
// getLineContent 边界测试
// ============================================================================

TEST_F(SourceManagerTest, GetLineContentWithInvalidBufferID) {
  auto result = sm_.getLineContent(BufferID::invalid(), 1);
  EXPECT_TRUE(result.empty());
}

TEST_F(SourceManagerTest, GetLineContentWithOversizedBufferID) {
  auto result = sm_.getLineContent(BufferID{999}, 1);
  EXPECT_TRUE(result.empty());
}

TEST_F(SourceManagerTest, GetLineContentWithZeroLine) {
  auto id = addSource("line1\nline2", "test.zero");
  auto result = sm_.getLineContent(id, 0);
  EXPECT_TRUE(result.empty());
}

TEST_F(SourceManagerTest, GetLineContentLastLineNoNewline) {
  auto id = addSource("line1\nline2", "test.zero");
  EXPECT_EQ(sm_.getLineContent(id, 2), "line2");
}

TEST_F(SourceManagerTest, GetLineContentWithCRLF) {
  auto id = addSource("line1\r\nline2", "test.zero");
  EXPECT_EQ(sm_.getLineContent(id, 1), "line1");
  EXPECT_EQ(sm_.getLineContent(id, 2), "line2");
}

TEST_F(SourceManagerTest, GetLineContentSingleLine) {
  auto id = addSource("single line", "test.zero");
  EXPECT_EQ(sm_.getLineContent(id, 1), "single line");
}

// ============================================================================
// Synthetic Buffer 测试
// ============================================================================

TEST_F(SourceManagerTest, IsSyntheticWithInvalidID) {
  EXPECT_FALSE(sm_.isSynthetic(BufferID::invalid()));
}

TEST_F(SourceManagerTest, IsSyntheticWithOversizedID) {
  EXPECT_FALSE(sm_.isSynthetic(BufferID{999}));
}

TEST_F(SourceManagerTest, GetParentBufferReturnsCorrectParent) {
  auto realId = addSource("real source", "real.zero");
  auto synthId = sm_.addSyntheticBuffer("synthetic", "<macro>", realId);

  auto parent = sm_.getParentBuffer(synthId);
  ASSERT_TRUE(parent.has_value());
  EXPECT_EQ(parent.value(), realId);
}

TEST_F(SourceManagerTest, GetParentBufferOfRealBufferReturnsNullopt) {
  auto realId = addSource("real source", "real.zero");
  auto parent = sm_.getParentBuffer(realId);
  EXPECT_FALSE(parent.has_value());
}

TEST_F(SourceManagerTest, GetParentBufferWithInvalidID) {
  auto parent = sm_.getParentBuffer(BufferID::invalid());
  EXPECT_FALSE(parent.has_value());
}

TEST_F(SourceManagerTest, GetParentBufferWithOversizedID) {
  auto parent = sm_.getParentBuffer(BufferID{999});
  EXPECT_FALSE(parent.has_value());
}

// ============================================================================
// File Chain 测试
// ============================================================================

TEST_F(SourceManagerTest, GetFileChainSingleBuffer) {
  auto id = addSource("source", "file.zero");
  auto chain = sm_.getFileChain(id);

  ASSERT_EQ(chain.size(), 1u);
  EXPECT_EQ(chain[0], "file.zero");
}

TEST_F(SourceManagerTest, GetFileChainWithSynthetic) {
  auto realId = addSource("real source", "real.zero");
  auto synthId = sm_.addSyntheticBuffer("synthetic", "<macro>", realId);

  auto chain = sm_.getFileChain(synthId);
  ASSERT_EQ(chain.size(), 2u);
  EXPECT_EQ(chain[0], "<macro>");
  EXPECT_EQ(chain[1], "real.zero");
}

TEST_F(SourceManagerTest, GetFileChainDeep) {
  auto id1 = addSource("source1", "file1.zero");
  auto id2 = sm_.addSyntheticBuffer("source2", "<macro1>", id1);
  auto id3 = sm_.addSyntheticBuffer("source3", "<macro2>", id2);

  auto chain = sm_.getFileChain(id3);
  ASSERT_EQ(chain.size(), 3u);
  EXPECT_EQ(chain[0], "<macro2>");
  EXPECT_EQ(chain[1], "<macro1>");
  EXPECT_EQ(chain[2], "file1.zero");
}

TEST_F(SourceManagerTest, GetFileChainWithInvalidID) {
  auto chain = sm_.getFileChain(BufferID::invalid());
  EXPECT_TRUE(chain.empty());
}

// ============================================================================
// ExpansionInfo 测试
// ============================================================================

TEST_F(SourceManagerTest, AddExpansionInfo) {
  SourceManager::ExpansionInfo info;
  info.callSiteBuffer = BufferID{1};
  info.callSiteOffset = 0;
  info.callSiteLine = 1;
  info.callSiteColumn = 1;
  info.macroDefBuffer = BufferID{2};
  info.macroNameOffset = 0;
  info.macroNameLength = 5;
  info.parent = ExpansionID::invalid();

  auto expId = sm_.addExpansionInfo(std::move(info));
  EXPECT_TRUE(expId.isValid());
}

TEST_F(SourceManagerTest, GetExpansionInfoValid) {
  SourceManager::ExpansionInfo info;
  info.callSiteBuffer = BufferID{1};
  info.callSiteOffset = 10;
  info.callSiteLine = 5;
  info.callSiteColumn = 3;
  info.macroDefBuffer = BufferID{2};
  info.macroNameOffset = 20;
  info.macroNameLength = 8;
  info.parent = ExpansionID::invalid();

  auto expId = sm_.addExpansionInfo(info);
  auto retrieved = sm_.getExpansionInfo(expId);

  ASSERT_TRUE(retrieved.has_value());
  EXPECT_EQ(retrieved->get().callSiteOffset, 10u);
  EXPECT_EQ(retrieved->get().macroNameOffset, 20u);
}

TEST_F(SourceManagerTest, GetExpansionInfoInvalid) {
  auto result = sm_.getExpansionInfo(ExpansionID::invalid());
  EXPECT_FALSE(result.has_value());
}

TEST_F(SourceManagerTest, GetExpansionInfoOversizedID) {
  auto result = sm_.getExpansionInfo(ExpansionID{999});
  EXPECT_FALSE(result.has_value());
}

} // namespace
} // namespace czc::lexer
