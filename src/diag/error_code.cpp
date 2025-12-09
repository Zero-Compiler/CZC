/**
 * @file error_code.cpp
 * @brief 错误码系统实现。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-12-04
 */

#include "czc/diag/error_code.hpp"

#include <format>

namespace czc::diag {

auto ErrorCode::toString() const -> std::string {
  return std::format("{}{:04d}", getCategoryPrefix(category), code);
}

auto ErrorRegistry::instance() -> ErrorRegistry & {
  static ErrorRegistry registry;
  return registry;
}

void ErrorRegistry::registerError(ErrorCode code, std::string_view brief,
                                  std::string_view explanationKey) {
  std::unique_lock lock(mutex_);
  entries_[code] = ErrorEntry{code, brief, explanationKey};
}

auto ErrorRegistry::lookup(ErrorCode code) const -> std::optional<ErrorEntry> {
  std::shared_lock lock(mutex_);
  auto it = entries_.find(code);
  if (it != entries_.end()) {
    return it->second;
  }
  return std::nullopt;
}

auto ErrorRegistry::allCodes() const -> std::vector<ErrorCode> {
  std::shared_lock lock(mutex_);
  std::vector<ErrorCode> codes;
  codes.reserve(entries_.size());
  for (const auto &[code, _] : entries_) {
    codes.push_back(code);
  }
  return codes;
}

auto ErrorRegistry::isRegistered(ErrorCode code) const -> bool {
  std::shared_lock lock(mutex_);
  return entries_.contains(code);
}

} // namespace czc::diag
