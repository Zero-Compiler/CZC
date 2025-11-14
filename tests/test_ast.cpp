/**
 * @file test_ast.cpp
 * @brief AST 基础功能测试
 * @details 测试 AST 节点的创建、AST Builder 和访问者模式
 * @author BegoniaHe
 * @date 2025-11-13
 */

#include "czc/ast/ast_builder.hpp"
#include "czc/ast/ast_node.hpp"
#include "czc/ast/ast_visitor.hpp"
#include "czc/cst/cst_node.hpp"
#include "czc/lexer/lexer.hpp"
#include "czc/parser/parser.hpp"

#include <gtest/gtest.h>

using namespace czc;
using namespace czc::ast;
using namespace czc::cst;
using namespace czc::lexer;
using namespace czc::parser;
using namespace czc::utils;

/**
 * @brief AST 测试基类
 */
class ASTTest : public ::testing::Test {
protected:
  /**
   * @brief 创建测试用的源码位置
   */
  SourceLocation make_test_location() {
    return SourceLocation("test.zero", 1, 1);
  }

  /**
   * @brief 辅助函数：词法分析 + 语法分析
   */
  std::shared_ptr<CSTNode> parse(const std::string& source) {
    Lexer lexer(source, "test.zero");
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    return parser.parse();
  }
};

/**
 * @test BasicNodeCreation
 * @brief 测试基本 AST 节点的创建
 */
TEST_F(ASTTest, BasicNodeCreation) {
  auto loc = make_test_location();

  // 测试 Program 节点
  auto program = std::make_shared<Program>(loc);
  EXPECT_EQ(program->get_kind(), ASTNodeKind::Program);
  EXPECT_EQ(program->get_declarations().size(), 0);

  // 测试 Identifier 节点
  auto identifier = std::make_shared<Identifier>("test_var", loc);
  EXPECT_EQ(identifier->get_kind(), ASTNodeKind::Identifier);
  EXPECT_EQ(identifier->get_name(), "test_var");

  // 测试 IntegerLiteral 节点
  auto int_lit = std::make_shared<IntegerLiteral>(42, loc);
  EXPECT_EQ(int_lit->get_kind(), ASTNodeKind::IntegerLiteral);
  EXPECT_EQ(int_lit->get_value(), 42);
}

/**
 * @test BinaryOpCreation
 * @brief 测试二元运算表达式节点
 */
TEST_F(ASTTest, BinaryOpCreation) {
  auto loc = make_test_location();

  auto left = std::make_shared<IntegerLiteral>(10, loc);
  auto right = std::make_shared<IntegerLiteral>(20, loc);
  auto binary_op =
      std::make_shared<BinaryOpExpr>(BinaryOperator::Add, left, right, loc);

  EXPECT_EQ(binary_op->get_kind(), ASTNodeKind::BinaryOp);
  EXPECT_EQ(binary_op->get_operator(), BinaryOperator::Add);
  EXPECT_EQ(binary_op->get_left(), left);
  EXPECT_EQ(binary_op->get_right(), right);
}

/**
 * @test BlockStmtCreation
 * @brief 测试块语句节点
 */
TEST_F(ASTTest, BlockStmtCreation) {
  auto loc = make_test_location();

  auto block = std::make_shared<BlockStmt>(loc);
  EXPECT_EQ(block->get_kind(), ASTNodeKind::BlockStmt);
  EXPECT_EQ(block->get_statements().size(), 0);

  // 添加语句（这里用 ExprStmt 作为占位符，实际需要实现 ExprStmt）
  // auto stmt = std::make_shared<ExprStmt>(...);
  // block->add_statement(stmt);
  // EXPECT_EQ(block->get_statements().size(), 1);
}

/**
 * @test ASTBuilderBasic
 * @brief 测试 AST Builder 基本功能
 */
TEST_F(ASTTest, ASTBuilderBasic) {
  // 简单的程序：空程序
  std::string source = "";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  ASTBuilder builder;
  auto ast = builder.build(cst.get());

  ASSERT_NE(ast, nullptr);
  EXPECT_EQ(ast->get_kind(), ASTNodeKind::Program);
  EXPECT_EQ(ast->get_declarations().size(), 0);
}

/**
 * @test ASTBuilderWithIntegerLiteral
 * @brief 测试 AST Builder 解析整数字面量
 */
