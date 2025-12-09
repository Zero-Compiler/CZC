/**
 * @file token_test.cpp
 * @brief Token 相关类型的单元测试。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-30
 */

#include "czc/lexer/token.hpp"

#include <gtest/gtest.h>

namespace czc::lexer {
namespace {

// ============================================================================
// SourceLocation 测试
// ============================================================================

TEST(SourceLocationTest, DefaultConstructorCreatesInvalidLocation) {
  SourceLocation loc;
  EXPECT_FALSE(loc.isValid());
  EXPECT_EQ(loc.line, 1u);
  EXPECT_EQ(loc.column, 1u);
  EXPECT_EQ(loc.offset, 0u);
}

TEST(SourceLocationTest, ParameterizedConstructor) {
  BufferID buf{1};
  SourceLocation loc(buf, 10, 5, 100);

  EXPECT_TRUE(loc.isValid());
  EXPECT_EQ(loc.buffer.value, 1u);
  EXPECT_EQ(loc.line, 10u);
  EXPECT_EQ(loc.column, 5u);
  EXPECT_EQ(loc.offset, 100u);
}

// ============================================================================
// Trivia 测试
// ============================================================================

class TriviaTest : public ::testing::Test {
protected:
  SourceManager sm_;

  BufferID addSource(std::string_view source, std::string filename) {
    return sm_.addBuffer(source, std::move(filename));
  }
};

TEST_F(TriviaTest, WhitespaceTriviaTextExtraction) {
  auto id = addSource("  hello", "test.zero");

  Trivia ws{};
  ws.kind = Trivia::Kind::kWhitespace;
  ws.buffer = id;
  ws.offset = 0;
  ws.length = 2;

  EXPECT_EQ(ws.text(sm_), "  ");
}

TEST_F(TriviaTest, NewlineTriviaKind) {
  Trivia nl{};
  nl.kind = Trivia::Kind::kNewline;
  EXPECT_EQ(nl.kind, Trivia::Kind::kNewline);
}

TEST_F(TriviaTest, CommentTriviaKind) {
  Trivia cmt{};
  cmt.kind = Trivia::Kind::kComment;
  EXPECT_EQ(cmt.kind, Trivia::Kind::kComment);
}

// ============================================================================
// TokenSpan 测试
// ============================================================================

TEST(TokenSpanTest, DefaultConstructor) {
  TokenSpan span;
  EXPECT_EQ(span.offset, 0u);
  EXPECT_EQ(span.length, 0u);
}

TEST(TokenSpanTest, ParameterizedConstructor) {
  BufferID buf{1};
  SourceLocation loc(buf, 1, 1, 0);
  TokenSpan span(buf, 10, 5, loc);

  EXPECT_EQ(span.buffer.value, 1u);
  EXPECT_EQ(span.offset, 10u);
  EXPECT_EQ(span.length, 5u);
}

// ============================================================================
// Token 测试
// ============================================================================

class TokenTest : public ::testing::Test {
protected:
  SourceManager sm_;

