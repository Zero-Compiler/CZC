/**
 * @file utf8_handler.hpp
 * @brief UTF-8 处理工具类定义
 * @author BegoniaHe
 * @date 2025-11-04
 */

#ifndef CZC_UTF8_HANDLER_HPP
#define CZC_UTF8_HANDLER_HPP

#include <string>

/**
 * @brief 提供用于处理 UTF-8 编码文本的静态工具函数。
 * @details
 *   此类封装了与 UTF-8 编码相关的底层操作，例如确定字符边界、
 *   验证字节序列以及在 Unicode 码点和 UTF-8 编码之间进行转换。
 *   所有方法都是静态的，因此该类不能被实例化。
 */
class Utf8Handler
{
public:
  /**
   * @brief 检查给定的字节是否为 UTF-8 序列中的续字节。
   * @details 续字节的格式为 10xxxxxx。
   * @param[in] ch 要检查的字节。
   * @return 如果该字节是续字节，则返回 true。
   */
  static bool is_continuation(unsigned char ch);

  /**
   * @brief 根据 UTF-8 序列的第一个字节确定该字符的总字节长度。
   * @param[in] first_byte UTF-8 字符的第一个（或唯一的）字节。
   * @return 返回该字符应占用的字节数（1 到 4）。
   * @warning 如果 `first_byte` 不是有效的起始字节，则行为未定义。
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
   * @param[in] input 从中读取的源字符串。
   * @param[in,out] pos
   *   输入时，为开始读取的字节位置；输出时，为下一个字符的起始字节位置。
   * @param[out] dest 读取到的 UTF-8 字符将被追加到此字符串。
   * @return 如果成功读取一个有效的 UTF-8 字符，则返回 true。
   */
  static bool read_char(const std::string &input, size_t &pos, std::string &dest);
};

#endif // CZC_UTF8_HANDLER_HPP