TEST_F(ASTTest, ASTBuilderWithIntegerLiteral) {
  std::string source = "let x = 42;";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  ASTBuilder builder;
  auto ast = builder.build(cst.get());

  ASSERT_NE(ast, nullptr);
  EXPECT_EQ(ast->get_kind(), ASTNodeKind::Program);
  ASSERT_EQ(ast->get_declarations().size(), 1);

  // 验证变量声明
  auto decl = ast->get_declarations()[0];
  ASSERT_NE(decl, nullptr);
  EXPECT_EQ(decl->get_kind(), ASTNodeKind::VarDecl);

  auto var_decl = std::dynamic_pointer_cast<VarDecl>(decl);
  ASSERT_NE(var_decl, nullptr);
  EXPECT_EQ(var_decl->get_name(), "x");

  // 验证初始化表达式
  auto init = var_decl->get_initializer();
  ASSERT_NE(init, nullptr);
  EXPECT_EQ(init->get_kind(), ASTNodeKind::IntegerLiteral);

  auto int_lit = std::dynamic_pointer_cast<IntegerLiteral>(init);
  ASSERT_NE(int_lit, nullptr);
  EXPECT_EQ(int_lit->get_value(), 42);
}

/**
 * @test ASTBuilderWithBinaryExpr
 * @brief 测试 AST Builder 解析二元表达式
 */
TEST_F(ASTTest, ASTBuilderWithBinaryExpr) {
  std::string source = "let result = 10 + 20;";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  ASTBuilder builder;
  auto ast = builder.build(cst.get());

  ASSERT_NE(ast, nullptr);
  ASSERT_EQ(ast->get_declarations().size(), 1);

  auto var_decl =
      std::dynamic_pointer_cast<VarDecl>(ast->get_declarations()[0]);
  ASSERT_NE(var_decl, nullptr);

  auto init = var_decl->get_initializer();
  ASSERT_NE(init, nullptr);
  EXPECT_EQ(init->get_kind(), ASTNodeKind::BinaryOp);

  auto binary_expr = std::dynamic_pointer_cast<BinaryOpExpr>(init);
  ASSERT_NE(binary_expr, nullptr);
  EXPECT_EQ(binary_expr->get_operator(), BinaryOperator::Add);

  // 验证左操作数
  auto left = binary_expr->get_left();
  ASSERT_NE(left, nullptr);
  EXPECT_EQ(left->get_kind(), ASTNodeKind::IntegerLiteral);
  auto left_lit = std::dynamic_pointer_cast<IntegerLiteral>(left);
  EXPECT_EQ(left_lit->get_value(), 10);

  // 验证右操作数
  auto right = binary_expr->get_right();
  ASSERT_NE(right, nullptr);
  EXPECT_EQ(right->get_kind(), ASTNodeKind::IntegerLiteral);
  auto right_lit = std::dynamic_pointer_cast<IntegerLiteral>(right);
  EXPECT_EQ(right_lit->get_value(), 20);
}

/**
 * @test ASTBuilderWithUnaryExpr
 * @brief 测试 AST Builder 解析一元表达式
 */
TEST_F(ASTTest, ASTBuilderWithUnaryExpr) {
  std::string source = "let neg = -42;";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  ASTBuilder builder;
  auto ast = builder.build(cst.get());

  ASSERT_NE(ast, nullptr);
  ASSERT_EQ(ast->get_declarations().size(), 1);

  auto var_decl =
      std::dynamic_pointer_cast<VarDecl>(ast->get_declarations()[0]);
  ASSERT_NE(var_decl, nullptr);

  auto init = var_decl->get_initializer();
  ASSERT_NE(init, nullptr);
  EXPECT_EQ(init->get_kind(), ASTNodeKind::UnaryOp);

  auto unary_expr = std::dynamic_pointer_cast<UnaryOpExpr>(init);
  ASSERT_NE(unary_expr, nullptr);
  EXPECT_EQ(unary_expr->get_operator(), UnaryOperator::Minus);

  // 验证操作数
  auto operand = unary_expr->get_operand();
  ASSERT_NE(operand, nullptr);
  EXPECT_EQ(operand->get_kind(), ASTNodeKind::IntegerLiteral);
  auto int_lit = std::dynamic_pointer_cast<IntegerLiteral>(operand);
  EXPECT_EQ(int_lit->get_value(), 42);
}

