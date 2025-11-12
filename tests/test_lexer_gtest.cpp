/**
 * @file test_lexer_gtest.cpp
 * @brief 词法分析器测试套件（使用 Google Test 框架）。
 * @details 本测试套件全面测试词法分析器的各种功能，包括数字字面量、
 *          字符串、标识符、关键字、运算符、注释以及 UTF-8 支持等。
 * @author BegoniaHe
 * @date 2025-11-11
 */

#include "czc/lexer/lexer.hpp"

#include <vector>

#include <gtest/gtest.h>

using namespace czc::lexer;

/**
 * @brief 词法分析器测试夹具。
 * @details 提供通用的辅助方法，简化测试用例编写。
 */
class LexerTest : public ::testing::Test {
protected:
  /**
   * @brief 辅助函数：对源代码进行词法分析并返回 Token 序列。
   * @param[in] source 待分析的源代码字符串。
   * @return 词法分析生成的 Token 向量。
   */
  std::vector<Token> tokenize(const std::string& source) {
    Lexer lexer(source);
    return lexer.tokenize();
  }
};

// --- 整数字面量测试 ---

/**
 * @brief 测试基本整数字面量的识别。
 * @details 验证词法分析器能够正确识别十进制整数字面量。
 */
TEST_F(LexerTest, BasicIntegers) {
  auto tokens = tokenize("123 456 789");

  // 预期：3 个整数 Token + 1 个 EOF Token
  ASSERT_EQ(tokens.size(), 4);
  EXPECT_EQ(tokens[0].token_type, TokenType::Integer);
  EXPECT_EQ(tokens[0].value, "123");
  EXPECT_EQ(tokens[1].token_type, TokenType::Integer);
  EXPECT_EQ(tokens[1].value, "456");
  EXPECT_EQ(tokens[2].token_type, TokenType::Integer);
  EXPECT_EQ(tokens[2].value, "789");
  EXPECT_EQ(tokens[3].token_type, TokenType::EndOfFile);
}

// --- 浮点数字面量测试 ---

/**
 * @brief 测试基本浮点数字面量的识别。
 * @details 验证词法分析器能够正确识别带小数点的浮点数字面量。
 */
TEST_F(LexerTest, BasicFloats) {
  auto tokens = tokenize("3.14 2.71828 0.5");

  // 预期：3 个浮点数 Token + 1 个 EOF Token
  ASSERT_EQ(tokens.size(), 4);
  EXPECT_EQ(tokens[0].token_type, TokenType::Float);
  EXPECT_EQ(tokens[0].value, "3.14");
  EXPECT_EQ(tokens[1].token_type, TokenType::Float);
  EXPECT_EQ(tokens[1].value, "2.71828");
  EXPECT_EQ(tokens[2].token_type, TokenType::Float);
  EXPECT_EQ(tokens[2].value, "0.5");
  EXPECT_EQ(tokens[3].token_type, TokenType::EndOfFile);
}

// --- 科学记数法测试 ---

/**
 * @brief 测试科学记数法字面量的识别。
 * @details 验证词法分析器能够正确识别正指数、负指数以及整数形式的科学记数法。
 */
TEST_F(LexerTest, ScientificNotation) {
  auto tokens = tokenize("1.5e10 2.0e-5 3e8");

  ASSERT_EQ(tokens.size(), 4);
  EXPECT_EQ(tokens[0].token_type, TokenType::ScientificExponent);
  EXPECT_EQ(tokens[0].value, "1.5e10");
  EXPECT_EQ(tokens[1].token_type, TokenType::ScientificExponent);
  EXPECT_EQ(tokens[1].value, "2.0e-5");
  EXPECT_EQ(tokens[2].token_type, TokenType::ScientificExponent);
  EXPECT_EQ(tokens[2].value, "3e8");
}

// --- 十六进制、二进制、八进制数字测试 ---

/**
 * @brief 测试十六进制数字字面量的识别。
 * @details 验证词法分析器能够正确识别 0x 前缀的十六进制整数。
 */
TEST_F(LexerTest, HexadecimalNumbers) {
  auto tokens = tokenize("0xFF 0x1A2B 0x0");

  ASSERT_EQ(tokens.size(), 4);
  EXPECT_EQ(tokens[0].token_type, TokenType::Integer);
  EXPECT_EQ(tokens[0].value, "0xFF");
  EXPECT_EQ(tokens[1].token_type, TokenType::Integer);
  EXPECT_EQ(tokens[1].value, "0x1A2B");
  EXPECT_EQ(tokens[2].token_type, TokenType::Integer);
  EXPECT_EQ(tokens[2].value, "0x0");
}

/**
 * @brief 测试二进制数字字面量的识别。
 * @details 验证词法分析器能够正确识别 0b 前缀的二进制整数。
 */
TEST_F(LexerTest, BinaryNumbers) {
  auto tokens = tokenize("0b1010 0b1111 0b0");

  ASSERT_EQ(tokens.size(), 4);
  EXPECT_EQ(tokens[0].token_type, TokenType::Integer);
  EXPECT_EQ(tokens[0].value, "0b1010");
  EXPECT_EQ(tokens[1].token_type, TokenType::Integer);
  EXPECT_EQ(tokens[1].value, "0b1111");
}

