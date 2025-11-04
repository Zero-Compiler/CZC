/**
 * @file error_collector.cpp
 * @brief 词法分析错误收集器实现
 * @author BegoniaHe
 * @date 2025-11-04
 */

#include "czc/lexer/error_collector.hpp"

namespace czc {
namespace lexer {

void LexErrorCollector::add(diagnostics::DiagnosticCode code,
                            const utils::SourceLocation &loc,
                            const std::vector<std::string> &args) {
  // 使用 emplace_back 直接在 vector 的末尾构造 LexerError 对象，
  // 避免了创建临时对象再进行拷贝或移动，从而提高效率。
  errors.emplace_back(code, loc, args);
}

} // namespace lexer
} // namespace czc
