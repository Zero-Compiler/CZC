/**
 * @file lexer.cpp
 * @brief 词法分析器主类的实现。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-29
 *
 * Lexer 是词法分析的门面类（Facade），协调各个 Scanner 组件完成词法分析。
 * 支持两种模式：
 * - 基础模式：快速扫描，忽略空白和注释
 * - Trivia 模式：保留空白和注释信息，用于 IDE 和格式化工具
 */

#include "czc/lexer/lexer.hpp"

namespace czc::lexer {

Lexer::Lexer(SourceManager &sm, BufferID buffer)
    : sm_(sm), reader_(sm, buffer), errors_(), identScanner_(),
      numberScanner_(), stringScanner_(), commentScanner_(), charScanner_() {}

Token Lexer::nextToken() {
  // 跳过空白和注释
  skipWhitespaceAndComments();

  // 检查是否到达文件末尾
  if (reader_.isAtEnd()) {
    return Token::makeEof(reader_.location());
  }

  // 扫描下一个 token
  return scanToken();
}

std::vector<Token> Lexer::tokenize() {
  std::vector<Token> tokens;
  tokens.reserve(1024); // 预分配以减少重新分配

  while (true) {
    Token token = nextToken();
    TokenType type = token.type();
    tokens.push_back(std::move(token));

    if (type == TokenType::TOKEN_EOF) {
      break;
    }
  }

  return tokens;
}

Token Lexer::nextTokenWithTrivia() {
  // 收集前置 trivia
  std::vector<Trivia> leadingTrivia = collectLeadingTrivia();

  // 检查是否到达文件末尾
  if (reader_.isAtEnd()) {
    Token eof = Token::makeEof(reader_.location());
    eof.setLeadingTrivia(std::move(leadingTrivia));
    return eof;
  }

  // 扫描下一个 token
  Token token = scanToken();

  // 设置前置 trivia
  token.setLeadingTrivia(std::move(leadingTrivia));

  // 收集并设置后置 trivia
  std::vector<Trivia> trailingTrivia = collectTrailingTrivia();
  token.setTrailingTrivia(std::move(trailingTrivia));

  return token;
}

std::vector<Token> Lexer::tokenizeWithTrivia() {
  std::vector<Token> tokens;
  tokens.reserve(1024);

  while (true) {
    Token token = nextTokenWithTrivia();
    TokenType type = token.type();
    tokens.push_back(std::move(token));

    if (type == TokenType::TOKEN_EOF) {
      break;
    }
  }

  return tokens;
}

std::span<const LexerError> Lexer::errors() const noexcept {
  return errors_.errors();
}

bool Lexer::hasErrors() const noexcept { return errors_.hasErrors(); }

void Lexer::skipWhitespaceAndComments() {
  ScanContext ctx(reader_, errors_);

  while (true) {
    // 跳过空白
    skipWhitespace();

    // 检查是否是注释
    if (commentScanner_.canScan(ctx)) {
      static_cast<void>(commentScanner_.scan(ctx));
      continue;
    }

    break;
  }
}

void Lexer::skipWhitespace() {
  while (!reader_.isAtEnd()) {
    auto ch = reader_.current();
    if (!ch.has_value()) {
      break;
    }

    char c = ch.value();
    if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
      reader_.advance();
    } else {
      break;
    }
  }
}

