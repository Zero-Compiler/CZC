/**
 * @file ident_scanner_test.cpp
 * @brief IdentScanner 单元测试。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-30
 */

#include "czc/lexer/ident_scanner.hpp"
#include "czc/lexer/lexer_error.hpp"
#include "czc/lexer/source_manager.hpp"
#include "czc/lexer/source_reader.hpp"

#include <gtest/gtest.h>

namespace czc::lexer {
namespace {

class IdentScannerTest : public ::testing::Test {
protected:
  SourceManager sm_;
  IdentScanner scanner_;

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
};

// ============================================================================
// canScan 测试
// ============================================================================

TEST_F(IdentScannerTest, CanScanAsciiLetter) {
  EXPECT_TRUE(canScan("abc"));
  EXPECT_TRUE(canScan("ABC"));
  EXPECT_TRUE(canScan("z"));
  EXPECT_TRUE(canScan("Z"));
}

TEST_F(IdentScannerTest, CanScanUnderscore) {
  EXPECT_TRUE(canScan("_abc"));
  EXPECT_TRUE(canScan("_"));
  EXPECT_TRUE(canScan("__"));
}

TEST_F(IdentScannerTest, CannotScanDigitStart) {
  EXPECT_FALSE(canScan("123"));
  EXPECT_FALSE(canScan("1abc"));
}

TEST_F(IdentScannerTest, CannotScanOperatorStart) {
  EXPECT_FALSE(canScan("+"));
  EXPECT_FALSE(canScan("-"));
  EXPECT_FALSE(canScan("="));
}

TEST_F(IdentScannerTest, CanScanUnicodeStart) {
  EXPECT_TRUE(canScan("变量"));
  EXPECT_TRUE(canScan("日本語"));
  EXPECT_TRUE(canScan("αβγ"));
}

TEST_F(IdentScannerTest, CannotScanEmpty) { EXPECT_FALSE(canScan("")); }

// ============================================================================
// 基本标识符扫描测试
// ============================================================================

TEST_F(IdentScannerTest, ScanSimpleIdentifier) {
  auto tok = scan("hello");

  EXPECT_EQ(tok.type(), TokenType::IDENTIFIER);
  EXPECT_EQ(tok.value(sm_), "hello");
}

TEST_F(IdentScannerTest, ScanIdentifierWithDigits) {
  auto tok = scan("var123");

  EXPECT_EQ(tok.type(), TokenType::IDENTIFIER);
  EXPECT_EQ(tok.value(sm_), "var123");
}

TEST_F(IdentScannerTest, ScanIdentifierWithUnderscore) {
  auto tok = scan("my_variable");

  EXPECT_EQ(tok.type(), TokenType::IDENTIFIER);
  EXPECT_EQ(tok.value(sm_), "my_variable");
}

TEST_F(IdentScannerTest, ScanUnderscoreOnly) {
  // IdentScanner 将单独的 _ 识别为 IDENTIFIER
  // 因为 _ 是合法的标识符起始字符
  auto tok = scan("_");

  EXPECT_EQ(tok.type(), TokenType::IDENTIFIER);
  EXPECT_EQ(tok.value(sm_), "_");
}

TEST_F(IdentScannerTest, ScanDoubleUnderscore) {
  auto tok = scan("__");

  EXPECT_EQ(tok.type(), TokenType::IDENTIFIER);
  EXPECT_EQ(tok.value(sm_), "__");
}

TEST_F(IdentScannerTest, ScanIdentifierStartingWithUnderscore) {
  auto tok = scan("_identifier");

  EXPECT_EQ(tok.type(), TokenType::IDENTIFIER);
  EXPECT_EQ(tok.value(sm_), "_identifier");
}

TEST_F(IdentScannerTest, ScanIdentifierEndingWithUnderscore) {
  auto tok = scan("identifier_");

  EXPECT_EQ(tok.type(), TokenType::IDENTIFIER);
  EXPECT_EQ(tok.value(sm_), "identifier_");
}

// ============================================================================
// 关键字识别测试
// ============================================================================

TEST_F(IdentScannerTest, ScanKeywordLet) {
  auto tok = scan("let");

  EXPECT_EQ(tok.type(), TokenType::KW_LET);
  EXPECT_EQ(tok.value(sm_), "let");
}

TEST_F(IdentScannerTest, ScanKeywordVar) {
  auto tok = scan("var");

  EXPECT_EQ(tok.type(), TokenType::KW_VAR);
}

TEST_F(IdentScannerTest, ScanKeywordFn) {
  auto tok = scan("fn");

  EXPECT_EQ(tok.type(), TokenType::KW_FN);
}

TEST_F(IdentScannerTest, ScanKeywordIf) {
  auto tok = scan("if");

  EXPECT_EQ(tok.type(), TokenType::KW_IF);
}

TEST_F(IdentScannerTest, ScanKeywordElse) {
  auto tok = scan("else");

  EXPECT_EQ(tok.type(), TokenType::KW_ELSE);
}

TEST_F(IdentScannerTest, ScanKeywordFor) {
  auto tok = scan("for");

  EXPECT_EQ(tok.type(), TokenType::KW_FOR);
}

TEST_F(IdentScannerTest, ScanKeywordWhile) {
  auto tok = scan("while");

  EXPECT_EQ(tok.type(), TokenType::KW_WHILE);
}

TEST_F(IdentScannerTest, ScanKeywordReturn) {
  auto tok = scan("return");

  EXPECT_EQ(tok.type(), TokenType::KW_RETURN);
}

// ============================================================================
// 布尔和 null 字面量测试
// ============================================================================

TEST_F(IdentScannerTest, ScanTrue) {
  auto tok = scan("true");

  EXPECT_EQ(tok.type(), TokenType::LIT_TRUE);
}

TEST_F(IdentScannerTest, ScanFalse) {
  auto tok = scan("false");

  EXPECT_EQ(tok.type(), TokenType::LIT_FALSE);
}

TEST_F(IdentScannerTest, ScanNull) {
  auto tok = scan("null");

  EXPECT_EQ(tok.type(), TokenType::LIT_NULL);
}

// ============================================================================
// 关键字前缀/后缀不误识别测试
// ============================================================================

TEST_F(IdentScannerTest, KeywordPrefixIsIdentifier) {
  auto tok = scan("letter");

  EXPECT_EQ(tok.type(), TokenType::IDENTIFIER);
  EXPECT_EQ(tok.value(sm_), "letter");
}

TEST_F(IdentScannerTest, KeywordSuffixIsIdentifier) {
  auto tok = scan("ifelse");

  EXPECT_EQ(tok.type(), TokenType::IDENTIFIER);
  EXPECT_EQ(tok.value(sm_), "ifelse");
}

TEST_F(IdentScannerTest, KeywordWithNumberIsIdentifier) {
  auto tok = scan("for1");

  EXPECT_EQ(tok.type(), TokenType::IDENTIFIER);
  EXPECT_EQ(tok.value(sm_), "for1");
}

TEST_F(IdentScannerTest, KeywordWithUnderscoreIsIdentifier) {
  auto tok = scan("return_");

  EXPECT_EQ(tok.type(), TokenType::IDENTIFIER);
  EXPECT_EQ(tok.value(sm_), "return_");
}

// ============================================================================
// Unicode 标识符测试
// ============================================================================

TEST_F(IdentScannerTest, ScanChineseIdentifier) {
  auto tok = scan("变量");

  EXPECT_EQ(tok.type(), TokenType::IDENTIFIER);
  EXPECT_EQ(tok.value(sm_), "变量");
}

TEST_F(IdentScannerTest, ScanMixedChineseAsciiIdentifier) {
  auto tok = scan("变量_1");

  EXPECT_EQ(tok.type(), TokenType::IDENTIFIER);
  EXPECT_EQ(tok.value(sm_), "变量_1");
}

TEST_F(IdentScannerTest, ScanIdentifierWithChineseSuffix) {
  auto tok = scan("test变量");

  EXPECT_EQ(tok.type(), TokenType::IDENTIFIER);
  EXPECT_EQ(tok.value(sm_), "test变量");
}

TEST_F(IdentScannerTest, ScanGreekIdentifier) {
  auto tok = scan("αβγ");

  EXPECT_EQ(tok.type(), TokenType::IDENTIFIER);
  EXPECT_EQ(tok.value(sm_), "αβγ");
}

TEST_F(IdentScannerTest, ScanJapaneseIdentifier) {
  auto tok = scan("日本語");

  EXPECT_EQ(tok.type(), TokenType::IDENTIFIER);
  EXPECT_EQ(tok.value(sm_), "日本語");
}

// ============================================================================
// 边界情况测试
// ============================================================================

TEST_F(IdentScannerTest, ScanStopsAtOperator) {
  auto tok = scan("abc+def");

  EXPECT_EQ(tok.type(), TokenType::IDENTIFIER);
  EXPECT_EQ(tok.value(sm_), "abc");
}

TEST_F(IdentScannerTest, ScanStopsAtWhitespace) {
  auto tok = scan("abc def");

  EXPECT_EQ(tok.type(), TokenType::IDENTIFIER);
  EXPECT_EQ(tok.value(sm_), "abc");
}

TEST_F(IdentScannerTest, ScanStopsAtDelimiter) {
  auto tok = scan("func(");

  EXPECT_EQ(tok.type(), TokenType::IDENTIFIER);
  EXPECT_EQ(tok.value(sm_), "func");
}

} // namespace
} // namespace czc::lexer
