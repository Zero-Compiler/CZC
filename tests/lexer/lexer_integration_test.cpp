/**
 * @file lexer_integration_test.cpp
 * @brief Lexer 模块集成测试。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-12-04
 *
 * @details
 *   本文件包含词法分析器的集成测试，验证：
 *   - 完整源文件的词法分析
 *   - 多文件并发处理
 *   - 错误恢复和诊断
 *   - 与 CLI 层的集成
 */

#include "czc/cli/context.hpp"
#include "czc/cli/phases/lexer_phase.hpp"
#include "czc/lexer/lexer.hpp"

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

namespace czc::lexer {
namespace {

class LexerIntegrationTest : public ::testing::Test {
protected:
  cli::CompilerContext ctx_;
  std::filesystem::path testDir_;

  void SetUp() override {
    // 创建临时测试目录
    testDir_ = std::filesystem::temp_directory_path() / "czc_lexer_test";
    std::filesystem::create_directories(testDir_);
  }

  void TearDown() override {
    // 清理临时测试目录
    std::filesystem::remove_all(testDir_);
  }

  /**
   * @brief 创建临时测试文件。
   */
  std::filesystem::path createTestFile(std::string_view filename,
                                       std::string_view content) {
    auto path = testDir_ / filename;
    std::ofstream ofs(path);
    ofs << content;
    return path;
  }
};

// ============================================================================
// 完整源文件测试
// ============================================================================

TEST_F(LexerIntegrationTest, TokenizeCompleteSourceFile) {
  auto path = createTestFile("src.zero", R"(
// 这是一个完整的源文件示例

fn add(a: i32, b: i32) -> i32 {
    return a + b;
}

fn main() {
    let x = 42;
    let y = 10;
    let result = add(x, y);
}
)");

  cli::LexerPhase phase(ctx_);
  auto result = phase.runOnFile(path);

  ASSERT_TRUE(result.has_value()) << "Lexer failed: " << result.error().message;
  EXPECT_FALSE(result->hasErrors);
  EXPECT_GT(result->tokens.size(), 20u);

  // 验证第一个有意义的 Token 是 fn 关键字
  // 跳过 TOKEN_COMMENT
  bool foundFn = false;
  for (const auto &token : result->tokens) {
    if (token.type() == TokenType::KW_FN) {
      foundFn = true;
      break;
    }
  }
  EXPECT_TRUE(foundFn) << "Expected 'fn' keyword in tokens";
}

TEST_F(LexerIntegrationTest, TokenizeWithTrivia) {
  ctx_.lexer().preserveTrivia = true;

  auto path = createTestFile("trivia.zero", R"(let x = 1; // comment
let y = 2;
)");

  cli::LexerPhase phase(ctx_);
  auto result = phase.runOnFile(path);

  ASSERT_TRUE(result.has_value());
  EXPECT_FALSE(result->hasErrors);

  // 检查是否有 Token 带有 trivia
  bool hasLeadingTrivia = false;
  bool hasTrailingTrivia = false;
  for (const auto &token : result->tokens) {
    if (!token.leadingTrivia().empty()) {
      hasLeadingTrivia = true;
    }
    if (!token.trailingTrivia().empty()) {
      hasTrailingTrivia = true;
    }
  }

  EXPECT_TRUE(hasLeadingTrivia || hasTrailingTrivia)
      << "Expected trivia when preserveTrivia is enabled";
}

// ============================================================================
// 错误处理测试
// ============================================================================

TEST_F(LexerIntegrationTest, HandleInvalidUtf8) {
  // 创建包含无效 UTF-8 序列的文件
  auto path = testDir_ / "invalid_utf8.zero";
  std::ofstream ofs(path, std::ios::binary);
  ofs << "let x = \x80\x81\x82;"; // 无效的 UTF-8 序列
  ofs.close();

  cli::LexerPhase phase(ctx_);
  auto result = phase.runOnFile(path);

  ASSERT_TRUE(result.has_value());
  // 即使有错误，也应该生成 Token（错误恢复）
  EXPECT_GT(result->tokens.size(), 0u);
}

TEST_F(LexerIntegrationTest, HandleUnterminatedString) {
  auto path = createTestFile("unterminated.zero", R"(
let s = "unterminated string
let x = 1;
)");

  cli::LexerPhase phase(ctx_);
  auto result = phase.runOnFile(path);

  ASSERT_TRUE(result.has_value());
  EXPECT_TRUE(result->hasErrors);

  // 尽管有错误，后续的 Token 仍应被解析（错误恢复）
  bool foundLet = false;
  bool foundX = false;
  for (const auto &token : result->tokens) {
    if (token.type() == TokenType::KW_LET) {
      foundLet = true;
    }
    if (token.type() == TokenType::IDENTIFIER) {
      foundX = true;
    }
  }
  EXPECT_TRUE(foundLet)
      << "Error recovery should allow parsing subsequent tokens";
}

// ============================================================================
// 多文件处理测试
// ============================================================================

TEST_F(LexerIntegrationTest, ProcessMultipleFiles) {
  auto path1 = createTestFile("file1.zero", "let a = 1;");
  auto path2 = createTestFile("file2.zero", "let b = 2;");

  cli::LexerPhase phase1(ctx_);
  cli::LexerPhase phase2(ctx_);

  auto result1 = phase1.runOnFile(path1);
  auto result2 = phase2.runOnFile(path2);

  ASSERT_TRUE(result1.has_value());
  ASSERT_TRUE(result2.has_value());

  // 验证两个文件的 Token 是独立的
  bool foundA = false;
  bool foundB = false;

  for (const auto &token : result1->tokens) {
    auto val = token.value(phase1.sourceManager());
    if (val == "a")
      foundA = true;
  }

  for (const auto &token : result2->tokens) {
    auto val = token.value(phase2.sourceManager());
    if (val == "b")
      foundB = true;
  }

  EXPECT_TRUE(foundA);
  EXPECT_TRUE(foundB);
}

// ============================================================================
// 边界条件测试
// ============================================================================

TEST_F(LexerIntegrationTest, HandleEmptyFile) {
  auto path = createTestFile("empty.zero", "");

  cli::LexerPhase phase(ctx_);
  auto result = phase.runOnFile(path);

  ASSERT_TRUE(result.has_value());
  EXPECT_FALSE(result->hasErrors);
  ASSERT_EQ(result->tokens.size(), 1u);
  EXPECT_EQ(result->tokens[0].type(), TokenType::TOKEN_EOF);
}

TEST_F(LexerIntegrationTest, HandleWhitespaceOnlyFile) {
  auto path = createTestFile("whitespace.zero", "   \n\t\n   ");

  cli::LexerPhase phase(ctx_);
  auto result = phase.runOnFile(path);

  ASSERT_TRUE(result.has_value());
  EXPECT_FALSE(result->hasErrors);
  ASSERT_EQ(result->tokens.size(), 1u);
  EXPECT_EQ(result->tokens[0].type(), TokenType::TOKEN_EOF);
}

TEST_F(LexerIntegrationTest, HandleNonExistentFile) {
  std::filesystem::path nonExistent = testDir_ / "does_not_exist.zero";

  cli::LexerPhase phase(ctx_);
  auto result = phase.runOnFile(nonExistent);

  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(result.error().code, "E001"); // File not found
}

// ============================================================================
// Unicode 支持测试
// ============================================================================

TEST_F(LexerIntegrationTest, HandleUnicodeIdentifiers) {
  auto path = createTestFile("unicode.zero", R"(
let 变量 = 1;
let αβγ = 2;
let emoji🎉 = 3;
)");

  cli::LexerPhase phase(ctx_);
  auto result = phase.runOnFile(path);

  ASSERT_TRUE(result.has_value());
  // 根据语言规范，某些 Unicode 字符可能不是有效的标识符
  // 这里主要验证不会崩溃
}

TEST_F(LexerIntegrationTest, HandleUnicodeStrings) {
  auto path = createTestFile("unicode_strings.zero", R"(
let hello = "你好世界";
let emoji = "🎉🎊🎁";
)");

  cli::LexerPhase phase(ctx_);
  auto result = phase.runOnFile(path);

  ASSERT_TRUE(result.has_value());
  EXPECT_FALSE(result->hasErrors);

  // 验证字符串字面量被正确解析
  int stringCount = 0;
  for (const auto &token : result->tokens) {
    if (token.type() == TokenType::LIT_STRING) {
      stringCount++;
    }
  }
  EXPECT_EQ(stringCount, 2);
}

} // namespace
} // namespace czc::lexer
