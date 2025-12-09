/**
 * @file i18n.hpp
 * @brief 国际化支持。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-12-04
 *
 * @details
 *   借鉴 rustc Fluent 翻译系统设计，使用 TOML 格式存储翻译。
 *   利用已有的 tomlplusplus 库。
 */

#ifndef CZC_DIAG_I18N_HPP
#define CZC_DIAG_I18N_HPP

#include "czc/common/config.hpp"
#include "czc/diag/error_code.hpp"
#include "czc/diag/message.hpp"

#include <filesystem>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>

namespace czc::diag::i18n {

/// 区域设置
enum class Locale : uint8_t {
  En,   ///< English (default)
  ZhCN, ///< 简体中文
  ZhTW, ///< 繁體中文
  Ja,   ///< 日本語
};

/// 获取区域设置的字符串表示
[[nodiscard]] auto localeToString(Locale locale) -> std::string_view;

/// 从字符串解析区域设置
[[nodiscard]] auto parseLocale(std::string_view str) -> Locale;

/// 翻译器
/// 借鉴 rustc Translator 设计，支持回退机制
/// 通过依赖注入方式使用，由 DiagContext 持有实例
class Translator {
public:
  /// 默认构造函数
  Translator();

  /// 设置当前语言
  void setLocale(Locale locale);

  /// 获取当前语言
  [[nodiscard]] auto currentLocale() const noexcept -> Locale;

  /// 加载翻译文件
  [[nodiscard]] auto loadFromFile(const std::filesystem::path &path) -> bool;

  /// 从内存加载翻译（TOML 格式）
  void loadFromMemory(std::string_view toml);

  /// 获取翻译（带回退到英文）
  [[nodiscard]] auto get(std::string_view key) const -> std::string_view;

  /// 获取翻译并格式化
  template <typename... Args>
  [[nodiscard]] auto get(std::string_view key, Args &&...args) const
      -> std::string {
    auto tmpl = get(key);
    if (tmpl.empty()) {
      return std::string(key);
    }
    return formatWithArgs(tmpl, std::forward<Args>(args)...);
  }

  /// 获取翻译或使用默认值
  [[nodiscard]] auto getOr(std::string_view key,
                           std::string_view fallback) const -> std::string_view;

  /// 获取错误的简短描述
  [[nodiscard]] auto getErrorBrief(ErrorCode code) const -> std::string_view;

  /// 获取错误的详细解释
  [[nodiscard]] auto getErrorExplanation(ErrorCode code) const -> Message;

  // 可拷贝（用于依赖注入场景的复制配置）
  Translator(const Translator &);
  auto operator=(const Translator &) -> Translator &;

  // 可移动
  Translator(Translator &&) noexcept;
  auto operator=(Translator &&) noexcept -> Translator &;

  ~Translator() = default;

private:
  /// 格式化辅助函数
  template <typename... Args>
  auto formatWithArgs(std::string_view tmpl, Args &&...args) const
      -> std::string {
    // 简单的占位符替换 {0}, {1}, ...
    return formatPlaceholders(tmpl, std::initializer_list<std::string>{
                                        toString(std::forward<Args>(args))...});
  }

  /// 转换参数为字符串
  template <typename T> static auto toString(T &&value) -> std::string {
    if constexpr (std::is_same_v<std::decay_t<T>, std::string>) {
      return std::forward<T>(value);
    } else if constexpr (std::is_same_v<std::decay_t<T>, std::string_view>) {
      return std::string(value);
    } else if constexpr (std::is_same_v<std::decay_t<T>, const char *>) {
      return std::string(value);
    } else if constexpr (std::is_arithmetic_v<std::decay_t<T>>) {
      return std::to_string(value);
    } else {
      return "<unknown>";
    }
  }

  /// 替换占位符
  [[nodiscard]] auto
  formatPlaceholders(std::string_view tmpl,
                     std::initializer_list<std::string> args) const
      -> std::string;

  mutable std::mutex mutex_;
  Locale locale_{Locale::En};
  std::unordered_map<std::string, std::string> translations_;
  std::unordered_map<std::string, std::string> fallback_; ///< 英文回退
};

/// RAII 临时语言切换
/// @note 需要传入 Translator 引用，不再依赖全局状态
class [[nodiscard]] TranslationScope {
public:
  /// 构造临时语言切换
  /// @param translator 翻译器引用
  /// @param tempLocale 临时语言
  TranslationScope(Translator &translator, Locale tempLocale);
  ~TranslationScope();

  TranslationScope(const TranslationScope &) = delete;
  auto operator=(const TranslationScope &) -> TranslationScope & = delete;
  TranslationScope(TranslationScope &&) = delete;
  auto operator=(TranslationScope &&) -> TranslationScope & = delete;

private:
  Translator &translator_;
  Locale previousLocale_;
};

} // namespace czc::diag::i18n

#endif // CZC_DIAG_I18N_HPP
