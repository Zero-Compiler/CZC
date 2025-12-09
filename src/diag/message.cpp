/**
 * @file message.cpp
 * @brief Markdown 消息实现。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-12-04
 *
 * @details
 *   集成 cmark 实现 Markdown 解析和渲染。
 */

#include "czc/diag/message.hpp"
#include "czc/diag/emitters/ansi_renderer.hpp"
#include "czc/diag/i18n.hpp"

#include <cmark.h>

namespace czc::diag {

Message::Message(std::string markdown) : markdown_(std::move(markdown)) {}

Message::Message(std::string_view markdown) : markdown_(markdown) {}

Message::Message(const char *markdown) : markdown_(markdown ? markdown : "") {}

Message::~Message() = default;

Message::Message(const Message &other)
    : markdown_(other.markdown_), cachedPlain_(other.cachedPlain_) {}

auto Message::operator=(const Message &other) -> Message & {
  if (this != &other) {
    markdown_ = other.markdown_;
    cachedPlain_ = other.cachedPlain_;
  }
  return *this;
}

Message::Message(Message &&other) noexcept
    : markdown_(std::move(other.markdown_)),
      cachedPlain_(std::move(other.cachedPlain_)) {}

auto Message::operator=(Message &&other) noexcept -> Message & {
  if (this != &other) {
    markdown_ = std::move(other.markdown_);
    cachedPlain_ = std::move(other.cachedPlain_);
  }
  return *this;
}

auto Message::markdown() const noexcept -> std::string_view {
  return markdown_;
}

namespace {
/// 手动遍历 cmark 节点树提取纯文本
void extractPlainText(cmark_node *node, std::string &out) {
  if (node == nullptr) {
    return;
  }

  cmark_node_type nodeType = cmark_node_get_type(node);

  // 处理文本节点
  if (nodeType == CMARK_NODE_TEXT || nodeType == CMARK_NODE_CODE) {
    const char *literal = cmark_node_get_literal(node);
    if (literal != nullptr) {
      out += literal;
    }
  } else if (nodeType == CMARK_NODE_SOFTBREAK ||
             nodeType == CMARK_NODE_LINEBREAK) {
    out += '\n';
  } else if (nodeType == CMARK_NODE_PARAGRAPH && !out.empty() &&
             out.back() != '\n') {
    out += '\n';
  }

  // 递归处理子节点
  cmark_node *child = cmark_node_first_child(node);
  while (child != nullptr) {
    extractPlainText(child, out);
    child = cmark_node_next(child);
  }

  // 段落后添加换行
  if (nodeType == CMARK_NODE_PARAGRAPH) {
    out += '\n';
  }
}
} // namespace

auto Message::renderPlainText() const -> std::string {
  if (cachedPlain_) {
    return *cachedPlain_;
  }

  // 使用 cmark 解析
  cmark_node *doc = cmark_parse_document(markdown_.data(), markdown_.size(),
                                         CMARK_OPT_DEFAULT);

  if (doc == nullptr) {
    cachedPlain_ = markdown_;
    return *cachedPlain_;
  }

  std::string result;
  extractPlainText(doc, result);
  cmark_node_free(doc);

  // 移除末尾换行
  while (!result.empty() && result.back() == '\n') {
    result.pop_back();
  }

  cachedPlain_ = std::move(result);
  return *cachedPlain_;
}

auto Message::renderHtml() const -> std::string {
  cmark_node *doc = cmark_parse_document(markdown_.data(), markdown_.size(),
                                         CMARK_OPT_DEFAULT);

  if (doc == nullptr) {
    return markdown_;
  }

  char *rendered = cmark_render_html(doc, CMARK_OPT_DEFAULT);
  cmark_node_free(doc);

  if (rendered != nullptr) {
    std::string result(rendered);
    free(rendered);
    return result;
  }

  return markdown_;
}

auto Message::renderAnsi(const AnsiStyle &style) const -> std::string {
  AnsiRenderer renderer(style);
  return renderer.renderMessage(markdown_);
}

auto Message::isEmpty() const noexcept -> bool { return markdown_.empty(); }

// ============================================================================
// MessageRef 实现
// ============================================================================

MessageRef::MessageRef(const Message &msg) : ref_(&msg) {}

MessageRef::MessageRef(std::string_view literal) : ref_(literal) {}

MessageRef::MessageRef(const char *literal)
    : ref_(literal ? std::string_view(literal) : std::string_view{}) {}

auto MessageRef::resolve(const i18n::Translator * /*translator*/) const
    -> std::string {
  if (std::holds_alternative<const Message *>(ref_)) {
    auto *msg = std::get<const Message *>(ref_);
    return msg ? msg->renderPlainText() : "";
  }
  return std::string(std::get<std::string_view>(ref_));
}

auto MessageRef::isEmpty() const noexcept -> bool {
  if (std::holds_alternative<const Message *>(ref_)) {
    auto *msg = std::get<const Message *>(ref_);
    return msg == nullptr || msg->isEmpty();
  }
  return std::get<std::string_view>(ref_).empty();
}

} // namespace czc::diag
