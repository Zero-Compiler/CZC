/**
 * @file string_scanner.hpp
 * @brief 字符串字面量扫描器。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-29
 *
 * @details
 *   StringScanner 负责扫描各种字符串字面量：
 *   - 普通字符串: "hello\nworld"
 *   - 原始字符串: r"raw", r#"contains "quote""#
 *   - TeX 字符串: t"latex content"
 *
 *   支持的转义序列（仅普通字符串）：
 *   - \\ -> \
 *   - \" -> "
 *   - \\n -> 换行(这里多了一个反斜杠,防止被解析)
 *   - \r -> 回车
 *   - \t -> 制表符
 *   - \0 -> 空字符
 *   - \xHH -> 十六进制字节
 *   - \u{HHHH} 或 \u{HHHHHH} -> Unicode 码点
 */

#ifndef CZC_LEXER_STRING_SCANNER_HPP
#define CZC_LEXER_STRING_SCANNER_HPP

#include "czc/common/config.hpp"

#include "czc/lexer/scanner.hpp"

#include <string>

namespace czc::lexer {

/**
 * @brief 字符串扫描器。
 *
 * @details
 *   扫描各种字符串字面量，处理转义序列。
 */
class StringScanner {
public:
  StringScanner() = default;

  /**
   * @brief 检查当前字符是否可由此扫描器处理。
   *
   * @param ctx 扫描上下文
   * @return 若当前字符为 " 或 r" 或 t" 返回 true
   */
  [[nodiscard]] bool canScan(const ScanContext &ctx) const noexcept;

  /**
   * @brief 执行扫描。
   *
   * @param ctx 扫描上下文
   * @return 扫描得到的 Token（LIT_STRING, LIT_RAW_STRING, LIT_TEX_STRING）
   */
  [[nodiscard]] Token scan(ScanContext &ctx) const;

private:
  /**
   * @brief 扫描普通字符串。
   *
   * @param ctx 扫描上下文
   * @param startOffset 起始偏移
   * @param startLoc 起始位置
   * @return 扫描得到的 Token
   */
  [[nodiscard]] Token scanNormalString(ScanContext &ctx,
                                       std::size_t startOffset,
                                       SourceLocation startLoc) const;

  /**
   * @brief 扫描原始字符串。
   *
   * @param ctx 扫描上下文
   * @param startOffset 起始偏移
   * @param startLoc 起始位置
   * @return 扫描得到的 Token
   */
  [[nodiscard]] Token scanRawString(ScanContext &ctx, std::size_t startOffset,
                                    SourceLocation startLoc) const;

  /**
   * @brief 扫描 TeX 字符串。
   *
   * @param ctx 扫描上下文
   * @param startOffset 起始偏移
   * @param startLoc 起始位置
   * @return 扫描得到的 Token
   */
  [[nodiscard]] Token scanTexString(ScanContext &ctx, std::size_t startOffset,
                                    SourceLocation startLoc) const;

  /**
   * @brief 解析转义序列。
   *
   * @param ctx 扫描上下文
   * @param[out] result 解析结果字符串
   * @param[out] flags 转义标记
   * @return 若成功解析返回 true
   */
  [[nodiscard]] bool parseEscapeSequence(ScanContext &ctx, std::string &result,
                                         EscapeFlags &flags) const;

  /**
   * @brief 解析十六进制转义。
   *
   * @param ctx 扫描上下文
   * @param[out] result 解析结果字符串
   * @return 若成功解析返回 true
   */
  [[nodiscard]] bool parseHexEscape(ScanContext &ctx,
                                    std::string &result) const;

  /**
   * @brief 解析 Unicode 转义。
   *
   * @param ctx 扫描上下文
   * @param[out] result 解析结果字符串
   * @return 若成功解析返回 true
   */
  [[nodiscard]] bool parseUnicodeEscape(ScanContext &ctx,
                                        std::string &result) const;

  /**
   * @brief 计算原始字符串的 # 数量。
   *
   * @param ctx 扫描上下文
   * @return # 的数量
   */
  [[nodiscard]] std::size_t countHashes(ScanContext &ctx) const;
};

} // namespace czc::lexer

#endif // CZC_LEXER_STRING_SCANNER_HPP
