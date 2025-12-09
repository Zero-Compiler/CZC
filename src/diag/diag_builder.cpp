/**
 * @file diag_builder.cpp
 * @brief 诊断构建器实现。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-12-04
 */

#include "czc/diag/diag_builder.hpp"
#include "czc/diag/diag_context.hpp"

namespace czc::diag {

DiagBuilder::DiagBuilder(Level level, Message message)
    : diag_(level, std::move(message)) {}

DiagBuilder::DiagBuilder(Level level, Message message, ErrorCode code)
    : diag_(level, std::move(message), code) {}

auto DiagBuilder::code(ErrorCode c) -> DiagBuilder & {
  diag_.code = c;
  return *this;
}

auto DiagBuilder::span(Span s) -> DiagBuilder & {
  diag_.spans.addPrimary(s, "");
  return *this;
}

auto DiagBuilder::spanLabel(Span s, std::string_view label) -> DiagBuilder & {
  diag_.spans.addPrimary(s, label);
  return *this;
}

auto DiagBuilder::secondarySpan(Span s, std::string_view label)
    -> DiagBuilder & {
  diag_.spans.addSecondary(s, label);
  return *this;
}

auto DiagBuilder::note(std::string_view message) -> DiagBuilder & {
  diag_.children.emplace_back(Level::Note, std::string(message));
  return *this;
}

auto DiagBuilder::note(Span s, std::string_view message) -> DiagBuilder & {
  diag_.children.emplace_back(Level::Note, std::string(message), s);
  return *this;
}

auto DiagBuilder::help(std::string_view message) -> DiagBuilder & {
  diag_.children.emplace_back(Level::Help, std::string(message));
  return *this;
}

auto DiagBuilder::help(Span s, std::string_view message) -> DiagBuilder & {
  diag_.children.emplace_back(Level::Help, std::string(message), s);
  return *this;
}

auto DiagBuilder::suggestion(Span s, std::string replacement,
                             std::string_view message,
                             Applicability applicability) -> DiagBuilder & {
  diag_.suggestions.emplace_back(s, std::move(replacement),
                                 std::string(message), applicability);
  return *this;
}

auto DiagBuilder::build() && -> Diagnostic { return std::move(diag_); }

void DiagBuilder::emit(DiagContext &dcx) && { dcx.emit(std::move(diag_)); }

auto DiagBuilder::emitError(DiagContext &dcx) && -> ErrorGuaranteed {
  return dcx.emitError(std::move(diag_));
}

} // namespace czc::diag