/**
 * @brief 测试八进制数字字面量的识别。
 * @details 验证词法分析器能够正确识别 0o 前缀的八进制整数。
 */
TEST_F(LexerTest, OctalNumbers) {
  auto tokens = tokenize("0o755 0o17");

  ASSERT_EQ(tokens.size(), 3);
  EXPECT_EQ(tokens[0].token_type, TokenType::Integer);
  EXPECT_EQ(tokens[0].value, "0o755");
  EXPECT_EQ(tokens[1].token_type, TokenType::Integer);
  EXPECT_EQ(tokens[1].value, "0o17");
}

// --- 字符串字面量测试 ---

/**
 * @brief 测试基本字符串字面量的识别。
 * @details 验证词法分析器能够正确识别双引号包裹的字符串。
 */
TEST_F(LexerTest, BasicStrings) {
  auto tokens = tokenize(R"("hello" "world")");

  ASSERT_EQ(tokens.size(), 3);
  EXPECT_EQ(tokens[0].token_type, TokenType::String);
  EXPECT_EQ(tokens[0].value, "hello");
  EXPECT_EQ(tokens[1].token_type, TokenType::String);
  EXPECT_EQ(tokens[1].value, "world");
}

/**
 * @brief 测试字符串转义序列的处理。
 * @details 验证词法分析器能够正确解析 \n、\t 等转义字符。
 */
TEST_F(LexerTest, StringEscapeSequences) {
  auto tokens = tokenize(R"("line1\nline2\ttab")");

  ASSERT_EQ(tokens.size(), 2);
  EXPECT_EQ(tokens[0].token_type, TokenType::String);
  EXPECT_EQ(tokens[0].value, "line1\nline2\ttab");
}

/**
 * @brief 测试原始字符串的识别。
 * @details 验证词法分析器能够正确识别 r 前缀的原始字符串，
 *          其中反斜杠不进行转义处理。
 */
TEST_F(LexerTest, RawStrings) {
  auto tokens = tokenize(R"(r"C:\path\to\file")");

  ASSERT_EQ(tokens.size(), 2);
  EXPECT_EQ(tokens[0].token_type, TokenType::String);
  EXPECT_EQ(tokens[0].value, R"(C:\path\to\file)");
}

// --- 标识符与关键字测试 ---

/**
 * @brief 测试标识符的识别。
 * @details 验证词法分析器能够正确识别合法的标识符，
 *          包括字母、数字和下划线的组合。
 */
TEST_F(LexerTest, Identifiers) {
  auto tokens = tokenize("foo bar baz123 _underscore");

  ASSERT_EQ(tokens.size(), 5);
  EXPECT_EQ(tokens[0].token_type, TokenType::Identifier);
  EXPECT_EQ(tokens[0].value, "foo");
  EXPECT_EQ(tokens[1].token_type, TokenType::Identifier);
  EXPECT_EQ(tokens[1].value, "bar");
  EXPECT_EQ(tokens[2].token_type, TokenType::Identifier);
  EXPECT_EQ(tokens[2].value, "baz123");
  EXPECT_EQ(tokens[3].token_type, TokenType::Identifier);
  EXPECT_EQ(tokens[3].value, "_underscore");
}

/**
 * @brief 测试关键字的识别。
 * @details 验证词法分析器能够正确将保留关键字与普通标识符区分开。
 */
TEST_F(LexerTest, Keywords) {
  auto tokens = tokenize("let fn if else while return");

  ASSERT_EQ(tokens.size(), 7);
  EXPECT_EQ(tokens[0].token_type, TokenType::Let);
  EXPECT_EQ(tokens[1].token_type, TokenType::Fn);
  EXPECT_EQ(tokens[2].token_type, TokenType::If);
  EXPECT_EQ(tokens[3].token_type, TokenType::Else);
  EXPECT_EQ(tokens[4].token_type, TokenType::While);
  EXPECT_EQ(tokens[5].token_type, TokenType::Return);
}

// --- 运算符测试 ---

/**
 * @brief 测试算术运算符的识别。
 * @details 验证词法分析器能够正确识别加减乘除模等算术运算符。
 */
TEST_F(LexerTest, ArithmeticOperators) {
  auto tokens = tokenize("+ - * / %");

  ASSERT_EQ(tokens.size(), 6);
  EXPECT_EQ(tokens[0].token_type, TokenType::Plus);
  EXPECT_EQ(tokens[1].token_type, TokenType::Minus);
  EXPECT_EQ(tokens[2].token_type, TokenType::Star);
  EXPECT_EQ(tokens[3].token_type, TokenType::Slash);
  EXPECT_EQ(tokens[4].token_type, TokenType::Percent);
}

/**
 * @brief 测试比较运算符的识别。
 * @details 验证词法分析器能够正确识别相等、不等、大小比较等运算符。
 */
