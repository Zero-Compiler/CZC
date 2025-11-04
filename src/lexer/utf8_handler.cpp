/**
 * @file utf8_handler.cpp
 * @brief UTF-8 处理工具实现
 * @author BegoniaHe
 * @date 2025-11-04
 */

#include "czc/lexer/utf8_handler.hpp"

namespace czc {
namespace lexer {

bool Utf8Handler::is_continuation(unsigned char ch) {
  // UTF-8 的续字节 (Continuation Byte) 的二进制格式总是以 "10" 开头。
  // 掩码 0xC0 (二进制 11000000) 用于提取高两位。
  // 结果与 0x80 (二进制 10000000) 比较，以判断是否匹配 "10" 前缀。
  return (ch & 0xC0) == 0x80;
}

size_t Utf8Handler::get_char_length(unsigned char first_byte) {
  // --- 根据 UTF-8 编码规则，通过第一个字节确定字符序列的总长度 ---
  // 0xxxxxxx -> 1-byte character (ASCII)
  if ((first_byte & 0x80) == 0) { // 检查最高位是否为 0
    return 1;
  }
  // 110xxxxx -> 2-byte character
  if ((first_byte & 0xE0) == 0xC0) { // 检查高三位是否为 110
    return 2;
  }
  // 1110xxxx -> 3-byte character
  if ((first_byte & 0xF0) == 0xE0) { // 检查高四位是否为 1110
    return 3;
  }
  // 11110xxx -> 4-byte character
  if ((first_byte & 0xF8) == 0xF0) { // 检查高五位是否为 11110
    return 4;
  }
  // 如果不匹配以上任何模式，则它不是一个有效的 UTF-8 起始字节。
  return 0;
}

std::string Utf8Handler::codepoint_to_utf8(unsigned int codepoint) {
  std::string result;

  // --- Unicode 码点到 UTF-8 字节序列的转换算法 ---
  // 根据码点的大小，决定需要 1、2、3 还是 4 个字节来表示。

  // 1-byte sequence (ASCII)
  if (codepoint <= 0x7F) {
    result.push_back(static_cast<char>(codepoint));
  }
  // 2-byte sequence: 110xxxxx 10xxxxxx
  else if (codepoint <= 0x7FF) {
    result.push_back(static_cast<char>(0xC0 | ((codepoint >> 6) & 0x1F)));
    result.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
  }
  // 3-byte sequence: 1110xxxx 10xxxxxx 10xxxxxx
  else if (codepoint <= 0xFFFF) {
    result.push_back(static_cast<char>(0xE0 | ((codepoint >> 12) & 0x0F)));
    result.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
    result.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
  }
  // 4-byte sequence: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
  else if (codepoint <= 0x10FFFF) {
    result.push_back(static_cast<char>(0xF0 | ((codepoint >> 18) & 0x07)));
    result.push_back(static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F)));
    result.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
    result.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
  }

  return result;
}

bool Utf8Handler::read_char(const std::string &input, size_t &pos,
                            std::string &dest) {
  if (pos >= input.size()) {
    return false;
  }

  unsigned char first_byte = static_cast<unsigned char>(input[pos]);
  size_t char_len = get_char_length(first_byte);

  // 检查计算出的字符长度是否有效，以及输入字符串中是否还有足够的字节来构成一个完整的字符。
  if (char_len == 0 || pos + char_len > input.size()) {
    // 如果起始字节无效或剩余字节不足，则这是一个无效的 UTF-8 序列。
    return false;
  }

  // 对于多字节字符，验证其后的所有字节是否都是合法的续字节。
  // 这是确保 UTF-8 序列有效性的关键步骤。
  for (size_t i = 1; i < char_len; i++) {
    if (!is_continuation(static_cast<unsigned char>(input[pos + i]))) {
      return false; // 如果任何一个后续字节不是续字节，则序列无效。
    }
  }

  // 如果所有检查都通过，则将这组字节作为一个完整的 UTF-8
  // 字符追加到目标字符串中。
  dest.append(input, pos, char_len);
  // 更新位置指针，跳过刚刚读取的整个字符。
  pos += char_len;

  return true;
}

} // namespace lexer
} // namespace czc
