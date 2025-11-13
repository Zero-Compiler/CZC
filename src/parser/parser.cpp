/**
 * @file parser.cpp
 * @brief `Parser` 类的功能实现。
 * @author BegoniaHe
 * @date 2025-11-11
 */

#include "czc/parser/parser.hpp"

#include "czc/diagnostics/diagnostic_code.hpp"

#include <algorithm>
#include <unordered_set>

namespace czc::parser {

using namespace czc::cst;
using namespace czc::diagnostics;
using namespace czc::lexer;
using namespace czc::utils;

Parser::Parser(const std::vector<Token>& tokens, const std::string& filename)
    : tokens(tokens), current(0), filename(filename) {}

Token Parser::current_token() const {
  if (current < tokens.size()) {
    return tokens[current];
  }
  // NOTE: 返回一个 EOF Token 作为哨兵（Sentinel）。这简化了调用方的代码，
  //       使其不必在每次调用前都检查是否已到达 Token 流的末尾。
  return Token::makeEOF();
}

Token Parser::peek(size_t offset) const {
  size_t index = current + offset;
  if (index < tokens.size()) {
    return tokens[index];
  }
  return Token::makeEOF();
}

Token Parser::advance() {
  Token token = current_token();
  if (current < tokens.size()) {
    current++;
  }
  return token;
}

bool Parser::check(TokenType type) const {
  return current_token().token_type == type;
}

bool Parser::match_token(const std::vector<TokenType>& types) {
  for (const auto& type : types) {
    if (check(type)) {
      advance();
      return true;
    }
  }
  return false;
}

std::optional<Token> Parser::consume(TokenType type) {
  if (check(type)) {
    return advance();
  }

  // 记录错误
  std::vector<std::string> args = {
      token_type_to_string(type),                      // 期望的 token
      token_type_to_string(current_token().token_type) // 实际的 token
  };
  report_error(DiagnosticCode::P0001_UnexpectedToken, make_location(), args);

  // --- 错误恢复策略 ---
  // 根据现代编译器实践（rustc, Swift），我们在某些关键位置
  // 返回虚拟 Token 以便解析器能够继续工作并发现更多错误。

  // 如果期望的是分号，跳过到下一个语句
  if (type == TokenType::Semicolon) {
    synchronize_to_semicolon();
    // 返回虚拟分号以继续解析（标记为 synthetic）
    return Token(TokenType::Semicolon, ";", current_token().line,
                 current_token().column, true);
  }

  // 如果期望的是右括号、右方括号、右大括号，尝试同步
  if (type == TokenType::RightParen || type == TokenType::RightBracket ||
      type == TokenType::RightBrace) {
    // 在匹配的分隔符丢失时，返回虚拟 Token（标记为 synthetic）
    return Token(type, "", current_token().line, current_token().column, true);
  }

  return std::nullopt;
}

void Parser::synchronize_to_semicolon() {
  while (!check(TokenType::EndOfFile)) {
    // 如果找到分号，消费它并停止
    if (check(TokenType::Semicolon)) {
      advance();
      return;
    }

    // 如果遇到可能是下一个语句的开始，停止
    if (check(TokenType::RightBrace) || check(TokenType::Let) ||
        check(TokenType::Var) || check(TokenType::Fn) ||
        check(TokenType::Return) || check(TokenType::If) ||
        check(TokenType::While)) {
      return;
    }

    advance();
  }
}

void Parser::synchronize_to_statement_start() {
  while (!check(TokenType::EndOfFile)) {
    Token current = current_token();

    // 停在语句关键字
    if (current.token_type == TokenType::Let ||
        current.token_type == TokenType::Var ||
        current.token_type == TokenType::Fn ||
        current.token_type == TokenType::Return ||
        current.token_type == TokenType::If ||
        current.token_type == TokenType::While ||
        current.token_type == TokenType::RightBrace) {
      return;
    }

    // 如果找到分号，消费它并停止
    if (current.token_type == TokenType::Semicolon) {
      advance();
      return;
    }

    advance();
  }
}

void Parser::synchronize_to_block_end() {
  int brace_depth = 1; // 我们已经在一个代码块内

  while (!check(TokenType::EndOfFile) && brace_depth > 0) {
    if (check(TokenType::LeftBrace)) {
      brace_depth++;
    } else if (check(TokenType::RightBrace)) {
      brace_depth--;
      if (brace_depth == 0) {
        return; // 找到匹配的右大括号，但不消费它
      }
    }
    advance();
  }
}

void Parser::report_error(DiagnosticCode code, const SourceLocation& location,
                          const std::vector<std::string>& args) {
  ParserError error(code, location, args);
  error_collector.add(error);
}

SourceLocation Parser::make_location() const {
  Token token = current_token();
  return SourceLocation(filename, token.line, token.column);
}

std::unique_ptr<CSTNode> Parser::parse() {
  auto program = make_cst_node(CSTNodeType::Program, make_location());

  while (!check(TokenType::EndOfFile)) {
    // 处理注释：将注释作为 CST 节点添加到程序中
    if (check(TokenType::Comment)) {
      auto comment_token = advance();
      auto comment_node = make_cst_node(CSTNodeType::Comment, comment_token);
      program->add_child(std::move(comment_node));
      continue;
    }

    auto stmt = declaration();
    if (stmt) {
      program->add_child(std::move(stmt));
    } else {
      // --- 增强的错误恢复 ---
      // 当声明解析失败时，使用专门的同步方法恢复到下一个语句开始
      synchronize_to_statement_start();
    }
  }

  return program;
}

std::unique_ptr<CSTNode>
Parser::parse_array_suffix(std::unique_ptr<CSTNode> base_type) {
  // NOTE: 此函数处理类型表达式后的数组声明符，支持多维数组。
  //       通过循环包装 base_type，直到没有更多左方括号为止。
  while (check(TokenType::LeftBracket)) {
    Token left_bracket = advance();

    if (check(TokenType::Integer)) {
      // 固定大小数组 T[N]
      auto sized_array =
          make_cst_node(CSTNodeType::SizedArrayType, make_location());
      sized_array->add_child(std::move(base_type));

      auto lbracket_node = make_cst_node(CSTNodeType::Delimiter, left_bracket);
      sized_array->add_child(std::move(lbracket_node));

      Token size_token = advance();
      auto size_node = make_cst_node(CSTNodeType::IntegerLiteral, size_token);
      sized_array->add_child(std::move(size_node));

      auto right_bracket = consume(TokenType::RightBracket);
      if (right_bracket) {
        auto rbracket_node =
            make_cst_node(CSTNodeType::Delimiter, *right_bracket);
        sized_array->add_child(std::move(rbracket_node));
      }

      base_type = std::move(sized_array);
    } else {
      // 动态数组 T[]
      auto array_type = make_cst_node(CSTNodeType::ArrayType, make_location());
      array_type->add_child(std::move(base_type));

      auto lbracket_node = make_cst_node(CSTNodeType::Delimiter, left_bracket);
      array_type->add_child(std::move(lbracket_node));

      auto right_bracket = consume(TokenType::RightBracket);
      if (right_bracket) {
        auto rbracket_node =
            make_cst_node(CSTNodeType::Delimiter, *right_bracket);
        array_type->add_child(std::move(rbracket_node));
      }

      base_type = std::move(array_type);
    }
  }

  return base_type;
}

} // namespace czc::parser