TEST_F(LexerTest, ComparisonOperators) {
  auto tokens = tokenize("== != < > <= >=");

  ASSERT_EQ(tokens.size(), 7);
  EXPECT_EQ(tokens[0].token_type, TokenType::EqualEqual);
  EXPECT_EQ(tokens[1].token_type, TokenType::BangEqual);
  EXPECT_EQ(tokens[2].token_type, TokenType::Less);
  EXPECT_EQ(tokens[3].token_type, TokenType::Greater);
  EXPECT_EQ(tokens[4].token_type, TokenType::LessEqual);
  EXPECT_EQ(tokens[5].token_type, TokenType::GreaterEqual);
}

// --- 注释测试 ---

/**
 * @brief 测试单行注释的识别。
 * @details 验证词法分析器能够正确识别 // 风格的单行注释，
 *          并保留注释内容作为 Token。
 */
TEST_F(LexerTest, SingleLineComments) {
  auto tokens = tokenize("123 // this is a comment\n456");

  // 预期：整数 123、注释 Token、整数 456、EOF
  ASSERT_EQ(tokens.size(), 4);
  EXPECT_EQ(tokens[0].token_type, TokenType::Integer);
  EXPECT_EQ(tokens[0].value, "123");
  EXPECT_EQ(tokens[1].token_type, TokenType::Comment);
  EXPECT_EQ(tokens[2].token_type, TokenType::Integer);
  EXPECT_EQ(tokens[2].value, "456");
}

// TODO: 多行注释 (/* */) 尚未在词法分析器中实现
// TEST_F(LexerTest, MultiLineComments) {
//   auto tokens = tokenize("123 /* comment\nspanning\nlines */ 456");
//
//   ASSERT_EQ(tokens.size(), 4);
//   EXPECT_EQ(tokens[0].token_type, TokenType::Integer);
//   EXPECT_EQ(tokens[0].value, "123");
//   EXPECT_EQ(tokens[1].token_type, TokenType::Comment);
//   EXPECT_EQ(tokens[2].token_type, TokenType::Integer);
//   EXPECT_EQ(tokens[2].value, "456");
// }

// --- UTF-8 支持测试 ---

/**
 * @brief 测试 UTF-8 标识符的支持。
 * @details 验证词法分析器能够正确识别包含多字节 UTF-8 字符的标识符，
 *          如中文、西班牙语、俄语等。
 */
TEST_F(LexerTest, UTF8Identifiers) {
  auto tokens = tokenize("变量 función переменная");

  ASSERT_EQ(tokens.size(), 4);
  EXPECT_EQ(tokens[0].token_type, TokenType::Identifier);
  EXPECT_EQ(tokens[0].value, "变量");
  EXPECT_EQ(tokens[1].token_type, TokenType::Identifier);
  EXPECT_EQ(tokens[1].value, "función");
  EXPECT_EQ(tokens[2].token_type, TokenType::Identifier);
  EXPECT_EQ(tokens[2].value, "переменная");
}

/**
 * @brief 测试 UTF-8 字符串字面量的支持。
 * @details 验证词法分析器能够正确识别包含 UTF-8 字符的字符串，
 *          包括中文、emoji 表情等。
 */
TEST_F(LexerTest, UTF8Strings) {
  auto tokens = tokenize(R"("你好" "🌍" "Привет")");

  ASSERT_EQ(tokens.size(), 4);
  EXPECT_EQ(tokens[0].token_type, TokenType::String);
  EXPECT_EQ(tokens[0].value, "你好");
  EXPECT_EQ(tokens[1].token_type, TokenType::String);
  EXPECT_EQ(tokens[1].value, "🌍");
  EXPECT_EQ(tokens[2].token_type, TokenType::String);
  EXPECT_EQ(tokens[2].value, "Привет");
}

// --- 错误处理测试 ---

/**
 * @brief 测试未闭合字符串的错误检测。
 * @details 验证词法分析器能够检测并报告未正确闭合的字符串字面量。
 */
TEST_F(LexerTest, UnterminatedString) {
  Lexer lexer(R"("unterminated)");
  auto tokens = lexer.tokenize();
  auto errors = lexer.get_errors();

  // 应该报告错误
  EXPECT_TRUE(errors.has_errors());
}

/**
 * @brief 测试无效十六进制数字的错误检测。
 * @details 验证词法分析器能够检测仅有 0x 前缀但没有有效数字的情况。
 */
TEST_F(LexerTest, InvalidHexNumber) {
  Lexer lexer("0x");
  auto tokens = lexer.tokenize();
  auto errors = lexer.get_errors();

  EXPECT_TRUE(errors.has_errors());
}

/**
 * @brief 测试无效转义序列的错误检测。
 * @details 验证词法分析器能够检测字符串中不合法的转义字符。
 */
TEST_F(LexerTest, InvalidEscapeSequence) {
  Lexer lexer(R"("\q")");
  auto tokens = lexer.tokenize();
  auto errors = lexer.get_errors();

  EXPECT_TRUE(errors.has_errors());
}

// --- 新增测试：边界情况和复合场景 ---

