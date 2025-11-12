/**
 * @file diagnostic_code.cpp
 * @brief `DiagnosticCode` 相关工具函数的实现。
 * @author BegoniaHe
 * @date 2025-11-11
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
  // NOTE: 这种基于数值范围的分类方法是一种简单而有效的设计，它允许我们
  //       在不使用复杂数据结构（如 map）的情况下，快速地将诊断码映射到
  //       其所属的编译器模块（L=Lexer, T=TokenPreprocessor, P=Parser, etc.）。
  //       这使得错误码的管理和扩展变得直观。
  if (code_num < 1000) {
    // L-prefix codes are for the Lexer.
    prefix = 'L';
    offset = code_num;
  } else if (code_num < 2000) {
    // T-prefix codes are for the Token Preprocessor.
    prefix = 'T';
    offset = code_num - 1000;
  } else if (code_num < 3000) {
    // P-prefix codes are for the Parser.
    prefix = 'P';
    offset = code_num - 2000;
  } else {
    // NOTE: 对于未知的错误代码范围，返回一个默认的 "U0000" (Unknown)
    //       字符串，这是一种防御性编程，可以防止程序在遇到意外
    //       错误码时崩溃。
    return "U0000";
  }

  // --- 格式化输出 ---
  // NOTE: 使用 ostringstream 和 iomanip 操纵符（setw, setfill）是
  //       C++ 中实现零填充数字格式化的标准且安全的方式。这确保了
  //       所有的诊断代码（如 "L0001", "T0012"）都具有统一的视觉长度，
  //       提高了可读性。
  std::ostringstream oss;
  oss << prefix << std::setw(4) << std::setfill('0') << offset;
  return oss.str();
}

} // namespace diagnostics
} // namespace czc
