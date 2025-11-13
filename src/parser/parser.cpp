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

namespace czc {
namespace parser {

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
  return Token(TokenType::EndOfFile, "", 0, 0);
}

Token Parser::peek(size_t offset) const {
  size_t index = current + offset;
  if (index < tokens.size()) {
    return tokens[index];
  }
  return Token(TokenType::EndOfFile, "", 0, 0);
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

    // 如果遇到可能是下一个语句的开始，停止（但不消费）
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
  error_collector.add(code, location, args);
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

std::unique_ptr<CSTNode> Parser::declaration() {
  if (match_token({TokenType::Let, TokenType::Var})) {
    return var_declaration();
  } else if (match_token({TokenType::Fn})) {
    return fn_declaration();
  } else if (match_token({TokenType::Struct})) {
    return struct_declaration();
  } else if (match_token({TokenType::Type})) {
    return type_alias_declaration();
  } else {
    return statement();
  }
}

std::unique_ptr<CSTNode> Parser::var_declaration() {
  auto node = make_cst_node(CSTNodeType::VarDeclaration, make_location());

  // NOTE: CST 的核心特性是保留所有语法符号。`match_token` 已经消耗了
  //       `let` 或 `var` 关键字，但我们通过 `tokens[current - 1]`
  //       回溯一个位置来获取它，并将其作为一个 `Delimiter` 类型的子节点
  //       添加到 CST 中。
  Token keyword_token = tokens[current - 1];
  auto keyword_node = make_cst_node(CSTNodeType::Delimiter, keyword_token);
  node->add_child(std::move(keyword_node));

  // 解析标识符
  auto name_token = consume(TokenType::Identifier);
  if (!name_token) {
    // 错误恢复：如果没有标识符，同步到分号
    synchronize_to_semicolon();
    return node; // 返回不完整的节点以继续解析
  }
  auto name_node = make_cst_node(CSTNodeType::Identifier, *name_token);
  node->add_child(std::move(name_node));

  // 解析可选的类型注解
  if (match_token({TokenType::Colon})) {
    // 保留冒号
    Token colon = tokens[current - 1];
    auto colon_node = make_cst_node(CSTNodeType::Delimiter, colon);
    node->add_child(std::move(colon_node));

    // 解析类型表达式（支持元组、联合等复杂类型）
    auto type_node = parse_type_expression();
    if (type_node) {
      node->add_child(std::move(type_node));
    } else {
      // 类型解析失败，但继续尝试解析后续部分
      synchronize_to_semicolon();
      return node;
    }
  }

  // 解析可选的初始化表达式
  if (match_token({TokenType::Equal})) {
    // 保留等号
    Token equal = tokens[current - 1];
    auto equal_node = make_cst_node(CSTNodeType::Operator, equal);
    node->add_child(std::move(equal_node));

    // 解析表达式
    auto expr = expression();
    if (expr) {
      node->add_child(std::move(expr));
    } else {
      // 表达式解析失败，同步到分号
      synchronize_to_semicolon();
      return node;
    }
  }

  // 消费分号
  auto semicolon = consume(TokenType::Semicolon);
  if (semicolon) {
    auto semi_node = make_cst_node(CSTNodeType::Delimiter, *semicolon);
    node->add_child(std::move(semi_node));
  }

  // 检查是否有行内注释
  if (check(TokenType::Comment)) {
    auto comment_token = advance();
    auto comment_node = make_cst_node(CSTNodeType::Comment, comment_token);
    node->add_child(std::move(comment_node));
  }

  return node;
}

std::unique_ptr<CSTNode> Parser::fn_declaration() {
  auto node = make_cst_node(CSTNodeType::FnDeclaration, make_location());

  // fn 关键字
  Token fn_keyword = tokens[current - 1];
  auto fn_node = make_cst_node(CSTNodeType::Delimiter, fn_keyword);
  node->add_child(std::move(fn_node));

  // 解析函数名
  auto name_token = consume(TokenType::Identifier);
  if (!name_token) {
    // 错误恢复：函数名缺失，尝试同步到代码块或下一个声明
    synchronize_to_statement_start();
    return node;
  }
  auto name_node = make_cst_node(CSTNodeType::Identifier, *name_token);
  node->add_child(std::move(name_node));

  // 消费左括号
  auto left_paren = consume(TokenType::LeftParen);
  if (left_paren) {
    auto lparen_node = make_cst_node(CSTNodeType::Delimiter, *left_paren);
    node->add_child(std::move(lparen_node));
  } else {
    // 错误恢复：左括号缺失，插入虚拟 token 继续解析
    Token synthetic_lparen(TokenType::LeftParen, "(", current_token().line,
                           current_token().column, true);
    auto lparen_node = make_cst_node(CSTNodeType::Delimiter, synthetic_lparen);
    node->add_child(std::move(lparen_node));
  }

  // 解析参数列表
  auto param_list = make_cst_node(CSTNodeType::ParameterList, make_location());

  if (!check(TokenType::RightParen)) {
    do {
      // 解析参数名
      auto param_name = consume(TokenType::Identifier);
      if (!param_name) {
        // 参数名缺失，跳过到逗号或右括号
        while (!check(TokenType::EndOfFile) && !check(TokenType::Comma) &&
               !check(TokenType::RightParen)) {
          advance();
        }
        if (check(TokenType::Comma)) {
          advance();
          continue;
        }
        break;
      }

      auto param_node = make_cst_node(CSTNodeType::Parameter, *param_name);
      auto param_name_node =
          make_cst_node(CSTNodeType::Identifier, *param_name);
      param_node->add_child(std::move(param_name_node));

      // 解析可选的类型注解
      if (match_token({TokenType::Colon})) {
        Token colon = tokens[current - 1];
        auto colon_node = make_cst_node(CSTNodeType::Delimiter, colon);
        param_node->add_child(std::move(colon_node));

        // 使用 parse_type_expression 以支持元组、函数等复杂类型
        auto type_node = parse_type_expression();
        if (type_node) {
          param_node->add_child(std::move(type_node));
        }
      }

      param_list->add_child(std::move(param_node));

      // 检查是否有逗号
      if (match_token({TokenType::Comma})) {
        Token comma = tokens[current - 1];
        auto comma_node = make_cst_node(CSTNodeType::Delimiter, comma);
        param_list->add_child(std::move(comma_node));
      } else {
        break;
      }
    } while (true);
  }

  node->add_child(std::move(param_list));

  // 消费右括号
  auto right_paren = consume(TokenType::RightParen);
  if (right_paren) {
    auto rparen_node = make_cst_node(CSTNodeType::Delimiter, *right_paren);
    node->add_child(std::move(rparen_node));
  }

  // 解析可选的返回类型
  if (match_token({TokenType::Arrow})) {
    Token arrow = tokens[current - 1];
    auto arrow_node = make_cst_node(CSTNodeType::Delimiter, arrow);
    node->add_child(std::move(arrow_node));

    // 使用 parse_type_expression 以支持元组、函数等复杂类型
    auto return_type = parse_type_expression();
    if (return_type) {
      node->add_child(std::move(return_type));
    }
  }

  // 解析函数体
  auto body = block_statement();
  if (body) {
    node->add_child(std::move(body));
  } else {
    // 函数体解析失败，同步到下一个顶层声明
    synchronize_to_statement_start();
  }

  return node;
}

std::unique_ptr<CSTNode> Parser::parse_type() {
  std::unique_ptr<CSTNode> base_type = nullptr;

  // --- 基础类型解析 ---
  Token token = current_token();
  if (token.token_type == TokenType::Identifier) {
    advance();
    base_type = make_cst_node(CSTNodeType::TypeAnnotation, token);
  } else {
    // 无法识别的类型名称
    std::vector<std::string> args = {"type annotation",
                                     token_type_to_string(token.token_type)};
    report_error(DiagnosticCode::P0011_ExpectedTypeAnnotation, make_location(),
                 args);
    return nullptr;
  }

  // 处理后缀数组类型: T[], T[N], T[][]
  return parse_array_suffix(std::move(base_type));
}

std::unique_ptr<CSTNode> Parser::struct_declaration() {
  auto node = make_cst_node(CSTNodeType::StructDeclaration, make_location());

  // struct 关键字
  Token struct_keyword = tokens[current - 1];
  auto struct_node = make_cst_node(CSTNodeType::Delimiter, struct_keyword);
  node->add_child(std::move(struct_node));

  // 解析结构体名
  auto name_token = consume(TokenType::Identifier);
  if (!name_token) {
    std::vector<std::string> args = {
        token_type_to_string(current_token().token_type)};
    report_error(DiagnosticCode::S0001_ExpectedStructName, make_location(),
                 args);
    synchronize_to_statement_start();
    return node;
  }
  auto name_node = make_cst_node(CSTNodeType::Identifier, *name_token);
  node->add_child(std::move(name_node));

  // 消费左花括号
  auto left_brace = consume(TokenType::LeftBrace);
  if (!left_brace) {
    std::vector<std::string> args = {
        token_type_to_string(current_token().token_type)};
    report_error(DiagnosticCode::S0002_ExpectedLeftBraceInStruct,
                 make_location(), args);
    synchronize_to_statement_start();
    return node;
  }
  auto lbrace_node = make_cst_node(CSTNodeType::Delimiter, *left_brace);
  node->add_child(std::move(lbrace_node));

  // 解析字段列表
  // NOTE: 使用 unordered_set 提供 O(1) 平均查找时间，避免 O(n²) 复杂度。
  std::unordered_set<std::string> field_names; // 用于检测重复字段名

  if (!check(TokenType::RightBrace)) {
    do {
      // 跳过注释
      while (check(TokenType::Comment)) {
        auto comment_token = advance();
        auto comment_node = make_cst_node(CSTNodeType::Comment, comment_token);
        node->add_child(std::move(comment_node));
      }

      if (check(TokenType::RightBrace))
        break;

      // 解析字段名
      auto field_name = consume(TokenType::Identifier);
      if (!field_name) {
        std::vector<std::string> args = {
            token_type_to_string(current_token().token_type)};
        report_error(DiagnosticCode::S0003_ExpectedFieldName, make_location(),
                     args);
        // 跳过到逗号或右花括号
        while (!check(TokenType::EndOfFile) && !check(TokenType::Comma) &&
               !check(TokenType::RightBrace)) {
          advance();
        }
        if (check(TokenType::Comma)) {
          advance();
          continue;
        }
        break;
      }

      // 检查重复字段名（O(1) 平均时间）
      if (field_names.count(field_name->value) > 0) {
        std::vector<std::string> args = {field_name->value};
        report_error(DiagnosticCode::S0012_DuplicateFieldName, make_location(),
                     args);
      } else {
        field_names.insert(field_name->value);
      }

      auto field_node = make_cst_node(CSTNodeType::StructField, *field_name);
      auto field_name_node =
          make_cst_node(CSTNodeType::Identifier, *field_name);
      field_node->add_child(std::move(field_name_node));

      // 解析冒号
      auto colon_token = consume(TokenType::Colon);
      if (!colon_token) {
        std::vector<std::string> args = {
            field_name->value,
            token_type_to_string(current_token().token_type)};
        report_error(DiagnosticCode::S0004_ExpectedColonAfterFieldName,
                     make_location(), args);
        // 继续尝试解析下一个字段
        while (!check(TokenType::EndOfFile) && !check(TokenType::Comma) &&
               !check(TokenType::RightBrace)) {
          advance();
        }
        if (check(TokenType::Comma)) {
          advance();
          continue;
        }
        break;
      }
      auto colon_node = make_cst_node(CSTNodeType::Delimiter, *colon_token);
      field_node->add_child(std::move(colon_node));

      // 解析字段类型 (使用 parse_type_expression 以支持复杂类型)
      auto type_node = parse_type_expression();
      if (!type_node) {
        std::vector<std::string> args = {
            token_type_to_string(current_token().token_type)};
        report_error(DiagnosticCode::S0005_ExpectedFieldType, make_location(),
                     args);
        // 跳过到逗号或右花括号
        while (!check(TokenType::EndOfFile) && !check(TokenType::Comma) &&
               !check(TokenType::RightBrace)) {
          advance();
        }
        if (check(TokenType::Comma)) {
          advance();
          continue;
        }
        break;
      }
      field_node->add_child(std::move(type_node));

      node->add_child(std::move(field_node));

      // 检查逗号或右花括号
      if (match_token({TokenType::Comma})) {
        Token comma = tokens[current - 1];
        auto comma_node = make_cst_node(CSTNodeType::Delimiter, comma);
        node->add_child(std::move(comma_node));
        // 允许尾随逗号
        if (check(TokenType::RightBrace)) {
          break;
        }
      } else if (check(TokenType::RightBrace)) {
        break;
      } else {
        std::vector<std::string> args = {
            token_type_to_string(current_token().token_type)};
        report_error(DiagnosticCode::S0006_ExpectedCommaOrRightBrace,
                     make_location(), args);
        break;
      }
    } while (true);
  }

  // 消费右花括号
  auto right_brace = consume(TokenType::RightBrace);
  if (right_brace) {
    auto rbrace_node = make_cst_node(CSTNodeType::Delimiter, *right_brace);
    node->add_child(std::move(rbrace_node));
  }

  // 消费可选的分号（为了与现代语言习惯保持一致）
  // NOTE: 分号现在是可选的，但如果存在则会被保留在 CST 中用于格式化。
  if (check(TokenType::Semicolon)) {
    Token semicolon = advance();
    auto semicolon_node = make_cst_node(CSTNodeType::Delimiter, semicolon);
    node->add_child(std::move(semicolon_node));
  }

  return node;
}

std::unique_ptr<CSTNode> Parser::type_alias_declaration() {
  auto node = make_cst_node(CSTNodeType::TypeAliasDeclaration, make_location());

  // type 关键字
  Token type_keyword = tokens[current - 1];
  auto type_node = make_cst_node(CSTNodeType::Delimiter, type_keyword);
  node->add_child(std::move(type_node));

  // 解析类型别名名称
  auto name_token = consume(TokenType::Identifier);
  if (!name_token) {
    std::vector<std::string> args = {
        token_type_to_string(current_token().token_type)};
    report_error(DiagnosticCode::S0007_ExpectedTypeName, make_location(), args);
    synchronize_to_semicolon();
    return node;
  }
  auto name_node = make_cst_node(CSTNodeType::Identifier, *name_token);
  node->add_child(std::move(name_node));

  // 消费等号
  auto equal_token = consume(TokenType::Equal);
  if (!equal_token) {
    std::vector<std::string> args = {
        token_type_to_string(current_token().token_type)};
    report_error(DiagnosticCode::S0008_ExpectedEqualInTypeAlias,
                 make_location(), args);
    synchronize_to_semicolon();
    return node;
  }
  auto equal_node = make_cst_node(CSTNodeType::Delimiter, *equal_token);
  node->add_child(std::move(equal_node));

  // 解析类型表达式
  auto type_expr = parse_type_expression();
  if (!type_expr) {
    std::vector<std::string> args = {
        token_type_to_string(current_token().token_type)};
    report_error(DiagnosticCode::S0009_ExpectedTypeExpression, make_location(),
                 args);
    synchronize_to_semicolon();
    return node;
  }
  node->add_child(std::move(type_expr));

  // 消费分号
  auto semicolon = consume(TokenType::Semicolon);
  if (semicolon) {
    auto semicolon_node = make_cst_node(CSTNodeType::Delimiter, *semicolon);
    node->add_child(std::move(semicolon_node));
  }

  return node;
}

std::unique_ptr<CSTNode> Parser::parse_type_expression() {
  return parse_type_union();
}

std::unique_ptr<CSTNode> Parser::parse_type_union() {
  auto left = parse_type_intersection();
  if (!left)
    return nullptr;

  // 检查联合类型运算符 |
  while (match_token({TokenType::Or})) {
    Token op = tokens[current - 1];
    auto union_node = make_cst_node(CSTNodeType::UnionType, make_location());

    union_node->add_child(std::move(left));

    auto op_node = make_cst_node(CSTNodeType::Operator, op);
    union_node->add_child(std::move(op_node));

    auto right = parse_type_intersection();
    if (!right) {
      std::vector<std::string> args = {
          token_type_to_string(current_token().token_type)};
      report_error(DiagnosticCode::S0009_ExpectedTypeExpression,
                   make_location(), args);
      return union_node;
    }
    union_node->add_child(std::move(right));

    left = std::move(union_node);
  }

  return left;
}

std::unique_ptr<CSTNode> Parser::parse_type_intersection() {
  auto left = parse_type_primary();
  if (!left)
    return nullptr;

  // 检查交集类型运算符 &
  while (match_token({TokenType::And})) {
    Token op = tokens[current - 1];
    auto intersection_node =
        make_cst_node(CSTNodeType::IntersectionType, make_location());

    intersection_node->add_child(std::move(left));

    auto op_node = make_cst_node(CSTNodeType::Operator, op);
    intersection_node->add_child(std::move(op_node));

    auto right = parse_type_primary();
    if (!right) {
      std::vector<std::string> args = {
          token_type_to_string(current_token().token_type)};
      report_error(DiagnosticCode::S0009_ExpectedTypeExpression,
                   make_location(), args);
      return intersection_node;
    }
    intersection_node->add_child(std::move(right));

    left = std::move(intersection_node);
  }

  return left;
}

std::unique_ptr<CSTNode> Parser::parse_type_primary() {
  // 否定类型: ~Type
  if (match_token({TokenType::Tilde})) {
    Token tilde_token = tokens[current - 1];
    auto negation_node =
        make_cst_node(CSTNodeType::NegationType, make_location());

    auto tilde_node = make_cst_node(CSTNodeType::Operator, tilde_token);
    negation_node->add_child(std::move(tilde_node));

    auto inner_type = parse_type_primary();
    if (!inner_type) {
      std::vector<std::string> args = {
          token_type_to_string(current_token().token_type)};
      report_error(DiagnosticCode::S0009_ExpectedTypeExpression,
                   make_location(), args);
      return negation_node;
    }
    negation_node->add_child(std::move(inner_type));

    return negation_node;
  }

  // 匿名结构体类型: struct { field: Type, ... }
  if (match_token({TokenType::Struct})) {
    Token struct_keyword = tokens[current - 1];
    auto anon_struct =
        make_cst_node(CSTNodeType::AnonymousStructType, make_location());

    auto struct_node = make_cst_node(CSTNodeType::Delimiter, struct_keyword);
    anon_struct->add_child(std::move(struct_node));

    // 消费左花括号
    auto left_brace = consume(TokenType::LeftBrace);
    if (!left_brace) {
      std::vector<std::string> args = {
          token_type_to_string(current_token().token_type)};
      report_error(DiagnosticCode::S0002_ExpectedLeftBraceInStruct,
                   make_location(), args);
      return anon_struct;
    }
    auto lbrace_node = make_cst_node(CSTNodeType::Delimiter, *left_brace);
    anon_struct->add_child(std::move(lbrace_node));

    // 解析字段（与 struct_declaration 类似）
    if (!check(TokenType::RightBrace)) {
      do {
        auto field_name = consume(TokenType::Identifier);
        if (!field_name) {
          break;
        }

        auto field_node = make_cst_node(CSTNodeType::StructField, *field_name);
        auto field_name_node =
            make_cst_node(CSTNodeType::Identifier, *field_name);
        field_node->add_child(std::move(field_name_node));

        auto colon_token = consume(TokenType::Colon);
        if (!colon_token) {
          break;
        }
        auto colon_node = make_cst_node(CSTNodeType::Delimiter, *colon_token);
        field_node->add_child(std::move(colon_node));

        auto type_node = parse_type_expression();
        if (!type_node) {
          break;
        }
        field_node->add_child(std::move(type_node));

        anon_struct->add_child(std::move(field_node));

        if (match_token({TokenType::Comma})) {
          Token comma = tokens[current - 1];
          auto comma_node = make_cst_node(CSTNodeType::Delimiter, comma);
          anon_struct->add_child(std::move(comma_node));
          if (check(TokenType::RightBrace)) {
            break;
          }
        } else {
          break;
        }
      } while (true);
    }

    auto right_brace = consume(TokenType::RightBrace);
    if (right_brace) {
      auto rbrace_node = make_cst_node(CSTNodeType::Delimiter, *right_brace);
      anon_struct->add_child(std::move(rbrace_node));
    }

    return anon_struct;
  }

  // 元组类型或函数签名: (T1, T2, ...) [-> (T3, T4, ...)]
  if (match_token({TokenType::LeftParen})) {
    Token lparen_token = tokens[current - 1];
    auto lparen_node = make_cst_node(CSTNodeType::Delimiter, lparen_token);

    // 收集类型参数
    std::vector<std::unique_ptr<CSTNode>> type_list;
    type_list.push_back(std::move(lparen_node));

    if (!check(TokenType::RightParen)) {
      do {
        auto type_elem = parse_type_expression();
        if (!type_elem) {
          break;
        }
        type_list.push_back(std::move(type_elem));

        if (match_token({TokenType::Comma})) {
          Token comma = tokens[current - 1];
          auto comma_node = make_cst_node(CSTNodeType::Delimiter, comma);
          type_list.push_back(std::move(comma_node));
        } else {
          break;
        }
      } while (true);
    }

    auto rparen_token = consume(TokenType::RightParen);
    if (!rparen_token) {
      std::vector<std::string> args = {
          token_type_to_string(current_token().token_type)};
      report_error(DiagnosticCode::S0010_ExpectedRightParenInTuple,
                   make_location(), args);
      // 尝试创建一个不完整的元组
      auto tuple_node = make_cst_node(CSTNodeType::TupleType, make_location());
      for (auto& child : type_list) {
        tuple_node->add_child(std::move(child));
      }
      return tuple_node;
    }

    // 检查是否是函数签名 (-> ...)
    if (check(TokenType::Arrow)) {
      auto func_sig_node =
          make_cst_node(CSTNodeType::FunctionSignatureType, make_location());

      // 添加参数列表
      for (auto& child : type_list) {
        func_sig_node->add_child(std::move(child));
      }

      auto rparen_node = make_cst_node(CSTNodeType::Delimiter, *rparen_token);
      func_sig_node->add_child(std::move(rparen_node));

      // 消费箭头
      Token arrow_token = advance();
      auto arrow_node = make_cst_node(CSTNodeType::Delimiter, arrow_token);
      func_sig_node->add_child(std::move(arrow_node));

      // 解析返回类型（可以是单个类型或元组）
      if (match_token({TokenType::LeftParen})) {
        Token ret_lparen = tokens[current - 1];
        auto ret_lparen_node =
            make_cst_node(CSTNodeType::Delimiter, ret_lparen);
        func_sig_node->add_child(std::move(ret_lparen_node));

        if (!check(TokenType::RightParen)) {
          do {
            auto ret_type = parse_type_expression();
            if (!ret_type) {
              break;
            }
            func_sig_node->add_child(std::move(ret_type));

            if (match_token({TokenType::Comma})) {
              Token comma = tokens[current - 1];
              auto comma_node = make_cst_node(CSTNodeType::Delimiter, comma);
              func_sig_node->add_child(std::move(comma_node));
            } else {
              break;
            }
          } while (true);
        }

        auto ret_rparen = consume(TokenType::RightParen);
        if (ret_rparen) {
          auto ret_rparen_node =
              make_cst_node(CSTNodeType::Delimiter, *ret_rparen);
          func_sig_node->add_child(std::move(ret_rparen_node));
        } else {
          std::vector<std::string> args = {
              token_type_to_string(current_token().token_type)};
          report_error(DiagnosticCode::S0011_ExpectedRightParenInFuncSig,
                       make_location(), args);
        }
      } else {
        // 单个返回类型
        auto ret_type = parse_type_expression();
        if (ret_type) {
          func_sig_node->add_child(std::move(ret_type));
        }
      }

      // 处理后缀数组类型: ((T) -> R)[]
      return parse_array_suffix(std::move(func_sig_node));
    }

    // 是元组类型
    auto tuple_node = make_cst_node(CSTNodeType::TupleType, make_location());
    for (auto& child : type_list) {
      tuple_node->add_child(std::move(child));
    }
    auto rparen_node = make_cst_node(CSTNodeType::Delimiter, *rparen_token);
    tuple_node->add_child(std::move(rparen_node));

    // 处理后缀数组类型: (T1, T2)[]
    return parse_array_suffix(std::move(tuple_node));
  }

  // 基本类型：标识符（支持后缀数组类型）
  if (check(TokenType::Identifier)) {
    Token type_token = advance();
    auto base_type = make_cst_node(CSTNodeType::TypeAnnotation, type_token);

    // 处理后缀数组类型: T[], T[5], T[][]
    return parse_array_suffix(std::move(base_type));
  }

  // 无法识别的类型
  std::vector<std::string> args = {
      token_type_to_string(current_token().token_type)};
  report_error(DiagnosticCode::S0009_ExpectedTypeExpression, make_location(),
               args);
  return nullptr;
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

std::unique_ptr<CSTNode> Parser::statement() {
  if (match_token({TokenType::Return})) {
    return return_statement();
  } else if (match_token({TokenType::If})) {
    return if_statement();
  } else if (match_token({TokenType::While})) {
    return while_statement();
  } else if (match_token({TokenType::LeftBrace})) {
    return block_statement();
  } else {
    return expression_statement();
  }
}

std::unique_ptr<CSTNode> Parser::return_statement() {
  auto node = make_cst_node(CSTNodeType::ReturnStmt, make_location());

  // 保留 return 关键字
  Token return_keyword = tokens[current - 1];
  auto return_node = make_cst_node(CSTNodeType::Delimiter, return_keyword);
  node->add_child(std::move(return_node));

  // 解析可选的返回值
  if (!check(TokenType::Semicolon)) {
    auto expr = expression();
    if (expr) {
      node->add_child(std::move(expr));
    }
  }

  // 消费分号
  auto semicolon = consume(TokenType::Semicolon);
  if (semicolon) {
    auto semi_node = make_cst_node(CSTNodeType::Delimiter, *semicolon);
    node->add_child(std::move(semi_node));
  }

  return node;
}

std::unique_ptr<CSTNode> Parser::if_statement() {
  auto node = make_cst_node(CSTNodeType::IfStmt, make_location());

  // 保留 if 关键字
  Token if_keyword = tokens[current - 1];
  auto if_node = make_cst_node(CSTNodeType::Delimiter, if_keyword);
  node->add_child(std::move(if_node));

  // 解析条件表达式
  auto condition = expression();
  if (condition) {
    node->add_child(std::move(condition));
  }

  // 解析 then 分支
  auto then_branch = block_statement();
  if (then_branch) {
    node->add_child(std::move(then_branch));
  }

  // 解析可选的 else 或 else if 分支
  if (match_token({TokenType::Else})) {
    Token else_keyword = tokens[current - 1];
    auto else_node = make_cst_node(CSTNodeType::Delimiter, else_keyword);
    node->add_child(std::move(else_node));

    // 检查是否是 else if
    if (match_token({TokenType::If})) {
      // else if: 递归解析为嵌套的 if 语句
      auto else_if_branch = if_statement();
      if (else_if_branch) {
        node->add_child(std::move(else_if_branch));
      }
    } else {
      // 普通 else: 解析代码块
      auto else_branch = block_statement();
      if (else_branch) {
        node->add_child(std::move(else_branch));
      }
    }
  }

  return node;
}

std::unique_ptr<CSTNode> Parser::while_statement() {
  auto node = make_cst_node(CSTNodeType::WhileStmt, make_location());

  Token while_keyword = tokens[current - 1];
  auto while_node = make_cst_node(CSTNodeType::Delimiter, while_keyword);
  node->add_child(std::move(while_node));

  auto condition = expression();
  if (condition) {
    node->add_child(std::move(condition));
  }

  auto body = block_statement();
  if (body) {
    node->add_child(std::move(body));
  }

  return node;
}

std::unique_ptr<CSTNode> Parser::block_statement() {
  auto node = make_cst_node(CSTNodeType::BlockStmt, make_location());

  if (tokens[current - 1].token_type != TokenType::LeftBrace) {
    auto left_brace = consume(TokenType::LeftBrace);
    if (left_brace) {
      auto lbrace_node = make_cst_node(CSTNodeType::Delimiter, *left_brace);
      node->add_child(std::move(lbrace_node));
    } else {
      // 左大括号缺失，插入虚拟 token 继续解析
      Token synthetic_lbrace(TokenType::LeftBrace, "{", current_token().line,
                             current_token().column, true);
      auto lbrace_node =
          make_cst_node(CSTNodeType::Delimiter, synthetic_lbrace);
      node->add_child(std::move(lbrace_node));
    }
  } else {
    Token left_brace = tokens[current - 1];
    auto lbrace_node = make_cst_node(CSTNodeType::Delimiter, left_brace);
    node->add_child(std::move(lbrace_node));
  }

  auto stmt_list = make_cst_node(CSTNodeType::StatementList, make_location());
  while (!check(TokenType::RightBrace) && !check(TokenType::EndOfFile)) {
    // 处理块中的注释
    if (check(TokenType::Comment)) {
      auto comment_token = advance();
      auto comment_node = make_cst_node(CSTNodeType::Comment, comment_token);
      stmt_list->add_child(std::move(comment_node));
      continue;
    }

    auto stmt = declaration();
    if (stmt) {
      stmt_list->add_child(std::move(stmt));
    } else {
      // 错误恢复：语句解析失败，同步到下一个语句或块结束
      synchronize_to_statement_start();
      // 如果已经到达块结束，退出循环
      if (check(TokenType::RightBrace) || check(TokenType::EndOfFile)) {
        break;
      }
    }
  }
  node->add_child(std::move(stmt_list));

  // 消费右大括号
  auto right_brace = consume(TokenType::RightBrace);
  if (right_brace) {
    auto rbrace_node = make_cst_node(CSTNodeType::Delimiter, *right_brace);
    node->add_child(std::move(rbrace_node));
  }

  return node;
}

std::unique_ptr<CSTNode> Parser::expression_statement() {
  auto node = make_cst_node(CSTNodeType::ExprStmt, make_location());

  auto expr = expression();
  if (expr) {
    node->add_child(std::move(expr));
  }

  auto semicolon = consume(TokenType::Semicolon);
  if (semicolon) {
    auto semi_node = make_cst_node(CSTNodeType::Delimiter, *semicolon);
    node->add_child(std::move(semi_node));
  }

  // 检查是否有行内注释
  if (check(TokenType::Comment)) {
    auto comment_token = advance();
    auto comment_node = make_cst_node(CSTNodeType::Comment, comment_token);
    node->add_child(std::move(comment_node));
  }

  return node;
}

// --- 表达式解析的入口 ---
// NOTE: 表达式的解析遵循操作符优先级规则。这是通过一系列函数的链式调用
//       来实现的，每个函数负责处理一个特定优先级的操作符。调用链从最低
//       优先级的 `assignment` 开始，逐级向下，直到最高优先级的 `primary`。
//       例如，`term`（加减）会调用 `factor`（乘除），确保了乘除法
//       会先于加减法被组合成子树。
std::unique_ptr<CSTNode> Parser::expression() {
  return assignment();
}

std::unique_ptr<CSTNode> Parser::assignment() {
  auto expr = logical_or();

  if (match_token({TokenType::Equal})) {
    Token equal = tokens[current - 1];

    // NOTE: 赋值操作符是右结合的。例如 `a = b = c` 被解析为 `a = (b = c)`。
    //       这是通过在 `assignment` 函数中递归调用 `assignment()` 来解析
    //       右侧表达式实现的。

    // 检查左侧表达式是否成功解析
    if (!expr) {
      // 左侧表达式解析失败，报告错误并返回空节点
      report_error(DiagnosticCode::P0001_UnexpectedToken, make_location(),
                   {token_type_to_string(current_token().token_type)});
      return nullptr;
    }

    // 检查左侧是否为有效的左值（l-value），即可以被赋值的东西。
    // 在当前语言中，变量名（Identifier）、数组成员（IndexExpr）和
    // 结构体成员（MemberExpr）是有效的左值。
    if (expr->get_type() == CSTNodeType::Identifier ||
        expr->get_type() == CSTNodeType::IndexExpr ||
        expr->get_type() == CSTNodeType::MemberExpr) {
      CSTNodeType assign_type;
      if (expr->get_type() == CSTNodeType::IndexExpr) {
        assign_type = CSTNodeType::IndexAssignExpr;
      } else if (expr->get_type() == CSTNodeType::MemberExpr) {
        assign_type = CSTNodeType::MemberAssignExpr;
      } else {
        assign_type = CSTNodeType::AssignExpr;
      }

      auto assign_node = make_cst_node(assign_type, make_location());

      assign_node->add_child(std::move(expr));

      auto equal_node = make_cst_node(CSTNodeType::Operator, equal);
      assign_node->add_child(std::move(equal_node));

      auto rvalue = assignment();
      if (rvalue) {
        assign_node->add_child(std::move(rvalue));
      }

      return assign_node;
    } else {
      // 无效的赋值目标
      report_error(DiagnosticCode::P0013_InvalidAssignmentTarget,
                   make_location());
    }
  }

  return expr;
}

std::unique_ptr<CSTNode> Parser::logical_or() {
  auto expr = logical_and();

  // NOTE: 这是一个典型的左结合二元操作符的解析循环。
  //       1. 首先解析一个更高优先级的表达式（`logical_and`）作为左操作数
  //       `expr`。
  //       2. 然后进入一个 `while` 循环，检查当前 Token
  //       是否是本级别的操作符（`||`）。
  //       3.
  //       如果是，就消耗掉操作符，并解析另一个更高优先级的表达式作为右操作数。
  //       4. 将左操作数、操作符和右操作数组合成一个新的 `BinaryExpr` 节点。
  //       5. 将这个新节点赋值给 `expr`，使其成为下一次循环的左操作数。
  //       这个过程确保了 `a || b || c` 被解析为 `(a || b) || c`。
  while (match_token({TokenType::OrOr})) {
    Token op = tokens[current - 1];
    auto binary_node = make_cst_node(CSTNodeType::BinaryExpr, make_location());

    binary_node->add_child(std::move(expr));

    auto op_node = make_cst_node(CSTNodeType::Operator, op);
    binary_node->add_child(std::move(op_node));

    auto right = logical_and();
    if (right) {
      binary_node->add_child(std::move(right));
    }

    expr = std::move(binary_node);
  }

  return expr;
}

std::unique_ptr<CSTNode> Parser::logical_and() {
  auto expr = equality();

  while (match_token({TokenType::AndAnd})) {
    Token op = tokens[current - 1];
    auto binary_node = make_cst_node(CSTNodeType::BinaryExpr, make_location());

    binary_node->add_child(std::move(expr));

    auto op_node = make_cst_node(CSTNodeType::Operator, op);
    binary_node->add_child(std::move(op_node));

    auto right = equality();
    if (right) {
      binary_node->add_child(std::move(right));
    }

    expr = std::move(binary_node);
  }

  return expr;
}

std::unique_ptr<CSTNode> Parser::equality() {
  auto expr = comparison();

  while (match_token({TokenType::EqualEqual, TokenType::BangEqual})) {
    Token op = tokens[current - 1];
    auto binary_node = make_cst_node(CSTNodeType::BinaryExpr, make_location());

    binary_node->add_child(std::move(expr));

    auto op_node = make_cst_node(CSTNodeType::Operator, op);
    binary_node->add_child(std::move(op_node));

    auto right = comparison();
    if (right) {
      binary_node->add_child(std::move(right));
    }

    expr = std::move(binary_node);
  }

  return expr;
}

std::unique_ptr<CSTNode> Parser::comparison() {
  auto expr = term();

  while (match_token({TokenType::Greater, TokenType::GreaterEqual,
                      TokenType::Less, TokenType::LessEqual})) {
    Token op = tokens[current - 1];
    auto binary_node = make_cst_node(CSTNodeType::BinaryExpr, make_location());

    binary_node->add_child(std::move(expr));

    auto op_node = make_cst_node(CSTNodeType::Operator, op);
    binary_node->add_child(std::move(op_node));

    auto right = term();
    if (right) {
      binary_node->add_child(std::move(right));
    }

    expr = std::move(binary_node);
  }

  return expr;
}

std::unique_ptr<CSTNode> Parser::term() {
  auto expr = factor();

  while (match_token({TokenType::Plus, TokenType::Minus})) {
    Token op = tokens[current - 1];
    auto binary_node = make_cst_node(CSTNodeType::BinaryExpr, make_location());

    binary_node->add_child(std::move(expr));

    auto op_node = make_cst_node(CSTNodeType::Operator, op);
    binary_node->add_child(std::move(op_node));

    auto right = factor();
    if (right) {
      binary_node->add_child(std::move(right));
    }

    expr = std::move(binary_node);
  }

  return expr;
}

std::unique_ptr<CSTNode> Parser::factor() {
  auto expr = unary();

  while (match_token({TokenType::Star, TokenType::Slash, TokenType::Percent})) {
    Token op = tokens[current - 1];
    auto binary_node = make_cst_node(CSTNodeType::BinaryExpr, make_location());

    binary_node->add_child(std::move(expr));

    auto op_node = make_cst_node(CSTNodeType::Operator, op);
    binary_node->add_child(std::move(op_node));

    auto right = unary();
    if (right) {
      binary_node->add_child(std::move(right));
    }

    expr = std::move(binary_node);
  }

  return expr;
}

std::unique_ptr<CSTNode> Parser::unary() {
  if (match_token({TokenType::Bang, TokenType::Minus})) {
    Token op = tokens[current - 1];
    auto unary_node = make_cst_node(CSTNodeType::UnaryExpr, make_location());

    auto op_node = make_cst_node(CSTNodeType::Operator, op);
    unary_node->add_child(std::move(op_node));

    auto operand = unary();
    if (operand) {
      unary_node->add_child(std::move(operand));
    }

    return unary_node;
  }

  return call();
}

std::unique_ptr<CSTNode> Parser::call() {
  auto expr = primary();

  while (true) {
    if (match_token({TokenType::LeftParen})) {
      // 函数调用
      Token left_paren = tokens[current - 1];
      auto call_node = make_cst_node(CSTNodeType::CallExpr, make_location());

      call_node->add_child(std::move(expr));

      auto lparen_node = make_cst_node(CSTNodeType::Delimiter, left_paren);
      call_node->add_child(std::move(lparen_node));

      // 解析参数列表
      auto arg_list = make_cst_node(CSTNodeType::ArgumentList, make_location());
      if (!check(TokenType::RightParen)) {
        do {
          auto arg = expression();
          if (arg) {
            arg_list->add_child(std::move(arg));
          }

          if (match_token({TokenType::Comma})) {
            Token comma = tokens[current - 1];
            auto comma_node = make_cst_node(CSTNodeType::Delimiter, comma);
            arg_list->add_child(std::move(comma_node));
          } else {
            break;
          }
        } while (true);
      }
      call_node->add_child(std::move(arg_list));

      auto right_paren = consume(TokenType::RightParen);
      if (right_paren) {
        auto rparen_node = make_cst_node(CSTNodeType::Delimiter, *right_paren);
        call_node->add_child(std::move(rparen_node));
      }

      expr = std::move(call_node);
    } else if (match_token({TokenType::LeftBracket})) {
      // 索引访问
      Token left_bracket = tokens[current - 1];
      auto index_node = make_cst_node(CSTNodeType::IndexExpr, make_location());

      index_node->add_child(std::move(expr));

      auto lbracket_node = make_cst_node(CSTNodeType::Delimiter, left_bracket);
      index_node->add_child(std::move(lbracket_node));

      auto index = expression();
      if (index) {
        index_node->add_child(std::move(index));
      }

      auto right_bracket = consume(TokenType::RightBracket);
      if (right_bracket) {
        auto rbracket_node =
            make_cst_node(CSTNodeType::Delimiter, *right_bracket);
        index_node->add_child(std::move(rbracket_node));
      }

      expr = std::move(index_node);
    } else if (match_token({TokenType::Dot})) {
      // 成员访问
      Token dot = tokens[current - 1];
      auto member_node =
          make_cst_node(CSTNodeType::MemberExpr, make_location());

      member_node->add_child(std::move(expr));

      auto dot_node = make_cst_node(CSTNodeType::Delimiter, dot);
      member_node->add_child(std::move(dot_node));

      // 右侧必须是标识符
      auto member_name = consume(TokenType::Identifier);
      if (member_name) {
        auto member_name_node =
            make_cst_node(CSTNodeType::Identifier, *member_name);
        member_node->add_child(std::move(member_name_node));
      }

      expr = std::move(member_node);
    } else if (match_token({TokenType::LeftBrace})) {
      // 需要区分结构体字面量和代码块
      // 结构体字面量: TypeName { field: value, ... }
      // 代码块: if flag { statements; }

      // 使用前瞻来判断：
      // - 如果 { 后面是 Identifier : ，那是结构体字面量
      // - 如果 { 后面是 }，可能是结构体字面量或空代码块，这里假设是结构体
      // - 否则是代码块

      bool is_struct_literal = false;
      if (check(TokenType::RightBrace)) {
        // 空的 {} - 假设是空结构体字面量
        is_struct_literal = true;
      } else if (check(TokenType::Identifier)) {
        // 前瞻检查下一个 token 是否是冒号
        Token next = peek(1);
        if (next.token_type == TokenType::Colon) {
          is_struct_literal = true;
        }
      }

      if (!is_struct_literal) {
        // 不是结构体字面量，回退并退出 call() 循环
        current--;
        break;
      }

      // 确认是结构体字面量
      Token left_brace = tokens[current - 1];
      auto struct_lit_node =
          make_cst_node(CSTNodeType::StructLiteral, make_location());

      struct_lit_node->add_child(std::move(expr));

      auto lbrace_node = make_cst_node(CSTNodeType::Delimiter, left_brace);
      struct_lit_node->add_child(std::move(lbrace_node));

      // 解析字段初始化列表
      if (!check(TokenType::RightBrace)) {
        do {
          // 跳过注释
          while (check(TokenType::Comment)) {
            auto comment_token = advance();
            auto comment_node =
                make_cst_node(CSTNodeType::Comment, comment_token);
            struct_lit_node->add_child(std::move(comment_node));
          }

          if (check(TokenType::RightBrace))
            break;

          // 解析字段名
          auto field_name = consume(TokenType::Identifier);
          if (!field_name) {
            std::vector<std::string> args = {
                token_type_to_string(current_token().token_type)};
            report_error(DiagnosticCode::S0013_ExpectedStructFieldInit,
                         make_location(), args);
            // 跳过到逗号或右花括号
            while (!check(TokenType::EndOfFile) && !check(TokenType::Comma) &&
                   !check(TokenType::RightBrace)) {
              advance();
            }
            if (check(TokenType::Comma)) {
              advance();
              continue;
            }
            break;
          }

          auto field_name_node =
              make_cst_node(CSTNodeType::Identifier, *field_name);
          struct_lit_node->add_child(std::move(field_name_node));

          // 解析冒号
          auto colon_token = consume(TokenType::Colon);
          if (!colon_token) {
            // 跳过到逗号或右花括号
            while (!check(TokenType::EndOfFile) && !check(TokenType::Comma) &&
                   !check(TokenType::RightBrace)) {
              advance();
            }
            if (check(TokenType::Comma)) {
              advance();
              continue;
            }
            break;
          }
          auto colon_node = make_cst_node(CSTNodeType::Delimiter, *colon_token);
          struct_lit_node->add_child(std::move(colon_node));

          // 解析字段值（表达式）
          auto value_expr = expression();
          if (!value_expr) {
            // 跳过到逗号或右花括号
            while (!check(TokenType::EndOfFile) && !check(TokenType::Comma) &&
                   !check(TokenType::RightBrace)) {
              advance();
            }
            if (check(TokenType::Comma)) {
              advance();
              continue;
            }
            break;
          }
          struct_lit_node->add_child(std::move(value_expr));

          // 检查逗号
          if (match_token({TokenType::Comma})) {
            Token comma = tokens[current - 1];
            auto comma_node = make_cst_node(CSTNodeType::Delimiter, comma);
            struct_lit_node->add_child(std::move(comma_node));
            // 允许尾随逗号
            if (check(TokenType::RightBrace)) {
              break;
            }
          } else if (check(TokenType::RightBrace)) {
            break;
          } else {
            // 缺少逗号或右花括号
            std::vector<std::string> args = {
                token_type_to_string(current_token().token_type)};
            report_error(DiagnosticCode::S0006_ExpectedCommaOrRightBrace,
                         make_location(), args);
            break;
          }
        } while (true);
      }

      auto right_brace = consume(TokenType::RightBrace);
      if (right_brace) {
        auto rbrace_node = make_cst_node(CSTNodeType::Delimiter, *right_brace);
        struct_lit_node->add_child(std::move(rbrace_node));
      }

      expr = std::move(struct_lit_node);
    } else {
      break;
    }
  }

  return expr;
}

std::unique_ptr<CSTNode> Parser::primary() {
  // 布尔字面量
  if (match_token({TokenType::True, TokenType::False})) {
    Token token = tokens[current - 1];
    auto node = make_cst_node(CSTNodeType::BooleanLiteral, token);
    return node;
  }

  // 整数字面量
  if (match_token({TokenType::Integer})) {
    Token token = tokens[current - 1];
    auto node = make_cst_node(CSTNodeType::IntegerLiteral, token);
    return node;
  }

  // 浮点数字面量
  if (match_token({TokenType::Float})) {
    Token token = tokens[current - 1];
    auto node = make_cst_node(CSTNodeType::FloatLiteral, token);
    return node;
  }

  // 字符串字面量
  if (match_token({TokenType::String})) {
    Token token = tokens[current - 1];
    auto node = make_cst_node(CSTNodeType::StringLiteral, token);
    return node;
  }

  // 函数字面量: fn (params) { body }
  if (match_token({TokenType::Fn})) {
    Token fn_token = tokens[current - 1];
    auto func_lit_node =
        make_cst_node(CSTNodeType::FunctionLiteral, make_location());

    auto fn_node = make_cst_node(CSTNodeType::Delimiter, fn_token);
    func_lit_node->add_child(std::move(fn_node));

    // 消费左括号
    auto left_paren = consume(TokenType::LeftParen);
    if (left_paren) {
      auto lparen_node = make_cst_node(CSTNodeType::Delimiter, *left_paren);
      func_lit_node->add_child(std::move(lparen_node));
    }

    // 解析参数列表
    auto param_list =
        make_cst_node(CSTNodeType::ParameterList, make_location());
    if (!check(TokenType::RightParen)) {
      do {
        auto param_name = consume(TokenType::Identifier);
        if (!param_name) {
          break;
        }

        auto param_node = make_cst_node(CSTNodeType::Parameter, *param_name);
        auto param_name_node =
            make_cst_node(CSTNodeType::Identifier, *param_name);
        param_node->add_child(std::move(param_name_node));

        // 解析可选的类型注解
        if (match_token({TokenType::Colon})) {
          Token colon = tokens[current - 1];
          auto colon_node = make_cst_node(CSTNodeType::Delimiter, colon);
          param_node->add_child(std::move(colon_node));

          // 使用 parse_type_expression 以支持元组、函数等复杂类型
          auto type_node = parse_type_expression();
          if (type_node) {
            param_node->add_child(std::move(type_node));
          }
        }

        param_list->add_child(std::move(param_node));

        if (match_token({TokenType::Comma})) {
          Token comma = tokens[current - 1];
          auto comma_node = make_cst_node(CSTNodeType::Delimiter, comma);
          param_list->add_child(std::move(comma_node));
        } else {
          break;
        }
      } while (true);
    }
    func_lit_node->add_child(std::move(param_list));

    // 消费右括号
    auto right_paren = consume(TokenType::RightParen);
    if (right_paren) {
      auto rparen_node = make_cst_node(CSTNodeType::Delimiter, *right_paren);
      func_lit_node->add_child(std::move(rparen_node));
    }

    // 解析函数体
    auto body = block_statement();
    if (body) {
      func_lit_node->add_child(std::move(body));
    }

    return func_lit_node;
  }

  // 标识符
  if (match_token({TokenType::Identifier})) {
    Token token = tokens[current - 1];
    auto node = make_cst_node(CSTNodeType::Identifier, token);
    return node;
  }

  // 括号表达式或元组字面量
  if (match_token({TokenType::LeftParen})) {
    Token left_paren = tokens[current - 1];

    // 先尝试解析第一个表达式
    auto first_expr = expression();

    // 检查是否有逗号（元组）
    if (check(TokenType::Comma)) {
      // 这是一个元组字面量
      auto tuple_node =
          make_cst_node(CSTNodeType::TupleLiteral, make_location());

      auto lparen_node = make_cst_node(CSTNodeType::Delimiter, left_paren);
      tuple_node->add_child(std::move(lparen_node));

      if (first_expr) {
        tuple_node->add_child(std::move(first_expr));
      }

      // 解析剩余元素
      while (match_token({TokenType::Comma})) {
        Token comma = tokens[current - 1];
        auto comma_node = make_cst_node(CSTNodeType::Delimiter, comma);
        tuple_node->add_child(std::move(comma_node));

        // 允许尾随逗号
        if (check(TokenType::RightParen)) {
          break;
        }

        auto elem = expression();
        if (elem) {
          tuple_node->add_child(std::move(elem));
        } else {
          break;
        }
      }

      auto right_paren = consume(TokenType::RightParen);
      if (right_paren) {
        auto rparen_node = make_cst_node(CSTNodeType::Delimiter, *right_paren);
        tuple_node->add_child(std::move(rparen_node));
      }

      return tuple_node;
    } else {
      // 这是一个括号表达式
      auto paren_node = make_cst_node(CSTNodeType::ParenExpr, make_location());

      auto lparen_node = make_cst_node(CSTNodeType::Delimiter, left_paren);
      paren_node->add_child(std::move(lparen_node));

      if (first_expr) {
        paren_node->add_child(std::move(first_expr));
      }

      auto right_paren = consume(TokenType::RightParen);
      if (right_paren) {
        auto rparen_node = make_cst_node(CSTNodeType::Delimiter, *right_paren);
        paren_node->add_child(std::move(rparen_node));
      }

      return paren_node;
    }
  }

  // 数组字面量
  if (match_token({TokenType::LeftBracket})) {
    Token left_bracket = tokens[current - 1];
    auto array_node = make_cst_node(CSTNodeType::ArrayLiteral, make_location());

    auto lbracket_node = make_cst_node(CSTNodeType::Delimiter, left_bracket);
    array_node->add_child(std::move(lbracket_node));

    if (!check(TokenType::RightBracket)) {
      do {
        auto elem = expression();
        if (elem) {
          array_node->add_child(std::move(elem));
        }

        if (match_token({TokenType::Comma})) {
          Token comma = tokens[current - 1];
          auto comma_node = make_cst_node(CSTNodeType::Delimiter, comma);
          array_node->add_child(std::move(comma_node));

          if (check(TokenType::RightBracket)) {
            break;
          }
        } else {
          break;
        }
      } while (true);
    }

    auto right_bracket = consume(TokenType::RightBracket);
    if (right_bracket) {
      auto rbracket_node =
          make_cst_node(CSTNodeType::Delimiter, *right_bracket);
      array_node->add_child(std::move(rbracket_node));
    }

    return array_node;
  }

  // 无法识别的表达式
  std::vector<std::string> args = {
      token_type_to_string(current_token().token_type)};
  report_error(DiagnosticCode::P0005_ExpectedExpression, make_location(), args);
  return nullptr;
}

} // namespace parser
} // namespace czc