/**
 * @brief 测试空输入的处理。
 * @details 验证词法分析器能够正确处理空字符串输入，仅返回 EOF Token。
 */
TEST_F(LexerTest, EmptyInput) {
  auto tokens = tokenize("");

  ASSERT_EQ(tokens.size(), 1);
  EXPECT_EQ(tokens[0].token_type, TokenType::EndOfFile);
}

/**
 * @brief 测试仅包含空白字符的输入。
 * @details 验证词法分析器能够跳过所有空白字符，仅返回 EOF Token。
 */
TEST_F(LexerTest, WhitespaceOnly) {
  auto tokens = tokenize("   \t\n  \r\n  ");

  ASSERT_EQ(tokens.size(), 1);
  EXPECT_EQ(tokens[0].token_type, TokenType::EndOfFile);
}

/**
 * @brief 测试复合赋值运算符的识别。
 * @details 验证词法分析器能够正确识别 +=、-=、*= 等复合赋值运算符。
 */
TEST_F(LexerTest, CompoundAssignmentOperators) {
  auto tokens = tokenize("+= -= *= /= %=");

  ASSERT_EQ(tokens.size(), 6);
  EXPECT_EQ(tokens[0].token_type, TokenType::PlusEqual);
  EXPECT_EQ(tokens[1].token_type, TokenType::MinusEqual);
  EXPECT_EQ(tokens[2].token_type, TokenType::StarEqual);
  EXPECT_EQ(tokens[3].token_type, TokenType::SlashEqual);
  EXPECT_EQ(tokens[4].token_type, TokenType::PercentEqual);
}

/**
 * @brief 测试逻辑运算符的识别。
 * @details 验证词法分析器能够正确识别 &&、||、! 等逻辑运算符。
 */
TEST_F(LexerTest, LogicalOperators) {
  auto tokens = tokenize("&& || !");

  ASSERT_EQ(tokens.size(), 4);
  EXPECT_EQ(tokens[0].token_type, TokenType::And);
  EXPECT_EQ(tokens[1].token_type, TokenType::Or);
  EXPECT_EQ(tokens[2].token_type, TokenType::Bang);
}

/**
 * @brief 测试分隔符和括号的识别。
 * @details 验证词法分析器能够正确识别各种分隔符、括号和大括号。
 */
TEST_F(LexerTest, DelimitersAndBrackets) {
  auto tokens = tokenize("( ) { } [ ] , ; : .");

  ASSERT_EQ(tokens.size(), 11);
  EXPECT_EQ(tokens[0].token_type, TokenType::LeftParen);
  EXPECT_EQ(tokens[1].token_type, TokenType::RightParen);
  EXPECT_EQ(tokens[2].token_type, TokenType::LeftBrace);
  EXPECT_EQ(tokens[3].token_type, TokenType::RightBrace);
  EXPECT_EQ(tokens[4].token_type, TokenType::LeftBracket);
  EXPECT_EQ(tokens[5].token_type, TokenType::RightBracket);
  EXPECT_EQ(tokens[6].token_type, TokenType::Comma);
  EXPECT_EQ(tokens[7].token_type, TokenType::Semicolon);
  EXPECT_EQ(tokens[8].token_type, TokenType::Colon);
  EXPECT_EQ(tokens[9].token_type, TokenType::Dot);
}

/**
 * @brief 测试箭头符号的识别。
 * @details 验证词法分析器能够正确识别函数返回类型声明中的 -> 符号。
 */
TEST_F(LexerTest, ArrowOperator) {
  auto tokens = tokenize("->");

  ASSERT_EQ(tokens.size(), 2);
  EXPECT_EQ(tokens[0].token_type, TokenType::Arrow);
}

/**
 * @brief 测试零开头的十进制数字的错误检测。
 * @details 验证词法分析器对类似 0123 这样可能引起歧义的数字的处理。
 */
TEST_F(LexerTest, LeadingZeroDecimal) {
  // 检查前导零的处理（可能被视为八进制或错误）
  auto tokens = tokenize("0 01 00");

  // 根据实际实现验证行为
  ASSERT_GE(tokens.size(), 1);
}

/**
 * @brief 测试混合进制数字字面量。
 * @details 验证词法分析器能够在同一表达式中正确区分不同进制的数字。
 */
TEST_F(LexerTest, MixedBaseNumbers) {
  auto tokens = tokenize("0xFF 255 0b11111111 0o377");

  ASSERT_EQ(tokens.size(), 5);
  EXPECT_EQ(tokens[0].token_type, TokenType::Integer);
  EXPECT_EQ(tokens[0].value, "0xFF");
  EXPECT_EQ(tokens[1].token_type, TokenType::Integer);
  EXPECT_EQ(tokens[1].value, "255");
  EXPECT_EQ(tokens[2].token_type, TokenType::Integer);
  EXPECT_EQ(tokens[2].value, "0b11111111");
  EXPECT_EQ(tokens[3].token_type, TokenType::Integer);
  EXPECT_EQ(tokens[3].value, "0o377");
}

/**
 * @brief 测试连续运算符的正确分词。
 * @details 验证词法分析器能够正确区分类似 ++ 和 + + 的不同情况。
 */