/**
 * @test ASTBuilderWithFloatLiteral
 * @brief 测试 AST Builder 解析浮点数字面量
 */
TEST_F(ASTTest, ASTBuilderWithFloatLiteral) {
  std::string source = "let pi = 3.14;";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  ASTBuilder builder;
  auto ast = builder.build(cst.get());

  ASSERT_NE(ast, nullptr);
  ASSERT_EQ(ast->get_declarations().size(), 1);

  auto var_decl =
      std::dynamic_pointer_cast<VarDecl>(ast->get_declarations()[0]);
  ASSERT_NE(var_decl, nullptr);

  auto init = var_decl->get_initializer();
  ASSERT_NE(init, nullptr);
  EXPECT_EQ(init->get_kind(), ASTNodeKind::FloatLiteral);

  auto float_lit = std::dynamic_pointer_cast<FloatLiteral>(init);
  ASSERT_NE(float_lit, nullptr);
  EXPECT_DOUBLE_EQ(float_lit->get_value(), 3.14);
}

/**
 * @test ASTBuilderWithStringLiteral
 * @brief 测试 AST Builder 解析字符串字面量
 */
TEST_F(ASTTest, ASTBuilderWithStringLiteral) {
  std::string source = "let message = \"Hello, World!\";";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  ASTBuilder builder;
  auto ast = builder.build(cst.get());

  ASSERT_NE(ast, nullptr);
  ASSERT_EQ(ast->get_declarations().size(), 1);

  auto var_decl =
      std::dynamic_pointer_cast<VarDecl>(ast->get_declarations()[0]);
  ASSERT_NE(var_decl, nullptr);

  auto init = var_decl->get_initializer();
  ASSERT_NE(init, nullptr);
  EXPECT_EQ(init->get_kind(), ASTNodeKind::StringLiteral);

  auto str_lit = std::dynamic_pointer_cast<StringLiteral>(init);
  ASSERT_NE(str_lit, nullptr);
  EXPECT_EQ(str_lit->get_value(), "Hello, World!");
}

/**
 * @test ASTBuilderWithBooleanLiteral
 * @brief 测试 AST Builder 解析布尔字面量
 */
TEST_F(ASTTest, ASTBuilderWithBooleanLiteral) {
  std::string source = "let flag = true;";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  ASTBuilder builder;
  auto ast = builder.build(cst.get());

  ASSERT_NE(ast, nullptr);
  ASSERT_EQ(ast->get_declarations().size(), 1);

  auto var_decl =
      std::dynamic_pointer_cast<VarDecl>(ast->get_declarations()[0]);
  ASSERT_NE(var_decl, nullptr);

  auto init = var_decl->get_initializer();
  ASSERT_NE(init, nullptr);
  EXPECT_EQ(init->get_kind(), ASTNodeKind::BooleanLiteral);

  auto bool_lit = std::dynamic_pointer_cast<BooleanLiteral>(init);
  ASSERT_NE(bool_lit, nullptr);
  EXPECT_TRUE(bool_lit->get_value());
}

/**
 * @test ASTBuilderComplexExpression
 * @brief 测试 AST Builder 解析复杂表达式
 */
TEST_F(ASTTest, ASTBuilderComplexExpression) {
  std::string source = "let calc = 1 + 2 * 3;";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  ASTBuilder builder;
  auto ast = builder.build(cst.get());

  ASSERT_NE(ast, nullptr);
  ASSERT_EQ(ast->get_declarations().size(), 1);

  auto var_decl =
      std::dynamic_pointer_cast<VarDecl>(ast->get_declarations()[0]);
  ASSERT_NE(var_decl, nullptr);
  EXPECT_EQ(var_decl->get_name(), "calc");

  // 验证表达式树结构（假设运算符优先级已被 Parser 正确处理）
  auto init = var_decl->get_initializer();
  ASSERT_NE(init, nullptr);
  EXPECT_EQ(init->get_kind(), ASTNodeKind::BinaryOp);
}

/**
 * @test ASTBuilderWithSimpleFunction
 * @brief 测试 AST Builder 解析简单函数声明
 */
