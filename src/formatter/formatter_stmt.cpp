/**
 * @file formatter_stmt.cpp
 * @brief 语句节点格式化实现 (if, while, return, block, expr)
 * @details 实现所有语句节点的格式化逻辑，包括条件语句、循环语句、
 *          返回语句、代码块和表达式语句。
 * @author BegoniaHe
 * @date 2025-11-14
 */

#include "czc/formatter/formatter.hpp"

#include <sstream>

namespace czc::formatter {

std::string Formatter::visit_program(const cst::CSTNode* node) {
  std::ostringstream result;
  // Program: 顶层节点，逐个格式化其子节点（通常是声明或语句）
  for (const auto& child : node->get_children()) {
    if (child->get_type() == cst::CSTNodeType::Comment) {
      result << format_standalone_comment(child.get());
    } else {
      result << format_node(child.get());
    }
  }
  return result.str();
}

std::string Formatter::visit_return_stmt(const cst::CSTNode* node) {
  std::ostringstream result;
  // ReturnStmt: return a + b;
  result << get_indent() << "return" << ONE_WIDTH_SPACE_STRING;
  for (const auto& child : node->get_children()) {
    if (child->get_type() != cst::CSTNodeType::Delimiter ||
        (child->get_token().has_value() &&
         child->get_token()->token_type != lexer::TokenType::Return &&
         child->get_token()->token_type != lexer::TokenType::Semicolon)) {
      result << format_node(child.get());
    } else if (child->get_token().has_value() &&
               child->get_token()->token_type == lexer::TokenType::Semicolon) {
      result << ";";
    }
  }
  result << "\n";
  return result.str();
}

std::string Formatter::visit_if_stmt(const cst::CSTNode* node) {
  std::ostringstream result;
  result << get_indent();

  // if 语句结构: if condition { block } [else if condition { block }]* [else {
  // block }]
  const auto& children = node->get_children();

  for (size_t i = 0; i < children.size(); ++i) {
    const auto& child = children[i];

    if (child->get_type() == cst::CSTNodeType::Delimiter) {
      const auto& token = child->get_token();
      if (token.has_value()) {
        if (token->token_type == lexer::TokenType::If) {
          // if 关键字
          result << "if";
          result << ONE_WIDTH_SPACE_STRING;
        } else if (token->token_type == lexer::TokenType::LeftParen) {
          if (options.space_before_paren) {
            result << ONE_WIDTH_SPACE_STRING;
          }
          result << "(";
        } else if (token->token_type == lexer::TokenType::RightParen) {
          result << ")";
        } else if (token->token_type == lexer::TokenType::Else) {
          // else 关键字前添加空格
          result << ONE_WIDTH_SPACE_STRING << "else";

          // 检查下一个子节点是否是 if 语句 (else if 情况)
          if (i + 1 < children.size() &&
              children[i + 1]->get_type() == cst::CSTNodeType::IfStmt) {
            result << ONE_WIDTH_SPACE_STRING;
          }
        } else {
          result << format_node(child.get());
        }
      }
    } else if (child->get_type() == cst::CSTNodeType::BlockStmt) {
      if (!options.newline_before_brace) {
        result << ONE_WIDTH_SPACE_STRING;
      }
      result << format_node(child.get());
    } else if (child->get_type() == cst::CSTNodeType::IfStmt) {
      // else if 语句：不添加缩进，因为它是同一级别的
      std::string nested_if = visit_if_stmt(child.get());
      // 移除嵌套 if 语句的缩进（因为它已经在 else 后面）
      size_t first_non_space = nested_if.find_first_not_of(" \t");
      if (first_non_space != std::string::npos) {
        result << nested_if.substr(first_non_space);
      } else {
        result << nested_if;
      }
    } else {
      result << format_node(child.get());
    }
  }

  return result.str();
}

std::string Formatter::visit_while_stmt(const cst::CSTNode* node) {
  std::ostringstream result;
  result << get_indent() << "while";

  // while 语句结构: while condition { block }
  const auto& children = node->get_children();
  for (size_t i = 0; i < children.size(); ++i) {
    const auto& child = children[i];

    if (child->get_type() == cst::CSTNodeType::Delimiter) {
      const auto& token = child->get_token();
      if (token.has_value()) {
        if (token->token_type == lexer::TokenType::LeftParen) {
          if (options.space_before_paren) {
            result << ONE_WIDTH_SPACE_STRING;
          }
          result << "(";
        } else if (token->token_type == lexer::TokenType::RightParen) {
          result << ")";
        } else {
          result << format_node(child.get());
        }
      }
    } else if (child->get_type() == cst::CSTNodeType::BlockStmt) {
      if (!options.newline_before_brace) {
        result << ONE_WIDTH_SPACE_STRING;
      }
      result << format_node(child.get());
    } else {
      result << format_node(child.get());
    }
  }

  return result.str();
}

std::string Formatter::visit_block_stmt(const cst::CSTNode* node) {
  std::ostringstream result;
  // BlockStmt: { statements }
  for (const auto& child : node->get_children()) {
    if (child->get_type() == cst::CSTNodeType::Delimiter) {
      const auto& token = child->get_token();
      if (token.has_value() &&
          token->token_type == lexer::TokenType::LeftBrace) {
        result << "{\n";
        increase_indent();
      } else if (token.has_value() &&
                 token->token_type == lexer::TokenType::RightBrace) {
        decrease_indent();
        result << get_indent() << "}\n";
      }
    } else if (child->get_type() == cst::CSTNodeType::StatementList) {
      result << format_node(child.get());
    }
  }
  return result.str();
}

std::string Formatter::visit_expr_stmt(const cst::CSTNode* node) {
  std::ostringstream result;
  // ExprStmt: 表达式语句，通常是一个函数调用或赋值
  result << get_indent();
  for (size_t i = 0; i < node->get_children().size(); ++i) {
    const auto& child = node->get_children()[i];

    if (child->get_type() == cst::CSTNodeType::Comment) {
      result << format_inline_comment(child.get());
      continue;
    }

    result << format_node(child.get());
  }
  result << "\n";
  return result.str();
}

} // namespace czc::formatter