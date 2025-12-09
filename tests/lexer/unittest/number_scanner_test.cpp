/**
 * @file number_scanner_test.cpp
 * @brief NumberScanner 单元测试。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-30
 */

#include "czc/lexer/lexer_error.hpp"
#include "czc/lexer/number_scanner.hpp"
#include "czc/lexer/source_manager.hpp"
#include "czc/lexer/source_reader.hpp"

#include <gtest/gtest.h>

namespace czc::lexer {
namespace {

class NumberScannerTest : public ::testing::Test {
protected:
  SourceManager sm_;
  NumberScanner scanner_;

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

TEST_F(NumberScannerTest, CanScanDigit) {
  EXPECT_TRUE(canScan("0"));
  EXPECT_TRUE(canScan("1"));
  EXPECT_TRUE(canScan("9"));
  EXPECT_TRUE(canScan("123"));
}

TEST_F(NumberScannerTest, CannotScanNonDigit) {
  EXPECT_FALSE(canScan("abc"));
  EXPECT_FALSE(canScan("_"));
  EXPECT_FALSE(canScan("+"));
  EXPECT_FALSE(canScan("-"));
  EXPECT_FALSE(canScan(""));
}

// ============================================================================
// 十进制整数测试
// ============================================================================

TEST_F(NumberScannerTest, ScanSimpleInteger) {
  auto tok = scan("123");

  EXPECT_EQ(tok.type(), TokenType::LIT_INT);
  EXPECT_EQ(tok.value(sm_), "123");
}

TEST_F(NumberScannerTest, ScanZero) {
  auto tok = scan("0");

  EXPECT_EQ(tok.type(), TokenType::LIT_INT);
  EXPECT_EQ(tok.value(sm_), "0");
}

TEST_F(NumberScannerTest, ScanLargeInteger) {
  auto tok = scan("12345678901234567890");

  EXPECT_EQ(tok.type(), TokenType::LIT_INT);
  EXPECT_EQ(tok.value(sm_), "12345678901234567890");
}

TEST_F(NumberScannerTest, ScanIntegerWithUnderscores) {
  auto tok = scan("1_000_000");

  EXPECT_EQ(tok.type(), TokenType::LIT_INT);
  EXPECT_EQ(tok.value(sm_), "1_000_000");
}

// ============================================================================
// 十六进制整数测试
// ============================================================================

TEST_F(NumberScannerTest, ScanHexadecimalLowercase) {
  auto tok = scan("0x1a2b");

  EXPECT_EQ(tok.type(), TokenType::LIT_INT);
  EXPECT_EQ(tok.value(sm_), "0x1a2b");
}

TEST_F(NumberScannerTest, ScanHexadecimalUppercase) {
  auto tok = scan("0X1A2B");

  EXPECT_EQ(tok.type(), TokenType::LIT_INT);
  EXPECT_EQ(tok.value(sm_), "0X1A2B");
}

TEST_F(NumberScannerTest, ScanHexadecimalMixed) {
  auto tok = scan("0xDEADbeef");

  EXPECT_EQ(tok.type(), TokenType::LIT_INT);
  EXPECT_EQ(tok.value(sm_), "0xDEADbeef");
}

TEST_F(NumberScannerTest, ScanHexWithUnderscores) {
  auto tok = scan("0xFF_FF");

  EXPECT_EQ(tok.type(), TokenType::LIT_INT);
  EXPECT_EQ(tok.value(sm_), "0xFF_FF");
}

// ============================================================================
// 二进制整数测试
// ============================================================================

TEST_F(NumberScannerTest, ScanBinaryLowercase) {
  auto tok = scan("0b1010");

  EXPECT_EQ(tok.type(), TokenType::LIT_INT);
  EXPECT_EQ(tok.value(sm_), "0b1010");
}

TEST_F(NumberScannerTest, ScanBinaryUppercase) {
  auto tok = scan("0B1111");

  EXPECT_EQ(tok.type(), TokenType::LIT_INT);
  EXPECT_EQ(tok.value(sm_), "0B1111");
}

TEST_F(NumberScannerTest, ScanBinaryWithUnderscores) {
  auto tok = scan("0b1111_0000");

  EXPECT_EQ(tok.type(), TokenType::LIT_INT);
  EXPECT_EQ(tok.value(sm_), "0b1111_0000");
}

// ============================================================================
// 八进制整数测试
// ============================================================================

TEST_F(NumberScannerTest, ScanOctalLowercase) {
  auto tok = scan("0o755");

  EXPECT_EQ(tok.type(), TokenType::LIT_INT);
  EXPECT_EQ(tok.value(sm_), "0o755");
}

TEST_F(NumberScannerTest, ScanOctalUppercase) {
  auto tok = scan("0O644");

  EXPECT_EQ(tok.type(), TokenType::LIT_INT);
  EXPECT_EQ(tok.value(sm_), "0O644");
}

// ============================================================================
// 浮点数测试
// ============================================================================

TEST_F(NumberScannerTest, ScanSimpleFloat) {
  auto tok = scan("3.14");

  EXPECT_EQ(tok.type(), TokenType::LIT_FLOAT);
  EXPECT_EQ(tok.value(sm_), "3.14");
}

TEST_F(NumberScannerTest, ScanFloatStartingWithZero) {
  auto tok = scan("0.5");

  EXPECT_EQ(tok.type(), TokenType::LIT_FLOAT);
  EXPECT_EQ(tok.value(sm_), "0.5");
}

TEST_F(NumberScannerTest, ScanFloatWithMultipleDecimals) {
  auto tok = scan("123.456789");

  EXPECT_EQ(tok.type(), TokenType::LIT_FLOAT);
  EXPECT_EQ(tok.value(sm_), "123.456789");
}

// ============================================================================
// 科学计数法测试
// ============================================================================

TEST_F(NumberScannerTest, ScanScientificNotation) {
  auto tok = scan("1e10");

  EXPECT_EQ(tok.type(), TokenType::LIT_FLOAT);
  EXPECT_EQ(tok.value(sm_), "1e10");
}

TEST_F(NumberScannerTest, ScanScientificNotationUppercase) {
  auto tok = scan("1E10");

  EXPECT_EQ(tok.type(), TokenType::LIT_FLOAT);
  EXPECT_EQ(tok.value(sm_), "1E10");
}

TEST_F(NumberScannerTest, ScanScientificNotationWithPlus) {
  auto tok = scan("1e+5");

  EXPECT_EQ(tok.type(), TokenType::LIT_FLOAT);
  EXPECT_EQ(tok.value(sm_), "1e+5");
}

TEST_F(NumberScannerTest, ScanScientificNotationWithMinus) {
  auto tok = scan("1e-5");

  EXPECT_EQ(tok.type(), TokenType::LIT_FLOAT);
  EXPECT_EQ(tok.value(sm_), "1e-5");
}

TEST_F(NumberScannerTest, ScanFloatWithExponent) {
  auto tok = scan("1.23e10");

  EXPECT_EQ(tok.type(), TokenType::LIT_FLOAT);
  EXPECT_EQ(tok.value(sm_), "1.23e10");
}

// ============================================================================
// 类型后缀测试
// ============================================================================

TEST_F(NumberScannerTest, ScanIntegerWithI8Suffix) {
  auto tok = scan("1i8");

  EXPECT_EQ(tok.type(), TokenType::LIT_INT);
  EXPECT_EQ(tok.value(sm_), "1i8");
}

TEST_F(NumberScannerTest, ScanIntegerWithU64Suffix) {
  auto tok = scan("100u64");

  EXPECT_EQ(tok.type(), TokenType::LIT_INT);
  EXPECT_EQ(tok.value(sm_), "100u64");
}

TEST_F(NumberScannerTest, ScanFloatWithF32Suffix) {
  auto tok = scan("3.14f32");

  EXPECT_EQ(tok.type(), TokenType::LIT_FLOAT);
  EXPECT_EQ(tok.value(sm_), "3.14f32");
}

TEST_F(NumberScannerTest, ScanFloatWithF64Suffix) {
  auto tok = scan("3.14f64");

  EXPECT_EQ(tok.type(), TokenType::LIT_FLOAT);
  EXPECT_EQ(tok.value(sm_), "3.14f64");
}

// ============================================================================
// 定点数测试
// ============================================================================

TEST_F(NumberScannerTest, ScanDecimalWithDSuffix) {
  auto tok = scan("11.0d");

  EXPECT_EQ(tok.type(), TokenType::LIT_DECIMAL);
  EXPECT_EQ(tok.value(sm_), "11.0d");
}

TEST_F(NumberScannerTest, ScanDecimalWithDec64Suffix) {
  auto tok = scan("12.0dec64");

  EXPECT_EQ(tok.type(), TokenType::LIT_DECIMAL);
  EXPECT_EQ(tok.value(sm_), "12.0dec64");
}

// ============================================================================
// 边界情况测试
// ============================================================================

TEST_F(NumberScannerTest, NumberStopsAtOperator) {
  auto tok = scan("123+456");

  EXPECT_EQ(tok.type(), TokenType::LIT_INT);
  EXPECT_EQ(tok.value(sm_), "123");
}

TEST_F(NumberScannerTest, NumberStopsAtWhitespace) {
  auto tok = scan("123 456");

  EXPECT_EQ(tok.type(), TokenType::LIT_INT);
  EXPECT_EQ(tok.value(sm_), "123");
}

TEST_F(NumberScannerTest, NumberStopsAtDelimiter) {
  auto tok = scan("123;");

  EXPECT_EQ(tok.type(), TokenType::LIT_INT);
  EXPECT_EQ(tok.value(sm_), "123");
}

TEST_F(NumberScannerTest, FloatStopsAtSecondDot) {
  // 3.14. 应该是 3.14 后跟 .
  auto tok = scan("3.14.");

  EXPECT_EQ(tok.type(), TokenType::LIT_FLOAT);
  EXPECT_EQ(tok.value(sm_), "3.14");
}

TEST_F(NumberScannerTest, IntegerFollowedByDotDot) {
  // 0..10 应该是 0 后跟 ..
  auto tok = scan("0..10");

  EXPECT_EQ(tok.type(), TokenType::LIT_INT);
  EXPECT_EQ(tok.value(sm_), "0");
}

} // namespace
} // namespace czc::lexer
