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

} // namespace czc::parser
