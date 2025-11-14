/**
 * @file parser_decl.cpp
 * @brief `Parser` 类的声明解析实现（变量、函数、结构体、类型别名）。
 * @author BegoniaHe
 * @date 2025-11-13
 */

#include "czc/diagnostics/diagnostic_code.hpp"
#include "czc/parser/parser.hpp"

#include <algorithm>
#include <unordered_set>

namespace czc::parser {

using namespace czc::cst;
using namespace czc::diagnostics;
using namespace czc::lexer;
using namespace czc::utils;

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

} // namespace czc::parser