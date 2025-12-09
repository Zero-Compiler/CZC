/**
 * @file lexer_error_codes.hpp
 * @brief Lexer 错误码定义。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-12-04
 *
 * @details
 *   为 Lexer 模块注册诊断系统错误码。
 */

#ifndef CZC_LEXER_LEXER_ERROR_CODES_HPP
#define CZC_LEXER_LEXER_ERROR_CODES_HPP

#include "czc/diag/error_code.hpp"

CZC_BEGIN_ERROR_CODES(lexer)

// ========== 数字相关 (1001-1010) ==========

/// "0x" 后缺少十六进制数字
CZC_DECLARE_ERROR(kMissingHexDigits, Lexer, 1001);

/// "0b" 后缺少二进制数字
CZC_DECLARE_ERROR(kMissingBinaryDigits, Lexer, 1002);

/// "0o" 后缺少八进制数字
CZC_DECLARE_ERROR(kMissingOctalDigits, Lexer, 1003);

/// 科学计数法指数部分缺少数字
CZC_DECLARE_ERROR(kMissingExponentDigits, Lexer, 1004);

/// 数字字面量后跟随无效字符
CZC_DECLARE_ERROR(kInvalidTrailingChar, Lexer, 1005);

/// 无效的数字后缀
CZC_DECLARE_ERROR(kInvalidNumberSuffix, Lexer, 1006);

// ========== 字符串相关 (1011-1020) ==========

/// 无效的转义序列
CZC_DECLARE_ERROR(kInvalidEscapeSequence, Lexer, 1011);

/// 字符串未闭合
CZC_DECLARE_ERROR(kUnterminatedString, Lexer, 1012);

/// 无效的十六进制转义
CZC_DECLARE_ERROR(kInvalidHexEscape, Lexer, 1013);

/// 无效的 Unicode 转义
CZC_DECLARE_ERROR(kInvalidUnicodeEscape, Lexer, 1014);

/// 原始字符串未闭合
CZC_DECLARE_ERROR(kUnterminatedRawString, Lexer, 1015);

// ========== 字符相关 (1021-1030) ==========

/// 无效字符
CZC_DECLARE_ERROR(kInvalidCharacter, Lexer, 1021);

/// 无效的 UTF-8 序列
CZC_DECLARE_ERROR(kInvalidUtf8Sequence, Lexer, 1022);

// ========== 注释相关 (1031-1040) ==========

/// 块注释未闭合
CZC_DECLARE_ERROR(kUnterminatedBlockComment, Lexer, 1031);

// ========== 通用错误 (1041-1050) ==========

/// Token 长度超过限制
CZC_DECLARE_ERROR(kTokenTooLong, Lexer, 1041);

CZC_END_ERROR_CODES()

#endif // CZC_LEXER_LEXER_ERROR_CODES_HPP
