/**
 * @file test_utf8_edge_cases.cpp
 * @brief 测试 UTF-8 编码边界情况和错误处理。
 * @author BegoniaHe
 * @date 2025-11-10
 */

#include "czc/lexer/lexer.hpp"
#include "czc/lexer/utf8_handler.hpp"
#include <cassert>
#include <iostream>

using namespace czc::lexer;

/**
 * @brief 测试 4 字节 emoji 字符处理。
 */
void test_4_byte_emoji() {
  std::string source = "let emoji = \"🚀\";";

  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  assert(!lexer.has_errors() && "4字节emoji应该正确解析");

  // 检查字符串字面量
  bool found_string = false;
  for (const auto &token : tokens) {
    if (token.token_type == TokenType::String) {
      assert(token.value == "\"🚀\"" && "Emoji应该被正确保留");
      found_string = true;
    }
  }
  assert(found_string && "应该找到字符串字面量");

  std::cout << "✓ test_4_byte_emoji: 4字节emoji正确处理" << std::endl;
}

/**
 * @brief 测试多种 emoji 和特殊 Unicode 字符。
 */
void test_various_unicode_characters() {
  std::string source = R"(
    let emoji1 = "😀";     // 笑脸 U+1F600
    let emoji2 = "🔥";     // 火焰 U+1F525
    let chinese = "你好";  // 中文 3字节
    let japanese = "こんにちは"; // 日文 3字节
    let mixed = "Hello世界🌍"; // 混合
  )";

  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  assert(!lexer.has_errors() && "各种Unicode字符应该正确解析");

  std::cout << "✓ test_various_unicode_characters: 多种Unicode字符正确处理"
            << std::endl;
}

/**
 * @brief 测试 UTF-8 标识符（变量名）。
 */
void test_utf8_identifiers() {
  std::string source = R"(
    let 变量 = 10;
    let переменная = 20;
    let μετβλητή = 30;
    let 変数 = 40;
  )";

  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  assert(!lexer.has_errors() && "UTF-8标识符应该正确解析");

  // 检查是否识别为标识符
  int identifier_count = 0;
  for (const auto &token : tokens) {
    if (token.token_type == TokenType::Identifier) {
      identifier_count++;
    }
  }
  assert(identifier_count >= 4 && "应该识别出4个UTF-8标识符");

  std::cout << "✓ test_utf8_identifiers: UTF-8标识符正确处理" << std::endl;
}

/**
 * @brief 测试无效的 UTF-8 序列（非法起始字节）。
 */
void test_invalid_utf8_start_byte() {
  // 0xFF 是无效的 UTF-8 起始字节
  std::string source = "let x = \xFF;";

  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  // 应该产生错误
  assert(lexer.has_errors() && "无效UTF-8起始字节应该产生错误");

  std::cout << "✓ test_invalid_utf8_start_byte: 检测到无效UTF-8起始字节"
            << std::endl;
}

/**
 * @brief 测试不完整的 UTF-8 序列（缺少续字节）。
 */
void test_incomplete_utf8_sequence() {
  // 0xE4 表示一个3字节序列的开始，但后面没有跟完整的续字节
  std::string source = "let x = \"\xE4\";";

  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  // 应该产生错误
  assert(lexer.has_errors() && "不完整UTF-8序列应该产生错误");

  std::cout << "✓ test_incomplete_utf8_sequence: 检测到不完整UTF-8序列"
            << std::endl;
}

/**
 * @brief 测试无效的 UTF-8 续字节。
 */
void test_invalid_utf8_continuation() {
  // 0xC0 0x80 是过长编码（非法）
  std::string source = "let x = \"\xC0\x80\";";

  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  // 应该产生错误
  assert(lexer.has_errors() && "无效UTF-8续字节应该产生错误");

  std::cout << "✓ test_invalid_utf8_continuation: 检测到无效UTF-8续字节"
            << std::endl;
}

/**
 * @brief 测试 UTF-8 BOM（字节顺序标记）。
 */
void test_utf8_bom() {
  // UTF-8 BOM: EF BB BF
  std::string source = "\xEF\xBB\xBFlet x = 10;";

  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  // BOM应该被跳过，不产生错误
  assert(!lexer.has_errors() && "UTF-8 BOM应该被正确处理");

  // 检查第一个token是否为 let
  assert(tokens.size() > 0 && tokens[0].token_type == TokenType::Let &&
         "BOM后应该正确识别关键字");

  std::cout << "✓ test_utf8_bom: UTF-8 BOM正确处理" << std::endl;
}

