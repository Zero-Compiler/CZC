/**
 * @file formatter_expr.cpp
 * @brief 表达式节点格式化实现 (二元、一元、调用、赋值、字面量)
 * @details 实现所有表达式节点的格式化逻辑，包括二元运算、一元运算、
 *          函数调用、赋值表达式和各种字面量。
 * @author BegoniaHe
 * @date 2025-11-14
 */

#include "czc/formatter/formatter.hpp"

#include <sstream>

namespace czc::formatter {

std::string Formatter::visit_binary_expr(const cst::CSTNode* node) {
  std::ostringstream result;
  // BinaryExpr: a + b
  for (size_t i = 0; i < node->get_children().size(); ++i) {
    const auto& child = node->get_children()[i];

    if (child->get_type() == cst::CSTNodeType::Operator) {
      result << ONE_WIDTH_SPACE_STRING << format_node(child.get())
             << ONE_WIDTH_SPACE_STRING;
    } else {
      result << format_node(child.get());
    }
  }
  return result.str();
}

std::string Formatter::visit_unary_expr(const cst::CSTNode* node) {
  // UnaryExpr: 简单地格式化所有子节点
  std::ostringstream result;
  for (const auto& child : node->get_children()) {
    result << format_node(child.get());
  }
  return result.str();
}

std::string Formatter::visit_call_expr(const cst::CSTNode* node) {
  // CallExpr: 简单地格式化所有子节点
  std::ostringstream result;
  for (const auto& child : node->get_children()) {
    result << format_node(child.get());
  }
  return result.str();
}

std::string Formatter::visit_index_expr(const cst::CSTNode* node) {
  std::ostringstream result;
  // IndexExpr: array[index]
  const auto& children = node->get_children();
  for (size_t i = 0; i < children.size(); ++i) {
    const auto& child = children[i];

    if (child->get_type() == cst::CSTNodeType::Delimiter) {
      const auto& token = child->get_token();
      if (token.has_value()) {
        result << token->value;
      }
    } else {
      result << format_node(child.get());
    }
  }
  return result.str();
}

std::string Formatter::visit_member_expr(const cst::CSTNode* node) {
  std::ostringstream result;
  // MemberExpr: object.member
  const auto& children = node->get_children();
  for (size_t i = 0; i < children.size(); ++i) {
    const auto& child = children[i];

    if (child->get_type() == cst::CSTNodeType::Operator) {
      const auto& token = child->get_token();
      if (token.has_value() && token->token_type == lexer::TokenType::Dot) {
        result << ".";
      } else {
        result << format_node(child.get());
      }
    } else {
      result << format_node(child.get());
    }
  }
  return result.str();
}

std::string Formatter::visit_assign_expr(const cst::CSTNode* node) {
  std::ostringstream result;
  // AssignExpr: lvalue = rvalue
  const auto& children = node->get_children();
  for (size_t i = 0; i < children.size(); ++i) {
    const auto& child = children[i];

    if (child->get_type() == cst::CSTNodeType::Operator) {
      const auto& token = child->get_token();
      if (token.has_value() && token->token_type == lexer::TokenType::Equal) {
        result << ONE_WIDTH_SPACE_STRING << "=" << ONE_WIDTH_SPACE_STRING;
      } else {
        result << format_node(child.get());
      }
    } else {
      result << format_node(child.get());
    }
  }
  return result.str();
}

std::string Formatter::visit_index_assign_expr(const cst::CSTNode* node) {
  std::ostringstream result;
  // IndexAssignExpr: array[index] = value
  const auto& children = node->get_children();
  for (size_t i = 0; i < children.size(); ++i) {
    const auto& child = children[i];

    if (child->get_type() == cst::CSTNodeType::Operator) {
      const auto& token = child->get_token();
      if (token.has_value() && token->token_type == lexer::TokenType::Equal) {
        result << ONE_WIDTH_SPACE_STRING << "=" << ONE_WIDTH_SPACE_STRING;
      } else {
        result << format_node(child.get());
      }
    } else {
      result << format_node(child.get());
    }
  }
  return result.str();
}

std::string Formatter::visit_array_literal(const cst::CSTNode* node) {
  std::ostringstream result;
  // ArrayLiteral: [elem1, elem2, elem3]
  const auto& children = node->get_children();
  for (size_t i = 0; i < children.size(); ++i) {
    const auto& child = children[i];

    if (child->get_type() == cst::CSTNodeType::Delimiter) {
      const auto& token = child->get_token();
      if (token.has_value()) {
        if (token->token_type == lexer::TokenType::LeftBracket) {
          result << "[";
        } else if (token->token_type == lexer::TokenType::RightBracket) {
          result << "]";
        } else if (token->token_type == lexer::TokenType::Comma) {
          result << ",";
          if (options.space_after_comma) {
            result << ONE_WIDTH_SPACE_STRING;
          }
        } else {
          result << token->value;
        }
      }
    } else {
      result << format_node(child.get());
    }
  }
  return result.str();
}

std::string Formatter::visit_tuple_literal(const cst::CSTNode* node) {
  std::ostringstream result;
  // TupleLiteral: (expr1, expr2, ...)
  for (const auto& child : node->get_children()) {
    if (child->get_type() == cst::CSTNodeType::Delimiter) {
      const auto& token = child->get_token();
      if (token.has_value()) {
        if (token->token_type == lexer::TokenType::Comma) {
          result << "," << ONE_WIDTH_SPACE_STRING;
        } else {
          result << token->value;
        }
      }
    } else {
      result << format_node(child.get());
    }
  }
  return result.str();
}

std::string Formatter::visit_function_literal(const cst::CSTNode* node) {
  std::ostringstream result;
  // FunctionLiteral: fn (params) { body }
  for (const auto& child : node->get_children()) {
    if (child->get_type() == cst::CSTNodeType::Delimiter) {
      const auto& token = child->get_token();
      if (token.has_value()) {
        if (token->token_type == lexer::TokenType::Fn) {
          result << token->value << ONE_WIDTH_SPACE_STRING;
        } else {
          result << format_node(child.get());
        }
      }
    } else if (child->get_type() == cst::CSTNodeType::BlockStmt) {
      result << ONE_WIDTH_SPACE_STRING << format_node(child.get());
    } else {
      result << format_node(child.get());
    }
  }
  return result.str();
}

std::string Formatter::visit_struct_literal(const cst::CSTNode* node) {
  std::ostringstream result;

  // TypeName { field: value, ... }
  for (size_t i = 0; i < node->get_children().size(); ++i) {
    const auto& child = node->get_children()[i];

    if (child->get_type() == cst::CSTNodeType::Delimiter) {
      const auto& token = child->get_token();
      if (token.has_value()) {
        if (token->token_type == lexer::TokenType::LeftBrace) {
          result << ONE_WIDTH_SPACE_STRING << token->value << "\n";
          indent_level++;
        } else if (token->token_type == lexer::TokenType::RightBrace) {
          indent_level--;
          result << get_indent() << token->value;
        } else if (token->token_type == lexer::TokenType::Comma) {
          result << token->value << "\n";
        } else if (token->token_type == lexer::TokenType::Colon) {
          result << token->value << ONE_WIDTH_SPACE_STRING;
        } else {
          result << format_node(child.get());
        }
      }
    } else if (child->get_type() == cst::CSTNodeType::Identifier) {
      // 字段名或类型名
      if (i == 0) {
        // 类型名
        result << format_node(child.get());
      } else {
        // 字段名
        result << get_indent() << format_node(child.get());
      }
    } else if (child->get_type() == cst::CSTNodeType::Comment) {
      result << format_standalone_comment(child.get());
    } else {
      // 字段值表达式
      result << format_node(child.get());
    }
  }

  return result.str();
}

std::string Formatter::visit_paren_expr(const cst::CSTNode* node) {
  std::ostringstream result;
  // ParenExpr: (expression)
  const auto& children = node->get_children();
  for (size_t i = 0; i < children.size(); ++i) {
    const auto& child = children[i];

    if (child->get_type() == cst::CSTNodeType::Delimiter) {
      const auto& token = child->get_token();
      if (token.has_value()) {
        result << token->value;
      }
    } else {
      result << format_node(child.get());
    }
  }
  return result.str();
}

std::string Formatter::visit_integer_literal(const cst::CSTNode* node) {
  if (node->get_token().has_value()) {
    return node->get_token()->value;
  }
  return "";
}

std::string Formatter::visit_float_literal(const cst::CSTNode* node) {
  if (node->get_token().has_value()) {
    return node->get_token()->value;
  }
  return "";
}

std::string Formatter::visit_string_literal(const cst::CSTNode* node) {
  if (node->get_token().has_value()) {
    const auto& token = node->get_token().value();
    // 直接使用原始字面量文本
    return token.raw_literal;
  }
  return "";
}

std::string Formatter::visit_boolean_literal(const cst::CSTNode* node) {
  if (node->get_token().has_value()) {
    return node->get_token()->value;
  }
  return "";
}

std::string Formatter::visit_identifier(const cst::CSTNode* node) {
  if (node->get_token().has_value()) {
    return node->get_token()->value;
  }
  return "";
}

std::string Formatter::visit_operator(const cst::CSTNode* node) {
  if (node->get_token().has_value()) {
    const auto& token = node->get_token().value();
    // 跳过虚拟 Token
    if (token.is_synthetic) {
      return "";
    }
    return token.value;
  }
  return "";
}

std::string Formatter::visit_comment(const cst::CSTNode* node) {
  if (node->get_token().has_value()) {
    const auto& token = node->get_token().value();
    // 虚拟 Token 不会是注释，但为了一致性还是检查
    if (token.is_synthetic) {
      return "";
    }
    return token.value;
  }
  return "";
}

std::string Formatter::visit_argument_list(const cst::CSTNode* node) {
  std::ostringstream result;
  // ArgumentList: arg1, arg2, arg3 (不包含括号)
  const auto& children = node->get_children();
  for (size_t i = 0; i < children.size(); ++i) {
    const auto& child = children[i];

    if (child->get_type() == cst::CSTNodeType::Delimiter) {
      const auto& token = child->get_token();
      if (token.has_value() && token->token_type == lexer::TokenType::Comma) {
        result << ",";
        if (options.space_after_comma) {
          result << ONE_WIDTH_SPACE_STRING;
        }
      } else if (token.has_value()) {
        result << token->value;
      }
    } else {
      result << format_node(child.get());
    }
  }
  return result.str();
}
std::string Formatter::visit_delimiter(const cst::CSTNode* node) {
  if (node->get_token().has_value()) {
    const auto& token = node->get_token().value();
    // 跳过虚拟 Token（用于错误恢复的占位符）
    if (token.is_synthetic) {
      return "";
    }
    return token.value;
  }
  return "";
}

std::string Formatter::visit_member_assign_expr(const cst::CSTNode* node) {
  std::ostringstream result;
  // obj.member = value
  for (size_t i = 0; i < node->get_children().size(); ++i) {
    const auto& child = node->get_children()[i];
    if (child->get_type() == cst::CSTNodeType::Operator) {
      result << ONE_WIDTH_SPACE_STRING << format_node(child.get())
             << ONE_WIDTH_SPACE_STRING;
    } else {
      result << format_node(child.get());
    }
  }
  return result.str();
}

} // namespace czc::formatter