/**
 * @file parser_type.cpp
 * @brief `Parser` 类的类型表达式解析实现。
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

} // namespace czc::parser