/**
 * @brief 测试零宽度字符。
 */
void test_zero_width_characters() {
  // 零宽度空格 U+200B
  std::string source = "let\u200Bx = 10;"; // 在let和x之间插入零宽空格

  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  // 零宽字符可能被当作空白或标识符的一部分，具体取决于实现
  // 这里主要测试不崩溃
  assert(!lexer.has_errors() && "零宽度字符不应该导致崩溃");

  std::cout << "✓ test_zero_width_characters: 零宽度字符处理正常" << std::endl;
}

/**
 * @brief 测试 Utf8Handler::is_valid_utf8。
 */
void test_utf8_validation() {
  Utf8Handler handler;

  // 有效的UTF-8序列
  assert(handler.is_valid_utf8("Hello") && "ASCII应该有效");
  assert(handler.is_valid_utf8("你好") && "中文应该有效");
  assert(handler.is_valid_utf8("🚀") && "Emoji应该有效");

  // 无效的UTF-8序列
  std::string invalid1 = "\xFF\xFE";     // 无效起始字节
  std::string invalid2 = "\xC0\x80";     // 过长编码
  std::string invalid3 = "\xE0\x80\x80"; // 过长编码
  std::string invalid4 = "\xED\xA0\x80"; // 代理对（非法）

  assert(!handler.is_valid_utf8(invalid1) && "无效起始字节应该被拒绝");
  assert(!handler.is_valid_utf8(invalid2) && "过长编码应该被拒绝");
  assert(!handler.is_valid_utf8(invalid3) && "过长编码应该被拒绝");
  assert(!handler.is_valid_utf8(invalid4) && "代理对应该被拒绝");

  std::cout << "✓ test_utf8_validation: UTF-8验证功能正确" << std::endl;
}

/**
 * @brief 测试边界位置的 UTF-8 字符。
 */
void test_utf8_at_boundaries() {
  // 文件开头的UTF-8字符
  std::string source1 = "你好世界";
  Lexer lexer1(source1);
  auto tokens1 = lexer1.tokenize();
  assert(!lexer1.has_errors() && "文件开头的UTF-8应该正确处理");

  // 文件结尾的UTF-8字符
  std::string source2 = "let x = \"世界\"";
  Lexer lexer2(source2);
  auto tokens2 = lexer2.tokenize();
  assert(!lexer2.has_errors() && "文件结尾的UTF-8应该正确处理");

  // 注释中的UTF-8字符
  std::string source3 = "let x = 10; // 这是注释 🎉";
  Lexer lexer3(source3);
  auto tokens3 = lexer3.tokenize();
  assert(!lexer3.has_errors() && "注释中的UTF-8应该正确处理");

  std::cout << "✓ test_utf8_at_boundaries: 边界位置UTF-8字符正确处理"
            << std::endl;
}

/**
 * @brief 测试混合编码场景（模拟常见错误）。
 */
void test_mixed_encoding_scenarios() {
  // ASCII + UTF-8 混合
  std::string source = "let result = calculate(42, \"结果\");";

  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  assert(!lexer.has_errors() && "ASCII和UTF-8混合应该正确处理");

  std::cout << "✓ test_mixed_encoding_scenarios: 混合编码场景正确处理"
            << std::endl;
}

int main() {
  std::cout << "\n=== Testing UTF-8 Edge Cases ===" << std::endl;

  try {
    test_4_byte_emoji();
    test_various_unicode_characters();
    test_utf8_identifiers();
    test_invalid_utf8_start_byte();
    test_incomplete_utf8_sequence();
    test_invalid_utf8_continuation();
    test_utf8_bom();
    test_zero_width_characters();
    test_utf8_validation();
    test_utf8_at_boundaries();
    test_mixed_encoding_scenarios();

    std::cout << "\nAll UTF-8 edge case tests passed!" << std::endl;
    std::cout << "\nUTF-8 处理机制验证完成：" << std::endl;
    std::cout << "   1. 多字节字符（2-4字节）正确处理" << std::endl;
    std::cout << "   2. 无效序列能够被检测" << std::endl;
    std::cout << "   3. 边界情况稳定处理" << std::endl;
  } catch (const std::exception &e) {
    std::cerr << "\nTest failed with exception: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