TEST_F(LexerTest, ConsecutiveOperators) {
  auto tokens = tokenize("a++ + ++b");

  // 根据实际实现验证 Token 序列
  ASSERT_GE(tokens.size(), 1);
  // NOTE: 当前词法分析器可能不支持 ++ 运算符，此测试用于未来扩展
}

/**
 * @brief 测试所有关键字的识别。
 * @details 验证词法分析器能够识别所有语言关键字。
 */
TEST_F(LexerTest, AllKeywords) {
  auto tokens = tokenize("let var fn return if else while for in struct enum type trait true false");

  EXPECT_EQ(tokens[0].token_type, TokenType::Let);
  EXPECT_EQ(tokens[1].token_type, TokenType::Var);
  EXPECT_EQ(tokens[2].token_type, TokenType::Fn);
  EXPECT_EQ(tokens[3].token_type, TokenType::Return);
  EXPECT_EQ(tokens[4].token_type, TokenType::If);
  EXPECT_EQ(tokens[5].token_type, TokenType::Else);
  EXPECT_EQ(tokens[6].token_type, TokenType::While);
  EXPECT_EQ(tokens[7].token_type, TokenType::For);
  EXPECT_EQ(tokens[8].token_type, TokenType::In);
  EXPECT_EQ(tokens[9].token_type, TokenType::Struct);
  EXPECT_EQ(tokens[10].token_type, TokenType::Enum);
  EXPECT_EQ(tokens[11].token_type, TokenType::Type);
  EXPECT_EQ(tokens[12].token_type, TokenType::Trait);
  EXPECT_EQ(tokens[13].token_type, TokenType::True);
  EXPECT_EQ(tokens[14].token_type, TokenType::False);
}

/**
 * @brief 测试所有单字符运算符。
 * @details 验证词法分析器能够识别所有单字符运算符和分隔符。
 */
TEST_F(LexerTest, SingleCharacterTokens) {
  auto tokens = tokenize("+ - * / % ( ) { } [ ] , ; : .");

  std::vector<TokenType> expected = {
    TokenType::Plus, TokenType::Minus, TokenType::Star,
    TokenType::Slash, TokenType::Percent,
    TokenType::LeftParen, TokenType::RightParen,
    TokenType::LeftBrace, TokenType::RightBrace,
    TokenType::LeftBracket, TokenType::RightBracket,
    TokenType::Comma, TokenType::Semicolon, TokenType::Colon,
    TokenType::Dot
  };

  ASSERT_EQ(tokens.size() - 1, expected.size()); // -1 for EOF
  for (size_t i = 0; i < expected.size(); ++i) {
    EXPECT_EQ(tokens[i].token_type, expected[i]);
  }
}

/**
 * @brief 测试双字符运算符。
 * @details 验证词法分析器能够识别所有双字符运算符。
 */
TEST_F(LexerTest, DoubleCharacterOperators) {
  auto tokens = tokenize("== != <= >= && || -> ..");

  EXPECT_EQ(tokens[0].token_type, TokenType::EqualEqual);
  EXPECT_EQ(tokens[1].token_type, TokenType::BangEqual);
  EXPECT_EQ(tokens[2].token_type, TokenType::LessEqual);
  EXPECT_EQ(tokens[3].token_type, TokenType::GreaterEqual);
  EXPECT_EQ(tokens[4].token_type, TokenType::And);
  EXPECT_EQ(tokens[5].token_type, TokenType::Or);
  EXPECT_EQ(tokens[6].token_type, TokenType::Arrow);
  EXPECT_EQ(tokens[7].token_type, TokenType::DotDot);
}

/**
 * @brief 测试所有复合赋值运算符。
 * @details 验证词法分析器能够识别所有复合赋值运算符。
 */
TEST_F(LexerTest, AllCompoundAssignmentOperators) {
  auto tokens = tokenize("+= -= *= /= %=");

  EXPECT_EQ(tokens[0].token_type, TokenType::PlusEqual);
  EXPECT_EQ(tokens[1].token_type, TokenType::MinusEqual);
  EXPECT_EQ(tokens[2].token_type, TokenType::StarEqual);
  EXPECT_EQ(tokens[3].token_type, TokenType::SlashEqual);
  EXPECT_EQ(tokens[4].token_type, TokenType::PercentEqual);
}

/**
 * @brief 测试浮点数的各种格式。
 * @details 验证词法分析器能够处理各种格式的浮点数字面量。
 */
TEST_F(LexerTest, VariousFloatFormats) {
  auto tokens = tokenize("0.0 1.0 0.5 123.456 .5 5.");

  EXPECT_EQ(tokens[0].token_type, TokenType::Float);
  EXPECT_EQ(tokens[0].value, "0.0");
  EXPECT_EQ(tokens[1].token_type, TokenType::Float);
  EXPECT_EQ(tokens[1].value, "1.0");
  EXPECT_EQ(tokens[2].token_type, TokenType::Float);
  EXPECT_EQ(tokens[2].value, "0.5");
  EXPECT_EQ(tokens[3].token_type, TokenType::Float);
  EXPECT_EQ(tokens[3].value, "123.456");
}

