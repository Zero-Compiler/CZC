/**
 * @file diagnostic.cpp
 * @brief 诊断类型实现。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-12-04
 */

#include "czc/diag/diagnostic.hpp"

namespace czc::diag {

auto levelToString(Level level) -> std::string_view {
  switch (level) {
  case Level::Note:
    return "note";
  case Level::Help:
    return "help";
  case Level::Warning:
    return "warning";
  case Level::Error:
    return "error";
  case Level::Fatal:
    return "fatal error";
  case Level::Bug:
    return "internal compiler error";
  default:
    return "unknown";
  }
}

} // namespace czc::diag
