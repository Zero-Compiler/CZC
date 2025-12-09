/**
 * @file comment_scanner_test.cpp
 * @brief CommentScanner 单元测试。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-30
 */

#include "czc/lexer/comment_scanner.hpp"
#include "czc/lexer/lexer_error.hpp"
#include "czc/lexer/source_manager.hpp"
#include "czc/lexer/source_reader.hpp"

#include <gtest/gtest.h>

namespace czc::lexer {
namespace {

class CommentScannerTest : public ::testing::Test {
protected:
  SourceManager sm_;
  CommentScanner scanner_;

  /**
   * @brief 辅助方法：创建 ScanContext 并扫描。
   */
  Token scan(std::string_view source) {
    auto id = sm_.addBuffer(source, "test.zero");
    SourceReader reader(sm_, id);
    ErrorCollector errors;
    ScanContext ctx(reader, errors);
    return scanner_.scan(ctx);
  }

  /**
   * @brief 辅助方法：检查 canScan。
   */
  bool canScan(std::string_view source) {
    auto id = sm_.addBuffer(source, "test.zero");
    SourceReader reader(sm_, id);
    ErrorCollector errors;
    ScanContext ctx(reader, errors);
    return scanner_.canScan(ctx);
  }

  /**
   * @brief 辅助方法：扫描并检查是否有错误。
   */
  std::pair<Token, bool> scanWithErrors(std::string_view source) {
    auto id = sm_.addBuffer(source, "test.zero");
    SourceReader reader(sm_, id);
    ErrorCollector errors;
    ScanContext ctx(reader, errors);
    auto tok = scanner_.scan(ctx);
    return {tok, errors.hasErrors()};
  }
};

// ============================================================================
// canScan 测试
// ============================================================================

TEST_F(CommentScannerTest, CanScanLineComment) {
  EXPECT_TRUE(canScan("// comment"));
  EXPECT_TRUE(canScan("//"));
}

TEST_F(CommentScannerTest, CanScanBlockComment) {
  EXPECT_TRUE(canScan("/* comment */"));
  EXPECT_TRUE(canScan("/**/"));
}

TEST_F(CommentScannerTest, CanScanDocComment) {
  EXPECT_TRUE(canScan("/** doc */"));
}

TEST_F(CommentScannerTest, CannotScanNonComment) {
  EXPECT_FALSE(canScan("abc"));
  EXPECT_FALSE(canScan("/")); // 单独的 / 不是注释
  // 注意：/* 可以被识别为块注释开始，即使未闭合
  EXPECT_TRUE(canScan("/*"));
  EXPECT_FALSE(canScan(""));
}

TEST_F(CommentScannerTest, CannotScanDivision) {
  // / 后面不是 / 或 * 不能作为注释
  EXPECT_FALSE(canScan("/a"));
  EXPECT_FALSE(canScan("/ "));
}

// ============================================================================
// 行注释测试
// ============================================================================

TEST_F(CommentScannerTest, ScanSimpleLineComment) {
  auto tok = scan("// this is a comment");

  EXPECT_EQ(tok.type(), TokenType::COMMENT_LINE);
  EXPECT_EQ(tok.value(sm_), "// this is a comment");
}

TEST_F(CommentScannerTest, ScanEmptyLineComment) {
  auto tok = scan("//");

  EXPECT_EQ(tok.type(), TokenType::COMMENT_LINE);
  EXPECT_EQ(tok.value(sm_), "//");
}

TEST_F(CommentScannerTest, LineCommentStopsAtNewline) {
  auto tok = scan("// comment\ncode");

  EXPECT_EQ(tok.type(), TokenType::COMMENT_LINE);
  EXPECT_EQ(tok.value(sm_), "// comment");
}

TEST_F(CommentScannerTest, LineCommentWithUnicode) {
  auto tok = scan("// 这是中文注释");

  EXPECT_EQ(tok.type(), TokenType::COMMENT_LINE);
  EXPECT_EQ(tok.value(sm_), "// 这是中文注释");
}

// ============================================================================
// 块注释测试
// ============================================================================

TEST_F(CommentScannerTest, ScanSimpleBlockComment) {
  auto tok = scan("/* block comment */");

  EXPECT_EQ(tok.type(), TokenType::COMMENT_BLOCK);
  EXPECT_EQ(tok.value(sm_), "/* block comment */");
}

TEST_F(CommentScannerTest, ScanEmptyBlockComment) {
  auto tok = scan("/**/");

  EXPECT_EQ(tok.type(), TokenType::COMMENT_BLOCK);
  EXPECT_EQ(tok.value(sm_), "/**/");
}

TEST_F(CommentScannerTest, ScanMultiLineBlockComment) {
  auto tok = scan("/* line1\nline2\nline3 */");

  EXPECT_EQ(tok.type(), TokenType::COMMENT_BLOCK);
}

TEST_F(CommentScannerTest, BlockCommentWithStars) {
  auto tok = scan("/* * * * */");

  EXPECT_EQ(tok.type(), TokenType::COMMENT_BLOCK);
}

// ============================================================================
// 文档注释测试
// ============================================================================

TEST_F(CommentScannerTest, ScanDocComment) {
  auto tok = scan("/** doc comment */");

  EXPECT_EQ(tok.type(), TokenType::COMMENT_DOC);
  EXPECT_EQ(tok.value(sm_), "/** doc comment */");
}

TEST_F(CommentScannerTest, ScanMultiLineDocComment) {
  auto tok = scan("/**\n * line 1\n * line 2\n */");

  EXPECT_EQ(tok.type(), TokenType::COMMENT_DOC);
}

// ============================================================================
// 嵌套块注释测试
// ============================================================================

TEST_F(CommentScannerTest, ScanNestedBlockComment) {
  // 如果支持嵌套，应该正确解析
  auto tok = scan("/* outer /* inner */ outer */");

  // 根据实现，可能是 COMMENT_BLOCK
  // 嵌套注释的内部 */ 可能结束外部注释
  EXPECT_EQ(tok.type(), TokenType::COMMENT_BLOCK);
}

// ============================================================================
// 错误处理测试
// ============================================================================

TEST_F(CommentScannerTest, UnterminatedBlockCommentGeneratesError) {
  auto [tok, hasErrors] = scanWithErrors("/* unterminated");

  EXPECT_TRUE(hasErrors);
}

// ============================================================================
// 边界情况测试
// ============================================================================

TEST_F(CommentScannerTest, BlockCommentStopsCorrectly) {
  auto tok = scan("/* comment */ code");

  EXPECT_EQ(tok.type(), TokenType::COMMENT_BLOCK);
  EXPECT_EQ(tok.value(sm_), "/* comment */");
}

TEST_F(CommentScannerTest, ConsecutiveSlashesInLineComment) {
  auto tok = scan("/// triple slash");

  // 可能是 COMMENT_LINE 或 COMMENT_DOC，取决于实现
  EXPECT_TRUE(tok.type() == TokenType::COMMENT_LINE ||
              tok.type() == TokenType::COMMENT_DOC);
}

} // namespace
} // namespace czc::lexer
