/**
 * @file formatter_decl.cpp
 * @brief 声明节点格式化实现 (变量、函数、结构体、类型别名)
 * @details 实现所有声明节点的格式化逻辑，包括变量声明、函数声明、
 *          结构体声明和类型别名声明。
 * @author BegoniaHe
 * @date 2025-11-14
 */

#include "czc/formatter/formatter.hpp"

#include <sstream>

namespace czc::formatter {

std::string Formatter::visit_var_declaration(const cst::CSTNode* node) {
  std::ostringstream result;
  // VarDeclaration: let a = b; // comment
  result << get_indent();
  for (size_t i = 0; i < node->get_children().size(); ++i) {
    const auto& child = node->get_children()[i];

    if (child->get_type() == cst::CSTNodeType::Comment) {
      // 行内注释前加两个空格
      result << format_inline_comment(child.get());
      continue;
    }

    result << format_node(child.get());

    // 在关键字、标识符和值之间添加空格
    if (i + 1 < node->get_children().size()) {
      const auto& next = node->get_children()[i + 1];
      if (next->get_type() != cst::CSTNodeType::Delimiter ||
          (next->get_token().has_value() &&
           next->get_token()->token_type != lexer::TokenType::Semicolon)) {
        result << ONE_WIDTH_SPACE_STRING;
      }
    }
  }
  result << "\n";
  return result.str();
}

std::string Formatter::visit_fn_declaration(const cst::CSTNode* node) {
  std::ostringstream result;
  // FnDeclaration: fn func_name(params) [-> return_type] { body }
  //
  // 结构解析：
  // - Delimiter(fn) - fn 关键字
  // - Identifier - 函数名
  // - Delimiter('(') - 左括号
  // - ParameterList - 参数列表
  // - Delimiter(')') - 右括号
  // - [Delimiter('->') - 箭头（可选）]
  // - [TypeAnnotation - 返回类型（可选）]
  // - BlockStmt - 函数体

  result << get_indent();

  const auto& children = node->get_children();
  for (size_t i = 0; i < children.size(); ++i) {
    const auto& child = children[i];

    if (child->get_type() == cst::CSTNodeType::Delimiter) {
      const auto& token = child->get_token();
      if (token.has_value()) {
        if (token->token_type == lexer::TokenType::Fn) {
          // fn 关键字后加空格
          result << "fn" << ONE_WIDTH_SPACE_STRING;
        } else if (token->token_type == lexer::TokenType::LeftParen) {
          // 左括号紧跟函数名，不加空格
          result << "(";
        } else if (token->token_type == lexer::TokenType::RightParen) {
          // 右括号
          result << ")";
          // 检查下一个是否是箭头或代码块，如果是则需要加空格
          if (i + 1 < children.size()) {
            const auto& next = children[i + 1];
            if (next->get_type() == cst::CSTNodeType::Delimiter) {
              const auto& next_token = next->get_token();
              if (next_token.has_value() &&
                  next_token->token_type == lexer::TokenType::Arrow) {
                result << ONE_WIDTH_SPACE_STRING;
              }
            } else if (next->get_type() == cst::CSTNodeType::BlockStmt) {
              result << ONE_WIDTH_SPACE_STRING;
            }
          }
        } else if (token->token_type == lexer::TokenType::Arrow) {
          // 箭头：-> 后面加空格
          result << "->" << ONE_WIDTH_SPACE_STRING;
        } else {
          result << token->value;
        }
      }
    } else if (child->get_type() == cst::CSTNodeType::Identifier) {
      // 函数名
      result << format_node(child.get());
    } else if (child->get_type() == cst::CSTNodeType::ParameterList) {
      // 参数列表（不包含括号）
      result << format_node(child.get());
    } else if (child->get_type() == cst::CSTNodeType::TypeAnnotation ||
               child->get_type() == cst::CSTNodeType::ArrayType) {
      // 返回类型
      result << format_node(child.get());
      // 返回类型后面如果有代码块，加空格
      if (i + 1 < children.size() &&
          children[i + 1]->get_type() == cst::CSTNodeType::BlockStmt) {
        result << ONE_WIDTH_SPACE_STRING;
      }
    } else if (child->get_type() == cst::CSTNodeType::BlockStmt) {
      // 函数体（如果前面没加过空格，这里会被处理）
      result << format_node(child.get());
    } else {
      // 其他未预期的节点类型
      result << format_node(child.get());
    }
  }

  return result.str();
}

std::string Formatter::visit_struct_declaration(const cst::CSTNode* node) {
  std::ostringstream result;
  result << get_indent();

  // struct Name { field: Type, ... };
  for (size_t i = 0; i < node->get_children().size(); ++i) {
    const auto& child = node->get_children()[i];

    if (child->get_type() == cst::CSTNodeType::Delimiter) {
      const auto& token = child->get_token();
      if (token.has_value()) {
        if (token->token_type == lexer::TokenType::Struct) {
          result << token->value << ONE_WIDTH_SPACE_STRING;
        } else if (token->token_type == lexer::TokenType::LeftBrace) {
          result << ONE_WIDTH_SPACE_STRING << token->value << "\n";
          indent_level++;
        } else if (token->token_type == lexer::TokenType::RightBrace) {
          indent_level--;
          result << "\n" << get_indent() << token->value;
        } else if (token->token_type == lexer::TokenType::Semicolon) {
          result << token->value << "\n";
        } else if (token->token_type == lexer::TokenType::Comma) {
          result << token->value << "\n";
        } else {
          result << format_node(child.get());
        }
      }
    } else if (child->get_type() == cst::CSTNodeType::Identifier) {
      result << format_node(child.get());
    } else if (child->get_type() == cst::CSTNodeType::StructField) {
      result << get_indent() << format_node(child.get());
    } else if (child->get_type() == cst::CSTNodeType::Comment) {
      result << format_standalone_comment(child.get());
    }
  }

  return result.str();
}

std::string Formatter::visit_type_alias_declaration(const cst::CSTNode* node) {
  std::ostringstream result;
  result << get_indent();

  // type Name = TypeExpr;
  for (size_t i = 0; i < node->get_children().size(); ++i) {
    const auto& child = node->get_children()[i];

    if (child->get_type() == cst::CSTNodeType::Delimiter) {
      const auto& token = child->get_token();
      if (token.has_value()) {
        if (token->token_type == lexer::TokenType::Type) {
          result << token->value << ONE_WIDTH_SPACE_STRING;
        } else if (token->token_type == lexer::TokenType::Equal) {
          result << ONE_WIDTH_SPACE_STRING << token->value
                 << ONE_WIDTH_SPACE_STRING;
        } else if (token->token_type == lexer::TokenType::Semicolon) {
          result << token->value << "\n";
        } else {
          result << format_node(child.get());
        }
      }
    } else if (child->get_type() == cst::CSTNodeType::Identifier) {
      result << format_node(child.get());
    } else {
      result << format_node(child.get());
    }
  }

  return result.str();
}

std::string Formatter::visit_struct_field(const cst::CSTNode* node) {
  std::ostringstream result;
  // field: Type
  for (const auto& child : node->get_children()) {
    if (child->get_type() == cst::CSTNodeType::Delimiter) {
      const auto& token = child->get_token();
      if (token.has_value() && token->token_type == lexer::TokenType::Colon) {
        result << token->value << ONE_WIDTH_SPACE_STRING;
      } else {
        result << format_node(child.get());
      }
    } else {
      result << format_node(child.get());
    }
  }
  return result.str();
}

std::string Formatter::visit_parameter(const cst::CSTNode* node) {
  std::ostringstream result;
  // Parameter: name or name: type
  const auto& children = node->get_children();
  for (size_t i = 0; i < children.size(); ++i) {
    const auto& child = children[i];
    result << format_node(child.get());
  }
  return result.str();
}

std::string Formatter::visit_parameter_list(const cst::CSTNode* node) {
  std::ostringstream result;
  // ParameterList: a, b, c (不包含括号)
  const auto& children = node->get_children();
  for (size_t i = 0; i < children.size(); ++i) {
    const auto& child = children[i];
    if (child->get_type() == cst::CSTNodeType::Delimiter) {
      if (child->get_token().has_value() &&
          child->get_token()->token_type == lexer::TokenType::Comma) {
        result << "," << ONE_WIDTH_SPACE_STRING;
      }
    } else {
      result << format_node(child.get());
    }
  }
  return result.str();
}

std::string Formatter::visit_statement_list(const cst::CSTNode* node) {
  std::ostringstream result;
  // StatementList: 格式化块内的语句列表
  for (const auto& child : node->get_children()) {
    if (child->get_type() == cst::CSTNodeType::Comment) {
      result << format_standalone_comment(child.get());
    } else {
      result << format_node(child.get());
    }
  }
  return result.str();
}

} // namespace czc::formatter