/**
 * @file error_collector.cpp
 * @brief `CSTErrorCollector` 类的功能实现。
 * @author BegoniaHe
 * @date 2025-11-05
 */

#include "czc/cst/error_collector.hpp"

namespace czc {
namespace cst {

void CSTErrorCollector::add(diagnostics::DiagnosticCode code,
                            const utils::SourceLocation &loc,
                            const std::vector<std::string> &args) {
  // NOTE: 使用 emplace_back 直接在 vector 的末尾就地构造 CSTError 对象。
  //       这比 push_back 一个临时对象更高效，因为它避免了额外的拷贝或移动操作。
  errors.emplace_back(code, loc, args);
}

} // namespace cst
} // namespace czc
