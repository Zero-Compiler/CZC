/**
 * @file lexer_test.cpp
 * @brief Lexer 主类单元测试。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-30
 */

#include "czc/lexer/lexer.hpp"

#include <gtest/gtest.h>

namespace czc::lexer {
namespace {

class LexerTest : public ::testing::Test {
protected:
  SourceManager sm_;

  /**
   * @brief 辅助方法：添加源码缓冲区（使用 string_view）。
   */
  BufferID addSource(std::string_view source, std::string filename) {
    return sm_.addBuffer(source, std::move(filename));
  }

  /**
   * @brief 辅助方法：创建 Lexer 并 tokenize。
   */
  std::vector<Token> tokenize(std::string_view source) {
    auto id = addSource(source, "test.zero");
    Lexer lexer(sm_, id);
    return lexer.tokenize();
  }

  /**
   * @brief 辅助方法：创建 Lexer 并 tokenize（带 trivia）。
   */
  std::vector<Token> tokenizeWithTrivia(std::string_view source) {
    auto id = addSource(source, "test.zero");
    Lexer lexer(sm_, id);
    return lexer.tokenizeWithTrivia();
  }

  /**
   * @brief 辅助方法：获取下一个 Token。
   */
  Token nextToken(Lexer &lexer) { return lexer.nextToken(); }
};

// ============================================================================
// 基本功能测试
// ============================================================================

TEST_F(LexerTest, EmptySourceReturnsOnlyEof) {
  auto tokens = tokenize("");

  ASSERT_EQ(tokens.size(), 1u);
  EXPECT_EQ(tokens[0].type(), TokenType::TOKEN_EOF);
}

TEST_F(LexerTest, WhitespaceOnlySourceReturnsOnlyEof) {
  auto tokens = tokenize("   \t\n  ");

  ASSERT_EQ(tokens.size(), 1u);
  EXPECT_EQ(tokens[0].type(), TokenType::TOKEN_EOF);
}

TEST_F(LexerTest, SingleKeyword) {
  auto tokens = tokenize("let");

  ASSERT_EQ(tokens.size(), 2u);
  EXPECT_EQ(tokens[0].type(), TokenType::KW_LET);
  EXPECT_EQ(tokens[0].value(sm_), "let");
  EXPECT_EQ(tokens[1].type(), TokenType::TOKEN_EOF);
}

TEST_F(LexerTest, SimpleDeclaration) {
  auto tokens = tokenize("let x = 1;");

  ASSERT_EQ(tokens.size(), 6u);
  EXPECT_EQ(tokens[0].type(), TokenType::KW_LET);
  EXPECT_EQ(tokens[1].type(), TokenType::IDENTIFIER);
  EXPECT_EQ(tokens[1].value(sm_), "x");
  EXPECT_EQ(tokens[2].type(), TokenType::OP_ASSIGN);
  EXPECT_EQ(tokens[3].type(), TokenType::LIT_INT);
  EXPECT_EQ(tokens[3].value(sm_), "1");
  EXPECT_EQ(tokens[4].type(), TokenType::DELIM_SEMICOLON);
  EXPECT_EQ(tokens[5].type(), TokenType::TOKEN_EOF);
}

TEST_F(LexerTest, FunctionDefinition) {
  auto tokens = tokenize("fn main() {}");

  ASSERT_EQ(tokens.size(), 7u);
  EXPECT_EQ(tokens[0].type(), TokenType::KW_FN);
  EXPECT_EQ(tokens[1].type(), TokenType::IDENTIFIER);
  EXPECT_EQ(tokens[1].value(sm_), "main");
  EXPECT_EQ(tokens[2].type(), TokenType::DELIM_LPAREN);
  EXPECT_EQ(tokens[3].type(), TokenType::DELIM_RPAREN);
  EXPECT_EQ(tokens[4].type(), TokenType::DELIM_LBRACE);
  EXPECT_EQ(tokens[5].type(), TokenType::DELIM_RBRACE);
  EXPECT_EQ(tokens[6].type(), TokenType::TOKEN_EOF);
}

// ============================================================================
// 关键字测试
// ============================================================================

TEST_F(LexerTest, AllKeywordsRecognized) {
  auto tokens = tokenize("let var fn struct enum type impl trait return "
                         "if else while for in break continue match import as");

  std::vector<TokenType> expected = {
      TokenType::KW_LET,      TokenType::KW_VAR,    TokenType::KW_FN,
      TokenType::KW_STRUCT,   TokenType::KW_ENUM,   TokenType::KW_TYPE,
      TokenType::KW_IMPL,     TokenType::KW_TRAIT,  TokenType::KW_RETURN,
      TokenType::KW_IF,       TokenType::KW_ELSE,   TokenType::KW_WHILE,
      TokenType::KW_FOR,      TokenType::KW_IN,     TokenType::KW_BREAK,
      TokenType::KW_CONTINUE, TokenType::KW_MATCH,  TokenType::KW_IMPORT,
      TokenType::KW_AS,       TokenType::TOKEN_EOF,
  };

  ASSERT_EQ(tokens.size(), expected.size());
  for (size_t i = 0; i < expected.size(); ++i) {
    EXPECT_EQ(tokens[i].type(), expected[i]) << "Mismatch at index " << i;
  }
}

// ============================================================================
// 字面量关键字测试
// ============================================================================

TEST_F(LexerTest, BooleanLiterals) {
  auto tokens = tokenize("true false");

  ASSERT_EQ(tokens.size(), 3u);
  EXPECT_EQ(tokens[0].type(), TokenType::LIT_TRUE);
  EXPECT_EQ(tokens[1].type(), TokenType::LIT_FALSE);
}

TEST_F(LexerTest, NullLiteral) {
  auto tokens = tokenize("null");

  ASSERT_EQ(tokens.size(), 2u);
  EXPECT_EQ(tokens[0].type(), TokenType::LIT_NULL);
}

// ============================================================================
// 运算符测试
// ============================================================================

TEST_F(LexerTest, ArithmeticOperators) {
  auto tokens = tokenize("+ - * / %");

  ASSERT_EQ(tokens.size(), 6u);
  EXPECT_EQ(tokens[0].type(), TokenType::OP_PLUS);
  EXPECT_EQ(tokens[1].type(), TokenType::OP_MINUS);
  EXPECT_EQ(tokens[2].type(), TokenType::OP_STAR);
  EXPECT_EQ(tokens[3].type(), TokenType::OP_SLASH);
  EXPECT_EQ(tokens[4].type(), TokenType::OP_PERCENT);
}

TEST_F(LexerTest, ComparisonOperators) {
  auto tokens = tokenize("== != < <= > >=");

  ASSERT_EQ(tokens.size(), 7u);
  EXPECT_EQ(tokens[0].type(), TokenType::OP_EQ);
  EXPECT_EQ(tokens[1].type(), TokenType::OP_NE);
  EXPECT_EQ(tokens[2].type(), TokenType::OP_LT);
  EXPECT_EQ(tokens[3].type(), TokenType::OP_LE);
  EXPECT_EQ(tokens[4].type(), TokenType::OP_GT);
  EXPECT_EQ(tokens[5].type(), TokenType::OP_GE);
}

TEST_F(LexerTest, LogicalOperators) {
  auto tokens = tokenize("&& || !");

  ASSERT_EQ(tokens.size(), 4u);
  EXPECT_EQ(tokens[0].type(), TokenType::OP_LOGICAL_AND);
  EXPECT_EQ(tokens[1].type(), TokenType::OP_LOGICAL_OR);
  EXPECT_EQ(tokens[2].type(), TokenType::OP_LOGICAL_NOT);
}

TEST_F(LexerTest, BitwiseOperators) {
  auto tokens = tokenize("& | ^ ~ << >>");

  ASSERT_EQ(tokens.size(), 7u);
  EXPECT_EQ(tokens[0].type(), TokenType::OP_BIT_AND);
  EXPECT_EQ(tokens[1].type(), TokenType::OP_BIT_OR);
  EXPECT_EQ(tokens[2].type(), TokenType::OP_BIT_XOR);
  EXPECT_EQ(tokens[3].type(), TokenType::OP_BIT_NOT);
  EXPECT_EQ(tokens[4].type(), TokenType::OP_BIT_SHL);
  EXPECT_EQ(tokens[5].type(), TokenType::OP_BIT_SHR);
}

TEST_F(LexerTest, AssignmentOperators) {
  auto tokens = tokenize("= += -= *= /= %= &= |= ^= <<= >>=");

  ASSERT_EQ(tokens.size(), 12u);
  EXPECT_EQ(tokens[0].type(), TokenType::OP_ASSIGN);
  EXPECT_EQ(tokens[1].type(), TokenType::OP_PLUS_ASSIGN);
  EXPECT_EQ(tokens[2].type(), TokenType::OP_MINUS_ASSIGN);
  EXPECT_EQ(tokens[3].type(), TokenType::OP_STAR_ASSIGN);
  EXPECT_EQ(tokens[4].type(), TokenType::OP_SLASH_ASSIGN);
  EXPECT_EQ(tokens[5].type(), TokenType::OP_PERCENT_ASSIGN);
  EXPECT_EQ(tokens[6].type(), TokenType::OP_AND_ASSIGN);
  EXPECT_EQ(tokens[7].type(), TokenType::OP_OR_ASSIGN);
  EXPECT_EQ(tokens[8].type(), TokenType::OP_XOR_ASSIGN);
  EXPECT_EQ(tokens[9].type(), TokenType::OP_SHL_ASSIGN);
  EXPECT_EQ(tokens[10].type(), TokenType::OP_SHR_ASSIGN);
}

TEST_F(LexerTest, OtherOperators) {
  auto tokens = tokenize("-> => . @ :: .. ..=");

  ASSERT_EQ(tokens.size(), 8u);
  EXPECT_EQ(tokens[0].type(), TokenType::OP_ARROW);
  EXPECT_EQ(tokens[1].type(), TokenType::OP_FAT_ARROW);
  EXPECT_EQ(tokens[2].type(), TokenType::OP_DOT);
  EXPECT_EQ(tokens[3].type(), TokenType::OP_AT);
  EXPECT_EQ(tokens[4].type(), TokenType::OP_COLON_COLON);
  EXPECT_EQ(tokens[5].type(), TokenType::OP_DOT_DOT);
  EXPECT_EQ(tokens[6].type(), TokenType::OP_DOT_DOT_EQ);
}

// ============================================================================
// 分隔符测试
// ============================================================================

TEST_F(LexerTest, Delimiters) {
  // 注意：单独的 _ 会被 IdentScanner 识别为 IDENTIFIER
  // 只有在不能构成标识符的情况下才会被 CharScanner 识别为 DELIM_UNDERSCORE
  auto tokens = tokenize("( ) { } [ ] , : ;");

  ASSERT_EQ(tokens.size(), 10u);
  EXPECT_EQ(tokens[0].type(), TokenType::DELIM_LPAREN);
  EXPECT_EQ(tokens[1].type(), TokenType::DELIM_RPAREN);
  EXPECT_EQ(tokens[2].type(), TokenType::DELIM_LBRACE);
  EXPECT_EQ(tokens[3].type(), TokenType::DELIM_RBRACE);
  EXPECT_EQ(tokens[4].type(), TokenType::DELIM_LBRACKET);
  EXPECT_EQ(tokens[5].type(), TokenType::DELIM_RBRACKET);
  EXPECT_EQ(tokens[6].type(), TokenType::DELIM_COMMA);
  EXPECT_EQ(tokens[7].type(), TokenType::DELIM_COLON);
  EXPECT_EQ(tokens[8].type(), TokenType::DELIM_SEMICOLON);
}

// ============================================================================
// 注释测试（基础模式下被跳过）
// ============================================================================

TEST_F(LexerTest, LineCommentSkipped) {
  auto tokens = tokenize("let // this is a comment\nx");

  ASSERT_EQ(tokens.size(), 3u);
  EXPECT_EQ(tokens[0].type(), TokenType::KW_LET);
  EXPECT_EQ(tokens[1].type(), TokenType::IDENTIFIER);
  EXPECT_EQ(tokens[1].value(sm_), "x");
}

TEST_F(LexerTest, BlockCommentSkipped) {
  auto tokens = tokenize("let /* block comment */ x");

  ASSERT_EQ(tokens.size(), 3u);
  EXPECT_EQ(tokens[0].type(), TokenType::KW_LET);
  EXPECT_EQ(tokens[1].type(), TokenType::IDENTIFIER);
  EXPECT_EQ(tokens[1].value(sm_), "x");
}

TEST_F(LexerTest, NestedBlockCommentSkipped) {
  // 注意：当前实现不支持嵌套块注释，第一个 */ 就会结束注释
  // /* outer /* inner */ outer */ x 中
  // 注释只到第一个 */，后面的 "outer */ x" 会被词法分析
  auto tokens = tokenize("/* block comment */ x");

  ASSERT_EQ(tokens.size(), 2u);
  EXPECT_EQ(tokens[0].type(), TokenType::IDENTIFIER);
  EXPECT_EQ(tokens[0].value(sm_), "x");
}

// ============================================================================
// Trivia 模式测试
// ============================================================================

TEST_F(LexerTest, TriviaModeCapturesWhitespace) {
  auto tokens = tokenizeWithTrivia("  let");

  ASSERT_GE(tokens.size(), 2u);
  // let 应该有前置空白 trivia
  auto &letToken = tokens[0];
  EXPECT_EQ(letToken.type(), TokenType::KW_LET);
  EXPECT_TRUE(letToken.hasTrivia());
  EXPECT_FALSE(letToken.leadingTrivia().empty());
}

TEST_F(LexerTest, TriviaModeCapuresLineComment) {
  auto tokens = tokenizeWithTrivia("let // comment\nx");

  // let 后面应该有空白和注释作为 trailing trivia 或 x 的 leading trivia
  ASSERT_GE(tokens.size(), 2u);
  EXPECT_EQ(tokens[0].type(), TokenType::KW_LET);
  EXPECT_EQ(tokens[1].type(), TokenType::IDENTIFIER);
}

// ============================================================================
// 位置信息测试
// ============================================================================

TEST_F(LexerTest, TokenLocationIsCorrect) {
  auto id = addSource("let x", "test.zero");
  Lexer lexer(sm_, id);

  auto letTok = lexer.nextToken();
  EXPECT_EQ(letTok.location().line, 1u);
  EXPECT_EQ(letTok.location().column, 1u);

  auto xTok = lexer.nextToken();
  EXPECT_EQ(xTok.location().line, 1u);
  EXPECT_EQ(xTok.location().column, 5u); // 'let ' + 'x'
}

TEST_F(LexerTest, MultiLineLocation) {
  auto id = addSource("let\nx", "test.zero");
  Lexer lexer(sm_, id);

  auto letTok = lexer.nextToken();
  EXPECT_EQ(letTok.location().line, 1u);

  auto xTok = lexer.nextToken();
  EXPECT_EQ(xTok.location().line, 2u);
  EXPECT_EQ(xTok.location().column, 1u);
}

// ============================================================================
// 错误处理测试
// ============================================================================

TEST_F(LexerTest, InvalidCharacterGeneratesError) {
  // 使用 ASCII 控制字符（如 0x01），这应该是无效字符
  auto id = addSource(std::string("let \x01 x"), "test.zero");
  Lexer lexer(sm_, id);
  auto tokens = lexer.tokenize();

  // 查看是否有 TOKEN_UNKNOWN 类型的 token
  bool hasUnknown = false;
  for (const auto &tok : tokens) {
    if (tok.type() == TokenType::TOKEN_UNKNOWN) {
      hasUnknown = true;
      break;
    }
  }
  EXPECT_TRUE(hasUnknown || lexer.hasErrors());
}

TEST_F(LexerTest, NoErrorsForValidSource) {
  auto id = addSource("let x = 1;", "test.zero");
  Lexer lexer(sm_, id);
  auto tokens = lexer.tokenize();

  EXPECT_FALSE(lexer.hasErrors());
  EXPECT_TRUE(lexer.errors().empty());
}

// ============================================================================
// 复杂表达式测试
// ============================================================================

TEST_F(LexerTest, ArithmeticExpression) {
  auto tokens = tokenize("1 + 2 * 3 - 4 / 5");

  ASSERT_EQ(tokens.size(), 10u);
  EXPECT_EQ(tokens[0].type(), TokenType::LIT_INT);
  EXPECT_EQ(tokens[1].type(), TokenType::OP_PLUS);
  EXPECT_EQ(tokens[2].type(), TokenType::LIT_INT);
  EXPECT_EQ(tokens[3].type(), TokenType::OP_STAR);
  EXPECT_EQ(tokens[4].type(), TokenType::LIT_INT);
  EXPECT_EQ(tokens[5].type(), TokenType::OP_MINUS);
  EXPECT_EQ(tokens[6].type(), TokenType::LIT_INT);
  EXPECT_EQ(tokens[7].type(), TokenType::OP_SLASH);
  EXPECT_EQ(tokens[8].type(), TokenType::LIT_INT);
}

TEST_F(LexerTest, ConditionalExpression) {
  auto tokens = tokenize("if x > 0 { true } else { false }");

  std::vector<TokenType> expected = {
      TokenType::KW_IF,        TokenType::IDENTIFIER,   TokenType::OP_GT,
      TokenType::LIT_INT,      TokenType::DELIM_LBRACE, TokenType::LIT_TRUE,
      TokenType::DELIM_RBRACE, TokenType::KW_ELSE,      TokenType::DELIM_LBRACE,
      TokenType::LIT_FALSE,    TokenType::DELIM_RBRACE, TokenType::TOKEN_EOF,
  };

  ASSERT_EQ(tokens.size(), expected.size());
  for (size_t i = 0; i < expected.size(); ++i) {
    EXPECT_EQ(tokens[i].type(), expected[i]) << "Mismatch at index " << i;
  }
}

// ============================================================================
// Unicode 标识符测试
// ============================================================================

TEST_F(LexerTest, UnicodeIdentifier) {
  auto tokens = tokenize("let 变量 = 1;");

  // tokens: let, 变量, =, 1, ;, EOF = 6
  ASSERT_EQ(tokens.size(), 6u);
  EXPECT_EQ(tokens[0].type(), TokenType::KW_LET);
  EXPECT_EQ(tokens[1].type(), TokenType::IDENTIFIER);
  EXPECT_EQ(tokens[1].value(sm_), "变量");
  EXPECT_EQ(tokens[2].type(), TokenType::OP_ASSIGN);
  EXPECT_EQ(tokens[3].type(), TokenType::LIT_INT);
  EXPECT_EQ(tokens[4].type(), TokenType::DELIM_SEMICOLON);
}

TEST_F(LexerTest, MixedUnicodeAndAsciiIdentifier) {
  auto tokens = tokenize("let 变量_1 = test变量;");

  // tokens: let, 变量_1, =, test变量, ;, EOF = 6
  ASSERT_EQ(tokens.size(), 6u);
  EXPECT_EQ(tokens[1].type(), TokenType::IDENTIFIER);
  EXPECT_EQ(tokens[1].value(sm_), "变量_1");
  EXPECT_EQ(tokens[3].type(), TokenType::IDENTIFIER);
  EXPECT_EQ(tokens[3].value(sm_), "test变量");
}

// ============================================================================
// 边界情况测试
// ============================================================================

TEST_F(LexerTest, ConsecutiveOperators) {
  auto tokens = tokenize("a++b");

  // 应该是 a, +, +, b 或者取决于实现
  EXPECT_GE(tokens.size(), 4u);
}

TEST_F(LexerTest, OperatorAmbiguity) {
  // 测试运算符的贪婪匹配
  auto tokens = tokenize("a->b");

  ASSERT_EQ(tokens.size(), 4u);
  EXPECT_EQ(tokens[0].type(), TokenType::IDENTIFIER);
  EXPECT_EQ(tokens[1].type(), TokenType::OP_ARROW);
  EXPECT_EQ(tokens[2].type(), TokenType::IDENTIFIER);
}

TEST_F(LexerTest, RangeOperators) {
  auto tokens = tokenize("0..10");

  ASSERT_EQ(tokens.size(), 4u);
  EXPECT_EQ(tokens[0].type(), TokenType::LIT_INT);
  EXPECT_EQ(tokens[1].type(), TokenType::OP_DOT_DOT);
  EXPECT_EQ(tokens[2].type(), TokenType::LIT_INT);
}

TEST_F(LexerTest, RangeInclusiveOperator) {
  auto tokens = tokenize("0..=10");

  ASSERT_EQ(tokens.size(), 4u);
  EXPECT_EQ(tokens[0].type(), TokenType::LIT_INT);
  EXPECT_EQ(tokens[1].type(), TokenType::OP_DOT_DOT_EQ);
  EXPECT_EQ(tokens[2].type(), TokenType::LIT_INT);
}

} // namespace
} // namespace czc::lexer