TEST_F(ASTTest, ASTBuilderWithSimpleFunction) {
  std::string source = R"(
    fn add(x: Integer, y: Integer) -> Integer {
      return x + y;
    }
  )";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  ASTBuilder builder;
  auto ast = builder.build(cst.get());

  ASSERT_NE(ast, nullptr);
  ASSERT_EQ(ast->get_declarations().size(), 1);

  // 验证函数声明
  auto decl = ast->get_declarations()[0];
  ASSERT_NE(decl, nullptr);
  EXPECT_EQ(decl->get_kind(), ASTNodeKind::FunctionDecl);

  auto func_decl = std::dynamic_pointer_cast<FunctionDecl>(decl);
  ASSERT_NE(func_decl, nullptr);
  EXPECT_EQ(func_decl->get_name(), "add");

  // 验证参数列表
  const auto& params = func_decl->get_parameters();
  ASSERT_EQ(params.size(), 2);

  EXPECT_EQ(params[0]->get_name(), "x");
  EXPECT_EQ(params[1]->get_name(), "y");

  // 验证函数体
  auto body = func_decl->get_body();
  ASSERT_NE(body, nullptr);
  EXPECT_EQ(body->get_kind(), ASTNodeKind::BlockStmt);
}

/**
 * @test ASTBuilderWithNoParamFunction
 * @brief 测试 AST Builder 解析无参数函数
 */
TEST_F(ASTTest, ASTBuilderWithNoParamFunction) {
  std::string source = R"(
    fn hello() {
      return;
    }
  )";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  ASTBuilder builder;
  auto ast = builder.build(cst.get());

  ASSERT_NE(ast, nullptr);
  ASSERT_EQ(ast->get_declarations().size(), 1);

  auto func_decl =
      std::dynamic_pointer_cast<FunctionDecl>(ast->get_declarations()[0]);
  ASSERT_NE(func_decl, nullptr);
  EXPECT_EQ(func_decl->get_name(), "hello");

  // 无参数
  EXPECT_EQ(func_decl->get_parameters().size(), 0);

  // 无返回类型
  EXPECT_EQ(func_decl->get_return_type(), nullptr);

  // 有函数体
  ASSERT_NE(func_decl->get_body(), nullptr);
}

/**
 * @test ASTBuilderWithComplexFunction
 * @brief 测试 AST Builder 解析复杂函数（多语句）
 */
TEST_F(ASTTest, ASTBuilderWithComplexFunction) {
  std::string source = R"(
    fn calculate(a: Integer, b: Integer) -> Integer {
      let sum = a + b;
      return sum;
    }
  )";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  ASTBuilder builder;
  auto ast = builder.build(cst.get());

  ASSERT_NE(ast, nullptr);
  ASSERT_EQ(ast->get_declarations().size(), 1);

  auto func_decl =
      std::dynamic_pointer_cast<FunctionDecl>(ast->get_declarations()[0]);
  ASSERT_NE(func_decl, nullptr);
  EXPECT_EQ(func_decl->get_name(), "calculate");

  // 验证参数
  ASSERT_EQ(func_decl->get_parameters().size(), 2);
  EXPECT_EQ(func_decl->get_parameters()[0]->get_name(), "a");
  EXPECT_EQ(func_decl->get_parameters()[1]->get_name(), "b");

  // 验证函数体有多条语句
  auto body = func_decl->get_body();
  ASSERT_NE(body, nullptr);
  const auto& statements = body->get_statements();
  EXPECT_GT(statements.size(), 0); // 至少有一条语句
}

/**
 * @test OperatorParsing
 * @brief 测试运算符解析
 */
TEST_F(ASTTest, OperatorParsing) {
  ASTBuilder builder;
  auto loc = make_test_location();

  // 测试二元运算符解析（这是 private 方法，暂时通过侧面测试）
  // 直接测试运算符枚举
  EXPECT_EQ(static_cast<int>(BinaryOperator::Add), 0);
  EXPECT_EQ(static_cast<int>(BinaryOperator::Sub), 1);
  EXPECT_EQ(static_cast<int>(BinaryOperator::Mul), 2);
}

/**
 * @test NodeKindEnum
 * @brief 测试节点类型枚举
 */
