/**
 * @file i18n.cpp
 * @brief 国际化支持实现。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-12-04
 */

#include "czc/diag/i18n.hpp"

#include <toml++/toml.hpp>

#include <fstream>
#include <sstream>

namespace czc::diag::i18n {

auto localeToString(Locale locale) -> std::string_view {
  switch (locale) {
  case Locale::En:
    return "en";
  case Locale::ZhCN:
    return "zh-CN";
  case Locale::ZhTW:
    return "zh-TW";
  case Locale::Ja:
    return "ja";
  default:
    return "en";
  }
}

auto parseLocale(std::string_view str) -> Locale {
  if (str == "en" || str.starts_with("en_") || str.starts_with("en-")) {
    return Locale::En;
  }
  if (str == "zh-CN" || str == "zh_CN" || str.starts_with("zh_CN") ||
      str.starts_with("zh-Hans")) {
    return Locale::ZhCN;
  }
  if (str == "zh-TW" || str == "zh_TW" || str.starts_with("zh_TW") ||
      str.starts_with("zh-Hant")) {
    return Locale::ZhTW;
  }
  if (str == "ja" || str.starts_with("ja_") || str.starts_with("ja-")) {
    return Locale::Ja;
  }
  return Locale::En;
}

Translator::Translator() = default;

// 拷贝构造函数
Translator::Translator(const Translator &other) {
  std::lock_guard lock(other.mutex_);
  locale_ = other.locale_;
  translations_ = other.translations_;
  fallback_ = other.fallback_;
}

// 拷贝赋值运算符
auto Translator::operator=(const Translator &other) -> Translator & {
  if (this != &other) {
    std::scoped_lock lock(mutex_, other.mutex_);
    locale_ = other.locale_;
    translations_ = other.translations_;
    fallback_ = other.fallback_;
  }
  return *this;
}

// 移动构造函数
Translator::Translator(Translator &&other) noexcept {
  std::lock_guard lock(other.mutex_);
  locale_ = other.locale_;
  translations_ = std::move(other.translations_);
  fallback_ = std::move(other.fallback_);
}

// 移动赋值运算符
auto Translator::operator=(Translator &&other) noexcept -> Translator & {
  if (this != &other) {
    std::scoped_lock lock(mutex_, other.mutex_);
    locale_ = other.locale_;
    translations_ = std::move(other.translations_);
    fallback_ = std::move(other.fallback_);
  }
  return *this;
}

void Translator::setLocale(Locale locale) {
  std::lock_guard lock(mutex_);
  locale_ = locale;
}

auto Translator::currentLocale() const noexcept -> Locale { return locale_; }

auto Translator::loadFromFile(const std::filesystem::path &path) -> bool {
  std::ifstream file(path);
  if (!file) {
    return false;
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  loadFromMemory(buffer.str());
  return true;
}

void Translator::loadFromMemory(std::string_view toml) {
  std::lock_guard lock(mutex_);

  try {
    auto result = toml::parse(toml);

    // 递归遍历 TOML 表，将键值对添加到翻译表
    std::function<void(const toml::table &, const std::string &)> parseTable;
    parseTable = [&](const toml::table &table, const std::string &prefix) {
      for (const auto &[key, value] : table) {
        std::string fullKey = prefix.empty()
                                  ? std::string(key.str())
                                  : prefix + "." + std::string(key.str());

        if (value.is_string()) {
          translations_[fullKey] = std::string(value.as_string()->get());
        } else if (value.is_table()) {
          parseTable(*value.as_table(), fullKey);
        }
      }
    };

    parseTable(result, "");
  } catch (const toml::parse_error &) {
    // 解析失败，忽略
  }
}

auto Translator::get(std::string_view key) const -> std::string_view {
  std::lock_guard lock(mutex_);

  // 先查找当前语言
  auto it = translations_.find(std::string(key));
  if (it != translations_.end()) {
    return it->second;
  }

  // 回退到英文
  it = fallback_.find(std::string(key));
  if (it != fallback_.end()) {
    return it->second;
  }

  return {};
}

auto Translator::getOr(std::string_view key, std::string_view fallback) const
    -> std::string_view {
  auto result = get(key);
  return result.empty() ? fallback : result;
}

auto Translator::getErrorBrief(ErrorCode code) const -> std::string_view {
  auto entry = ErrorRegistry::instance().lookup(code);
  if (entry) {
    return entry->brief;
  }
  return {};
}

auto Translator::getErrorExplanation(ErrorCode code) const -> Message {
  auto entry = ErrorRegistry::instance().lookup(code);
  if (entry && !entry->explanationKey.empty()) {
    auto explanation = get(entry->explanationKey);
    if (!explanation.empty()) {
      return Message(explanation);
    }
  }
  return Message("");
}

auto Translator::formatPlaceholders(
    std::string_view tmpl, std::initializer_list<std::string> args) const
    -> std::string {
  std::string result(tmpl);
  size_t index = 0;

  for (const auto &arg : args) {
    std::string placeholder = "{" + std::to_string(index) + "}";
    size_t pos = 0;
    while ((pos = result.find(placeholder, pos)) != std::string::npos) {
      result.replace(pos, placeholder.length(), arg);
      pos += arg.length();
    }
    ++index;
  }

  return result;
}

// ============================================================================
// TranslationScope 实现
// ============================================================================

TranslationScope::TranslationScope(Translator &translator, Locale tempLocale)
    : translator_(translator), previousLocale_(translator.currentLocale()) {
  translator_.setLocale(tempLocale);
}

TranslationScope::~TranslationScope() {
  translator_.setLocale(previousLocale_);
}

} // namespace czc::diag::i18n
