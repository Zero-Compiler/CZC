/**
 * @file i18n_test.cpp
 * @brief Translator (i18n) 单元测试。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-12-04
 */

#include "czc/diag/i18n.hpp"

#include <gtest/gtest.h>

namespace czc::diag::i18n {
namespace {

class TranslatorTest : public ::testing::Test {
protected:
  Translator translator_;
};

// ============================================================================
// 构造函数测试
// ============================================================================

TEST_F(TranslatorTest, DefaultConstructor) {
  Translator t;
  // 默认 locale 应该是 English
  EXPECT_EQ(t.currentLocale(), Locale::En);
}

TEST_F(TranslatorTest, CopyConstructor) {
  translator_.setLocale(Locale::ZhCN);
  Translator copy(translator_);
  EXPECT_EQ(copy.currentLocale(), Locale::ZhCN);
}

TEST_F(TranslatorTest, MoveConstructor) {
  translator_.setLocale(Locale::ZhCN);
  Translator moved(std::move(translator_));
  EXPECT_EQ(moved.currentLocale(), Locale::ZhCN);
}

TEST_F(TranslatorTest, CopyAssignment) {
  Translator t;
  t.setLocale(Locale::ZhCN);
  translator_ = t;
  EXPECT_EQ(translator_.currentLocale(), Locale::ZhCN);
}

TEST_F(TranslatorTest, MoveAssignment) {
  Translator t;
  t.setLocale(Locale::ZhCN);
  translator_ = std::move(t);
  EXPECT_EQ(translator_.currentLocale(), Locale::ZhCN);
}

// ============================================================================
// Locale 测试
// ============================================================================

TEST_F(TranslatorTest, SetLocale) {
  translator_.setLocale(Locale::ZhCN);
  EXPECT_EQ(translator_.currentLocale(), Locale::ZhCN);

  translator_.setLocale(Locale::En);
  EXPECT_EQ(translator_.currentLocale(), Locale::En);
}

TEST_F(TranslatorTest, ParseLocaleEnglish) {
  EXPECT_EQ(parseLocale("en"), Locale::En);
  EXPECT_EQ(parseLocale("en-US"), Locale::En);
  EXPECT_EQ(parseLocale("en_US"), Locale::En);
}

TEST_F(TranslatorTest, ParseLocaleChinese) {
  // 仅支持完整的 locale 格式
  EXPECT_EQ(parseLocale("zh-CN"), Locale::ZhCN);
  EXPECT_EQ(parseLocale("zh_CN"), Locale::ZhCN);
  EXPECT_EQ(parseLocale("zh-Hans"), Locale::ZhCN);
}

TEST_F(TranslatorTest, ParseLocaleUnknown) {
  // 未知 locale 应该回退到 English
  EXPECT_EQ(parseLocale("unknown"), Locale::En);
  EXPECT_EQ(parseLocale(""), Locale::En);
}

// ============================================================================
// 翻译测试
// ============================================================================

TEST_F(TranslatorTest, TranslateUnknownKey) {
  // 未注册的 key 应该返回空字符串
  auto result = translator_.get("unknown.key");
  EXPECT_TRUE(result.empty());
}

TEST_F(TranslatorTest, TranslateWithFallback) {
  // 翻译失败时使用 fallback
  auto result = translator_.getOr("unknown.key", "fallback message");
  EXPECT_EQ(result, "fallback message");
}

// ============================================================================
// TranslationScope 测试
// ============================================================================

TEST_F(TranslatorTest, TranslationScopeRestoresLocale) {
  translator_.setLocale(Locale::En);

  {
    TranslationScope scope(translator_, Locale::ZhCN);
    EXPECT_EQ(translator_.currentLocale(), Locale::ZhCN);
  }

  // scope 结束后应该恢复
  EXPECT_EQ(translator_.currentLocale(), Locale::En);
}

TEST_F(TranslatorTest, TranslationScopeNestedScopes) {
  translator_.setLocale(Locale::En);

  {
    TranslationScope outer(translator_, Locale::ZhCN);
    EXPECT_EQ(translator_.currentLocale(), Locale::ZhCN);

    {
      TranslationScope inner(translator_, Locale::En);
      EXPECT_EQ(translator_.currentLocale(), Locale::En);
    }

    // 内层 scope 结束，恢复到 Chinese
    EXPECT_EQ(translator_.currentLocale(), Locale::ZhCN);
  }

  // 外层 scope 结束，恢复到 English
  EXPECT_EQ(translator_.currentLocale(), Locale::En);
}

} // namespace
} // namespace czc::diag::i18n
