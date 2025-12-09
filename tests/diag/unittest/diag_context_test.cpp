/**
 * @file diag_context_test.cpp
 * @brief DiagContext 单元测试。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-12-04
 */

#include "czc/diag/diag_context.hpp"
#include "czc/diag/emitter.hpp"
#include "czc/diag/i18n.hpp"
#include "czc/diag/message.hpp"

#include <gtest/gtest.h>
#include <sstream>
#include <vector>

namespace czc::diag {
namespace {

/// 测试用的 Mock Emitter
class MockEmitter : public Emitter {
public:
  void emit(const Diagnostic &diag, const SourceLocator *) override {
    emittedDiagnostics_.push_back(diag);
  }

  void emitSummary(const DiagnosticStats &) override { summaryEmitted_ = true; }

  void flush() override { flushed_ = true; }

  [[nodiscard]] auto emittedCount() const noexcept -> size_t {
    return emittedDiagnostics_.size();
  }

  [[nodiscard]] auto emittedDiagnostics() const
      -> const std::vector<Diagnostic> & {
    return emittedDiagnostics_;
  }

  [[nodiscard]] auto summaryEmitted() const noexcept -> bool {
    return summaryEmitted_;
  }

  [[nodiscard]] auto flushed() const noexcept -> bool { return flushed_; }

  void clear() {
    emittedDiagnostics_.clear();
    summaryEmitted_ = false;
    flushed_ = false;
  }

private:
  std::vector<Diagnostic> emittedDiagnostics_;
  bool summaryEmitted_{false};
  bool flushed_{false};
};

class DiagContextTest : public ::testing::Test {
protected:
  void SetUp() override {
    mockEmitter_ = new MockEmitter();
    ctx_ = std::make_unique<DiagContext>(std::unique_ptr<Emitter>(mockEmitter_),
                                         nullptr, DiagConfig{});
  }

