/**
 * @file formatter_type.cpp
 * @brief 类型表达式格式化实现 (数组、元组、联合、交集、函数签名)
 * @details 实现所有类型表达式的格式化逻辑，包括数组类型、元组类型、
 *          联合类型、交集类型、否定类型和函数签名类型。
 * @author BegoniaHe
 * @date 2025-11-14
 */

#include "czc/formatter/formatter.hpp"

#include <sstream>

namespace czc::formatter {

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

std::string Formatter::visit_sized_array_type(const cst::CSTNode* node) {
  std::ostringstream result;
  // SizedArrayType: Type[5]
  for (const auto& child : node->get_children()) {
    result << format_node(child.get());
  }
  return result.str();
}

std::string Formatter::visit_union_type(const cst::CSTNode* node) {
  std::ostringstream result;
  // T1 | T2
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

std::string Formatter::visit_intersection_type(const cst::CSTNode* node) {
  std::ostringstream result;
  // T1 & T2
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

std::string Formatter::visit_negation_type(const cst::CSTNode* node) {
  std::ostringstream result;
  // ~T
  for (const auto& child : node->get_children()) {
    result << format_node(child.get());
  }
  return result.str();
}

std::string Formatter::visit_tuple_type(const cst::CSTNode* node) {
  std::ostringstream result;
  // (T1, T2, T3)
  for (size_t i = 0; i < node->get_children().size(); ++i) {
    const auto& child = node->get_children()[i];
    if (child->get_type() == cst::CSTNodeType::Delimiter) {
      const auto& token = child->get_token();
      if (token.has_value() && token->token_type == lexer::TokenType::Comma) {
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

std::string Formatter::visit_function_signature_type(const cst::CSTNode* node) {
  std::ostringstream result;
  // (T1, T2) -> (T3, T4)
  for (size_t i = 0; i < node->get_children().size(); ++i) {
    const auto& child = node->get_children()[i];
    if (child->get_type() == cst::CSTNodeType::Delimiter) {
      const auto& token = child->get_token();
      if (token.has_value()) {
        if (token->token_type == lexer::TokenType::Arrow) {
          result << ONE_WIDTH_SPACE_STRING << token->value
                 << ONE_WIDTH_SPACE_STRING;
        } else if (token->token_type == lexer::TokenType::Comma) {
          result << token->value << ONE_WIDTH_SPACE_STRING;
        } else {
          result << format_node(child.get());
        }
      }
    } else {
      result << format_node(child.get());
    }
  }
  return result.str();
}

std::string Formatter::visit_anonymous_struct_type(const cst::CSTNode* node) {
  std::ostringstream result;
  // struct { field: Type, ... }
  for (size_t i = 0; i < node->get_children().size(); ++i) {
    const auto& child = node->get_children()[i];

    if (child->get_type() == cst::CSTNodeType::Delimiter) {
      const auto& token = child->get_token();
      if (token.has_value()) {
        if (token->token_type == lexer::TokenType::Struct) {
          result << token->value << ONE_WIDTH_SPACE_STRING;
        } else if (token->token_type == lexer::TokenType::LeftBrace) {
          result << token->value << ONE_WIDTH_SPACE_STRING;
        } else if (token->token_type == lexer::TokenType::RightBrace) {
          result << ONE_WIDTH_SPACE_STRING << token->value;
        } else if (token->token_type == lexer::TokenType::Comma) {
          result << token->value << ONE_WIDTH_SPACE_STRING;
        } else {
          result << format_node(child.get());
        }
      }
    } else if (child->get_type() == cst::CSTNodeType::StructField) {
      result << format_node(child.get());
    }
  }
  return result.str();
}

} // namespace czc::formatter