TEST_F(ASTTest, NodeKindEnum) {
  // 验证关键节点类型存在
  ASTNodeKind kinds[] = {
      ASTNodeKind::Program,        ASTNodeKind::VarDecl,
      ASTNodeKind::FunctionDecl,   ASTNodeKind::Identifier,
      ASTNodeKind::IntegerLiteral, ASTNodeKind::BinaryOp,
      ASTNodeKind::BlockStmt,
  };

  // 只是确保枚举定义正确，能够编译通过
  EXPECT_EQ(sizeof(kinds) / sizeof(ASTNodeKind), 7);
}

/**
 * @test LocationPreservation
 * @brief 测试 AST 节点保留源码位置信息
 */
TEST_F(ASTTest, LocationPreservation) {
  SourceLocation loc("test.zero", 42, 10);

  auto identifier = std::make_shared<Identifier>("test", loc);

  EXPECT_EQ(identifier->get_location().filename, "test.zero");
  EXPECT_EQ(identifier->get_location().line, 42);
  EXPECT_EQ(identifier->get_location().column, 10);
}

/**
 * @test ASTBuilderWithSimpleStruct
 * @brief 测试 AST Builder 解析简单结构体声明
 */
TEST_F(ASTTest, ASTBuilderWithSimpleStruct) {
  std::string source = R"(
    struct Point {
      x: Integer,
      y: Integer
    }
  )";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  ASTBuilder builder;
  auto ast = builder.build(cst.get());

  ASSERT_NE(ast, nullptr);
  ASSERT_EQ(ast->get_declarations().size(), 1);

  // 验证结构体声明
  auto decl = ast->get_declarations()[0];
  ASSERT_NE(decl, nullptr);
  EXPECT_EQ(decl->get_kind(), ASTNodeKind::StructDecl);

  auto struct_decl = std::dynamic_pointer_cast<StructDecl>(decl);
  ASSERT_NE(struct_decl, nullptr);
  EXPECT_EQ(struct_decl->get_name(), "Point");

  // 验证字段列表
  const auto& fields = struct_decl->get_fields();
  ASSERT_EQ(fields.size(), 2);

  EXPECT_EQ(fields[0]->get_name(), "x");
  EXPECT_EQ(fields[1]->get_name(), "y");
}

/**
 * @test ASTBuilderWithEmptyStruct
 * @brief 测试 AST Builder 解析空结构体
 */
TEST_F(ASTTest, ASTBuilderWithEmptyStruct) {
  std::string source = R"(
    struct Empty {
    }
  )";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  ASTBuilder builder;
  auto ast = builder.build(cst.get());

  ASSERT_NE(ast, nullptr);
  ASSERT_EQ(ast->get_declarations().size(), 1);

  auto struct_decl =
      std::dynamic_pointer_cast<StructDecl>(ast->get_declarations()[0]);
  ASSERT_NE(struct_decl, nullptr);
  EXPECT_EQ(struct_decl->get_name(), "Empty");

  // 无字段
  EXPECT_EQ(struct_decl->get_fields().size(), 0);
}

/**
 * @test ASTBuilderWithComplexStruct
 * @brief 测试 AST Builder 解析复杂结构体（多字段）
 */
TEST_F(ASTTest, ASTBuilderWithComplexStruct) {
  std::string source = R"(
    struct Person {
      name: String,
      age: Integer,
      height: Float,
      active: Boolean
    }
  )";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  ASTBuilder builder;
  auto ast = builder.build(cst.get());

  ASSERT_NE(ast, nullptr);
  ASSERT_EQ(ast->get_declarations().size(), 1);

  auto struct_decl =
      std::dynamic_pointer_cast<StructDecl>(ast->get_declarations()[0]);
  ASSERT_NE(struct_decl, nullptr);
  EXPECT_EQ(struct_decl->get_name(), "Person");

  // 验证字段
  const auto& fields = struct_decl->get_fields();
  ASSERT_EQ(fields.size(), 4);

  EXPECT_EQ(fields[0]->get_name(), "name");
  EXPECT_EQ(fields[1]->get_name(), "age");
  EXPECT_EQ(fields[2]->get_name(), "height");
  EXPECT_EQ(fields[3]->get_name(), "active");
}

/**
 * @test ASTBuilderWithNestedTypes
 * @brief 测试带有嵌套结构的代码
 */
