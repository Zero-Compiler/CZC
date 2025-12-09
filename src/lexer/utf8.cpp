/**
 * @file utf8.cpp
 * @brief UTF-8 工具函数实现。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-29
 */

#include "czc/lexer/utf8.hpp"

#include <unicode/uchar.h>
#include <unicode/utf8.h>

namespace czc::lexer::utf8 {

std::optional<char32_t> decodeChar(std::string_view str,
                                   std::size_t &bytesConsumed) {
  if (str.empty()) {
    bytesConsumed = 0;
    return std::nullopt;
  }

  int32_t i = 0;
  int32_t length = str.size();
  char32_t codepoint;

  // 转换为 const unsigned char* 以保证可移植性
  U8_NEXT(reinterpret_cast<const unsigned char *>(str.data()), i, length,
          codepoint);

  if (codepoint < 0) {
    bytesConsumed = 0;
    return std::nullopt;
  }

  bytesConsumed = i;
  return codepoint;
}

std::string encodeCodepoint(char32_t codepoint) {
  std::string result;
  result.resize(4); // UTF-8 最多 4 字节

  int32_t i = 0;
  UBool isError = false;
  U8_APPEND(reinterpret_cast<uint8_t *>(result.data()), i, 4, codepoint,
            isError);

  if (isError) {
    return {}; // 无效码点
  }

  result.resize(i);
  return result;
}

bool isValidUtf8(std::string_view str) noexcept {
  std::size_t pos = 0;
  while (pos < str.size()) {
    std::size_t consumed = 0;
    auto cp = decodeChar(str.substr(pos), consumed);
    if (!cp.has_value() || consumed == 0) {
      return false;
    }
    pos += consumed;
  }
  return true;
}

std::optional<std::size_t> charCount(std::string_view str) noexcept {
  std::size_t count = 0;
  std::size_t pos = 0;

  while (pos < str.size()) {
    std::size_t consumed = 0;
    auto cp = decodeChar(str.substr(pos), consumed);
    if (!cp.has_value() || consumed == 0) {
      return std::nullopt;
    }
    pos += consumed;
    ++count;
  }

  return count;
}

bool readChar(std::string_view str, std::size_t &pos, std::string &dest) {
  if (pos >= str.size()) {
    return false;
  }

  auto firstByte = static_cast<unsigned char>(str[pos]);
  std::size_t len = charLength(firstByte);

  if (len == 0 || pos + len > str.size()) {
    return false;
  }

  // 验证续字节
  for (std::size_t i = 1; i < len; ++i) {
    if (!isContinuationByte(static_cast<unsigned char>(str[pos + i]))) {
      return false;
    }
  }

  // 追加到目标字符串
  dest.append(str.data() + pos, len);
  pos += len;

  return true;
}

bool skipChar(std::string_view str, std::size_t &pos) noexcept {
  if (pos >= str.size()) {
    return false;
  }

  auto firstByte = static_cast<unsigned char>(str[pos]);
  std::size_t len = charLength(firstByte);

  if (len == 0 || pos + len > str.size()) {
    return false;
  }

  // 验证续字节
  for (std::size_t i = 1; i < len; ++i) {
    if (!isContinuationByte(static_cast<unsigned char>(str[pos + i]))) {
      return false;
    }
  }

  pos += len;
  return true;
}

bool isIdentStart(char32_t codepoint) noexcept {
  // ASCII 快速路径
  if (codepoint < 0x80) {
    char ch = static_cast<char>(codepoint);
    return isAsciiIdentStart(ch);
  }

  // 对于非 ASCII 字符，zerolang 允许所有 Unicode 字母作为标识符
  return u_hasBinaryProperty(codepoint, UCHAR_XID_START);
}

bool isIdentContinue(char32_t codepoint) noexcept {
  // ASCII 快速路径
  if (codepoint < 0x80) {
    char ch = static_cast<char>(codepoint);
    return isAsciiIdentContinue(ch);
  }

  // 对于非 ASCII 字符，与 isIdentStart 相同
  // 标识符后续字符还可以包含数字
  return u_hasBinaryProperty(codepoint, UCHAR_XID_CONTINUE);
}

} // namespace czc::lexer::utf8
