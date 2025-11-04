/**
 * @file diagnostic_code.hpp
 * @brief 诊断代码和级别定义
 * @author BegoniaHe
 * @date 2025-11-04
 */

#ifndef CZC_DIAGNOSTIC_CODE_HPP
#define CZC_DIAGNOSTIC_CODE_HPP

#include "czc/utils/source_location.hpp"
#include <string>

/**
 * @brief 诊断级别枚举
 */
enum class DiagnosticLevel
{
    Warning, // 警告
    Error,   // 错误
    Fatal    // 致命错误
};

/**
 * @brief 错误代码枚举
 */
enum class DiagnosticCode
{
    // === Lexer 警告/错误 (L0001-L0999) ===
    L0001_MissingHexDigits = 1,  // 0x 后缺少数字
    L0002_MissingBinaryDigits,   // 0b 后缺少数字
    L0003_MissingOctalDigits,    // 0o 后缺少数字
    L0004_MissingExponentDigits, // 科学计数法指数部分缺少数字
    L0005_InvalidTrailingChar,   // 数字后跟随无效字符
    L0006_InvalidEscapeSequence, // 无效的转义序列
    L0007_UnterminatedString,    // 未闭合的字符串
    L0008_InvalidHexEscape,      // 无效的十六进制转义
    L0009_InvalidUnicodeEscape,  // 无效的 Unicode 转义
    L0010_InvalidCharacter,      // 无效字符
    L0011_InvalidUtf8Sequence,   // 无效的 UTF-8 序列

    // === TokenPreprocessor 警告/错误 (T0001-T0999) ===
    T0001_ScientificIntOverflow = 1001, // 科学计数法整数溢出（已废弃，仅用于兼容）
    T0002_ScientificFloatOverflow,      // 科学计数法浮点数溢出
};

/**
 * @brief 获取错误代码字符串
 * @param code 诊断代码
 * @return 错误代码的字符串表示
 */
std::string diagnostic_code_to_string(DiagnosticCode code);

#endif // CZC_DIAGNOSTIC_CODE_HPP