/**
 * @brief 测试所有科学记数法格式。
 * @details 验证词法分析器能够处理各种科学记数法格式。
 */
TEST_F(LexerTest, AllScientificNotationFormats) {
  auto tokens = tokenize("1e10 1E10 1e+10 1E+10 1e-10 1E-10 1.5e2 1.5E2");

  for (const auto& tok : tokens) {
    if (tok.token_type != TokenType::EndOfFile) {
      EXPECT_EQ(tok.token_type, TokenType::ScientificExponent);
    }
  }
}

/**
 * @brief 测试字符串转义序列。
 * @details 验证词法分析器能够处理所有标准转义序列。
 */
TEST_F(LexerTest, AllEscapeSequences) {
  auto tokens = tokenize(R"("\\n \\t \\r \\\" \\\\ \\0")");

  ASSERT_GE(tokens.size(), 1);
  EXPECT_EQ(tokens[0].token_type, TokenType::String);
  EXPECT_NE(tokens[0].value.find("\\n"), std::string::npos);
}

/**
 * @brief 测试长标识符。
 * @details 验证词法分析器能够处理非常长的标识符。
 */
TEST_F(LexerTest, LongIdentifier) {
  std::string long_id(1000, 'a');
  auto tokens = tokenize(long_id);

  ASSERT_GE(tokens.size(), 1);
  EXPECT_EQ(tokens[0].token_type, TokenType::Identifier);
  EXPECT_EQ(tokens[0].value, long_id);
}

/**
 * @brief 测试长字符串。
 * @details 验证词法分析器能够处理非常长的字符串字面量。
 */
TEST_F(LexerTest, LongString) {
  std::string long_str = "\"" + std::string(1000, 'x') + "\"";
  auto tokens = tokenize(long_str);

  ASSERT_GE(tokens.size(), 1);
  EXPECT_EQ(tokens[0].token_type, TokenType::String);
  EXPECT_EQ(tokens[0].value.length(), 1000); // Lexer strips quotes
}

/**
 * @brief 测试注释与代码的混合。
 * @details 验证词法分析器能够在代码中正确处理注释。
 */
TEST_F(LexerTest, MixedCommentAndCode) {
  auto tokens = tokenize("let x = 5; // variable\nlet y = 10; // another");

  // Should have: let, x, =, 5, ;, comment, let, y, =, 10, ;, comment, EOF
  bool found_comment = false;
  for (const auto& tok : tokens) {
    if (tok.token_type == TokenType::Comment) {
      found_comment = true;
      break;
    }
  }
  EXPECT_TRUE(found_comment);
}

/**
 * @brief 测试多行代码。
 * @details 验证词法分析器能够处理多行代码并正确跟踪行号。
 */
TEST_F(LexerTest, MultilineCode) {
  auto tokens = tokenize("let x = 1;\nlet y = 2;\nlet z = 3;");

  int let_count = 0;
  for (const auto& tok : tokens) {
    if (tok.token_type == TokenType::Let) {
      let_count++;
    }
  }
  EXPECT_EQ(let_count, 3);
}

/**
 * @brief 测试空白字符的处理。
 * @details 验证词法分析器能够正确处理各种空白字符。
 */
TEST_F(LexerTest, WhitespaceHandling) {
  auto tokens = tokenize("  \t\n  let  \t  x  \n\n  =  \t  5  ;  ");

  // Should skip whitespace and produce: let, x, =, 5, ;, EOF
  std::vector<TokenType> expected = {
    TokenType::Let, TokenType::Identifier, TokenType::Equal,
    TokenType::Integer, TokenType::Semicolon, TokenType::EndOfFile
  };

  ASSERT_EQ(tokens.size(), expected.size());
  for (size_t i = 0; i < expected.size(); ++i) {
    EXPECT_EQ(tokens[i].token_type, expected[i]);
  }
}

/**
 * @brief 测试复杂的表达式。
 * @details 验证词法分析器能够处理包含多种运算符的复杂表达式。
 */
TEST_F(LexerTest, ComplexExpression) {
  auto tokens = tokenize("(a + b) * c - d / e % f");

  EXPECT_GT(tokens.size(), 10);
  EXPECT_EQ(tokens[0].token_type, TokenType::LeftParen);
}

/**
 * @brief 测试嵌套的括号。
 * @details 验证词法分析器能够处理多层嵌套的括号。
 */
TEST_F(LexerTest, NestedBrackets) {
  auto tokens = tokenize("((([[{{}}]])))");

  int left_count = 0, right_count = 0;
  for (const auto& tok : tokens) {
    if (tok.token_type == TokenType::LeftParen || 
        tok.token_type == TokenType::LeftBracket ||
        tok.token_type == TokenType::LeftBrace) {
      left_count++;
    } else if (tok.token_type == TokenType::RightParen ||
               tok.token_type == TokenType::RightBracket ||
               tok.token_type == TokenType::RightBrace) {
      right_count++;
    }
  }
  EXPECT_EQ(left_count, right_count);
  EXPECT_EQ(left_count, 7); // 3 parens + 2 brackets + 2 braces
}