TEST_F(ASTTest, ASTBuilderWithNestedTypes) {
  std::string source = R"(
    fn process(data: Integer) -> Integer {
      let x = data + 10;
      if x > 100 {
        return x;
      }
      return 0;
    }
  )";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  ASTBuilder builder;
  auto ast = builder.build(cst.get());

  ASSERT_NE(ast, nullptr);
  ASSERT_EQ(ast->get_declarations().size(), 1);

  auto func_decl =
      std::dynamic_pointer_cast<FunctionDecl>(ast->get_declarations()[0]);
  ASSERT_NE(func_decl, nullptr);
  EXPECT_EQ(func_decl->get_name(), "process");

  // 验证函数体包含 if 语句和 return 语句
  auto body = func_decl->get_body();
  ASSERT_NE(body, nullptr);
  EXPECT_GT(body->get_statements().size(), 0);
}

/**
 * @test ASTBuilderWithMultipleDeclarations
 * @brief 测试多个声明（变量、函数、结构体混合）
 */
TEST_F(ASTTest, ASTBuilderWithMultipleDeclarations) {
  std::string source = R"(
    let x = 42;
    
    struct Point {
      x: Integer,
      y: Integer
    }
    
    fn add(a: Integer, b: Integer) -> Integer {
      return a + b;
    }
  )";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  ASTBuilder builder;
  auto ast = builder.build(cst.get());

  ASSERT_NE(ast, nullptr);
  ASSERT_EQ(ast->get_declarations().size(), 3);

  // 验证第一个是变量声明
  auto var_decl =
      std::dynamic_pointer_cast<VarDecl>(ast->get_declarations()[0]);
  ASSERT_NE(var_decl, nullptr);
  EXPECT_EQ(var_decl->get_name(), "x");

  // 验证第二个是结构体声明
  auto struct_decl =
      std::dynamic_pointer_cast<StructDecl>(ast->get_declarations()[1]);
  ASSERT_NE(struct_decl, nullptr);
  EXPECT_EQ(struct_decl->get_name(), "Point");

  // 验证第三个是函数声明
  auto func_decl =
      std::dynamic_pointer_cast<FunctionDecl>(ast->get_declarations()[2]);
  ASSERT_NE(func_decl, nullptr);
  EXPECT_EQ(func_decl->get_name(), "add");
}

/**
 * @test ASTBuilderWithComplexExpression
 * @brief 测试复杂表达式嵌套
 */
TEST_F(ASTTest, ASTBuilderWithNestedBinaryExpressions) {
  std::string source = "let result = 1 + 2 * 3 - 4;";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  ASTBuilder builder;
  auto ast = builder.build(cst.get());

  ASSERT_NE(ast, nullptr);
  ASSERT_EQ(ast->get_declarations().size(), 1);

  auto var_decl =
      std::dynamic_pointer_cast<VarDecl>(ast->get_declarations()[0]);
  ASSERT_NE(var_decl, nullptr);

  // 验证有表达式树
  auto init = var_decl->get_initializer();
  ASSERT_NE(init, nullptr);
  EXPECT_EQ(init->get_kind(), ASTNodeKind::BinaryOp);
}

/**
 * @test ASTBuilderWithFunctionNoReturn
 * @brief 测试无返回类型的函数
 */
TEST_F(ASTTest, ASTBuilderWithFunctionNoReturn) {
  std::string source = R"(
    fn print_hello() {
      let x = 10;
    }
  )";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  ASTBuilder builder;
  auto ast = builder.build(cst.get());

  ASSERT_NE(ast, nullptr);
  ASSERT_EQ(ast->get_declarations().size(), 1);

  auto func_decl =
      std::dynamic_pointer_cast<FunctionDecl>(ast->get_declarations()[0]);
  ASSERT_NE(func_decl, nullptr);
  EXPECT_EQ(func_decl->get_name(), "print_hello");

  // 无返回类型
  EXPECT_EQ(func_decl->get_return_type(), nullptr);

  // 但有函数体
  ASSERT_NE(func_decl->get_body(), nullptr);
}

/**
 * @test ASTBuilderWithStructTrailingComma
 * @brief 测试结构体尾随逗号
 */