std::vector<Trivia> Lexer::collectLeadingTrivia() {
  std::vector<Trivia> trivias;
  ScanContext ctx(reader_, errors_);

  while (!reader_.isAtEnd()) {
    auto ch = reader_.current();
    if (!ch.has_value()) {
      break;
    }

    char c = ch.value();

    // 空白 trivia
    if (c == ' ' || c == '\t') {
      std::size_t start = reader_.offset();
      while (!reader_.isAtEnd()) {
        auto next = reader_.current();
        if (!next.has_value())
          break;
        char nc = next.value();
        if (nc != ' ' && nc != '\t')
          break;
        reader_.advance();
      }
      Trivia ws{};
      ws.kind = Trivia::Kind::kWhitespace;
      ws.buffer = reader_.buffer();
      ws.offset = static_cast<std::uint32_t>(start);
      ws.length = static_cast<std::uint16_t>(reader_.offset() - start);
      trivias.push_back(ws);
      continue;
    }

    // 换行 trivia
    if (c == '\n' || c == '\r') {
      std::size_t start = reader_.offset();
      reader_.advance();
      Trivia nl{};
      nl.kind = Trivia::Kind::kNewline;
      nl.buffer = reader_.buffer();
      nl.offset = static_cast<std::uint32_t>(start);
      nl.length = 1;
      trivias.push_back(nl);
      continue;
    }

    // 注释 trivia
    if (commentScanner_.canScan(ctx)) {
      std::size_t start = reader_.offset();
      Token comment = commentScanner_.scan(ctx);
      std::size_t length = reader_.offset() - start;

      Trivia cmt{};
      cmt.kind = Trivia::Kind::kComment;
      cmt.buffer = reader_.buffer();
      cmt.offset = static_cast<std::uint32_t>(start);
      cmt.length = static_cast<std::uint16_t>(length);
      trivias.push_back(cmt);
      continue;
    }

    // 遇到非 trivia 字符，结束
    break;
  }

  return trivias;
}

std::vector<Trivia> Lexer::collectTrailingTrivia() {
  std::vector<Trivia> trivias;
  ScanContext ctx(reader_, errors_);

  // 后置 trivia 只收集同一行的空白和行尾注释
  while (!reader_.isAtEnd()) {
    auto ch = reader_.current();
    if (!ch.has_value()) {
      break;
    }

    char c = ch.value();

    // 空白（不含换行）
    if (c == ' ' || c == '\t') {
      std::size_t start = reader_.offset();
      while (!reader_.isAtEnd()) {
        auto next = reader_.current();
        if (!next.has_value())
          break;
        char nc = next.value();
        if (nc != ' ' && nc != '\t')
          break;
        reader_.advance();
      }
      Trivia ws{};
      ws.kind = Trivia::Kind::kWhitespace;
      ws.buffer = reader_.buffer();
      ws.offset = static_cast<std::uint32_t>(start);
      ws.length = static_cast<std::uint16_t>(reader_.offset() - start);
      trivias.push_back(ws);
      continue;
    }

    // 行注释
    auto next = reader_.peek(1);
    if (c == '/' && next.has_value() && next.value() == '/') {
      std::size_t start = reader_.offset();
      static_cast<void>(commentScanner_.scan(ctx));
      std::size_t length = reader_.offset() - start;
      Trivia cmt{};
      cmt.kind = Trivia::Kind::kComment;
      cmt.buffer = reader_.buffer();
      cmt.offset = static_cast<std::uint32_t>(start);
      cmt.length = static_cast<std::uint16_t>(length);
      trivias.push_back(cmt);
      continue;
    }

    // 遇到换行或其他字符，结束后置 trivia
    break;
  }

  return trivias;
}

Token Lexer::scanToken() {
  ScanContext ctx(reader_, errors_);

  // 按优先级尝试各个 scanner

  // 1. 字符串字面量
  if (stringScanner_.canScan(ctx)) {
    return stringScanner_.scan(ctx);
  }

  // 2. 标识符
  if (identScanner_.canScan(ctx)) {
    return identScanner_.scan(ctx);
  }

  // 3. 数字字面量
  if (numberScanner_.canScan(ctx)) {
    return numberScanner_.scan(ctx);
  }

  // 4. 运算符和分隔符
  if (charScanner_.canScan(ctx)) {
    return charScanner_.scan(ctx);
  }

  // 5. 未知字符
  return scanUnknown(ctx);
}

Token Lexer::scanUnknown(ScanContext &ctx) {
  std::size_t startOffset = ctx.offset();
  SourceLocation startLoc = ctx.location();

  auto ch = ctx.current();
  if (ch.has_value()) {
    // 单个无效字符，长度为 1
    errors_.add(LexerError::make(LexerErrorCode::InvalidCharacter, startLoc, 1,
                                 "invalid character '{}'", ch.value()));
    ctx.advance();
  }

  return ctx.makeUnknown(startOffset, startLoc);
}

void Lexer::normalizeNewlines() {
  // \r\n 到 \n 的规范化在 SourceReader::advance() 中处理
}

} // namespace czc::lexer
