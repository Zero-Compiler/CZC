/**
 * @file lexer_error_codes.cpp
 * @brief Lexer 错误码注册。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-12-04
 */

#include "czc/lexer/lexer_error_codes.hpp"

namespace czc::lexer::errors {

// ========== 数字相关 (1001-1010) ==========

CZC_REGISTER_ERROR(kMissingHexDigits, "missing hexadecimal digits after `0x`",
                   "lexer.missing_hex_digits");

CZC_REGISTER_ERROR(kMissingBinaryDigits, "missing binary digits after `0b`",
                   "lexer.missing_binary_digits");

CZC_REGISTER_ERROR(kMissingOctalDigits, "missing octal digits after `0o`",
                   "lexer.missing_octal_digits");

CZC_REGISTER_ERROR(kMissingExponentDigits, "missing digits in exponent",
                   "lexer.missing_exponent_digits");

CZC_REGISTER_ERROR(kInvalidTrailingChar,
                   "invalid trailing character in number literal",
                   "lexer.invalid_trailing_char");

CZC_REGISTER_ERROR(kInvalidNumberSuffix, "invalid number suffix",
                   "lexer.invalid_number_suffix");

// ========== 字符串相关 (1011-1020) ==========

CZC_REGISTER_ERROR(kInvalidEscapeSequence, "invalid escape sequence",
                   "lexer.invalid_escape_sequence");

CZC_REGISTER_ERROR(kUnterminatedString, "unterminated string literal",
                   "lexer.unterminated_string");

CZC_REGISTER_ERROR(kInvalidHexEscape, "invalid hexadecimal escape sequence",
                   "lexer.invalid_hex_escape");

CZC_REGISTER_ERROR(kInvalidUnicodeEscape, "invalid Unicode escape sequence",
                   "lexer.invalid_unicode_escape");

CZC_REGISTER_ERROR(kUnterminatedRawString, "unterminated raw string literal",
                   "lexer.unterminated_raw_string");

// ========== 字符相关 (1021-1030) ==========

CZC_REGISTER_ERROR(kInvalidCharacter, "invalid character",
                   "lexer.invalid_character");

CZC_REGISTER_ERROR(kInvalidUtf8Sequence, "invalid UTF-8 sequence",
                   "lexer.invalid_utf8_sequence");

// ========== 注释相关 (1031-1040) ==========

CZC_REGISTER_ERROR(kUnterminatedBlockComment, "unterminated block comment",
                   "lexer.unterminated_block_comment");

// ========== 通用错误 (1041-1050) ==========

CZC_REGISTER_ERROR(kTokenTooLong, "token length exceeds limit",
                   "lexer.token_too_long");

} // namespace czc::lexer::errors
