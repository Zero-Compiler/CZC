/**
 * @file char_scanner_test.cpp
 * @brief CharScanner 单元测试。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-30
 */

#include "czc/lexer/char_scanner.hpp"
#include "czc/lexer/lexer_error.hpp"
#include "czc/lexer/source_manager.hpp"
#include "czc/lexer/source_reader.hpp"

#include <gtest/gtest.h>

namespace czc::lexer {
namespace {

class CharScannerTest : public ::testing::Test {
protected:
  SourceManager sm_;
  CharScanner scanner_;

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

TEST_F(CharScannerTest, CanScanOperators) {
  EXPECT_TRUE(canScan("+"));
  EXPECT_TRUE(canScan("-"));
  EXPECT_TRUE(canScan("*"));
  EXPECT_TRUE(canScan("/"));
  EXPECT_TRUE(canScan("%"));
  EXPECT_TRUE(canScan("="));
  EXPECT_TRUE(canScan("!"));
  EXPECT_TRUE(canScan("<"));
  EXPECT_TRUE(canScan(">"));
  EXPECT_TRUE(canScan("&"));
  EXPECT_TRUE(canScan("|"));
  EXPECT_TRUE(canScan("^"));
  EXPECT_TRUE(canScan("~"));
  EXPECT_TRUE(canScan("."));
  EXPECT_TRUE(canScan("@"));
}

TEST_F(CharScannerTest, CanScanDelimiters) {
  EXPECT_TRUE(canScan("("));
  EXPECT_TRUE(canScan(")"));
  EXPECT_TRUE(canScan("{"));
  EXPECT_TRUE(canScan("}"));
  EXPECT_TRUE(canScan("["));
  EXPECT_TRUE(canScan("]"));
  EXPECT_TRUE(canScan(","));
  EXPECT_TRUE(canScan(":"));
  EXPECT_TRUE(canScan(";"));
}

TEST_F(CharScannerTest, CannotScanNonOperators) {
  EXPECT_FALSE(canScan("abc"));
  EXPECT_FALSE(canScan("123"));
  EXPECT_FALSE(canScan(""));
}

// ============================================================================
// 单字符运算符测试
// ============================================================================

TEST_F(CharScannerTest, ScanPlus) {
  auto tok = scan("+");

  EXPECT_EQ(tok.type(), TokenType::OP_PLUS);
  EXPECT_EQ(tok.value(sm_), "+");
}

TEST_F(CharScannerTest, ScanMinus) {
  auto tok = scan("-");

  EXPECT_EQ(tok.type(), TokenType::OP_MINUS);
}

TEST_F(CharScannerTest, ScanStar) {
  auto tok = scan("*");

  EXPECT_EQ(tok.type(), TokenType::OP_STAR);
}

TEST_F(CharScannerTest, ScanSlash) {
  auto tok = scan("/");

  EXPECT_EQ(tok.type(), TokenType::OP_SLASH);
}

TEST_F(CharScannerTest, ScanPercent) {
  auto tok = scan("%");

  EXPECT_EQ(tok.type(), TokenType::OP_PERCENT);
}

TEST_F(CharScannerTest, ScanLogicalNot) {
  auto tok = scan("!");

  EXPECT_EQ(tok.type(), TokenType::OP_LOGICAL_NOT);
}

TEST_F(CharScannerTest, ScanBitNot) {
  auto tok = scan("~");

  EXPECT_EQ(tok.type(), TokenType::OP_BIT_NOT);
}

TEST_F(CharScannerTest, ScanAt) {
  auto tok = scan("@");

  EXPECT_EQ(tok.type(), TokenType::OP_AT);
}

// ============================================================================
// 单字符分隔符测试
// ============================================================================

TEST_F(CharScannerTest, ScanLeftParen) {
  auto tok = scan("(");

  EXPECT_EQ(tok.type(), TokenType::DELIM_LPAREN);
}

TEST_F(CharScannerTest, ScanRightParen) {
  auto tok = scan(")");

  EXPECT_EQ(tok.type(), TokenType::DELIM_RPAREN);
}

TEST_F(CharScannerTest, ScanLeftBrace) {
  auto tok = scan("{");

  EXPECT_EQ(tok.type(), TokenType::DELIM_LBRACE);
}

TEST_F(CharScannerTest, ScanRightBrace) {
  auto tok = scan("}");

  EXPECT_EQ(tok.type(), TokenType::DELIM_RBRACE);
}

TEST_F(CharScannerTest, ScanLeftBracket) {
  auto tok = scan("[");

  EXPECT_EQ(tok.type(), TokenType::DELIM_LBRACKET);
}

TEST_F(CharScannerTest, ScanRightBracket) {
  auto tok = scan("]");

  EXPECT_EQ(tok.type(), TokenType::DELIM_RBRACKET);
}

TEST_F(CharScannerTest, ScanComma) {
  auto tok = scan(",");

  EXPECT_EQ(tok.type(), TokenType::DELIM_COMMA);
}

TEST_F(CharScannerTest, ScanSemicolon) {
  auto tok = scan(";");

  EXPECT_EQ(tok.type(), TokenType::DELIM_SEMICOLON);
}

// ============================================================================
// 双字符运算符测试
// ============================================================================

TEST_F(CharScannerTest, ScanEqual) {
  auto tok = scan("==");

  EXPECT_EQ(tok.type(), TokenType::OP_EQ);
  EXPECT_EQ(tok.value(sm_), "==");
}

TEST_F(CharScannerTest, ScanNotEqual) {
  auto tok = scan("!=");

  EXPECT_EQ(tok.type(), TokenType::OP_NE);
}

TEST_F(CharScannerTest, ScanLessEqual) {
  auto tok = scan("<=");

  EXPECT_EQ(tok.type(), TokenType::OP_LE);
}

TEST_F(CharScannerTest, ScanGreaterEqual) {
  auto tok = scan(">=");

  EXPECT_EQ(tok.type(), TokenType::OP_GE);
}

TEST_F(CharScannerTest, ScanLessThan) {
  auto tok = scan("<");

  EXPECT_EQ(tok.type(), TokenType::OP_LT);
}

TEST_F(CharScannerTest, ScanGreaterThan) {
  auto tok = scan(">");

  EXPECT_EQ(tok.type(), TokenType::OP_GT);
}

TEST_F(CharScannerTest, ScanLogicalAnd) {
  auto tok = scan("&&");

  EXPECT_EQ(tok.type(), TokenType::OP_LOGICAL_AND);
}

TEST_F(CharScannerTest, ScanLogicalOr) {
  auto tok = scan("||");

  EXPECT_EQ(tok.type(), TokenType::OP_LOGICAL_OR);
}

TEST_F(CharScannerTest, ScanBitShl) {
  auto tok = scan("<<");

  EXPECT_EQ(tok.type(), TokenType::OP_BIT_SHL);
}

TEST_F(CharScannerTest, ScanBitShr) {
  auto tok = scan(">>");

  EXPECT_EQ(tok.type(), TokenType::OP_BIT_SHR);
}

TEST_F(CharScannerTest, ScanArrow) {
  auto tok = scan("->");

  EXPECT_EQ(tok.type(), TokenType::OP_ARROW);
}

TEST_F(CharScannerTest, ScanFatArrow) {
  auto tok = scan("=>");

  EXPECT_EQ(tok.type(), TokenType::OP_FAT_ARROW);
}

TEST_F(CharScannerTest, ScanColonColon) {
  auto tok = scan("::");

  EXPECT_EQ(tok.type(), TokenType::OP_COLON_COLON);
}

TEST_F(CharScannerTest, ScanDotDot) {
  auto tok = scan("..");

  EXPECT_EQ(tok.type(), TokenType::OP_DOT_DOT);
}

// ============================================================================
// 复合赋值运算符测试
// ============================================================================

TEST_F(CharScannerTest, ScanAssign) {
  auto tok = scan("=");

  EXPECT_EQ(tok.type(), TokenType::OP_ASSIGN);
}

TEST_F(CharScannerTest, ScanPlusAssign) {
  auto tok = scan("+=");

  EXPECT_EQ(tok.type(), TokenType::OP_PLUS_ASSIGN);
}

TEST_F(CharScannerTest, ScanMinusAssign) {
  auto tok = scan("-=");

  EXPECT_EQ(tok.type(), TokenType::OP_MINUS_ASSIGN);
}

TEST_F(CharScannerTest, ScanStarAssign) {
  auto tok = scan("*=");

  EXPECT_EQ(tok.type(), TokenType::OP_STAR_ASSIGN);
}

TEST_F(CharScannerTest, ScanSlashAssign) {
  auto tok = scan("/=");

  EXPECT_EQ(tok.type(), TokenType::OP_SLASH_ASSIGN);
}

TEST_F(CharScannerTest, ScanPercentAssign) {
  auto tok = scan("%=");

  EXPECT_EQ(tok.type(), TokenType::OP_PERCENT_ASSIGN);
}

TEST_F(CharScannerTest, ScanAndAssign) {
  auto tok = scan("&=");

  EXPECT_EQ(tok.type(), TokenType::OP_AND_ASSIGN);
}

TEST_F(CharScannerTest, ScanOrAssign) {
  auto tok = scan("|=");

  EXPECT_EQ(tok.type(), TokenType::OP_OR_ASSIGN);
}

TEST_F(CharScannerTest, ScanXorAssign) {
  auto tok = scan("^=");

  EXPECT_EQ(tok.type(), TokenType::OP_XOR_ASSIGN);
}

// ============================================================================
// 三字符运算符测试
// ============================================================================

TEST_F(CharScannerTest, ScanDotDotEq) {
  auto tok = scan("..=");

  EXPECT_EQ(tok.type(), TokenType::OP_DOT_DOT_EQ);
  EXPECT_EQ(tok.value(sm_), "..=");
}

TEST_F(CharScannerTest, ScanShlAssign) {
  auto tok = scan("<<=");

  EXPECT_EQ(tok.type(), TokenType::OP_SHL_ASSIGN);
}

TEST_F(CharScannerTest, ScanShrAssign) {
  auto tok = scan(">>=");

  EXPECT_EQ(tok.type(), TokenType::OP_SHR_ASSIGN);
}

// ============================================================================
// 贪婪匹配测试（最长匹配优先）
// ============================================================================

TEST_F(CharScannerTest, GreedyMatchArrow) {
  // -> 应该优先于 - 和 >
  auto tok = scan("->");

  EXPECT_EQ(tok.type(), TokenType::OP_ARROW);
}

TEST_F(CharScannerTest, GreedyMatchFatArrow) {
  // => 应该优先于 = 和 >
  auto tok = scan("=>");

  EXPECT_EQ(tok.type(), TokenType::OP_FAT_ARROW);
}

TEST_F(CharScannerTest, GreedyMatchDotDotEq) {
  // ..= 应该优先于 .. 和 =
  auto tok = scan("..=");

  EXPECT_EQ(tok.type(), TokenType::OP_DOT_DOT_EQ);
}

TEST_F(CharScannerTest, GreedyMatchShlAssign) {
  // <<= 应该优先于 << 和 =
  auto tok = scan("<<=");

  EXPECT_EQ(tok.type(), TokenType::OP_SHL_ASSIGN);
}

// ============================================================================
// 边界情况测试
// ============================================================================

TEST_F(CharScannerTest, OperatorFollowedByOther) {
  // + 后面跟着 1，只扫描 +
  auto tok = scan("+1");

  EXPECT_EQ(tok.type(), TokenType::OP_PLUS);
  EXPECT_EQ(tok.value(sm_), "+");
}

TEST_F(CharScannerTest, OperatorFollowedBySpace) {
  auto tok = scan("+ ");

  EXPECT_EQ(tok.type(), TokenType::OP_PLUS);
}

TEST_F(CharScannerTest, SingleDot) {
  auto tok = scan(".");

  EXPECT_EQ(tok.type(), TokenType::OP_DOT);
}

TEST_F(CharScannerTest, SingleColon) {
  auto tok = scan(":");

  EXPECT_EQ(tok.type(), TokenType::DELIM_COLON);
}

TEST_F(CharScannerTest, DoubleColonFollowedByIdent) {
  // :: 后跟标识符
  auto tok = scan("::name");

  EXPECT_EQ(tok.type(), TokenType::OP_COLON_COLON);
  EXPECT_EQ(tok.value(sm_), "::");
}

// ============================================================================
// 保留运算符测试
// ============================================================================

TEST_F(CharScannerTest, ScanHash) {
  auto tok = scan("#");

  EXPECT_EQ(tok.type(), TokenType::OP_HASH);
}

TEST_F(CharScannerTest, ScanDollar) {
  auto tok = scan("$");

  EXPECT_EQ(tok.type(), TokenType::OP_DOLLAR);
}

TEST_F(CharScannerTest, ScanBackslash) {
  auto tok = scan("\\");

  EXPECT_EQ(tok.type(), TokenType::OP_BACKSLASH);
}

} // namespace
} // namespace czc::lexer
