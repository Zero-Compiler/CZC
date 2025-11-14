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

} // namespace czc::parser