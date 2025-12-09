/**
 * @file number_scanner.cpp
 * @brief 数字字面量扫描器的实现。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-29
 */

#include "czc/lexer/number_scanner.hpp"
#include <cctype>

namespace czc::lexer {

bool NumberScanner::canScan(const ScanContext &ctx) const noexcept {
  auto ch = ctx.current();
  return ch.has_value() && std::isdigit(static_cast<unsigned char>(ch.value()));
}

Token NumberScanner::scan(ScanContext &ctx) const {
  std::size_t startOffset = ctx.offset();
  SourceLocation startLoc = ctx.location();

  auto firstCh = ctx.current();
  if (!firstCh.has_value()) {
    return ctx.makeUnknown(startOffset, startLoc);
  }

  // 检查是否是特殊进制前缀
  if (firstCh.value() == '0') {
    auto secondCh = ctx.peek(1);
    if (secondCh.has_value()) {
      char second = secondCh.value();
      if (second == 'x' || second == 'X') {
        return scanHexadecimal(ctx, startOffset, startLoc);
      }
      if (second == 'b' || second == 'B') {
        return scanBinary(ctx, startOffset, startLoc);
      }
      if (second == 'o' || second == 'O') {
        return scanOctal(ctx, startOffset, startLoc);
      }
    }
  }

  // 十进制数字
  return scanDecimal(ctx, startOffset, startLoc);
}

Token NumberScanner::scanDecimal(ScanContext &ctx, std::size_t startOffset,
                                 SourceLocation startLoc) const {
  // 消费整数部分
  consumeDigits(ctx);

  // 检查小数点（使用 lookahead 避免回退问题）
  bool isFloat = false;
  if (ctx.check('.')) {
    // 预检查小数点后是否有数字，避免消费后无法回退
    auto afterDot = ctx.peek(1);
    if (afterDot.has_value() &&
        std::isdigit(static_cast<unsigned char>(afterDot.value()))) {
      ctx.advance(); // 消费小数点
      isFloat = true;
      consumeDigits(ctx);
    }
    // 若小数点后无数字，不消费小数点（可能是成员访问如 123.method()）
  }

  // 检查科学计数法
  auto expCh = ctx.current();
  if (expCh.has_value() && (expCh.value() == 'e' || expCh.value() == 'E')) {
    ctx.advance();
    isFloat = true;

    // 可选的正负号
    auto signCh = ctx.current();
    if (signCh.has_value() &&
        (signCh.value() == '+' || signCh.value() == '-')) {
      ctx.advance();
    }

    // 指数部分必须有数字
    consumeDigits(ctx);
  }

  // 检查是否有定点后缀 d 或 dec64
  bool isDecimal = false;
  auto suffixCh = ctx.current();
  if (suffixCh.has_value() && suffixCh.value() == 'd') {
    isDecimal = true;
  }

  // 处理后缀
  consumeSuffix(ctx);

  TokenType type;
  if (isDecimal) {
    type = TokenType::LIT_DECIMAL;
  } else if (isFloat) {
    type = TokenType::LIT_FLOAT;
  } else {
    type = TokenType::LIT_INT;
  }
  return ctx.makeToken(type, startOffset, startLoc);
}

Token NumberScanner::scanHexadecimal(ScanContext &ctx, std::size_t startOffset,
                                     SourceLocation startLoc) const {
  // 消费 "0x" 或 "0X"
  ctx.advance(2);

  // 消费十六进制数字
  consumeHexDigits(ctx);

  // 处理后缀
  consumeSuffix(ctx);

  return ctx.makeToken(TokenType::LIT_INT, startOffset, startLoc);
}

Token NumberScanner::scanBinary(ScanContext &ctx, std::size_t startOffset,
                                SourceLocation startLoc) const {
  // 消费 "0b" 或 "0B"
  ctx.advance(2);

  // 消费二进制数字
  consumeBinaryDigits(ctx);

  // 处理后缀
  consumeSuffix(ctx);

  return ctx.makeToken(TokenType::LIT_INT, startOffset, startLoc);
}

Token NumberScanner::scanOctal(ScanContext &ctx, std::size_t startOffset,
                               SourceLocation startLoc) const {
  // 消费 "0o" 或 "0O"
  ctx.advance(2);

  // 消费八进制数字
  consumeOctalDigits(ctx);

  // 处理后缀
  consumeSuffix(ctx);

  return ctx.makeToken(TokenType::LIT_INT, startOffset, startLoc);
}

void NumberScanner::consumeDigits(ScanContext &ctx) const {
  while (true) {
    auto ch = ctx.current();
    if (!ch.has_value()) {
      break;
    }

    char c = ch.value();
    if (std::isdigit(static_cast<unsigned char>(c))) {
      ctx.advance();
    } else if (c == '_') {
      // 数字分隔符
      ctx.advance();
    } else {
      break;
    }
  }
}

void NumberScanner::consumeHexDigits(ScanContext &ctx) const {
  while (true) {
    auto ch = ctx.current();
    if (!ch.has_value()) {
      break;
    }

    char c = ch.value();
    if (std::isxdigit(static_cast<unsigned char>(c))) {
      ctx.advance();
    } else if (c == '_') {
      ctx.advance();
    } else {
      break;
    }
  }
}

void NumberScanner::consumeBinaryDigits(ScanContext &ctx) const {
  while (true) {
    auto ch = ctx.current();
    if (!ch.has_value()) {
      break;
    }

    char c = ch.value();
    if (c == '0' || c == '1') {
      ctx.advance();
    } else if (c == '_') {
      ctx.advance();
    } else {
      break;
    }
  }
}

void NumberScanner::consumeOctalDigits(ScanContext &ctx) const {
  while (true) {
    auto ch = ctx.current();
    if (!ch.has_value()) {
      break;
    }

    char c = ch.value();
    if (c >= '0' && c <= '7') {
      ctx.advance();
    } else if (c == '_') {
      ctx.advance();
    } else {
      break;
    }
  }
}

void NumberScanner::consumeSuffix(ScanContext &ctx) const {
  // 支持的后缀：
  // 整数: i8, i16, i32, i64, u8, u16, u32, u64
  // 浮点: f32, f64
  // 定点: d, dec64
  auto ch = ctx.current();
  if (!ch.has_value()) {
    return;
  }

  char c = ch.value();

  // 检查 u8, u16, u32, u64, i8, i16, i32, i64, f32, f64
  if (c == 'u' || c == 'i' || c == 'f') {
    ctx.advance();

    // 尝试匹配数字部分 (8, 16, 32, 64)
    while (true) {
      auto nextCh = ctx.current();
      if (!nextCh.has_value()) {
        break;
      }
      if (std::isdigit(static_cast<unsigned char>(nextCh.value()))) {
        ctx.advance();
      } else {
        break;
      }
    }
    return;
  }

  // 定点后缀: d 或 dec64
  if (c == 'd') {
    ctx.advance();

    // 检查是否是 dec64
    auto e = ctx.current();
    if (e.has_value() && e.value() == 'e') {
      ctx.advance();
      auto c2 = ctx.current();
      if (c2.has_value() && c2.value() == 'c') {
        ctx.advance();
        // 消费 64
        auto six = ctx.current();
        if (six.has_value() && six.value() == '6') {
          ctx.advance();
          auto four = ctx.current();
          if (four.has_value() && four.value() == '4') {
            ctx.advance();
          }
        }
      }
    }
  }
}

} // namespace czc::lexer
