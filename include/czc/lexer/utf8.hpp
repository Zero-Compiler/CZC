/**
 * @file utf8.hpp
 * @brief UTF-8 编码工具函数。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-29
 *
 * @details
 *   提供 UTF-8 编码相关的工具函数：
 *   - 字符长度计算
 *   - 码点解码/编码
 *   - 有效性验证
 *   - 字符分类（标识符起始/继续）
 *
 *   zerolang 支持 UTF-8 编码的 Unicode 标识符，
 *   标识符规则：[[:alpha:]_][[:alnum:]_]*
 *   其中非 ASCII 字符（UTF-8 多字节）均被视为有效标识符字符。
 */

#ifndef CZC_LEXER_UTF8_HPP
#define CZC_LEXER_UTF8_HPP

#include "czc/common/config.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace czc::lexer::utf8 {

/**
 * @brief 根据首字节判断 UTF-8 字符的字节长度。
 *
 * @param firstByte UTF-8 字符的首字节
 * @return 字符长度（1-4），若为无效首字节则返回 0
 */
[[nodiscard]] constexpr std::size_t
charLength(unsigned char firstByte) noexcept {
  if ((firstByte & 0x80) == 0x00)
    return 1; // 0xxxxxxx - ASCII
  if ((firstByte & 0xE0) == 0xC0)
    return 2; // 110xxxxx
  if ((firstByte & 0xF0) == 0xE0)
    return 3; // 1110xxxx
  if ((firstByte & 0xF8) == 0xF0)
    return 4; // 11110xxx
  return 0;   // 无效首字节（10xxxxxx 或 11111xxx）
}

/**
 * @brief 检查字节是否为 UTF-8 续字节。
 *
 * @param byte 待检查的字节
 * @return 若为续字节（10xxxxxx）返回 true
 */
[[nodiscard]] constexpr bool isContinuationByte(unsigned char byte) noexcept {
  return (byte & 0xC0) == 0x80;
}

/**
 * @brief 检查字节是否为 ASCII 字符。
 *
 * @param byte 待检查的字节
 * @return 若为 ASCII（0x00-0x7F）返回 true
 */
[[nodiscard]] constexpr bool isAscii(unsigned char byte) noexcept {
  return byte < 0x80;
}

/**
 * @brief 检查字节是否为 UTF-8 多字节字符的起始字节。
 *
 * @details
 *   UTF-8 多字节字符的起始字节 >= 0x80 且不是续字节。
 *   即：110xxxxx, 1110xxxx, 或 11110xxx
 *
 * @param byte 待检查的字节
 * @return 若为 UTF-8 多字节起始字节返回 true
 */
[[nodiscard]] constexpr bool isMultibyteStart(unsigned char byte) noexcept {
  return byte >= 0xC0 && byte < 0xF8;
}

/**
 * @brief 解码 UTF-8 字符为 Unicode 码点。
 *
 * @param str 字符串视图，从开头解码
 * @param[out] bytesConsumed 消耗的字节数（输出参数）
 * @return 解码成功返回码点，失败返回 std::nullopt
 */
[[nodiscard]] std::optional<char32_t> decodeChar(std::string_view str,
                                                 std::size_t &bytesConsumed);

/**
 * @brief 解码 UTF-8 字符为 Unicode 码点（简化版本）。
 *
 * @param str 字符串视图，从开头解码
 * @return 解码成功返回码点，失败返回 std::nullopt
 */
[[nodiscard]] inline std::optional<char32_t> decodeChar(std::string_view str) {
  std::size_t consumed = 0;
  return decodeChar(str, consumed);
}

/**
 * @brief 将 Unicode 码点编码为 UTF-8 字符串。
 *
 * @param codepoint Unicode 码点
 * @return 编码成功返回 UTF-8 字符串，失败返回空字符串
 */
[[nodiscard]] std::string encodeCodepoint(char32_t codepoint);

/**
 * @brief 验证字符串是否为有效的 UTF-8 编码。
 *
 * @param str 待验证的字符串
 * @return 若为有效 UTF-8 返回 true
 */
