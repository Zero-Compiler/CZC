/**
 * @file context_test.cpp
 * @brief CompilerContext 单元测试。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-12-04
 */

#include "czc/cli/context.hpp"
#include "czc/diag/diag_builder.hpp"
#include "czc/diag/message.hpp"

#include <gtest/gtest.h>

namespace czc::cli {
namespace {

class CompilerContextTest : public ::testing::Test {
protected:
  CompilerContext ctx_;
};

// ============================================================================
// GlobalOptions 测试
// ============================================================================

TEST_F(CompilerContextTest, DefaultGlobalOptions) {
  auto &global = ctx_.global();

  EXPECT_EQ(global.logLevel, LogLevel::Normal);
  EXPECT_TRUE(global.colorDiagnostics);
}

TEST_F(CompilerContextTest, ModifyGlobalOptions) {
  ctx_.global().logLevel = LogLevel::Verbose;
  ctx_.global().colorDiagnostics = false;

  EXPECT_EQ(ctx_.global().logLevel, LogLevel::Verbose);
  EXPECT_FALSE(ctx_.global().colorDiagnostics);
}

TEST_F(CompilerContextTest, IsVerbose) {
  EXPECT_FALSE(ctx_.isVerbose());

  ctx_.global().logLevel = LogLevel::Verbose;
  EXPECT_TRUE(ctx_.isVerbose());

  ctx_.global().logLevel = LogLevel::Debug;
  EXPECT_TRUE(ctx_.isVerbose());
}

TEST_F(CompilerContextTest, IsQuiet) {
  EXPECT_FALSE(ctx_.isQuiet());

  ctx_.global().logLevel = LogLevel::Quiet;
  EXPECT_TRUE(ctx_.isQuiet());
}

// ============================================================================
// OutputOptions 测试
// ============================================================================

TEST_F(CompilerContextTest, DefaultOutputOptions) {
  auto &output = ctx_.output();

  EXPECT_FALSE(output.file.has_value());
  EXPECT_EQ(output.format, OutputFormat::Text);
}

TEST_F(CompilerContextTest, SetOutputFile) {
  ctx_.output().file = std::filesystem::path("/tmp/output.txt");

  EXPECT_TRUE(ctx_.output().file.has_value());
  EXPECT_EQ(ctx_.output().file.value().string(), "/tmp/output.txt");
}

TEST_F(CompilerContextTest, SetOutputFormat) {
  ctx_.output().format = OutputFormat::Json;

  EXPECT_EQ(ctx_.output().format, OutputFormat::Json);
}

// ============================================================================
// LexerOptions 测试
// ============================================================================

TEST_F(CompilerContextTest, DefaultLexerOptions) {
  auto &lexer = ctx_.lexer();

  EXPECT_FALSE(lexer.preserveTrivia);
  EXPECT_FALSE(lexer.dumpTokens);
}

TEST_F(CompilerContextTest, ModifyLexerOptions) {
  ctx_.lexer().preserveTrivia = true;
  ctx_.lexer().dumpTokens = true;

  EXPECT_TRUE(ctx_.lexer().preserveTrivia);
  EXPECT_TRUE(ctx_.lexer().dumpTokens);
}

// ============================================================================
// DiagContext 测试
// ============================================================================

TEST_F(CompilerContextTest, DiagnosticsInitialState) {
  EXPECT_EQ(ctx_.diagContext().errorCount(), 0u);
  EXPECT_EQ(ctx_.diagContext().warningCount(), 0u);
  EXPECT_FALSE(ctx_.diagContext().hasErrors());
}

TEST_F(CompilerContextTest, ReportError) {
  ctx_.diagContext().emit(diag::error(diag::Message("test error")).build());

  EXPECT_EQ(ctx_.diagContext().errorCount(), 1u);
  EXPECT_TRUE(ctx_.diagContext().hasErrors());
}

TEST_F(CompilerContextTest, ReportWarning) {
  ctx_.diagContext().emit(diag::warning(diag::Message("test warning")).build());

  EXPECT_EQ(ctx_.diagContext().warningCount(), 1u);
  EXPECT_FALSE(ctx_.diagContext().hasErrors());
}

TEST_F(CompilerContextTest, ClearDiagnostics) {
  // 注意：DiagContext 目前不支持清除统计
  // 这个测试只验证可以发射多个诊断

  ctx_.diagContext().emit(diag::error(diag::Message("test error")).build());
  ctx_.diagContext().emit(diag::warning(diag::Message("test warning")).build());

  EXPECT_EQ(ctx_.diagContext().errorCount(), 1u);
  EXPECT_EQ(ctx_.diagContext().warningCount(), 1u);
}

} // namespace
} // namespace czc::cli