  MockEmitter *mockEmitter_;
  std::unique_ptr<DiagContext> ctx_;
};

// ============================================================================
// 构造函数测试
// ============================================================================

TEST_F(DiagContextTest, ConstructWithEmitter) {
  EXPECT_EQ(ctx_->errorCount(), 0);
  EXPECT_EQ(ctx_->warningCount(), 0);
  EXPECT_FALSE(ctx_->hasErrors());
}

TEST_F(DiagContextTest, ConstructWithTranslator) {
  auto translator = std::make_unique<i18n::Translator>();
  translator->setLocale(i18n::Locale::ZhCN);

  auto emitter = std::make_unique<MockEmitter>();
  DiagContext ctx(std::move(emitter), nullptr, DiagConfig{},
                  std::move(translator));

  EXPECT_EQ(ctx.translator().currentLocale(), i18n::Locale::ZhCN);
}

TEST_F(DiagContextTest, DefaultTranslator) {
  // 没有提供 translator 时应该创建默认实例
  EXPECT_EQ(ctx_->translator().currentLocale(), i18n::Locale::En);
}

TEST_F(DiagContextTest, TranslatorAccessor) {
  ctx_->translator().setLocale(i18n::Locale::ZhCN);
  EXPECT_EQ(ctx_->translator().currentLocale(), i18n::Locale::ZhCN);
}

// ============================================================================
// 诊断发射测试
// ============================================================================

TEST_F(DiagContextTest, EmitError) {
  Diagnostic diag(Level::Error, Message("test error"));
  ctx_->emit(diag);

  EXPECT_EQ(ctx_->errorCount(), 1);
  EXPECT_TRUE(ctx_->hasErrors());
  EXPECT_EQ(mockEmitter_->emittedCount(), 1);
}

TEST_F(DiagContextTest, EmitWarning) {
  Diagnostic diag(Level::Warning, Message("test warning"));
  ctx_->emit(diag);

  EXPECT_EQ(ctx_->warningCount(), 1);
  EXPECT_FALSE(ctx_->hasErrors());
  EXPECT_EQ(mockEmitter_->emittedCount(), 1);
}

TEST_F(DiagContextTest, EmitNote) {
  Diagnostic diag(Level::Note, Message("test note"));
  ctx_->emit(diag);

  EXPECT_EQ(ctx_->errorCount(), 0);
  EXPECT_EQ(ctx_->warningCount(), 0);
  EXPECT_EQ(mockEmitter_->emittedCount(), 1);
}

// ============================================================================
// 诊断去重测试
// ============================================================================

TEST_F(DiagContextTest, DeduplicateSameDiagnostics) {
  Diagnostic diag(Level::Error, Message("duplicate error"),
                  ErrorCode(ErrorCategory::Lexer, 100));
  diag.spans.addPrimary(Span::create(1, 0, 10));

  // 发射相同诊断两次
  ctx_->emit(diag);
  ctx_->emit(diag);

  // 应该只发射一次（去重）
  EXPECT_EQ(mockEmitter_->emittedCount(), 1);
  EXPECT_EQ(ctx_->errorCount(), 1);
}

TEST_F(DiagContextTest, DifferentMessagesNotDeduplicated) {
  Diagnostic diag1(Level::Error, Message("error 1"),
                   ErrorCode(ErrorCategory::Lexer, 100));
  diag1.spans.addPrimary(Span::create(1, 0, 10));

  Diagnostic diag2(Level::Error, Message("error 2"),
                   ErrorCode(ErrorCategory::Lexer, 100));
  diag2.spans.addPrimary(Span::create(1, 0, 10));

  ctx_->emit(diag1);
  ctx_->emit(diag2);

  // 不同消息应该都发射
  EXPECT_EQ(mockEmitter_->emittedCount(), 2);
  EXPECT_EQ(ctx_->errorCount(), 2);
}

TEST_F(DiagContextTest, DifferentCodesNotDeduplicated) {
  Diagnostic diag1(Level::Error, Message("same error"),
                   ErrorCode(ErrorCategory::Lexer, 100));
  diag1.spans.addPrimary(Span::create(1, 0, 10));

  Diagnostic diag2(Level::Error, Message("same error"),
                   ErrorCode(ErrorCategory::Lexer, 200));
  diag2.spans.addPrimary(Span::create(1, 0, 10));

  ctx_->emit(diag1);
  ctx_->emit(diag2);

  // 不同错误码应该都发射
  EXPECT_EQ(mockEmitter_->emittedCount(), 2);
}

TEST_F(DiagContextTest, DifferentSpansNotDeduplicated) {
  Diagnostic diag1(Level::Error, Message("same error"),
                   ErrorCode(ErrorCategory::Lexer, 100));
  diag1.spans.addPrimary(Span::create(1, 0, 10));

  Diagnostic diag2(Level::Error, Message("same error"),
                   ErrorCode(ErrorCategory::Lexer, 100));
  diag2.spans.addPrimary(Span::create(1, 20, 30));

  ctx_->emit(diag1);
  ctx_->emit(diag2);

  // 不同位置应该都发射
  EXPECT_EQ(mockEmitter_->emittedCount(), 2);
}

TEST_F(DiagContextTest, DeduplicationDisabled) {
  // 创建禁用去重的上下文
  auto emitter = new MockEmitter();
  DiagConfig config;
  config.deduplicate = false;

  DiagContext ctx(std::unique_ptr<Emitter>(emitter), nullptr, config);

  Diagnostic diag(Level::Error, Message("duplicate error"),
                  ErrorCode(ErrorCategory::Lexer, 100));
  diag.spans.addPrimary(Span::create(1, 0, 10));

  ctx.emit(diag);
  ctx.emit(diag);

  // 禁用去重后应该发射两次
  EXPECT_EQ(emitter->emittedCount(), 2);
}

// ============================================================================
// 配置测试
// ============================================================================

TEST_F(DiagContextTest, ConfigAccess) {
  EXPECT_TRUE(ctx_->config().deduplicate);
  EXPECT_EQ(ctx_->config().maxErrors, 0);
  EXPECT_FALSE(ctx_->config().treatWarningsAsErrors);
}

TEST_F(DiagContextTest, ConfigMutable) {
  ctx_->config().treatWarningsAsErrors = true;
  EXPECT_TRUE(ctx_->config().treatWarningsAsErrors);
}

TEST_F(DiagContextTest, TreatWarningsAsErrors) {
  ctx_->config().treatWarningsAsErrors = true;

  Diagnostic diag(Level::Warning, Message("test warning"));
  ctx_->emit(diag);

  // -Werror 模式下警告应该计入错误
  EXPECT_EQ(ctx_->errorCount(), 1);
  EXPECT_TRUE(ctx_->hasErrors());
}

// ============================================================================
// 统计测试
// ============================================================================

TEST_F(DiagContextTest, Stats) {
  ctx_->emit(Diagnostic(Level::Error, Message("error 1")));
  ctx_->emit(Diagnostic(Level::Error, Message("error 2")));
  ctx_->emit(Diagnostic(Level::Warning, Message("warning 1")));

  auto stats = ctx_->stats();
  EXPECT_EQ(stats.errorCount, 2);
  EXPECT_EQ(stats.warningCount, 1);
}

// ============================================================================
// Flush 和 Summary 测试
// ============================================================================

TEST_F(DiagContextTest, Flush) {
  ctx_->flush();
  EXPECT_TRUE(mockEmitter_->flushed());
}

TEST_F(DiagContextTest, EmitSummary) {
  ctx_->emitSummary();
  EXPECT_TRUE(mockEmitter_->summaryEmitted());
}

// ============================================================================
// 移动语义测试
// ============================================================================

TEST_F(DiagContextTest, MoveConstruct) {
  ctx_->emit(Diagnostic(Level::Error, Message("error")));
  EXPECT_EQ(ctx_->errorCount(), 1);

  DiagContext moved(std::move(*ctx_));
  EXPECT_EQ(moved.errorCount(), 1);
}

TEST_F(DiagContextTest, MoveAssign) {
  ctx_->emit(Diagnostic(Level::Error, Message("error")));

  auto emitter = std::make_unique<MockEmitter>();
  DiagContext other(std::move(emitter));

  other = std::move(*ctx_);
  EXPECT_EQ(other.errorCount(), 1);
}

} // namespace
} // namespace czc::diag
