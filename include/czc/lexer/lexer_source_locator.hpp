/**
 * @file lexer_source_locator.hpp
 * @brief Lexer 源码定位器适配器。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-12-04
 *
 * @details
 *   将 SourceManager 适配为 diag::SourceLocator 接口。
 *   提供 LexerError 到 Diagnostic 的转换函数。
 */

#ifndef CZC_LEXER_LEXER_SOURCE_LOCATOR_HPP
#define CZC_LEXER_LEXER_SOURCE_LOCATOR_HPP

#include "czc/diag/diag_context.hpp"
#include "czc/diag/diagnostic.hpp"
#include "czc/diag/i18n.hpp"
#include "czc/diag/source_locator.hpp"
#include "czc/lexer/lexer_error.hpp"
#include "czc/lexer/source_manager.hpp"

#include <span>

namespace czc::lexer {

/// Lexer 源码定位器适配器
/// 将 SourceManager 适配为 diag::SourceLocator 接口
class LexerSourceLocator final : public diag::SourceLocator {
public:
  /// 构造适配器
  explicit LexerSourceLocator(const SourceManager &sm);

  /// 析构函数
  ~LexerSourceLocator() override = default;

  // 禁止拷贝，允许移动
  LexerSourceLocator(const LexerSourceLocator &) = delete;
  auto operator=(const LexerSourceLocator &) -> LexerSourceLocator & = delete;
  LexerSourceLocator(LexerSourceLocator &&) noexcept = default;
  auto operator=(LexerSourceLocator &&) noexcept
      -> LexerSourceLocator & = default;

  /// 获取文件名
  [[nodiscard]] auto getFilename(diag::Span span) const
      -> std::string_view override;

  /// 偏移量转行列
  [[nodiscard]] auto getLineColumn(uint32_t fileId, uint32_t offset) const
      -> diag::LineColumn override;

  /// 获取某行内容
  [[nodiscard]] auto getLineContent(uint32_t fileId, uint32_t line) const
      -> std::string_view override;

  /// 获取源码片段
  [[nodiscard]] auto getSourceSlice(diag::Span span) const
      -> std::string_view override;

private:
  const SourceManager *sm_;
};

// ============================================================================
// ADL 可发现的桥接函数
// ============================================================================

/// 将 LexerError 转换为 Diagnostic
/// @param err 词法错误
/// @param sm 源码管理器
/// @param translator 翻译器（用于 i18n 标签和帮助信息）
[[nodiscard]] auto toDiagnostic(const LexerError &err, const SourceManager &sm,
                                const diag::i18n::Translator &translator)
    -> diag::Diagnostic;

/// 从 LexerError 提取 Span
[[nodiscard]] auto toSpan(const LexerError &err) -> diag::Span;

/// 批量发射 Lexer 错误
void emitLexerErrors(diag::DiagContext &dcx, std::span<const LexerError> errors,
                     const SourceManager &sm, BufferID bufferId);

} // namespace czc::lexer

#endif // CZC_LEXER_LEXER_SOURCE_LOCATOR_HPP
