/**
 * @file char_scanner.hpp
 * @brief 字符扫描器（运算符和分隔符）。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-29
 *
 * @details
 *   CharScanner 负责扫描单字符、双字符和三字符 Token：
 *   - 单字符: +, -, *, /, (, ), 等
 *   - 双字符: ==, !=, <=, >=, ->, =>, ::, .., 等
 *   - 三字符: ..=, <<=, >>=
 *
 *   使用查表法替代巨大的 switch-case，提高可维护性。
 *   采用贪婪匹配（最长匹配优先）。
 */

#ifndef CZC_LEXER_CHAR_SCANNER_HPP
#define CZC_LEXER_CHAR_SCANNER_HPP

#include "czc/common/config.hpp"

#include "czc/lexer/scanner.hpp"

#include <array>
#include <optional>
#include <unordered_map>
#include <vector>

namespace czc::lexer {

/**
 * @brief 字符扫描器。
 *
 * @details
 *   使用查表法扫描运算符和分隔符。
 *   先尝试三字符匹配，再双字符，最后单字符。
 */
class CharScanner {
public:
  /**
   * @brief 默认构造函数。
   *
   * @details
   *   使用静态查找表，无需运行时初始化。
   */
  CharScanner() = default;

  /**
   * @brief 检查当前字符是否可由此扫描器处理。
   *
   * @param ctx 扫描上下文
   * @return 若当前字符在单字符 Token 表中返回 true
   */
  [[nodiscard]] bool canScan(const ScanContext &ctx) const noexcept;

  /**
   * @brief 执行扫描。
   *
   * @param ctx 扫描上下文
   * @return 扫描得到的 Token
   */
  [[nodiscard]] Token scan(ScanContext &ctx) const;

private:
  /**
   * @brief 双字符 Token 条目。
   */
  struct TwoCharEntry {
    char second;    ///< 第二个字符
    TokenType type; ///< Token 类型
  };

  /**
   * @brief 三字符 Token 条目。
   */
  struct ThreeCharEntry {
    char second;    ///< 第二个字符
    char third;     ///< 第三个字符
    TokenType type; ///< Token 类型
  };

  // 注意：使用匿名命名空间中的静态查找表，无需成员变量

  /**
   * @brief 尝试匹配三字符 Token。
   *
   * @param ctx 扫描上下文
   * @param first 第一个字符
   * @return 若匹配成功返回 Token 类型
   */
  [[nodiscard]] std::optional<TokenType>
  tryMatchThreeChar(const ScanContext &ctx, char first) const;

  /**
   * @brief 尝试匹配双字符 Token。
   *
   * @param ctx 扫描上下文
   * @param first 第一个字符
   * @return 若匹配成功返回 Token 类型
   */
  [[nodiscard]] std::optional<TokenType> tryMatchTwoChar(const ScanContext &ctx,
                                                         char first) const;
};

} // namespace czc::lexer

#endif // CZC_LEXER_CHAR_SCANNER_HPP