  BufferID addSource(std::string_view source, std::string filename) {
    return sm_.addBuffer(source, std::move(filename));
  }
};

TEST_F(TokenTest, ConstructWithTokenSpan) {
  auto id = addSource("let x = 1;", "test.zero");
  SourceLocation loc(id, 1, 1, 0);
  TokenSpan span(id, 0, 3, loc);

  Token tok(TokenType::KW_LET, span);

  EXPECT_EQ(tok.type(), TokenType::KW_LET);
  EXPECT_EQ(tok.buffer(), id);
  EXPECT_EQ(tok.offset(), 0u);
  EXPECT_EQ(tok.length(), 3u);
  EXPECT_EQ(tok.value(sm_), "let");
}

TEST_F(TokenTest, ConstructWithExplicitFields) {
  auto id = addSource("identifier", "test.zero");
  SourceLocation loc(id, 1, 1, 0);

  Token tok(TokenType::IDENTIFIER, id, 0, 10, loc);

  EXPECT_EQ(tok.type(), TokenType::IDENTIFIER);
  EXPECT_EQ(tok.value(sm_), "identifier");
}

TEST_F(TokenTest, MakeEof) {
  auto id = addSource("", "test.zero");
  SourceLocation loc(id, 1, 1, 0);

  auto eof = Token::makeEof(loc);

  EXPECT_EQ(eof.type(), TokenType::TOKEN_EOF);
  EXPECT_EQ(eof.length(), 0u);
}

TEST_F(TokenTest, MakeUnknown) {
  auto id = addSource("@", "test.zero");
  SourceLocation loc(id, 1, 1, 0);
  TokenSpan span(id, 0, 1, loc);

  auto unknown = Token::makeUnknown(span);

  EXPECT_EQ(unknown.type(), TokenType::TOKEN_UNKNOWN);
}

TEST_F(TokenTest, RawLiteralForStrings) {
  auto id = addSource("\"hello\"", "test.zero");
  SourceLocation loc(id, 1, 1, 0);

  // Token value 只包含字符串内容，rawLiteral 包含引号
  Token tok(TokenType::LIT_STRING, id, 1, 5, loc); // "hello" 中的 hello
  tok.setRawLiteral(0, 7);                         // 包含引号

  EXPECT_EQ(tok.value(sm_), "hello");
  EXPECT_EQ(tok.rawLiteral(sm_), "\"hello\"");
}

TEST_F(TokenTest, TriviaManagement) {
  auto id = addSource("  let", "test.zero");
  SourceLocation loc(id, 1, 3, 2);
  TokenSpan span(id, 2, 3, loc);

  Token tok(TokenType::KW_LET, span);

  EXPECT_FALSE(tok.hasTrivia());
  EXPECT_TRUE(tok.leadingTrivia().empty());
  EXPECT_TRUE(tok.trailingTrivia().empty());

  // 添加前置 trivia
  Trivia ws{};
  ws.kind = Trivia::Kind::kWhitespace;
  ws.buffer = id;
  ws.offset = 0;
  ws.length = 2;
  tok.addLeadingTrivia(ws);

  EXPECT_TRUE(tok.hasTrivia());
  EXPECT_EQ(tok.leadingTrivia().size(), 1u);
}

TEST_F(TokenTest, SetTriviaWithMoveSemantics) {
  auto id = addSource("let", "test.zero");
  SourceLocation loc(id, 1, 1, 0);
  TokenSpan span(id, 0, 3, loc);

  Token tok(TokenType::KW_LET, span);

  std::vector<Trivia> trivias;
  Trivia ws{};
  ws.kind = Trivia::Kind::kWhitespace;
  trivias.push_back(ws);

  tok.setLeadingTrivia(std::move(trivias));

  EXPECT_EQ(tok.leadingTrivia().size(), 1u);
}

TEST_F(TokenTest, EscapeFlagsForStrings) {
  auto id = addSource("\"\\n\\t\"", "test.zero");
  SourceLocation loc(id, 1, 1, 0);
  TokenSpan span(id, 0, 6, loc);

  Token tok(TokenType::LIT_STRING, span);

  EXPECT_FALSE(tok.hasNamedEscape());
  EXPECT_FALSE(tok.hasHexEscape());
  EXPECT_FALSE(tok.hasUnicodeEscape());

  EscapeFlags flags;
  flags.set(kHasNamed);
  tok.setEscapeFlags(flags);

  EXPECT_TRUE(tok.hasNamedEscape());
  EXPECT_FALSE(tok.hasHexEscape());
}

TEST_F(TokenTest, MacroExpansionTracking) {
  auto id = addSource("x", "test.zero");
  SourceLocation loc(id, 1, 1, 0);
  TokenSpan span(id, 0, 1, loc);

  Token tok(TokenType::IDENTIFIER, span);

  EXPECT_FALSE(tok.isFromMacroExpansion());
  EXPECT_FALSE(tok.expansionId().isValid());

  tok.setExpansionId(ExpansionID{1});

  EXPECT_TRUE(tok.isFromMacroExpansion());
  EXPECT_TRUE(tok.expansionId().isValid());
}

// ============================================================================
// lookupKeyword 测试
// ============================================================================

TEST(LookupKeywordTest, ReturnsCorrectTokenTypeForKeywords) {
  EXPECT_EQ(lookupKeyword("let"), TokenType::KW_LET);
  EXPECT_EQ(lookupKeyword("var"), TokenType::KW_VAR);
  EXPECT_EQ(lookupKeyword("fn"), TokenType::KW_FN);
  EXPECT_EQ(lookupKeyword("struct"), TokenType::KW_STRUCT);
  EXPECT_EQ(lookupKeyword("enum"), TokenType::KW_ENUM);
  EXPECT_EQ(lookupKeyword("type"), TokenType::KW_TYPE);
  EXPECT_EQ(lookupKeyword("impl"), TokenType::KW_IMPL);
  EXPECT_EQ(lookupKeyword("trait"), TokenType::KW_TRAIT);
  EXPECT_EQ(lookupKeyword("return"), TokenType::KW_RETURN);
  EXPECT_EQ(lookupKeyword("if"), TokenType::KW_IF);
  EXPECT_EQ(lookupKeyword("else"), TokenType::KW_ELSE);
  EXPECT_EQ(lookupKeyword("while"), TokenType::KW_WHILE);
  EXPECT_EQ(lookupKeyword("for"), TokenType::KW_FOR);
  EXPECT_EQ(lookupKeyword("in"), TokenType::KW_IN);
  EXPECT_EQ(lookupKeyword("break"), TokenType::KW_BREAK);
  EXPECT_EQ(lookupKeyword("continue"), TokenType::KW_CONTINUE);
  EXPECT_EQ(lookupKeyword("match"), TokenType::KW_MATCH);
  EXPECT_EQ(lookupKeyword("import"), TokenType::KW_IMPORT);
  EXPECT_EQ(lookupKeyword("as"), TokenType::KW_AS);
}

TEST(LookupKeywordTest, ReturnsLiteralKeywords) {
  EXPECT_EQ(lookupKeyword("true"), TokenType::LIT_TRUE);
  EXPECT_EQ(lookupKeyword("false"), TokenType::LIT_FALSE);
  EXPECT_EQ(lookupKeyword("null"), TokenType::LIT_NULL);
}

TEST(LookupKeywordTest, ReturnsNulloptForNonKeywords) {
  EXPECT_EQ(lookupKeyword("hello"), std::nullopt);
  EXPECT_EQ(lookupKeyword("variable"), std::nullopt);
  EXPECT_EQ(lookupKeyword("Let"), std::nullopt); // 大小写敏感
  EXPECT_EQ(lookupKeyword("LET"), std::nullopt);
  EXPECT_EQ(lookupKeyword(""), std::nullopt);
}

// ============================================================================
// tokenTypeName 测试
// ============================================================================

TEST(TokenTypeNameTest, ReturnsCorrectNames) {
  EXPECT_EQ(tokenTypeName(TokenType::IDENTIFIER), "IDENTIFIER");
  EXPECT_EQ(tokenTypeName(TokenType::KW_LET), "KW_LET");
  EXPECT_EQ(tokenTypeName(TokenType::KW_FN), "KW_FN");
  EXPECT_EQ(tokenTypeName(TokenType::LIT_INT), "LIT_INT");
  EXPECT_EQ(tokenTypeName(TokenType::LIT_STRING), "LIT_STRING");
  EXPECT_EQ(tokenTypeName(TokenType::OP_PLUS), "OP_PLUS");
  EXPECT_EQ(tokenTypeName(TokenType::DELIM_LPAREN), "DELIM_LPAREN");
  EXPECT_EQ(tokenTypeName(TokenType::TOKEN_EOF), "TOKEN_EOF");
}

TEST(TokenTypeNameTest, AllKeywordNames) {
  EXPECT_EQ(tokenTypeName(TokenType::KW_VAR), "KW_VAR");
  EXPECT_EQ(tokenTypeName(TokenType::KW_STRUCT), "KW_STRUCT");
  EXPECT_EQ(tokenTypeName(TokenType::KW_ENUM), "KW_ENUM");
  EXPECT_EQ(tokenTypeName(TokenType::KW_TYPE), "KW_TYPE");
  EXPECT_EQ(tokenTypeName(TokenType::KW_IMPL), "KW_IMPL");
  EXPECT_EQ(tokenTypeName(TokenType::KW_TRAIT), "KW_TRAIT");
  EXPECT_EQ(tokenTypeName(TokenType::KW_RETURN), "KW_RETURN");
  EXPECT_EQ(tokenTypeName(TokenType::KW_IF), "KW_IF");
  EXPECT_EQ(tokenTypeName(TokenType::KW_ELSE), "KW_ELSE");
  EXPECT_EQ(tokenTypeName(TokenType::KW_WHILE), "KW_WHILE");
  EXPECT_EQ(tokenTypeName(TokenType::KW_FOR), "KW_FOR");
  EXPECT_EQ(tokenTypeName(TokenType::KW_IN), "KW_IN");
  EXPECT_EQ(tokenTypeName(TokenType::KW_BREAK), "KW_BREAK");
  EXPECT_EQ(tokenTypeName(TokenType::KW_CONTINUE), "KW_CONTINUE");
  EXPECT_EQ(tokenTypeName(TokenType::KW_MATCH), "KW_MATCH");
  EXPECT_EQ(tokenTypeName(TokenType::KW_IMPORT), "KW_IMPORT");
  EXPECT_EQ(tokenTypeName(TokenType::KW_AS), "KW_AS");
}

TEST(TokenTypeNameTest, AllCommentNames) {
  EXPECT_EQ(tokenTypeName(TokenType::COMMENT_LINE), "COMMENT_LINE");
  EXPECT_EQ(tokenTypeName(TokenType::COMMENT_BLOCK), "COMMENT_BLOCK");
  EXPECT_EQ(tokenTypeName(TokenType::COMMENT_DOC), "COMMENT_DOC");
}

TEST(TokenTypeNameTest, AllLiteralNames) {
  EXPECT_EQ(tokenTypeName(TokenType::LIT_FLOAT), "LIT_FLOAT");
  EXPECT_EQ(tokenTypeName(TokenType::LIT_DECIMAL), "LIT_DECIMAL");
  EXPECT_EQ(tokenTypeName(TokenType::LIT_RAW_STRING), "LIT_RAW_STRING");
  EXPECT_EQ(tokenTypeName(TokenType::LIT_TEX_STRING), "LIT_TEX_STRING");
  EXPECT_EQ(tokenTypeName(TokenType::LIT_TRUE), "LIT_TRUE");
  EXPECT_EQ(tokenTypeName(TokenType::LIT_FALSE), "LIT_FALSE");
  EXPECT_EQ(tokenTypeName(TokenType::LIT_NULL), "LIT_NULL");
}

TEST(TokenTypeNameTest, AllArithmeticOperatorNames) {
  EXPECT_EQ(tokenTypeName(TokenType::OP_MINUS), "OP_MINUS");
  EXPECT_EQ(tokenTypeName(TokenType::OP_STAR), "OP_STAR");
  EXPECT_EQ(tokenTypeName(TokenType::OP_SLASH), "OP_SLASH");
  EXPECT_EQ(tokenTypeName(TokenType::OP_PERCENT), "OP_PERCENT");
}

TEST(TokenTypeNameTest, AllComparisonOperatorNames) {
  EXPECT_EQ(tokenTypeName(TokenType::OP_EQ), "OP_EQ");
  EXPECT_EQ(tokenTypeName(TokenType::OP_NE), "OP_NE");
  EXPECT_EQ(tokenTypeName(TokenType::OP_LT), "OP_LT");
  EXPECT_EQ(tokenTypeName(TokenType::OP_LE), "OP_LE");
  EXPECT_EQ(tokenTypeName(TokenType::OP_GT), "OP_GT");
  EXPECT_EQ(tokenTypeName(TokenType::OP_GE), "OP_GE");
}

TEST(TokenTypeNameTest, AllLogicalOperatorNames) {
  EXPECT_EQ(tokenTypeName(TokenType::OP_LOGICAL_AND), "OP_LOGICAL_AND");
  EXPECT_EQ(tokenTypeName(TokenType::OP_LOGICAL_OR), "OP_LOGICAL_OR");
  EXPECT_EQ(tokenTypeName(TokenType::OP_LOGICAL_NOT), "OP_LOGICAL_NOT");
}

TEST(TokenTypeNameTest, AllBitwiseOperatorNames) {
  EXPECT_EQ(tokenTypeName(TokenType::OP_BIT_AND), "OP_BIT_AND");
  EXPECT_EQ(tokenTypeName(TokenType::OP_BIT_OR), "OP_BIT_OR");
  EXPECT_EQ(tokenTypeName(TokenType::OP_BIT_XOR), "OP_BIT_XOR");
  EXPECT_EQ(tokenTypeName(TokenType::OP_BIT_NOT), "OP_BIT_NOT");
  EXPECT_EQ(tokenTypeName(TokenType::OP_BIT_SHL), "OP_BIT_SHL");
  EXPECT_EQ(tokenTypeName(TokenType::OP_BIT_SHR), "OP_BIT_SHR");
}

TEST(TokenTypeNameTest, AllAssignmentOperatorNames) {
  EXPECT_EQ(tokenTypeName(TokenType::OP_ASSIGN), "OP_ASSIGN");
  EXPECT_EQ(tokenTypeName(TokenType::OP_PLUS_ASSIGN), "OP_PLUS_ASSIGN");
  EXPECT_EQ(tokenTypeName(TokenType::OP_MINUS_ASSIGN), "OP_MINUS_ASSIGN");
  EXPECT_EQ(tokenTypeName(TokenType::OP_STAR_ASSIGN), "OP_STAR_ASSIGN");
  EXPECT_EQ(tokenTypeName(TokenType::OP_SLASH_ASSIGN), "OP_SLASH_ASSIGN");
  EXPECT_EQ(tokenTypeName(TokenType::OP_PERCENT_ASSIGN), "OP_PERCENT_ASSIGN");
  EXPECT_EQ(tokenTypeName(TokenType::OP_AND_ASSIGN), "OP_AND_ASSIGN");
  EXPECT_EQ(tokenTypeName(TokenType::OP_OR_ASSIGN), "OP_OR_ASSIGN");
  EXPECT_EQ(tokenTypeName(TokenType::OP_XOR_ASSIGN), "OP_XOR_ASSIGN");
  EXPECT_EQ(tokenTypeName(TokenType::OP_SHL_ASSIGN), "OP_SHL_ASSIGN");
  EXPECT_EQ(tokenTypeName(TokenType::OP_SHR_ASSIGN), "OP_SHR_ASSIGN");
}

TEST(TokenTypeNameTest, AllOtherOperatorNames) {
  EXPECT_EQ(tokenTypeName(TokenType::OP_DOT_DOT), "OP_DOT_DOT");
  EXPECT_EQ(tokenTypeName(TokenType::OP_DOT_DOT_EQ), "OP_DOT_DOT_EQ");
  EXPECT_EQ(tokenTypeName(TokenType::OP_ARROW), "OP_ARROW");
  EXPECT_EQ(tokenTypeName(TokenType::OP_FAT_ARROW), "OP_FAT_ARROW");
  EXPECT_EQ(tokenTypeName(TokenType::OP_DOT), "OP_DOT");
  EXPECT_EQ(tokenTypeName(TokenType::OP_AT), "OP_AT");
  EXPECT_EQ(tokenTypeName(TokenType::OP_COLON_COLON), "OP_COLON_COLON");
}

TEST(TokenTypeNameTest, AllDelimiterNames) {
  EXPECT_EQ(tokenTypeName(TokenType::DELIM_RPAREN), "DELIM_RPAREN");
  EXPECT_EQ(tokenTypeName(TokenType::DELIM_LBRACE), "DELIM_LBRACE");
  EXPECT_EQ(tokenTypeName(TokenType::DELIM_RBRACE), "DELIM_RBRACE");
  EXPECT_EQ(tokenTypeName(TokenType::DELIM_LBRACKET), "DELIM_LBRACKET");
  EXPECT_EQ(tokenTypeName(TokenType::DELIM_RBRACKET), "DELIM_RBRACKET");
  EXPECT_EQ(tokenTypeName(TokenType::DELIM_COMMA), "DELIM_COMMA");
  EXPECT_EQ(tokenTypeName(TokenType::DELIM_COLON), "DELIM_COLON");
  EXPECT_EQ(tokenTypeName(TokenType::DELIM_SEMICOLON), "DELIM_SEMICOLON");
  EXPECT_EQ(tokenTypeName(TokenType::DELIM_UNDERSCORE), "DELIM_UNDERSCORE");
}

TEST(TokenTypeNameTest, AllReservedOperatorNames) {
  EXPECT_EQ(tokenTypeName(TokenType::OP_HASH), "OP_HASH");
  EXPECT_EQ(tokenTypeName(TokenType::OP_DOLLAR), "OP_DOLLAR");
  EXPECT_EQ(tokenTypeName(TokenType::OP_BACKSLASH), "OP_BACKSLASH");
}

TEST(TokenTypeNameTest, AllSpecialTokenNames) {
  EXPECT_EQ(tokenTypeName(TokenType::TOKEN_NEWLINE), "TOKEN_NEWLINE");
  EXPECT_EQ(tokenTypeName(TokenType::TOKEN_WHITESPACE), "TOKEN_WHITESPACE");
  EXPECT_EQ(tokenTypeName(TokenType::TOKEN_UNKNOWN), "TOKEN_UNKNOWN");
}

} // namespace
} // namespace czc::lexer