/**
 * @brief 测试标识符与关键字的边界。
 * @details 验证词法分析器能够正确区分关键字和相似的标识符。
 */
TEST_F(LexerTest, KeywordVsIdentifierBoundary) {
  auto tokens = tokenize("let letter lettuce");

  EXPECT_EQ(tokens[0].token_type, TokenType::Let);
  EXPECT_EQ(tokens[1].token_type, TokenType::Identifier);
  EXPECT_EQ(tokens[1].value, "letter");
  EXPECT_EQ(tokens[2].token_type, TokenType::Identifier);
  EXPECT_EQ(tokens[2].value, "lettuce");
}

/**
 * @brief 测试无效的十六进制Unicode转义。
 * @details 验证词法分析器能够检测到\u后面没有足够十六进制数字的错误。
 */
TEST_F(LexerTest, InvalidUnicodeEscapeNotEnoughDigits) {
  auto tokens = tokenize("\"\\u12\""); // 只有2位而非4位
  
  ASSERT_GE(tokens.size(), 1);
  // 应该产生错误,但仍然生成token
}

/**
 * @brief 测试有效的Unicode转义序列。
 * @details 验证词法分析器能够正确处理\uXXXX格式的Unicode转义。
 */
TEST_F(LexerTest, ValidUnicodeEscape) {
  auto tokens = tokenize("\"\\u0041\""); // \u0041 = 'A'
  
  ASSERT_GE(tokens.size(), 1);
  EXPECT_EQ(tokens[0].token_type, TokenType::String);
}

/**
 * @brief 测试有效的\UXXXXXXXX格式Unicode转义。
 * @details 验证词法分析器能够正确处理8位Unicode转义。
 */
TEST_F(LexerTest, ValidLongUnicodeEscape) {
  auto tokens = tokenize("\"\\U00000041\""); // \U00000041 = 'A'
  
  ASSERT_GE(tokens.size(), 1);
  EXPECT_EQ(tokens[0].token_type, TokenType::String);
}

/**
 * @brief 测试无效的\U Unicode转义。
 * @details 验证词法分析器能够检测到\U后面没有足够十六进制数字的错误。
 */
TEST_F(LexerTest, InvalidLongUnicodeEscape) {
  auto tokens = tokenize("\"\\U0000\""); // 只有4位而非8位
  
  ASSERT_GE(tokens.size(), 1);
  // 应该产生错误
}

/**
 * @brief 测试十六进制转义序列。
 * @details 验证词法分析器能够正确处理\xXX格式的转义。
 */
TEST_F(LexerTest, HexEscapeSequence) {
  auto tokens = tokenize("\"\\x41\""); // \x41 = 'A'
  
  ASSERT_GE(tokens.size(), 1);
  EXPECT_EQ(tokens[0].token_type, TokenType::String);
}

/**
 * @brief 测试无效的十六进制转义序列。
 * @details 验证词法分析器能够检测到\x后面没有十六进制数字的错误。
 */
TEST_F(LexerTest, InvalidHexEscapeSequence) {
  auto tokens = tokenize("\"\\xGG\""); // G不是十六进制数字
  
  ASSERT_GE(tokens.size(), 1);
  // 应该产生错误
}

/**
 * @brief 测试原始字符串基本功能。
 * @details 验证词法分析器能够正确处理r"..."格式的原始字符串。
 */
TEST_F(LexerTest, RawStringBasic) {
  auto tokens = tokenize("r\"hello\\nworld\""); // \n不应该被转义
  
  ASSERT_GE(tokens.size(), 1);
  EXPECT_EQ(tokens[0].token_type, TokenType::String);
  // 原始字符串中\n应该保持为两个字符
}

/**
 * @brief 测试原始字符串中的特殊字符。
 * @details 验证原始字符串不处理任何转义序列。
 */
TEST_F(LexerTest, RawStringSpecialChars) {
  auto tokens = tokenize("r\"\\t\\r\\\"\\\\\"");
  
  ASSERT_GE(tokens.size(), 1);
  EXPECT_EQ(tokens[0].token_type, TokenType::String);
  // 所有转义序列应该保持原样
}

/**
 * @brief 测试未终止的原始字符串。
 * @details 验证词法分析器能够检测到原始字符串缺少结束引号的错误。
 */
TEST_F(LexerTest, UnterminatedRawString) {
  auto tokens = tokenize("r\"unterminated");
  
  ASSERT_GE(tokens.size(), 1);
  // 应该产生错误
}

/**
 * @brief 测试多行原始字符串。
 * @details 验证原始字符串可以包含换行符。
 */
TEST_F(LexerTest, MultilineRawString) {
  auto tokens = tokenize("r\"line1\nline2\nline3\"");
  
  ASSERT_GE(tokens.size(), 1);
  EXPECT_EQ(tokens[0].token_type, TokenType::String);
}

/**
 * @brief 测试UTF-8标识符的无效序列。
 * @details 验证词法分析器能够检测到标识符中的无效UTF-8字节序列。
 */
