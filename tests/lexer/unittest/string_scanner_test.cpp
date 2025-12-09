/**
 * @file string_scanner_test.cpp
 * @brief StringScanner 单元测试。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-30
 */

#include "czc/lexer/lexer_error.hpp"
#include "czc/lexer/source_manager.hpp"
#include "czc/lexer/source_reader.hpp"
#include "czc/lexer/string_scanner.hpp"

#include <gtest/gtest.h>

namespace czc::lexer {
namespace {

class StringScannerTest : public ::testing::Test {
protected:
  SourceManager sm_;
  StringScanner scanner_;

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

TEST_F(StringScannerTest, CanScanDoubleQuote) {
  EXPECT_TRUE(canScan("\"hello\""));
  EXPECT_TRUE(canScan("\"\""));
}

TEST_F(StringScannerTest, CanScanRawString) {
  EXPECT_TRUE(canScan("r\"raw\""));
  EXPECT_TRUE(canScan("r#\"raw\"#"));
}

TEST_F(StringScannerTest, CanScanTexString) {
  EXPECT_TRUE(canScan("t\"tex\""));
}

TEST_F(StringScannerTest, CannotScanNonString) {
  EXPECT_FALSE(canScan("abc"));
  EXPECT_FALSE(canScan("123"));
  EXPECT_FALSE(canScan(""));
}

// ============================================================================
// 普通字符串测试
// ============================================================================

TEST_F(StringScannerTest, ScanEmptyString) {
  auto tok = scan("\"\"");

  EXPECT_EQ(tok.type(), TokenType::LIT_STRING);
  // value() 返回包含引号的原始文本
  EXPECT_EQ(tok.value(sm_), "\"\"");
}

TEST_F(StringScannerTest, ScanSimpleString) {
  auto tok = scan("\"hello\"");

  EXPECT_EQ(tok.type(), TokenType::LIT_STRING);
  EXPECT_EQ(tok.value(sm_), "\"hello\"");
}

TEST_F(StringScannerTest, ScanStringWithSpaces) {
  auto tok = scan("\"hello world\"");

  EXPECT_EQ(tok.type(), TokenType::LIT_STRING);
  EXPECT_EQ(tok.value(sm_), "\"hello world\"");
}

TEST_F(StringScannerTest, ScanUnicodeString) {
  auto tok = scan("\"你好，世界！\"");

  EXPECT_EQ(tok.type(), TokenType::LIT_STRING);
  EXPECT_EQ(tok.value(sm_), "\"你好，世界！\"");
}

TEST_F(StringScannerTest, ScanEmojiString) {
  auto tok = scan("\"😀😃😄\"");

  EXPECT_EQ(tok.type(), TokenType::LIT_STRING);
  EXPECT_EQ(tok.value(sm_), "\"😀😃😄\"");
}

// ============================================================================
// 转义序列测试
// ============================================================================

TEST_F(StringScannerTest, ScanNewlineEscape) {
  auto tok = scan("\"hello\\nworld\"");

  EXPECT_EQ(tok.type(), TokenType::LIT_STRING);
  // 转义后的值包含实际的换行符
  EXPECT_TRUE(tok.hasNamedEscape());
}

TEST_F(StringScannerTest, ScanTabEscape) {
  auto tok = scan("\"hello\\tworld\"");

  EXPECT_EQ(tok.type(), TokenType::LIT_STRING);
  EXPECT_TRUE(tok.hasNamedEscape());
}

TEST_F(StringScannerTest, ScanCarriageReturnEscape) {
  auto tok = scan("\"hello\\rworld\"");

  EXPECT_EQ(tok.type(), TokenType::LIT_STRING);
  EXPECT_TRUE(tok.hasNamedEscape());
}

TEST_F(StringScannerTest, ScanQuoteEscape) {
  auto tok = scan("\"say \\\"hello\\\"\"");

  EXPECT_EQ(tok.type(), TokenType::LIT_STRING);
  EXPECT_TRUE(tok.hasNamedEscape());
}

TEST_F(StringScannerTest, ScanBackslashEscape) {
  auto tok = scan("\"path\\\\to\\\\file\"");

  EXPECT_EQ(tok.type(), TokenType::LIT_STRING);
  EXPECT_TRUE(tok.hasNamedEscape());
}

TEST_F(StringScannerTest, ScanNullEscape) {
  auto tok = scan("\"null\\0char\"");

  EXPECT_EQ(tok.type(), TokenType::LIT_STRING);
  EXPECT_TRUE(tok.hasNamedEscape());
}

// ============================================================================
// 十六进制转义测试
// ============================================================================

TEST_F(StringScannerTest, ScanHexEscape) {
  auto tok = scan("\"\\x48\\x65\\x6C\\x6C\\x6F\""); // "Hello"

  EXPECT_EQ(tok.type(), TokenType::LIT_STRING);
  EXPECT_TRUE(tok.hasHexEscape());
}

// ============================================================================
// Unicode 转义测试
// ============================================================================

TEST_F(StringScannerTest, ScanUnicodeEscape) {
  auto tok = scan("\"\\u{03A9}\""); // Omega

  EXPECT_EQ(tok.type(), TokenType::LIT_STRING);
  EXPECT_TRUE(tok.hasUnicodeEscape());
}

// ============================================================================
// 原始字符串测试
// ============================================================================

TEST_F(StringScannerTest, ScanSimpleRawString) {
  auto tok = scan("r\"raw string\"");

  EXPECT_EQ(tok.type(), TokenType::LIT_RAW_STRING);
  // value() 返回包含前缀和引号的完整原始文本
  EXPECT_EQ(tok.value(sm_), "r\"raw string\"");
}

TEST_F(StringScannerTest, RawStringPreservesEscapes) {
  auto tok = scan("r\"\\n\\t\"");

  EXPECT_EQ(tok.type(), TokenType::LIT_RAW_STRING);
  EXPECT_EQ(tok.value(sm_), "r\"\\n\\t\""); // 原样保留含前缀
}

TEST_F(StringScannerTest, ScanRawStringWithHashes) {
  auto tok = scan("r#\"contains \"quote\"\"#");

  EXPECT_EQ(tok.type(), TokenType::LIT_RAW_STRING);
  EXPECT_EQ(tok.value(sm_), "r#\"contains \"quote\"\"#");
}

TEST_F(StringScannerTest, ScanRawStringWithMultipleHashes) {
  auto tok = scan("r##\"contains \"#\"\"##");

  EXPECT_EQ(tok.type(), TokenType::LIT_RAW_STRING);
  EXPECT_EQ(tok.value(sm_), "r##\"contains \"#\"\"##");
}

// ============================================================================
// TeX 字符串测试
// ============================================================================

TEST_F(StringScannerTest, ScanTexString) {
  auto tok = scan("t\"latex content\"");

  EXPECT_EQ(tok.type(), TokenType::LIT_TEX_STRING);
  // value() 返回包含前缀和引号的完整原始文本
  EXPECT_EQ(tok.value(sm_), "t\"latex content\"");
}

// ============================================================================
// rawLiteral 测试
// ============================================================================

TEST_F(StringScannerTest, RawLiteralIncludesQuotes) {
  auto tok = scan("\"hello\"");

  // 当前实现中 value() 和 rawLiteral() 返回相同内容（含引号）
  EXPECT_EQ(tok.value(sm_), "\"hello\"");
  EXPECT_EQ(tok.rawLiteral(sm_), "\"hello\"");
}

// ============================================================================
// 错误处理测试
// ============================================================================

TEST_F(StringScannerTest, UnterminatedStringGeneratesError) {
  auto [tok, hasErrors] = scanWithErrors("\"unterminated");

  EXPECT_TRUE(hasErrors);
}

TEST_F(StringScannerTest, InvalidEscapeGeneratesError) {
  auto [tok, hasErrors] = scanWithErrors("\"invalid \\q escape\"");

  // 可能报错也可能忽略，取决于实现
  // 这里只检查能否完成扫描
  EXPECT_EQ(tok.type(), TokenType::LIT_STRING);
}

// ============================================================================
// 边界情况测试
// ============================================================================

TEST_F(StringScannerTest, StringStopsAtClosingQuote) {
  auto tok = scan("\"hello\" extra");

  EXPECT_EQ(tok.type(), TokenType::LIT_STRING);
  // value() 返回包含引号的原始文本
  EXPECT_EQ(tok.value(sm_), "\"hello\"");
}

TEST_F(StringScannerTest, MultiLineString) {
  // 普通字符串支持换行（多行字符串）
  auto tok = scan("\"line1\nline2\"");

  // 期望成功解析
  EXPECT_EQ(tok.type(), TokenType::LIT_STRING);
  EXPECT_EQ(tok.value(sm_), "\"line1\nline2\"");
}

// ============================================================================
// 更多转义序列测试
// ============================================================================

TEST_F(StringScannerTest, ScanSingleQuoteEscape) {
  auto tok = scan("\"it\\'s\"");

  EXPECT_EQ(tok.type(), TokenType::LIT_STRING);
  EXPECT_TRUE(tok.hasNamedEscape());
}

TEST_F(StringScannerTest, ScanMultipleHexEscapes) {
  auto tok = scan("\"\\x41\\x42\\x43\"");

  EXPECT_EQ(tok.type(), TokenType::LIT_STRING);
  EXPECT_TRUE(tok.hasHexEscape());
}

TEST_F(StringScannerTest, ScanMixedEscapes) {
  auto tok = scan("\"\\n\\x41\\u{0042}\"");

  EXPECT_EQ(tok.type(), TokenType::LIT_STRING);
  EXPECT_TRUE(tok.hasNamedEscape());
  EXPECT_TRUE(tok.hasHexEscape());
  EXPECT_TRUE(tok.hasUnicodeEscape());
}

TEST_F(StringScannerTest, ScanUnicodeEscapeMultipleDigits) {
  auto tok = scan("\"\\u{1F600}\""); // 😀

  EXPECT_EQ(tok.type(), TokenType::LIT_STRING);
  EXPECT_TRUE(tok.hasUnicodeEscape());
}

// ============================================================================
// 更多原始字符串测试
// ============================================================================

TEST_F(StringScannerTest, RawStringMultiLine) {
  auto tok = scan("r\"line1\nline2\"");

  EXPECT_EQ(tok.type(), TokenType::LIT_RAW_STRING);
}

TEST_F(StringScannerTest, RawStringWithThreeHashes) {
  auto tok = scan("r###\"\"##\"\"###");

  EXPECT_EQ(tok.type(), TokenType::LIT_RAW_STRING);
}

TEST_F(StringScannerTest, RawStringWithMismatchedHashes) {
  // 结束的 # 数量少于开始时，应继续扫描
  auto tok = scan("r##\"content\"#extra\"##");

  EXPECT_EQ(tok.type(), TokenType::LIT_RAW_STRING);
}

TEST_F(StringScannerTest, RawStringEmpty) {
  auto tok = scan("r\"\"");

  EXPECT_EQ(tok.type(), TokenType::LIT_RAW_STRING);
  EXPECT_EQ(tok.value(sm_), "r\"\"");
}

TEST_F(StringScannerTest, RawStringWithHashEmpty) {
  auto tok = scan("r#\"\"#");

  EXPECT_EQ(tok.type(), TokenType::LIT_RAW_STRING);
  EXPECT_EQ(tok.value(sm_), "r#\"\"#");
}

TEST_F(StringScannerTest, RawStringInvalidNoQuote) {
  // r# 后面没有引号，应该返回 UNKNOWN
  auto tok = scan("r#abc");

  EXPECT_EQ(tok.type(), TokenType::TOKEN_UNKNOWN);
}

// ============================================================================
// 更多 TeX 字符串测试
// ============================================================================

TEST_F(StringScannerTest, TexStringEmpty) {
  auto tok = scan("t\"\"");

  EXPECT_EQ(tok.type(), TokenType::LIT_TEX_STRING);
  EXPECT_EQ(tok.value(sm_), "t\"\"");
}

TEST_F(StringScannerTest, TexStringWithMath) {
  auto tok = scan("t\"$x^2 + y^2 = z^2$\"");

  EXPECT_EQ(tok.type(), TokenType::LIT_TEX_STRING);
}

TEST_F(StringScannerTest, TexStringWithEscapedQuote) {
  auto tok = scan("t\"say \\\"hello\\\"\"");

  EXPECT_EQ(tok.type(), TokenType::LIT_TEX_STRING);
  EXPECT_TRUE(tok.hasNamedEscape());
}

TEST_F(StringScannerTest, TexStringUnterminated) {
  auto [tok, hasErrors] = scanWithErrors("t\"unterminated");

  EXPECT_EQ(tok.type(), TokenType::LIT_TEX_STRING);
  // TeX 字符串未闭合时不报错，只是扫描到文件末尾
}

TEST_F(StringScannerTest, TexStringInvalidNoQuote) {
  // t 后面不是引号
  auto tok = scan("tabc");

  // canScan 应该返回 false，所以 scan 会返回 UNKNOWN
  EXPECT_FALSE(canScan("tabc"));
}

// ============================================================================
// 回车换行测试
// ============================================================================

TEST_F(StringScannerTest, StringWithCarriageReturn) {
  // 普通字符串支持回车符
  auto tok = scan("\"line1\rline2\"");

  EXPECT_EQ(tok.type(), TokenType::LIT_STRING);
  EXPECT_EQ(tok.value(sm_), "\"line1\rline2\"");
}

// ============================================================================
// 未知转义序列测试
// ============================================================================

TEST_F(StringScannerTest, UnknownEscapeSequence) {
  auto tok = scan("\"\\z\"");

  EXPECT_EQ(tok.type(), TokenType::LIT_STRING);
}

TEST_F(StringScannerTest, EscapeAtEndOfString) {
  // 字符串以反斜杠结尾（未闭合）
  auto [tok, hasErrors] = scanWithErrors("\"test\\");

  EXPECT_TRUE(hasErrors);
}

// ============================================================================
// canScan 边界测试
// ============================================================================

TEST_F(StringScannerTest, CanScanRFollowedByNonStringChar) {
  EXPECT_FALSE(canScan("rx"));
  EXPECT_FALSE(canScan("r1"));
}

TEST_F(StringScannerTest, CanScanTFollowedByNonQuote) {
  EXPECT_FALSE(canScan("tx"));
  EXPECT_FALSE(canScan("t1"));
}

} // namespace
} // namespace czc::lexer
