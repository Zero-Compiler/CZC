/**
 * @file comment_scanner.hpp
 * @brief 注释扫描器。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-29
 *
 * @details
 *   CommentScanner 负责扫描各种注释：
 *   - 行注释: \/\/ ...(这里多了两个反斜杠,防止被解析)
 *   - 块注释: /\* ... *\/(这里多了两个反斜杠,防止被解析)
 *   - 文档注释: /\** ... *\/(这里多了两个反斜杠,防止被解析)
 *
 *   注意：块注释不支持嵌套。
 */

#ifndef CZC_LEXER_COMMENT_SCANNER_HPP
#define CZC_LEXER_COMMENT_SCANNER_HPP

#include "czc/common/config.hpp"

#include "czc/lexer/scanner.hpp"

namespace czc::lexer {

/**
 * @brief 注释扫描器。
 *
 * @details
 *   扫描各种注释类型。
 *   在 Trivia 模式下，注释作为 Trivia 附加到 Token。
 */
class CommentScanner {
public:
  CommentScanner() = default;

  /**
   * @brief 检查当前字符是否可由此扫描器处理。
   *
   * @param ctx 扫描上下文
   * @return 若当前字符为 / 且下一个为 / 或 * 返回 true
   */
  [[nodiscard]] bool canScan(const ScanContext &ctx) const noexcept;

  /**
   * @brief 执行扫描。
   *
   * @param ctx 扫描上下文
   * @return 扫描得到的 Token（COMMENT_LINE, COMMENT_BLOCK, COMMENT_DOC）
   */
  [[nodiscard]] Token scan(ScanContext &ctx) const;

  /**
   * @brief 扫描注释作为 Trivia。
   *
   * @details
   *   在 Trivia 模式下使用，返回 Trivia 而非 Token。
   *
   * @param ctx 扫描上下文
   * @return 扫描得到的 Trivia
   */
  [[nodiscard]] Trivia scanAsTrivia(ScanContext &ctx) const;

private:
  /**
   * @brief 扫描行注释。
   *
   * @param ctx 扫描上下文
   * @param startOffset 起始偏移
   * @param startLoc 起始位置
   * @return 扫描得到的 Token
   */
  [[nodiscard]] Token scanLineComment(ScanContext &ctx, std::size_t startOffset,
                                      SourceLocation startLoc) const;

  /**
   * @brief 扫描块注释。
   *
   * @param ctx 扫描上下文
   * @param startOffset 起始偏移
   * @param startLoc 起始位置
   * @return 扫描得到的 Token
   */
  [[nodiscard]] Token scanBlockComment(ScanContext &ctx,
                                       std::size_t startOffset,
                                       SourceLocation startLoc) const;

  /**
   * @brief 检查是否为文档注释。
   *
   * @param ctx 扫描上下文
   * @return 若为 /\** 开头返回 true (这里多了一个反斜杠,防止被解析)
   */
  [[nodiscard]] bool isDocComment(const ScanContext &ctx) const noexcept;
};

} // namespace czc::lexer

#endif // CZC_LEXER_COMMENT_SCANNER_HPP
