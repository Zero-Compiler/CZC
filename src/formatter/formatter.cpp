/**
 * @file formatter.cpp
 * @brief `Formatter` 类的功能实现。
 * @author BegoniaHe
 * @date 2025-11-11
 */

#include "czc/formatter/formatter.hpp"

#include <sstream>

namespace czc {
namespace formatter {

Formatter::Formatter(const FormatOptions& options)
    : options(options), error_collector(), indent_level(0) {}

std::string Formatter::format(const cst::CSTNode* root) {
  if (!root) {
    return "";
  }
  indent_level = 0;
  error_collector.clear();
  return format_node(root);
}

std::string Formatter::format_node(const cst::CSTNode* node) {
  if (!node) {
    return "";
  }

  // 使用访问者模式分派到具体的 visit 方法
  switch (node->get_type()) {
  case cst::CSTNodeType::Program:
    return visit_program(node);
  case cst::CSTNodeType::VarDeclaration:
    return visit_var_declaration(node);
  case cst::CSTNodeType::FnDeclaration:
    return visit_fn_declaration(node);
  case cst::CSTNodeType::ReturnStmt:
    return visit_return_stmt(node);
  case cst::CSTNodeType::IfStmt:
    return visit_if_stmt(node);
  case cst::CSTNodeType::WhileStmt:
    return visit_while_stmt(node);
  case cst::CSTNodeType::BlockStmt:
    return visit_block_stmt(node);
  case cst::CSTNodeType::ExprStmt:
    return visit_expr_stmt(node);
  case cst::CSTNodeType::BinaryExpr:
    return visit_binary_expr(node);
  case cst::CSTNodeType::UnaryExpr:
    return visit_unary_expr(node);
  case cst::CSTNodeType::CallExpr:
    return visit_call_expr(node);
  case cst::CSTNodeType::IndexExpr:
    return visit_index_expr(node);
  case cst::CSTNodeType::MemberExpr:
    return visit_member_expr(node);
  case cst::CSTNodeType::AssignExpr:
    return visit_assign_expr(node);
  case cst::CSTNodeType::IndexAssignExpr:
    return visit_index_assign_expr(node);
  case cst::CSTNodeType::ArrayLiteral:
    return visit_array_literal(node);
  case cst::CSTNodeType::ParenExpr:
    return visit_paren_expr(node);
  case cst::CSTNodeType::IntegerLiteral:
    return visit_integer_literal(node);
  case cst::CSTNodeType::FloatLiteral:
    return visit_float_literal(node);
  case cst::CSTNodeType::StringLiteral:
    return visit_string_literal(node);
  case cst::CSTNodeType::BooleanLiteral:
    return visit_boolean_literal(node);
  case cst::CSTNodeType::Identifier:
    return visit_identifier(node);
  case cst::CSTNodeType::TypeAnnotation:
    return visit_type_annotation(node);
  case cst::CSTNodeType::ArrayType:
    return visit_array_type(node);
  case cst::CSTNodeType::Parameter:
    return visit_parameter(node);
  case cst::CSTNodeType::ParameterList:
    return visit_parameter_list(node);
  case cst::CSTNodeType::ArgumentList:
    return visit_argument_list(node);
  case cst::CSTNodeType::StatementList:
    return visit_statement_list(node);
  case cst::CSTNodeType::Operator:
    return visit_operator(node);
  case cst::CSTNodeType::Delimiter:
    return visit_delimiter(node);
  case cst::CSTNodeType::Comment:
    return visit_comment(node);
  default:
    // 未处理的节点类型，递归格式化子节点
    std::ostringstream result;
    for (const auto& child : node->get_children()) {
      result << format_node(child.get());
    }
    return result.str();
  }
}

// --- 访问者方法实现 ---

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
  // FnDeclaration: fn add(a, b) { ... }
  result << get_indent();
  for (size_t i = 0; i < node->get_children().size(); ++i) {
    const auto& child = node->get_children()[i];

    if (child->get_type() == cst::CSTNodeType::Delimiter) {
      const auto& token = child->get_token();
      if (token.has_value()) {
        result << token->value;
        // `fn` 关键字后加空格
        if (token->token_type == lexer::TokenType::Fn) {
          result << ONE_WIDTH_SPACE_STRING;
        }
      }
    } else if (child->get_type() == cst::CSTNodeType::ParameterList) {
      result << format_node(child.get());
    } else if (child->get_type() == cst::CSTNodeType::BlockStmt) {
      result << ONE_WIDTH_SPACE_STRING << format_node(child.get());
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
  result << get_indent() << "if";

  // if 语句结构: if condition { block } [else { block }]
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
    return node->get_token()->value;
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

std::string Formatter::visit_type_annotation(const cst::CSTNode* node) {
  std::ostringstream result;
  // TypeAnnotation: : type
  const auto& children = node->get_children();
  for (size_t i = 0; i < children.size(); ++i) {
    const auto& child = children[i];

    if (child->get_type() == cst::CSTNodeType::Delimiter) {
      const auto& token = child->get_token();
      if (token.has_value() && token->token_type == lexer::TokenType::Colon) {
        result << ":" << ONE_WIDTH_SPACE_STRING;
      } else if (token.has_value()) {
        result << token->value;
      }
    } else {
      result << format_node(child.get());
    }
  }

  // Fallback to token value if no children
  if (children.empty() && node->get_token().has_value()) {
    return node->get_token()->value;
  }

  return result.str();
}

std::string Formatter::visit_array_type(const cst::CSTNode* node) {
  std::ostringstream result;
  // ArrayType: Type[]
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

std::string Formatter::get_indent() const {
  if (options.indent_style == IndentStyle::SPACES) {
    return std::string(indent_level * options.indent_width, ' ');
  } else {
    return std::string(indent_level, '\t');
  }
}

std::string Formatter::format_inline_comment(const cst::CSTNode* comment) {
  if (!comment) {
    return "";
  }
  std::ostringstream result;
  result << TWO_WIDTH_SPACE_STRING;
  if (comment->get_token().has_value()) {
    result << comment->get_token()->value;
  }
  return result.str();
}

std::string Formatter::format_standalone_comment(const cst::CSTNode* comment) {
  if (!comment) {
    return "";
  }
  std::ostringstream result;
  result << get_indent();
  if (comment->get_token().has_value()) {
    result << comment->get_token()->value;
  }
  result << "\n";
  return result.str();
}

} // namespace formatter
} // namespace czc
