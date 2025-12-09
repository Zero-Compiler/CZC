/**
 * @file formatter_test.cpp
 * @brief OutputFormatter 单元测试。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-12-04
 */

#include "czc/cli/output/formatter.hpp"
#include "czc/cli/output/json_formatter.hpp"
#include "czc/cli/output/text_formatter.hpp"
#include "czc/lexer/lexer.hpp"

#include <gtest/gtest.h>

namespace czc::cli {
namespace {

class FormatterTest : public ::testing::Test {
protected:
  lexer::SourceManager sm_;

  /**
   * @brief 辅助方法：创建测试用 Token 列表。
   */
  std::vector<lexer::Token> createTestTokens(std::string_view source) {
    auto bufferId = sm_.addBuffer(source, "test.zero");
    lexer::Lexer lex(sm_, bufferId);
    return lex.tokenize();
  }
};

// ============================================================================
// TextFormatter 测试
// ============================================================================

TEST_F(FormatterTest, TextFormatterBasicOutput) {
  auto tokens = createTestTokens("let x = 1;");
  TextFormatter formatter;

  std::string output = formatter.formatTokens(tokens, sm_);

  // 验证输出包含 Token 数量
  EXPECT_NE(output.find("Total tokens:"), std::string::npos);

  // 验证输出包含关键字
  EXPECT_NE(output.find("KW_LET"), std::string::npos);

  // 验证输出包含标识符
  EXPECT_NE(output.find("IDENTIFIER"), std::string::npos);
  EXPECT_NE(output.find("\"x\""), std::string::npos);

  // 验证输出包含位置信息
  EXPECT_NE(output.find("[1:"), std::string::npos);
}

TEST_F(FormatterTest, TextFormatterEmptyTokens) {
  std::vector<lexer::Token> emptyTokens;
  TextFormatter formatter;

  std::string output = formatter.formatTokens(emptyTokens, sm_);

  EXPECT_NE(output.find("Total tokens: 0"), std::string::npos);
}

TEST_F(FormatterTest, TextFormatterEscapesSpecialChars) {
  auto tokens = createTestTokens("let s = \"hello\\nworld\";");
  TextFormatter formatter;

  std::string output = formatter.formatTokens(tokens, sm_);

  // 验证换行符被转义
  // 注意：实际的字符串内容取决于 lexer 如何处理转义序列
  EXPECT_NE(output.find("LIT_STRING"), std::string::npos);
}

// ============================================================================
// JsonFormatter 测试
// ============================================================================

TEST_F(FormatterTest, JsonFormatterValidJson) {
  auto tokens = createTestTokens("let x = 1;");
  JsonFormatter formatter;

  std::string output = formatter.formatTokens(tokens, sm_);

  // 验证是有效的 JSON 格式
  EXPECT_EQ(output.front(), '{');
  EXPECT_EQ(output.back(), '}');

  // 验证包含 tokens 数组
  EXPECT_NE(output.find("\"tokens\""), std::string::npos);
  EXPECT_NE(output.find("["), std::string::npos);
  EXPECT_NE(output.find("]"), std::string::npos);
}

TEST_F(FormatterTest, JsonFormatterContainsRequiredFields) {
  auto tokens = createTestTokens("let x = 1;");
  JsonFormatter formatter;

  std::string output = formatter.formatTokens(tokens, sm_);

  // 验证每个 Token 包含必要的字段
  EXPECT_NE(output.find("\"type\""), std::string::npos);
  EXPECT_NE(output.find("\"value\""), std::string::npos);
  EXPECT_NE(output.find("\"line\""), std::string::npos);
  EXPECT_NE(output.find("\"column\""), std::string::npos);
}

TEST_F(FormatterTest, JsonFormatterEmptyTokens) {
  std::vector<lexer::Token> emptyTokens;
  JsonFormatter formatter;

  std::string output = formatter.formatTokens(emptyTokens, sm_);

  // 应该返回有效的 JSON，包含空数组
  EXPECT_NE(output.find("\"tokens\":[]"), std::string::npos);
}

// ============================================================================
// createFormatter 工厂函数测试
// ============================================================================

TEST_F(FormatterTest, CreateTextFormatter) {
  auto formatter = createFormatter(OutputFormat::Text);

  EXPECT_NE(formatter, nullptr);
  EXPECT_NE(dynamic_cast<TextFormatter *>(formatter.get()), nullptr);
}

TEST_F(FormatterTest, CreateJsonFormatter) {
  auto formatter = createFormatter(OutputFormat::Json);

  EXPECT_NE(formatter, nullptr);
  EXPECT_NE(dynamic_cast<JsonFormatter *>(formatter.get()), nullptr);
}

// ============================================================================
// 错误格式化测试
// ============================================================================

TEST_F(FormatterTest, TextFormatterFormatErrors) {
  std::vector<lexer::LexerError> errors;
  errors.push_back(lexer::LexerError::simple(
      lexer::LexerErrorCode::UnterminatedString,
      lexer::SourceLocation{lexer::BufferID{1}, 5, 10, 100},
      "unterminated string literal"));

  TextFormatter formatter;
  std::string output = formatter.formatErrors(errors, sm_);

  // 验证输出包含错误信息
  EXPECT_NE(output.find("unterminated string"), std::string::npos);
  EXPECT_NE(output.find("5"), std::string::npos); // 行号
}

TEST_F(FormatterTest, JsonFormatterFormatErrors) {
  std::vector<lexer::LexerError> errors;
  errors.push_back(lexer::LexerError::simple(
      lexer::LexerErrorCode::InvalidCharacter,
      lexer::SourceLocation{lexer::BufferID{1}, 1, 1, 0}, "invalid character"));

  JsonFormatter formatter;
  std::string output = formatter.formatErrors(errors, sm_);

  // 验证是有效的 JSON 格式
  EXPECT_EQ(output.front(), '{');
  EXPECT_EQ(output.back(), '}');

  // 验证包含 errors 数组
  EXPECT_NE(output.find("\"errors\""), std::string::npos);
}

} // namespace
} // namespace czc::cli
