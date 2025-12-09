/**
 * @file number_scanner.hpp
 * @brief 数字字面量扫描器。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-29
 *
 * @details
 *   NumberScanner 负责扫描各种数字字面量：
 *   - 十进制整数: 123, 456
 *   - 十六进制整数: 0x1A2B, 0XFF
 *   - 二进制整数: 0b1010, 0B1111
 *   - 八进制整数: 0o755, 0O644
 *   - 浮点数: 3.14, 0.5
 *   - 科学计数法: 1.23e10, 1e-5
 *   - 定点数: 3.14d, 3.14dec64
 *
 *   支持类型后缀：i8, i16, i32, i64, u8, u16, u32, u64, f32, f64
 */

#ifndef CZC_LEXER_NUMBER_SCANNER_HPP
#define CZC_LEXER_NUMBER_SCANNER_HPP

#include "czc/common/config.hpp"

#include "czc/lexer/scanner.hpp"

namespace czc::lexer {

/**
 * @brief 数字扫描器。
 *
 * @details
 *   扫描各种数字字面量，支持多种进制和类型后缀。
 */
class NumberScanner {
public:
  NumberScanner() = default;

  /**
   * @brief 检查当前字符是否可由此扫描器处理。
   *
   * @param ctx 扫描上下文
   * @return 若当前字符为数字返回 true
   */
  [[nodiscard]] bool canScan(const ScanContext &ctx) const noexcept;

  /**
   * @brief 执行扫描。
   *
   * @param ctx 扫描上下文
   * @return 扫描得到的 Token（LIT_INT, LIT_FLOAT, LIT_DECIMAL）
   */
  [[nodiscard]] Token scan(ScanContext &ctx) const;

private:
  /**
   * @brief 扫描十进制数。
   *
   * @param ctx 扫描上下文
   * @param startOffset 起始偏移
   * @param startLoc 起始位置
   * @return 扫描得到的 Token
   */
  [[nodiscard]] Token scanDecimal(ScanContext &ctx, std::size_t startOffset,
                                  SourceLocation startLoc) const;

  /**
   * @brief 扫描十六进制数。
   *
   * @param ctx 扫描上下文
   * @param startOffset 起始偏移
   * @param startLoc 起始位置
   * @return 扫描得到的 Token
   */
  [[nodiscard]] Token scanHexadecimal(ScanContext &ctx, std::size_t startOffset,
                                      SourceLocation startLoc) const;

  /**
   * @brief 扫描二进制数。
   *
   * @param ctx 扫描上下文
   * @param startOffset 起始偏移
   * @param startLoc 起始位置
   * @return 扫描得到的 Token
   */
  [[nodiscard]] Token scanBinary(ScanContext &ctx, std::size_t startOffset,
                                 SourceLocation startLoc) const;

  /**
   * @brief 扫描八进制数。
   *
   * @param ctx 扫描上下文
   * @param startOffset 起始偏移
   * @param startLoc 起始位置
   * @return 扫描得到的 Token
   */
  [[nodiscard]] Token scanOctal(ScanContext &ctx, std::size_t startOffset,
                                SourceLocation startLoc) const;

  /**
   * @brief 扫描指数部分（科学计数法）。
   *
   * @param ctx 扫描上下文
   * @return 若成功扫描指数返回 true
   */
  [[nodiscard]] bool scanExponent(ScanContext &ctx) const;

  /**
   * @brief 扫描数字后缀。
   *
   * @param ctx 扫描上下文
   * @param[out] isFloat 是否为浮点后缀
   * @param[out] isDecimal 是否为定点后缀
   * @return 若有有效后缀返回 true
   */
  [[nodiscard]] bool scanSuffix(ScanContext &ctx, bool &isFloat,
                                bool &isDecimal) const;

  /**
   * @brief 消费十进制数字（含分隔符 _）。
   * @param ctx 扫描上下文
   */
  void consumeDigits(ScanContext &ctx) const;

  /**
   * @brief 消费十六进制数字（含分隔符 _）。
   * @param ctx 扫描上下文
   */
  void consumeHexDigits(ScanContext &ctx) const;

  /**
   * @brief 消费二进制数字（含分隔符 _）。
   * @param ctx 扫描上下文
   */
  void consumeBinaryDigits(ScanContext &ctx) const;

  /**
   * @brief 消费八进制数字（含分隔符 _）。
   * @param ctx 扫描上下文
   */
  void consumeOctalDigits(ScanContext &ctx) const;

  /**
   * @brief 消费类型后缀。
   * @param ctx 扫描上下文
   */
  void consumeSuffix(ScanContext &ctx) const;
};

} // namespace czc::lexer

#endif // CZC_LEXER_NUMBER_SCANNER_HPP
