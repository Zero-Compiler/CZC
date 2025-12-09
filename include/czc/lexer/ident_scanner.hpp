/**
 * @file ident_scanner.hpp
 * @brief 标识符和关键字扫描器。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-29
 *
 * @details
 *   IdentScanner 负责扫描：
 *   - 标识符（以字母或下划线开头，支持 UTF-8 字符）
 *   - 关键字（通过哈希表查找）
 *   - 布尔字面量 (true, false)
 *   - null 字面量
 *
 *   标识符规则：[[:alpha:]_][[:alnum:]_]*
 *   其中 [:alpha:] 和 [:alnum:] 包含 Unicode 字母和数字。
 */

#ifndef CZC_LEXER_IDENT_SCANNER_HPP
#define CZC_LEXER_IDENT_SCANNER_HPP

#include "czc/common/config.hpp"

#include "czc/lexer/scanner.hpp"

namespace czc::lexer {

/**
 * @brief 标识符扫描器。
 *
 * @details
 *   扫描标识符和关键字，支持 UTF-8 编码的 Unicode 字符。
 *   使用哈希表进行 O(1) 关键字查找。
 */
class IdentScanner {
public:
  IdentScanner() = default;

  /**
   * @brief 检查当前字符是否可由此扫描器处理。
   *
   * @details
   *   标识符起始字符：
   *   - ASCII 字母 (a-z, A-Z)
   *   - 下划线 (_)
   *   - UTF-8 多字节字符（非 ASCII，首字节 >= 0x80）
   *
   * @param ctx 扫描上下文
   * @return 若当前字符为标识符起始字符返回 true
   */
  [[nodiscard]] bool canScan(const ScanContext &ctx) const noexcept;

  /**
   * @brief 执行扫描。
   *
   * @param ctx 扫描上下文
   * @return 扫描得到的 Token（IDENTIFIER 或关键字）
   */
  [[nodiscard]] Token scan(ScanContext &ctx) const;

private:
  /**
   * @brief 检查 ASCII 字符是否为标识符起始。
   *
   * @details
   *   ASCII 标识符起始：字母 (a-z, A-Z) 或下划线 (_)
   *
   * @param ch 待检查的字符
   * @return 若可作为标识符起始返回 true
   */
  [[nodiscard]] static bool isAsciiIdentStart(char ch) noexcept;

  /**
   * @brief 检查 ASCII 字符是否为标识符后续。
   *
   * @details
   *   ASCII 标识符后续：字母、数字 (0-9) 或下划线
   *
   * @param ch 待检查的字符
   * @return 若可作为标识符后续返回 true
   */
  [[nodiscard]] static bool isAsciiIdentContinue(char ch) noexcept;

  /**
   * @brief 检查字节是否为 UTF-8 多字节字符的起始字节。
   *
   * @details
   *   UTF-8 多字节字符起始字节 >= 0x80
   *   这些字符被视为有效的标识符字符（支持 Unicode 标识符）
   *
   * @param ch 待检查的字节
   * @return 若为 UTF-8 起始字节返回 true
   */
  [[nodiscard]] static bool isUtf8Start(unsigned char ch) noexcept;

  /**
   * @brief 读取一个完整的 UTF-8 字符。
   *
   * @details
   *   从当前位置读取一个完整的 UTF-8 多字节字符，
   *   并更新扫描上下文的位置。
   *
   * @param ctx 扫描上下文
   * @return 若成功读取返回 true
   */
  [[nodiscard]] bool consumeUtf8Char(ScanContext &ctx) const;
};

} // namespace czc::lexer

#endif // CZC_LEXER_IDENT_SCANNER_HPP
