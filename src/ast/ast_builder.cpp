/**
 * @file ast_builder.cpp
 * @brief CST 到 AST 转换器实现
 * @author BegoniaHe
 * @date 2025-11-13
 */

#include "czc/ast/ast_builder.hpp"

#include "czc/cst/cst_node.hpp"

#include <stdexcept>

namespace czc {
namespace ast {

std::shared_ptr<Program> ASTBuilder::build(const cst::CSTNode* cst_root) {
  if (cst_root == nullptr) {
    throw std::runtime_error("CST root is null");
  }

  if (cst_root->get_type() != cst::CSTNodeType::Program) {
    throw std::runtime_error("CST root must be a Program node");
  }

  return build_program(cst_root);
}

std::shared_ptr<Program>
ASTBuilder::build_program(const cst::CSTNode* cst_node) {
  auto program = std::make_shared<Program>(cst_node->get_location());

  // 遍历 CST 的所有顶层子节点，转换为声明
  for (const auto& child : cst_node->get_children()) {
    auto decl = build_declaration(child.get());
    if (decl) {
      program->add_declaration(decl);
    }
  }

  return program;
}

std::shared_ptr<Declaration>
ASTBuilder::build_declaration(const cst::CSTNode* cst_node) {
  if (cst_node == nullptr) {
    return nullptr;
  }

  switch (cst_node->get_type()) {
  case cst::CSTNodeType::VarDeclaration:
    return build_var_declaration(cst_node);

  case cst::CSTNodeType::FnDeclaration:
    return build_function_declaration(cst_node);

  case cst::CSTNodeType::StructDeclaration:
    return build_struct_declaration(cst_node);

  default:
    // TODO: 添加更多声明类型的支持
    return nullptr;
  }
}

std::shared_ptr<Statement>
ASTBuilder::build_statement(const cst::CSTNode* cst_node) {
  if (cst_node == nullptr) {
    return nullptr;
  }

  switch (cst_node->get_type()) {
  case cst::CSTNodeType::BlockStmt:
    return build_block_statement(cst_node);

  case cst::CSTNodeType::ExprStmt:
    return build_expr_statement(cst_node);

  case cst::CSTNodeType::ReturnStmt:
    return build_return_statement(cst_node);

  case cst::CSTNodeType::IfStmt:
    return build_if_statement(cst_node);

  default:
    // TODO: 添加更多语句类型的支持
    return nullptr;
  }
}

std::shared_ptr<Expression>
ASTBuilder::build_expression(const cst::CSTNode* cst_node) {
  if (cst_node == nullptr) {
    return nullptr;
  }

  switch (cst_node->get_type()) {
  case cst::CSTNodeType::BinaryExpr:
    return build_binary_expr(cst_node);

  case cst::CSTNodeType::UnaryExpr:
    return build_unary_expr(cst_node);

  case cst::CSTNodeType::ParenExpr:
    return build_paren_expr(cst_node);

  case cst::CSTNodeType::CallExpr:
    return build_call_expr(cst_node);

  case cst::CSTNodeType::IndexExpr:
    return build_index_expr(cst_node);

  case cst::CSTNodeType::MemberExpr:
    return build_member_expr(cst_node);

  case cst::CSTNodeType::Identifier:
    return build_identifier(cst_node);

  case cst::CSTNodeType::IntegerLiteral:
  case cst::CSTNodeType::FloatLiteral:
  case cst::CSTNodeType::StringLiteral:
  case cst::CSTNodeType::BooleanLiteral:
    return build_literal(cst_node);

  default:
    // TODO: 添加更多表达式类型的支持
    return nullptr;
  }
}

std::shared_ptr<Type> ASTBuilder::build_type(const cst::CSTNode* cst_node) {
  // TODO: 实现类型转换
  return nullptr;
}

// === 具体节点转换实现（骨架） ===

std::shared_ptr<Declaration>
ASTBuilder::build_var_declaration(const cst::CSTNode* cst_node) {
  // CST 结构：VarDeclaration
  //   - Delimiter (let/var 关键字)
  //   - Identifier (变量名)
  //   - [Delimiter (冒号)]
  //   - [TypeAnnotation (类型)]
  //   - [Operator (等号)]
  //   - [Expression (初始化表达式)]
  //   - [Delimiter (分号)]

  std::string var_name;
  std::shared_ptr<Type> type_annotation = nullptr;
  std::shared_ptr<Expression> initializer = nullptr;

  // 遍历子节点提取信息
  for (const auto& child : cst_node->get_children()) {
    switch (child->get_type()) {
    case cst::CSTNodeType::Identifier: {
      // 变量名
      const auto& token = child->get_token();
      if (token.has_value()) {
        var_name = token->value;
      }
      break;
    }

    case cst::CSTNodeType::TypeAnnotation: {
      // 类型注解
      type_annotation = build_type(child.get());
      break;
    }

    case cst::CSTNodeType::BinaryExpr:
    case cst::CSTNodeType::UnaryExpr:
    case cst::CSTNodeType::IntegerLiteral:
    case cst::CSTNodeType::FloatLiteral:
    case cst::CSTNodeType::StringLiteral:
    case cst::CSTNodeType::BooleanLiteral:
    case cst::CSTNodeType::ArrayLiteral:
    case cst::CSTNodeType::TupleLiteral:
    case cst::CSTNodeType::FunctionLiteral:
    case cst::CSTNodeType::StructLiteral:
    case cst::CSTNodeType::CallExpr:
    case cst::CSTNodeType::IndexExpr:
    case cst::CSTNodeType::MemberExpr: {
      // 初始化表达式
      initializer = build_expression(child.get());
      break;
    }

    default:
      // 跳过分隔符、运算符等语法噪音
      break;
    }
  }

  return std::make_shared<VarDecl>(var_name, type_annotation, initializer,
                                   cst_node->get_location());
}

std::shared_ptr<Declaration>
ASTBuilder::build_function_declaration(const cst::CSTNode* cst_node) {
  // CST 结构：FnDeclaration
  //   - Delimiter (fn 关键字)
  //   - Identifier (函数名)
  //   - Delimiter (左括号)
  //   - ParameterList
  //     - Parameter
  //       - Identifier (参数名)
  //       - [Delimiter (冒号)]
  //       - [TypeAnnotation (类型)]
  //     - [Delimiter (逗号)]
  //     - ...
  //   - Delimiter (右括号)
  //   - [Delimiter (箭头)]
  //   - [TypeAnnotation (返回类型)]
  //   - BlockStmt (函数体)

  std::string func_name;
  std::vector<std::shared_ptr<Parameter>> parameters;
  std::shared_ptr<Type> return_type = nullptr;
  std::shared_ptr<BlockStmt> body = nullptr;

  // 遍历子节点提取信息
  for (const auto& child : cst_node->get_children()) {
    switch (child->get_type()) {
    case cst::CSTNodeType::Identifier: {
      // 函数名（第一个 Identifier）
      if (func_name.empty()) {
        const auto& token = child->get_token();
        if (token.has_value()) {
          func_name = token->value;
        }
      }
      break;
    }

    case cst::CSTNodeType::ParameterList: {
      // 解析参数列表
      for (const auto& param_child : child->get_children()) {
        if (param_child->get_type() == cst::CSTNodeType::Parameter) {
          auto param = build_parameter(param_child.get());
          if (param) {
            parameters.push_back(param);
          }
        }
        // 跳过逗号等分隔符
      }
      break;
    }

    case cst::CSTNodeType::TypeAnnotation: {
      // 返回类型（在参数列表之后的类型注解）
      return_type = build_type(child.get());
      break;
    }

    case cst::CSTNodeType::BlockStmt: {
      // 函数体
      auto stmt = build_statement(child.get());
      body = std::dynamic_pointer_cast<BlockStmt>(stmt);
      break;
    }

    default:
      // 跳过分隔符等
      break;
    }
  }

  return std::make_shared<FunctionDecl>(func_name, parameters, return_type,
                                        body, cst_node->get_location());
}

std::shared_ptr<Parameter>
ASTBuilder::build_parameter(const cst::CSTNode* cst_node) {
  // CST 结构：Parameter
  //   - Identifier (参数名)
  //   - [Delimiter (冒号)]
  //   - [TypeAnnotation (类型)]

  std::string param_name;
  std::shared_ptr<Type> param_type = nullptr;

  for (const auto& child : cst_node->get_children()) {
    switch (child->get_type()) {
    case cst::CSTNodeType::Identifier: {
      const auto& token = child->get_token();
      if (token.has_value()) {
        param_name = token->value;
      }
      break;
    }

    case cst::CSTNodeType::TypeAnnotation: {
      param_type = build_type(child.get());
      break;
    }

    default:
      // 跳过分隔符
      break;
    }
  }

  return std::make_shared<Parameter>(param_name, param_type,
                                     cst_node->get_location());
}

std::shared_ptr<Declaration>
ASTBuilder::build_struct_declaration(const cst::CSTNode* cst_node) {
  // CST 结构：StructDeclaration
  //   - Delimiter (struct 关键字)
  //   - Identifier (结构体名)
  //   - Delimiter (左大括号)
  //   - StructField (字段1)
  //     - Identifier (字段名)
  //     - Delimiter (冒号)
  //     - TypeAnnotation (字段类型)
  //   - [Delimiter (逗号)]
  //   - StructField (字段2)
  //     - ...
  //   - Delimiter (右大括号)
  //   - [Delimiter (分号)]

  std::string struct_name;
  std::vector<std::shared_ptr<StructField>> fields;

  // 遍历子节点提取信息
  for (const auto& child : cst_node->get_children()) {
    switch (child->get_type()) {
    case cst::CSTNodeType::Identifier: {
      // 结构体名（第一个 Identifier）
      if (struct_name.empty()) {
        const auto& token = child->get_token();
        if (token.has_value()) {
          struct_name = token->value;
        }
      }
      break;
    }

    case cst::CSTNodeType::StructField: {
      // 解析结构体字段
      auto field = build_struct_field(child.get());
      if (field) {
        fields.push_back(field);
      }
      break;
    }

    default:
      // 跳过分隔符、注释等
      break;
    }
  }

  return std::make_shared<StructDecl>(struct_name, fields,
                                      cst_node->get_location());
}

std::shared_ptr<StructField>
ASTBuilder::build_struct_field(const cst::CSTNode* cst_node) {
  // CST 结构：StructField
  //   - Identifier (字段名)
  //   - Delimiter (冒号)
  //   - TypeAnnotation (字段类型)

  std::string field_name;
  std::shared_ptr<Type> field_type = nullptr;

  // 遍历子节点提取信息
  for (const auto& child : cst_node->get_children()) {
    switch (child->get_type()) {
    case cst::CSTNodeType::Identifier: {
      // 字段名
      if (field_name.empty()) {
        const auto& token = child->get_token();
        if (token.has_value()) {
          field_name = token->value;
        }
      }
      break;
    }

    case cst::CSTNodeType::TypeAnnotation: {
      // 字段类型
      field_type = build_type(child.get());
      break;
    }

    default:
      // 跳过冒号等分隔符
      break;
    }
  }

  return std::make_shared<StructField>(field_name, field_type,
                                       cst_node->get_location());
}

std::shared_ptr<Statement>
ASTBuilder::build_block_statement(const cst::CSTNode* cst_node) {
  auto block = std::make_shared<BlockStmt>(cst_node->get_location());

  // CST 结构：BlockStmt
  //   - Delimiter (左大括号)
  //   - StatementList (包含所有语句)
  //     - Statement
  //     - Statement
  //     - ...
  //   - Delimiter (右大括号)

  // 查找 StatementList 节点
  for (const auto& child : cst_node->get_children()) {
    if (child->get_type() == cst::CSTNodeType::StatementList) {
      // 遍历 StatementList 中的所有语句
      for (const auto& stmt_child : child->get_children()) {
        // 跳过注释
        if (stmt_child->get_type() == cst::CSTNodeType::Comment) {
          continue;
        }

        auto stmt = build_statement(stmt_child.get());
        if (stmt) {
          block->add_statement(stmt);
        }
      }
      break;
    }
  }

  return block;
}

std::shared_ptr<Statement>
ASTBuilder::build_expr_statement(const cst::CSTNode* cst_node) {
  // CST 结构：ExprStmt
  //   - Expression
  //   - [Delimiter (分号)]

  std::shared_ptr<Expression> expr = nullptr;

  // 遍历子节点，查找表达式
  for (const auto& child : cst_node->get_children()) {
    if (child->get_type() != cst::CSTNodeType::Delimiter &&
        child->get_type() != cst::CSTNodeType::Comment) {
      expr = build_expression(child.get());
      break;
    }
  }

  if (!expr) {
    return nullptr;
  }

  return std::make_shared<ExprStmt>(expr, cst_node->get_location());
}

std::shared_ptr<Statement>
ASTBuilder::build_return_statement(const cst::CSTNode* cst_node) {
  // CST 结构：ReturnStmt
  //   - Delimiter (return 关键字)
  //   - [Expression (返回值)]
  //   - [Delimiter (分号)]

  std::shared_ptr<Expression> return_value = nullptr;

  // 遍历子节点，查找返回值表达式
  for (const auto& child : cst_node->get_children()) {
    if (child->get_type() != cst::CSTNodeType::Delimiter &&
        child->get_type() != cst::CSTNodeType::Comment) {
      return_value = build_expression(child.get());
      break;
    }
  }

  // 返回值可以为空（void 返回）
  return std::make_shared<ReturnStmt>(return_value, cst_node->get_location());
}

std::shared_ptr<Statement>
ASTBuilder::build_if_statement(const cst::CSTNode* cst_node) {
  // CST 结构：IfStmt
  //   - Delimiter (if 关键字)
  //   - Expression (条件)
  //   - BlockStmt (then 分支)
  //   - [Delimiter (else 关键字)]
  //   - [BlockStmt/IfStmt (else 分支)]

  std::shared_ptr<Expression> condition = nullptr;
  std::shared_ptr<Statement> then_branch = nullptr;
  std::shared_ptr<Statement> else_branch = nullptr;

  bool found_else = false;

  // 遍历子节点
  for (const auto& child : cst_node->get_children()) {
    if (child->get_type() == cst::CSTNodeType::Delimiter) {
      // 检查是否是 else 关键字
      const auto& token = child->get_token();
      if (token.has_value() && token->value == "else") {
        found_else = true;
      }
      continue;
    }

    if (child->get_type() == cst::CSTNodeType::BlockStmt ||
        child->get_type() == cst::CSTNodeType::IfStmt) {
      // 语句块或嵌套的 if 语句
      auto stmt = build_statement(child.get());
      if (!then_branch && !found_else) {
        then_branch = stmt;
      } else if (found_else) {
        else_branch = stmt;
      }
    } else {
      // 表达式（条件）
      if (!condition) {
        condition = build_expression(child.get());
      }
    }
  }

  if (!condition || !then_branch) {
    return nullptr;
  }

  return std::make_shared<IfStmt>(condition, then_branch, else_branch,
                                  cst_node->get_location());
}

std::shared_ptr<Expression>
ASTBuilder::build_binary_expr(const cst::CSTNode* cst_node) {
  // CST 结构：BinaryExpr
  //   - Expression (左操作数)
  //   - Operator (运算符)
  //   - Expression (右操作数)

  std::shared_ptr<Expression> left = nullptr;
  std::shared_ptr<Expression> right = nullptr;
  BinaryOperator op;
  bool found_operator = false;

  // 遍历子节点
  for (const auto& child : cst_node->get_children()) {
    if (child->get_type() == cst::CSTNodeType::Operator) {
      // 提取运算符
      const auto& token = child->get_token();
      if (token.has_value()) {
        op = parse_binary_operator(token->value);
        found_operator = true;
      }
    } else {
      // 提取操作数
      auto expr = build_expression(child.get());
      if (expr) {
        if (!left) {
          left = expr;
        } else {
          right = expr;
        }
      }
    }
  }

  if (!left || !right || !found_operator) {
    // 错误情况：表达式不完整
    return nullptr;
  }

  return std::make_shared<BinaryOpExpr>(op, left, right,
                                        cst_node->get_location());
}

std::shared_ptr<Expression>
ASTBuilder::build_unary_expr(const cst::CSTNode* cst_node) {
  // CST 结构：UnaryExpr
  //   - Operator (运算符)
  //   - Expression (操作数)

  UnaryOperator op;
  std::shared_ptr<Expression> operand = nullptr;
  bool found_operator = false;

  // 遍历子节点
  for (const auto& child : cst_node->get_children()) {
    if (child->get_type() == cst::CSTNodeType::Operator) {
      // 提取运算符
      const auto& token = child->get_token();
      if (token.has_value()) {
        op = parse_unary_operator(token->value);
        found_operator = true;
      }
    } else {
      // 提取操作数
      operand = build_expression(child.get());
    }
  }

  if (!operand || !found_operator) {
    // 错误情况：表达式不完整
    return nullptr;
  }

  return std::make_shared<UnaryOpExpr>(op, operand, cst_node->get_location());
}

std::shared_ptr<Expression>
ASTBuilder::build_literal(const cst::CSTNode* cst_node) {
  const auto& token = cst_node->get_token();
  if (!token.has_value()) {
    return nullptr;
  }

  switch (cst_node->get_type()) {
  case cst::CSTNodeType::IntegerLiteral: {
    int64_t value = parse_integer_literal(token->value);
    return std::make_shared<IntegerLiteral>(value, cst_node->get_location());
  }

  case cst::CSTNodeType::FloatLiteral: {
    double value = parse_float_literal(token->value);
    return std::make_shared<FloatLiteral>(value, cst_node->get_location());
  }

  case cst::CSTNodeType::StringLiteral: {
    std::string value = parse_string_literal(token->value);
    return std::make_shared<StringLiteral>(value, cst_node->get_location());
  }

  case cst::CSTNodeType::BooleanLiteral: {
    bool value = (token->value == "true");
    return std::make_shared<BooleanLiteral>(value, cst_node->get_location());
  }

  default:
    return nullptr;
  }
}

std::shared_ptr<Expression>
ASTBuilder::build_identifier(const cst::CSTNode* cst_node) {
  const auto& token = cst_node->get_token();
  if (!token.has_value()) {
    return nullptr;
  }

  return std::make_shared<Identifier>(token->value, cst_node->get_location());
}

// === 辅助方法实现 ===

BinaryOperator ASTBuilder::parse_binary_operator(const std::string& op_str) {
  if (op_str == "+")
    return BinaryOperator::Add;
  if (op_str == "-")
    return BinaryOperator::Sub;
  if (op_str == "*")
    return BinaryOperator::Mul;
  if (op_str == "/")
    return BinaryOperator::Div;
  if (op_str == "%")
    return BinaryOperator::Mod;
  if (op_str == "==")
    return BinaryOperator::Eq;
  if (op_str == "!=")
    return BinaryOperator::Ne;
  if (op_str == "<")
    return BinaryOperator::Lt;
  if (op_str == "<=")
    return BinaryOperator::Le;
  if (op_str == ">")
    return BinaryOperator::Gt;
  if (op_str == ">=")
    return BinaryOperator::Ge;
  if (op_str == "&&")
    return BinaryOperator::And;
  if (op_str == "||")
    return BinaryOperator::Or;

  throw std::runtime_error("Unknown binary operator: " + op_str);
}

UnaryOperator ASTBuilder::parse_unary_operator(const std::string& op_str) {
  if (op_str == "+")
    return UnaryOperator::Plus;
  if (op_str == "-")
    return UnaryOperator::Minus;
  if (op_str == "!")
    return UnaryOperator::Not;

  throw std::runtime_error("Unknown unary operator: " + op_str);
}

int64_t ASTBuilder::parse_integer_literal(const std::string& literal_str) {
  try {
    return std::stoll(literal_str);
  } catch (const std::exception& e) {
    throw std::runtime_error("Failed to parse integer literal: " + literal_str);
  }
}

double ASTBuilder::parse_float_literal(const std::string& literal_str) {
  try {
    return std::stod(literal_str);
  } catch (const std::exception& e) {
    throw std::runtime_error("Failed to parse float literal: " + literal_str);
  }
}

std::string ASTBuilder::parse_string_literal(const std::string& literal_str) {
  // TODO: 实现字符串字面量解析（处理转义字符）
  // 现在简单地移除引号
  if (literal_str.length() >= 2 && literal_str.front() == '"' &&
      literal_str.back() == '"') {
    return literal_str.substr(1, literal_str.length() - 2);
  }
  return literal_str;
}

std::shared_ptr<Expression>
ASTBuilder::build_paren_expr(const cst::CSTNode* cst_node) {
  // CST 结构：ParenExpr
  //   - Delimiter (左括号)
  //   - Expression (内部表达式)
  //   - Delimiter (右括号)

  std::shared_ptr<Expression> expr = nullptr;

  // 遍历子节点,跳过分隔符
  for (const auto& child : cst_node->get_children()) {
    if (child->get_type() != cst::CSTNodeType::Delimiter) {
      expr = build_expression(child.get());
      break;
    }
  }

  if (!expr) {
    return nullptr;
  }

  return std::make_shared<ParenExpr>(expr, cst_node->get_location());
}

std::shared_ptr<Expression>
ASTBuilder::build_call_expr(const cst::CSTNode* cst_node) {
  // CST 结构：CallExpr
  //   - Expression (被调用的函数)
  //   - Delimiter (左括号)
  //   - ArgumentList
  //     - Expression (参数1)
  //     - Delimiter (逗号)
  //     - Expression (参数2)
  //     - ...
  //   - Delimiter (右括号)

  std::shared_ptr<Expression> callee = nullptr;
  std::vector<std::shared_ptr<Expression>> arguments;

  // 遍历子节点
  for (const auto& child : cst_node->get_children()) {
    if (child->get_type() == cst::CSTNodeType::ArgumentList) {
      // 解析参数列表
      for (const auto& arg_child : child->get_children()) {
        if (arg_child->get_type() != cst::CSTNodeType::Delimiter &&
            arg_child->get_type() != cst::CSTNodeType::Comment) {
          auto arg = build_expression(arg_child.get());
          if (arg) {
            arguments.push_back(arg);
          }
        }
      }
    } else if (child->get_type() != cst::CSTNodeType::Delimiter &&
               child->get_type() != cst::CSTNodeType::Comment) {
      // 第一个非分隔符表达式是被调用的函数
      if (!callee) {
        callee = build_expression(child.get());
      }
    }
  }

  if (!callee) {
    return nullptr;
  }

  return std::make_shared<CallExpr>(callee, arguments,
                                    cst_node->get_location());
}

std::shared_ptr<Expression>
ASTBuilder::build_index_expr(const cst::CSTNode* cst_node) {
  // CST 结构：IndexExpr
  //   - Expression (被索引的对象)
  //   - Delimiter (左方括号)
  //   - Expression (索引)
  //   - Delimiter (右方括号)

  std::shared_ptr<Expression> object = nullptr;
  std::shared_ptr<Expression> index = nullptr;

  // 遍历子节点
  int expr_count = 0;
  for (const auto& child : cst_node->get_children()) {
    if (child->get_type() != cst::CSTNodeType::Delimiter &&
        child->get_type() != cst::CSTNodeType::Comment) {
      auto expr = build_expression(child.get());
      if (expr) {
        if (expr_count == 0) {
          object = expr;
        } else if (expr_count == 1) {
          index = expr;
        }
        expr_count++;
      }
    }
  }

  if (!object || !index) {
    return nullptr;
  }

  return std::make_shared<IndexExpr>(object, index, cst_node->get_location());
}

std::shared_ptr<Expression>
ASTBuilder::build_member_expr(const cst::CSTNode* cst_node) {
  // CST 结构：MemberExpr
  //   - Expression (对象)
  //   - Delimiter (点号)
  //   - Identifier (成员名)

  std::shared_ptr<Expression> object = nullptr;
  std::string member;

  // 遍历子节点
  for (const auto& child : cst_node->get_children()) {
    if (child->get_type() == cst::CSTNodeType::Identifier) {
      // 成员名
      const auto& token = child->get_token();
      if (token.has_value()) {
        member = token->value;
      }
    } else if (child->get_type() != cst::CSTNodeType::Delimiter &&
               child->get_type() != cst::CSTNodeType::Comment) {
      // 对象表达式
      if (!object) {
        object = build_expression(child.get());
      }
    }
  }

  if (!object || member.empty()) {
    return nullptr;
  }

  return std::make_shared<MemberExpr>(object, member, cst_node->get_location());
}

} // namespace ast
} // namespace czc
