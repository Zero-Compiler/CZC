/**
 * @file parser.cpp
 * @brief `Parser` 类的功能实现。
 * @author BegoniaHe
 * @date 2025-11-11
 */

#include "czc/parser/parser.hpp"

#include "czc/diagnostics/diagnostic_code.hpp"

#include <algorithm>

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

    // 解析类型
    auto type_node = parse_type();
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

        auto type_node = parse_type();
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

    auto return_type = parse_type();
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
  // 检查数组类型
  if (check(TokenType::LeftBracket)) {
    auto array_type = make_cst_node(CSTNodeType::ArrayType, make_location());

    // 保留左方括号
    Token left_bracket = advance();
    auto lbracket_node = make_cst_node(CSTNodeType::Delimiter, left_bracket);
    array_type->add_child(std::move(lbracket_node));

    // 递归解析元素类型
    auto element_type = parse_type();
    if (element_type) {
      array_type->add_child(std::move(element_type));
    } else {
      // 元素类型缺失，报告错误但继续
      std::vector<std::string> args = {"array element type"};
      report_error(DiagnosticCode::P0011_ExpectedTypeAnnotation,
                   make_location(), args);
    }

    // 消费右方括号
    auto right_bracket = consume(TokenType::RightBracket);
    if (right_bracket) {
      auto rbracket_node =
          make_cst_node(CSTNodeType::Delimiter, *right_bracket);
      array_type->add_child(std::move(rbracket_node));
    }

    return array_type;
  }

  // --- 类型解析 ---
  Token token = current_token();
  if (token.token_type == TokenType::Identifier) {
    advance();
    auto type_node = make_cst_node(CSTNodeType::TypeAnnotation, token);
    return type_node;
  }

  // 无法识别的类型名称
  std::vector<std::string> args = {"type annotation",
                                   token_type_to_string(token.token_type)};
  report_error(DiagnosticCode::P0011_ExpectedTypeAnnotation, make_location(),
               args);
  return nullptr;
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
    // 在当前语言中，只有变量名（Identifier）和数组成员（IndexExpr）是有效的左值。
    if (expr->get_type() == CSTNodeType::Identifier ||
        expr->get_type() == CSTNodeType::IndexExpr) {
      auto assign_node =
          make_cst_node(expr->get_type() == CSTNodeType::IndexExpr
                            ? CSTNodeType::IndexAssignExpr
                            : CSTNodeType::AssignExpr,
                        make_location());

      assign_node->add_child(std::move(expr));

      auto equal_node = make_cst_node(CSTNodeType::Operator, equal);
      assign_node->add_child(std::move(equal_node));

      auto rvalue = assignment();
      if (rvalue) {
        assign_node->add_child(std::move(rvalue));
      }

      return assign_node;
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
  while (match_token({TokenType::Or})) {
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

  while (match_token({TokenType::And})) {
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

  // 标识符
  if (match_token({TokenType::Identifier})) {
    Token token = tokens[current - 1];
    auto node = make_cst_node(CSTNodeType::Identifier, token);
    return node;
  }

  // 括号表达式
  if (match_token({TokenType::LeftParen})) {
    Token left_paren = tokens[current - 1];
    auto paren_node = make_cst_node(CSTNodeType::ParenExpr, make_location());

    auto lparen_node = make_cst_node(CSTNodeType::Delimiter, left_paren);
    paren_node->add_child(std::move(lparen_node));

    auto expr = expression();
    if (expr) {
      paren_node->add_child(std::move(expr));
    }

    auto right_paren = consume(TokenType::RightParen);
    if (right_paren) {
      auto rparen_node = make_cst_node(CSTNodeType::Delimiter, *right_paren);
      paren_node->add_child(std::move(rparen_node));
    }

    return paren_node;
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
