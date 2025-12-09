/**
 * @file message.hpp
 * @brief Markdown 消息类型定义。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-12-04
 *
 * @details
 *   集成 cmark 实现 Markdown 解析，支持延迟渲染。
 *   借鉴 rustc DiagMessage::FluentIdentifier 的延迟翻译设计。
 */

#ifndef CZC_DIAG_MESSAGE_HPP
#define CZC_DIAG_MESSAGE_HPP

#include "czc/common/config.hpp"

#include <format>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <variant>

namespace czc::diag {

// 前向声明
struct AnsiStyle;

namespace i18n {
class Translator;
} // namespace i18n

/// Markdown 消息 - 持有格式化文本
/// 延迟解析：仅在需要渲染时才调用 cmark
class Message {
public:
  /// 默认构造
  Message() = default;

  /// 从 Markdown 文本构造
  explicit Message(std::string markdown);

  /// 从 string_view 构造
  explicit Message(std::string_view markdown);

  /// 从 C 字符串构造
  explicit Message(const char *markdown);

  /// 析构函数
  ~Message();

  // 可拷贝
  Message(const Message &other);
  auto operator=(const Message &other) -> Message &;

  // 可移动
  Message(Message &&other) noexcept;
  auto operator=(Message &&other) noexcept -> Message &;

  /// 格式化构造（使用 std::format）
  template <typename... Args>
  [[nodiscard]] static auto format(std::format_string<Args...> fmt,
                                   Args &&...args) -> Message {
    return Message(std::format(fmt, std::forward<Args>(args)...));
  }

  /// 获取原始 Markdown
  [[nodiscard]] auto markdown() const noexcept -> std::string_view;

  /// 渲染为纯文本（移除 Markdown 格式）
  [[nodiscard]] auto renderPlainText() const -> std::string;

  /// 渲染为 HTML
  [[nodiscard]] auto renderHtml() const -> std::string;

  /// 渲染为 ANSI 终端格式
  [[nodiscard]] auto renderAnsi(const AnsiStyle &style) const -> std::string;

  /// 检查是否为空
  [[nodiscard]] auto isEmpty() const noexcept -> bool;

private:
  std::string markdown_;
  mutable std::optional<std::string> cachedPlain_; ///< 延迟计算缓存
};

/// 消息轻量引用 - 避免不必要的拷贝
/// 可从 Message、string_view 或 i18n 键构造
class MessageRef {
public:
  /// 默认构造
  MessageRef() = default;

  /// 从 Message 引用构造
  MessageRef(const Message &msg);

  /// 从字符串字面量构造
  MessageRef(std::string_view literal);

  /// 从 C 字符串构造
  MessageRef(const char *literal);

  /// 解析为字符串（可选使用翻译器）
  [[nodiscard]] auto resolve(const i18n::Translator *translator = nullptr) const
      -> std::string;

  /// 检查是否为空
  [[nodiscard]] auto isEmpty() const noexcept -> bool;

private:
  std::variant<const Message *, std::string_view> ref_{std::string_view{}};
};

} // namespace czc::diag

#endif // CZC_DIAG_MESSAGE_HPP
