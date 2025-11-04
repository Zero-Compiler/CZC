/**
 * @file diagnostic_code.cpp
 * @brief 诊断代码转换实现
 * @author BegoniaHe
 * @date 2025-11-04
 */

#include "czc/diagnostics/diagnostic_code.hpp"
#include <iomanip>
#include <sstream>

namespace czc {
namespace diagnostics {

std::string diagnostic_code_to_string(DiagnosticCode code) {
  int code_num = static_cast<int>(code);
  char prefix;
  int offset = 0;

  // --- 根据错误码的数值范围决定前缀和偏移量 ---
  // 这种设计允许我们通过简单的数值范围对错误进行分类。
  if (code_num < 1000) {
    // 以 L 为前缀的代码用于词法分析器。
    prefix = 'L';
    offset = code_num;
  } else if (code_num < 2000) {
    // 以 T 为前缀的代码用于 Token 预处理器。
    prefix = 'T';
    offset = code_num - 1000;
  } else {
    // 未知的错误代码范围，返回一个默认字符串以避免崩溃。
    return "U0000";
  }

  // --- 格式化输出 ---
  // 使用 stringstream 确保数字部分总是4位数，不足则补零。
  // 例如，偏移量为 1 的词法错误将被格式化为 "L0001"。
  std::ostringstream oss;
  oss << prefix << std::setw(4) << std::setfill('0') << offset;
  return oss.str();
}

} // namespace diagnostics
} // namespace czc
