/**
 * @file diagnostic_code.hpp
 * @brief 定义了诊断消息的严重级别、唯一的诊断代码以及相关工具函数。
 * @author BegoniaHe
 * @date 2025-11-11
 */

#ifndef CZC_DIAGNOSTIC_CODE_HPP
#define CZC_DIAGNOSTIC_CODE_HPP

#include "czc/utils/source_location.hpp"

#include <string>

namespace czc {
namespace diagnostics {

/**
 * @brief 定义诊断消息的严重级别。
 * @details
 *   此枚举用于区分不同类型的诊断事件，例如，错误会阻止编译，而警告则不会。
 */
enum class DiagnosticLevel {
  Warning, // 提示潜在问题，但不影响编译。
  Error,   // 编译错误，将导致编译失败。
  Fatal    // 致命错误，导致编译器立即中止。
};

/**
 * @brief 定义编译器中所有唯一的诊断代码。
 * @details
 *   每个代码都与一条特定的诊断消息相关联。代码按模块进行分组（例如，L-Lexer,
 * T-TokenPreprocessor）， 以便于管理和识别。
 */
enum class DiagnosticCode {
  // === Lexer 警告/错误 (L0001-L0999) ===
  L0001_MissingHexDigits = 1,  // "0x" 后缺少十六进制数字
  L0002_MissingBinaryDigits,   // "0b" 后缺少二进制数字
  L0003_MissingOctalDigits,    // "0o" 后缺少八进制数字
  L0004_MissingExponentDigits, // 科学计数法指数部分缺少数字
  L0005_InvalidTrailingChar,   // 数字字面量后跟随无效字符
  L0006_InvalidEscapeSequence, // 字符串中存在无效的转义序列
  L0007_UnterminatedString,    // 字符串字面量未正确闭合
  L0008_InvalidHexEscape,      // 十六进制转义序列格式无效
  L0009_InvalidUnicodeEscape,  // Unicode 转义序列格式无效
  L0010_InvalidCharacter,      // 在源文件中遇到无效字符
  L0011_InvalidUtf8Sequence,   // 无效的 UTF-8 编码序列

  // === TokenPreprocessor 警告/错误 (T0001-T0999) ===
  // DEPRECATED: 此代码已废弃，相关逻辑已移除。
  T0001_ScientificIntOverflow = 1001,
  T0002_ScientificFloatOverflow, // 科学计数法表示的浮点数溢出

  // === Parser 错误 (P0001-P0999) ===
  P0001_UnexpectedToken = 2001,  // 遇到意外的 Token
  P0002_UnexpectedEOF,           // 意外的文件结束
  P0003_ExpectedSemicolon,       // 期望分号
  P0004_ExpectedIdentifier,      // 期望标识符
  P0005_ExpectedExpression,      // 期望表达式
  P0006_ExpectedLeftParen,       // 期望左括号
  P0007_ExpectedRightParen,      // 期望右括号
  P0008_ExpectedLeftBrace,       // 期望左花括号
  P0009_ExpectedRightBrace,      // 期望右花括号
  P0010_ExpectedRightBracket,    // 期望右方括号
  P0011_ExpectedTypeAnnotation,  // 期望类型注解
  P0012_ExpectedArrow,           // 期望箭头 ->
  P0013_InvalidAssignmentTarget, // 无效的赋值目标
};

/**
 * @brief 将 DiagnosticCode 枚举转换为其字符串表示形式。
 * @details 例如，`DiagnosticCode::L0001_MissingHexDigits` 将被转换为 "L0001"。
 * @param[in] code 要转换的诊断代码。
 * @return 代表该代码的字符串。
 */
std::string diagnostic_code_to_string(DiagnosticCode code);

} // namespace diagnostics
} // namespace czc

#endif // CZC_DIAGNOSTIC_CODE_HPP