[[nodiscard]] bool isValidUtf8(std::string_view str) noexcept;

/**
 * @brief 计算 UTF-8 字符串的字符数（码点数）。
 *
 * @param str UTF-8 字符串
 * @return 字符数，若包含无效序列则返回 std::nullopt
 */
[[nodiscard]] std::optional<std::size_t>
charCount(std::string_view str) noexcept;

/**
 * @brief 从字符串指定位置读取一个完整的 UTF-8 字符。
 *
 * @details
 *   参考旧版 Utf8Handler::read_char 实现。
 *   读取从 pos 开始的一个完整 UTF-8 字符，并更新 pos 到下一个字符位置。
 *
 * @param str 源字符串
 * @param[in,out] pos 输入时为读取起始位置，输出时为下一个字符位置
 * @param[out] dest 读取到的 UTF-8 字符将追加到此字符串
 * @return 若成功读取返回 true，若遇到无效序列或越界返回 false
 */
[[nodiscard]] bool readChar(std::string_view str, std::size_t &pos,
                            std::string &dest);

/**
 * @brief 跳过一个完整的 UTF-8 字符。
 *
 * @details
 *   仅更新位置，不保存字符内容。用于快速跳过字符。
 *
 * @param str 源字符串
 * @param[in,out] pos 输入时为当前位置，输出时为下一个字符位置
 * @return 若成功跳过返回 true
 */
[[nodiscard]] bool skipChar(std::string_view str, std::size_t &pos) noexcept;

/**
 * @brief 检查码点是否可作为标识符起始字符。
 *
 * @details
 *   标识符起始字符：
 *   - ASCII 字母 (a-z, A-Z)
 *   - 下划线 (_)
 *   - Unicode 字母类别 (Lu, Ll, Lt, Lm, Lo, Nl)
 *
 * @param codepoint Unicode 码点
 * @return 若可作为标识符起始返回 true
 */
[[nodiscard]] bool isIdentStart(char32_t codepoint) noexcept;

/**
 * @brief 检查码点是否可作为标识符后续字符。
 *
 * @details
 *   标识符后续字符：
 *   - 所有标识符起始字符
 *   - ASCII 数字 (0-9)
 *   - Unicode 数字类别 (Nd)
 *   - Unicode 连接符 (Pc)
 *   - Unicode 组合标记 (Mn, Mc)
 *
 * @param codepoint Unicode 码点
 * @return 若可作为标识符后续返回 true
 */
[[nodiscard]] bool isIdentContinue(char32_t codepoint) noexcept;

/**
 * @brief 检查 ASCII 字符是否可作为标识符起始。
 *
 * @param ch ASCII 字符
 * @return 若可作为标识符起始返回 true
 */
[[nodiscard]] constexpr bool isAsciiIdentStart(char ch) noexcept {
  return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_';
}

/**
 * @brief 检查 ASCII 字符是否可作为标识符后续。
 *
 * @param ch ASCII 字符
 * @return 若可作为标识符后续返回 true
 */
[[nodiscard]] constexpr bool isAsciiIdentContinue(char ch) noexcept {
  return isAsciiIdentStart(ch) || (ch >= '0' && ch <= '9');
}

/**
 * @brief 检查 ASCII 字符是否为十六进制数字。
 *
 * @param ch ASCII 字符
 * @return 若为十六进制数字返回 true
 */
[[nodiscard]] constexpr bool isHexDigit(char ch) noexcept {
  return (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') ||
         (ch >= 'A' && ch <= 'F');
}

/**
 * @brief 将十六进制字符转换为数值。
 *
 * @param ch 十六进制字符
 * @return 数值（0-15），若不是十六进制字符返回 -1
 */
[[nodiscard]] constexpr int hexDigitValue(char ch) noexcept {
  if (ch >= '0' && ch <= '9')
    return ch - '0';
  if (ch >= 'a' && ch <= 'f')
    return ch - 'a' + 10;
  if (ch >= 'A' && ch <= 'F')
    return ch - 'A' + 10;
  return -1;
}

} // namespace czc::lexer::utf8

#endif // CZC_LEXER_UTF8_HPP