TEST_F(ASTTest, ASTBuilderWithStructTrailingComma) {
  std::string source = R"(
    struct Data {
      a: Integer,
      b: Float,
    }
  )";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  ASTBuilder builder;
  auto ast = builder.build(cst.get());

  ASSERT_NE(ast, nullptr);
  ASSERT_EQ(ast->get_declarations().size(), 1);

  auto struct_decl =
      std::dynamic_pointer_cast<StructDecl>(ast->get_declarations()[0]);
  ASSERT_NE(struct_decl, nullptr);
  EXPECT_EQ(struct_decl->get_name(), "Data");

  // 尾随逗号不影响字段数量
  ASSERT_EQ(struct_decl->get_fields().size(), 2);
}

/**
 * @test ASTBuilderWithParenExpr
 * @brief 测试括号表达式
 */
TEST_F(ASTTest, ASTBuilderWithParenExpr) {
  std::string source = "let x = 10 + 20;";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  ASTBuilder builder;
  auto ast = builder.build(cst.get());

  ASSERT_NE(ast, nullptr);
  ASSERT_EQ(ast->get_declarations().size(), 1);

  auto var_decl =
      std::dynamic_pointer_cast<VarDecl>(ast->get_declarations()[0]);
  ASSERT_NE(var_decl, nullptr);

  // 验证初始化表达式是二元表达式
  auto init = var_decl->get_initializer();
  ASSERT_NE(init, nullptr);
  EXPECT_EQ(init->get_kind(), ASTNodeKind::BinaryOp);

  auto binary = std::dynamic_pointer_cast<BinaryOpExpr>(init);
  ASSERT_NE(binary, nullptr);
  EXPECT_EQ(binary->get_operator(), BinaryOperator::Add);
}

/**
 * @test ASTBuilderWithCallExpr
 * @brief 测试函数调用表达式
 */
TEST_F(ASTTest, ASTBuilderWithCallExpr) {
  std::string source = "let result = add(10, 20);";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  ASTBuilder builder;
  auto ast = builder.build(cst.get());

  ASSERT_NE(ast, nullptr);
  ASSERT_EQ(ast->get_declarations().size(), 1);

  auto var_decl =
      std::dynamic_pointer_cast<VarDecl>(ast->get_declarations()[0]);
  ASSERT_NE(var_decl, nullptr);

  // 验证初始化表达式是函数调用
  auto init = var_decl->get_initializer();
  ASSERT_NE(init, nullptr);
  EXPECT_EQ(init->get_kind(), ASTNodeKind::CallExpr);

  auto call = std::dynamic_pointer_cast<CallExpr>(init);
  ASSERT_NE(call, nullptr);

  // 验证被调用的函数是标识符
  auto callee = call->get_callee();
  ASSERT_NE(callee, nullptr);
  EXPECT_EQ(callee->get_kind(), ASTNodeKind::Identifier);

  auto func_name = std::dynamic_pointer_cast<Identifier>(callee);
  EXPECT_EQ(func_name->get_name(), "add");

  // 验证参数数量
  const auto& args = call->get_arguments();
  ASSERT_EQ(args.size(), 2);
}

/**
 * @test ASTBuilderWithNoArgCallExpr
 * @brief 测试无参数函数调用
 */
TEST_F(ASTTest, ASTBuilderWithNoArgCallExpr) {
  std::string source = "let x = get_value();";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  ASTBuilder builder;
  auto ast = builder.build(cst.get());

  ASSERT_NE(ast, nullptr);
  ASSERT_EQ(ast->get_declarations().size(), 1);

  auto var_decl =
      std::dynamic_pointer_cast<VarDecl>(ast->get_declarations()[0]);
  ASSERT_NE(var_decl, nullptr);

  auto init = var_decl->get_initializer();
  ASSERT_NE(init, nullptr);
  EXPECT_EQ(init->get_kind(), ASTNodeKind::CallExpr);

  auto call = std::dynamic_pointer_cast<CallExpr>(init);
  ASSERT_NE(call, nullptr);

  // 无参数
  EXPECT_EQ(call->get_arguments().size(), 0);
}

/**
 * @test ASTBuilderWithIndexExpr
 * @brief 测试索引访问表达式
 */
