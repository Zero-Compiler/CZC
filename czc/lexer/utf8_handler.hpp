/**
 * @file utf8_handler.hpp
 * @brief 定义了 `Utf8Handler` 类，提供处理 UTF-8 编码文本的静态工具函数。
 * @author BegoniaHe
 * @date 2025-11-11
 */

#ifndef CZC_UTF8_HANDLER_HPP
#define CZC_UTF8_HANDLER_HPP

#include <string>
#include <vector>

namespace czc {
namespace lexer {

/**
 * @brief 提供处理 UTF-8 编码文本的静态核心工具函数。
 * @details
 *   为了支持现代编程语言中常见的 Unicode 标识符和字符串内容，对 UTF-8
 *   编码的正确处理至关重要。此类提供了一组无状态的静态方法，用于处理
 *   UTF-8 字节流的底层细节，包括验证字节序列、确定多字节字符的边界、
 *   以及在 Unicode 码点和 UTF-8 编码之间进行转换。
 *
 * @note 此类不应被实例化；所有功能均通过静态方法提供。
 * @property {线程安全} 所有方法均为纯函数，因此是线程安全的。
 */
class Utf8Handler {
public:
  /**
   * @brief 检查给定的字节是否为 UTF-8 序列中的续字节 (continuation byte)。
   * @details
   *   一个有效的 UTF-8 续字节的二进制格式必须是 `10xxxxxx`。
   * @param[in] ch 要检查的字节。
   * @return 如果该字节是续字节，则返回 `true`，否则返回 `false`。
   */
  static bool is_continuation(unsigned char ch);

  /**
   * @brief 根据 UTF-8 序列的第一个字节确定该字符的总字节长度。
   * @param[in] first_byte UTF-8 字符的第一个（或唯一的）字节。
   * @return 返回该字符应占用的字节数（1 到 4）。
   * @warning
   *   调用者必须确保 `first_byte` 是一个有效的起始字节。如果传入一个续字节，
   *   此函数的行为是未定义的。
   */
  static size_t get_char_length(unsigned char first_byte);

  /**
   * @brief 将一个 Unicode 码点编码为 UTF-8 字符串。
   * @param[in] codepoint 要编码的 Unicode 码点。
   * @return 返回表示该码点的 UTF-8 编码字符串。
   */
  static std::string codepoint_to_utf8(unsigned int codepoint);

  /**
   * @brief 从输入字符串的指定位置读取一个完整的 UTF-8 字符。
   * @param[in]     input 从中读取的源字符串。
   * @param[in,out] pos
   *   输入时，为开始读取的字节位置；成功读取后，将更新为下一个字符的起始位置。
   * @param[out]    dest 读取到的 UTF-8 字符将被追加到此字符串。
   * @return 如果成功读取一个有效的 UTF-8 字符，则返回 `true`。
   */
  static bool read_char(const std::string& input, size_t& pos,
                        std::string& dest);

  /**
   * @brief 从输入字符向量的指定位置读取一个完整的 UTF-8 字符（重载版本）。
   * @param[in]     input 从中读取的源字符向量。
   * @param[in,out] pos
   *   输入时，为开始读取的字节位置；成功读取后，将更新为下一个字符的起始位置。
   * @param[out]    dest 读取到的 UTF-8 字符将被追加到此字符串。
   * @return 如果成功读取一个有效的 UTF-8 字符，则返回 `true`。
   */
  static bool read_char(const std::vector<char>& input, size_t& pos,
                        std::string& dest);
};

} // namespace lexer
} // namespace czc

#endif // CZC_UTF8_HANDLER_HPP
