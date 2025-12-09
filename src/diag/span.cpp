/**
 * @file span.cpp
 * @brief 源码位置抽象实现。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-12-04
 */

#include "czc/diag/span.hpp"

namespace czc::diag {

void MultiSpan::addPrimary(Span span, std::string_view label) {
  spans_.emplace_back(span, label, true);
}

void MultiSpan::addSecondary(Span span, std::string_view label) {
  spans_.emplace_back(span, label, false);
}

auto MultiSpan::primary() const -> std::optional<LabeledSpan> {
  for (const auto &ls : spans_) {
    if (ls.isPrimary) {
      return ls;
    }
  }
  return std::nullopt;
}

auto MultiSpan::secondaries() const -> std::vector<LabeledSpan> {
  std::vector<LabeledSpan> result;
  for (const auto &ls : spans_) {
    if (!ls.isPrimary) {
      result.push_back(ls);
    }
  }
  return result;
}

} // namespace czc::diag