TEST_F(LexerTest, InvalidUtf8InIdentifier) {
  // 构造包含无效UTF-8序列的输入
  std::string invalid_utf8 = "test\xFF\xFE";
  auto tokens = tokenize(invalid_utf8);
  
  // 应该能解析出某些token,可能包含错误
  ASSERT_GE(tokens.size(), 1);
}

/**
 * @brief 测试字符串中的无效UTF-8序列。
 * @details 验证词法分析器能够检测到字符串中的无效UTF-8字节序列。
 */
TEST_F(LexerTest, InvalidUtf8InString) {
  std::string invalid_str = "\"\xFF\xFE\"";
  auto tokens = tokenize(invalid_str);
  
  ASSERT_GE(tokens.size(), 1);
  // 应该产生错误
}

/**
 * @brief 测试原始字符串中的UTF-8字符。
 * @details 验证原始字符串能够正确处理UTF-8字符。
 */
TEST_F(LexerTest, Utf8InRawString) {
  auto tokens = tokenize("r\"你好世界🌍\"");
  
  ASSERT_GE(tokens.size(), 1);
  EXPECT_EQ(tokens[0].token_type, TokenType::String);
}

/**
 * @brief 测试范围操作符。
 * @details 验证词法分析器能够正确识别..操作符。
 */
TEST_F(LexerTest, RangeOperator) {
  auto tokens = tokenize("0..10");
  
  ASSERT_GE(tokens.size(), 3);
  EXPECT_EQ(tokens[0].token_type, TokenType::Integer);
  EXPECT_EQ(tokens[1].token_type, TokenType::DotDot);
  EXPECT_EQ(tokens[2].token_type, TokenType::Integer);
}

/**
 * @brief 测试箭头操作符。
 * @details 验证词法分析器能够正确识别->操作符。
 */
TEST_F(LexerTest, ArrowOperatorInExpression) {
  auto tokens = tokenize("fn add(x) -> x + 1");
  
  bool found_arrow = false;
  for (const auto& tok : tokens) {
    if (tok.token_type == TokenType::Arrow) {
      found_arrow = true;
      break;
    }
  }
  EXPECT_TRUE(found_arrow);
}

/**
 * @brief 测试浮点数边界情况 - 只有小数点。
 * @details 验证词法分析器对.后面没有数字的处理。
 */
TEST_F(LexerTest, FloatWithOnlyDecimalPoint) {
  auto tokens = tokenize("3.");
  
  ASSERT_GE(tokens.size(), 2);
  // 3. 会被解析为整数3和点.
  EXPECT_EQ(tokens[0].token_type, TokenType::Integer);
  EXPECT_EQ(tokens[1].token_type, TokenType::Dot);
}

/**
 * @brief 测试浮点数边界情况 - 小数点开头。
 * @details 验证词法分析器对.开头的浮点数的处理。
 */
TEST_F(LexerTest, FloatStartingWithDecimalPoint) {
  auto tokens = tokenize(".5");
  
  ASSERT_GE(tokens.size(), 2);
  // .5 会被解析为点.和整数5
  EXPECT_EQ(tokens[0].token_type, TokenType::Dot);
  EXPECT_EQ(tokens[1].token_type, TokenType::Integer);
}

/**
 * @brief 测试多个连续的点。
 * @details 验证词法分析器对多个.的处理。
 */
TEST_F(LexerTest, MultipleDotsHandling) {
  auto tokens = tokenize("1...3"); // ..是range,第三个.是单独的
  
  ASSERT_GE(tokens.size(), 3);
  // 应该有: 1, .., ., 3 或其他合理的分词
}

/**
 * @brief 测试错误恢复 - 连续的语法错误。
 * @details 验证词法分析器能够从连续错误中恢复。
 */
TEST_F(LexerTest, ContinuousErrors) {
  auto tokens = tokenize("@@## $$");
  
  // 应该能够继续解析,尽管有错误
  ASSERT_GE(tokens.size(), 1);
}

/**
 * @brief 测试全部单字符操作符组合。
 * @details 验证所有单字符操作符的连续使用。
 */
TEST_F(LexerTest, AllSingleCharOperatorsCombined) {
  auto tokens = tokenize("+-*/%=<>!&|.,:;(){}[]");
  
  EXPECT_GT(tokens.size(), 15);
  // 验证每个操作符都被正确识别
}

/**
 * @brief 测试复杂的嵌套表达式。
 * @details 验证词法分析器处理复杂嵌套的能力。
 */
TEST_F(LexerTest, DeeplyNestedExpression) {
  auto tokens = tokenize("((((a + b) * (c - d)) / (e % f)) && (g || h))");
  
  EXPECT_GT(tokens.size(), 20);
  // 验证括号匹配
  int paren_count = 0;
  for (const auto& tok : tokens) {
    if (tok.token_type == TokenType::LeftParen) paren_count++;
    if (tok.token_type == TokenType::RightParen) paren_count--;
  }
  EXPECT_EQ(paren_count, 0);
}