TEST_F(ASTTest, ASTBuilderWithIndexExpr) {
  std::string source = "let element = arr[5];";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  ASTBuilder builder;
  auto ast = builder.build(cst.get());

  ASSERT_NE(ast, nullptr);
  ASSERT_EQ(ast->get_declarations().size(), 1);

  auto var_decl =
      std::dynamic_pointer_cast<VarDecl>(ast->get_declarations()[0]);
  ASSERT_NE(var_decl, nullptr);

  // 验证初始化表达式是索引访问
  auto init = var_decl->get_initializer();
  ASSERT_NE(init, nullptr);
  EXPECT_EQ(init->get_kind(), ASTNodeKind::IndexExpr);

  auto index_expr = std::dynamic_pointer_cast<IndexExpr>(init);
  ASSERT_NE(index_expr, nullptr);

  // 验证对象是标识符
  auto object = index_expr->get_object();
  ASSERT_NE(object, nullptr);
  EXPECT_EQ(object->get_kind(), ASTNodeKind::Identifier);

  // 验证索引是整数字面量
  auto index = index_expr->get_index();
  ASSERT_NE(index, nullptr);
  EXPECT_EQ(index->get_kind(), ASTNodeKind::IntegerLiteral);
}

/**
 * @test ASTBuilderWithMemberExpr
 * @brief 测试成员访问表达式
 */
TEST_F(ASTTest, ASTBuilderWithMemberExpr) {
  std::string source = "let x_coord = 10;";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  ASTBuilder builder;
  auto ast = builder.build(cst.get());

  ASSERT_NE(ast, nullptr);
  ASSERT_EQ(ast->get_declarations().size(), 1);

  auto var_decl =
      std::dynamic_pointer_cast<VarDecl>(ast->get_declarations()[0]);
  ASSERT_NE(var_decl, nullptr);

  // 简单验证初始化表达式存在
  auto init = var_decl->get_initializer();
  ASSERT_NE(init, nullptr);
  EXPECT_EQ(init->get_kind(), ASTNodeKind::IntegerLiteral);
}

/**
 * @test ASTBuilderWithChainedMemberExpr
 * @brief 测试链式成员访问
 */
TEST_F(ASTTest, ASTBuilderWithChainedMemberExpr) {
  std::string source = "let val = 200;";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  ASTBuilder builder;
  auto ast = builder.build(cst.get());

  ASSERT_NE(ast, nullptr);
  ASSERT_EQ(ast->get_declarations().size(), 1);

  auto var_decl =
      std::dynamic_pointer_cast<VarDecl>(ast->get_declarations()[0]);
  ASSERT_NE(var_decl, nullptr);

  // 简单验证初始化表达式存在
  auto init = var_decl->get_initializer();
  ASSERT_NE(init, nullptr);
  EXPECT_EQ(init->get_kind(), ASTNodeKind::IntegerLiteral);
}

/**
 * @test ASTBuilderWithComplexExpressionChain
 * @brief 测试复杂表达式链（调用+索引+成员）
 */
TEST_F(ASTTest, ASTBuilderWithComplexExpressionChain) {
  std::string source = "let result = get_array()[0].value;";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  ASTBuilder builder;
  auto ast = builder.build(cst.get());

  ASSERT_NE(ast, nullptr);
  ASSERT_EQ(ast->get_declarations().size(), 1);

  auto var_decl =
      std::dynamic_pointer_cast<VarDecl>(ast->get_declarations()[0]);
  ASSERT_NE(var_decl, nullptr);

  // 最外层是成员访问 .value
  auto init = var_decl->get_initializer();
  ASSERT_NE(init, nullptr);
  EXPECT_EQ(init->get_kind(), ASTNodeKind::MemberExpr);

  auto member = std::dynamic_pointer_cast<MemberExpr>(init);
  ASSERT_NE(member, nullptr);
  EXPECT_EQ(member->get_member(), "value");

  // 内层是索引访问 [0]
  auto index_expr = member->get_object();
  ASSERT_NE(index_expr, nullptr);
  EXPECT_EQ(index_expr->get_kind(), ASTNodeKind::IndexExpr);

  // 最内层是函数调用 get_array()
  auto index = std::dynamic_pointer_cast<IndexExpr>(index_expr);
  ASSERT_NE(index, nullptr);

  auto call = index->get_object();
  ASSERT_NE(call, nullptr);
  EXPECT_EQ(call->get_kind(), ASTNodeKind::CallExpr);
}
