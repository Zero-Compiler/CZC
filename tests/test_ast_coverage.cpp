/**
 * @file test_ast_coverage.cpp
 * @brief Additional AST tests to improve function coverage
 * @details Tests for AST node types that were previously not covered
 * @author BegoniaHe
 * @date 2025-11-13
 */

#include "czc/ast/ast_node.hpp"
#include "czc/utils/source_location.hpp"

#include <gtest/gtest.h>

using namespace czc;
using namespace czc::ast;
using namespace czc::utils;

class ASTCoverageTest : public ::testing::Test {
protected:
  SourceLocation make_test_location() {
    return SourceLocation("test.zero", 1, 1);
  }
};

/**
 * @test FloatLiteralNode
 * @brief Test FloatLiteral node creation and methods
 */
TEST_F(ASTCoverageTest, FloatLiteralNode) {
  auto loc = make_test_location();
  auto float_lit = std::make_shared<FloatLiteral>(3.14159, loc);

  EXPECT_EQ(float_lit->get_kind(), ASTNodeKind::FloatLiteral);
  EXPECT_DOUBLE_EQ(float_lit->get_value(), 3.14159);
  EXPECT_EQ(float_lit->get_location().filename, "test.zero");
}

/**
 * @test StringLiteralNode
 * @brief Test StringLiteral node creation and methods
 */
TEST_F(ASTCoverageTest, StringLiteralNode) {
  auto loc = make_test_location();
  auto str_lit = std::make_shared<StringLiteral>("Hello, World!", loc);

  EXPECT_EQ(str_lit->get_kind(), ASTNodeKind::StringLiteral);
  EXPECT_EQ(str_lit->get_value(), "Hello, World!");
  EXPECT_EQ(str_lit->get_location().filename, "test.zero");
}

/**
 * @test BooleanLiteralNode
 * @brief Test BooleanLiteral node creation and methods
 */
TEST_F(ASTCoverageTest, BooleanLiteralNode) {
  auto loc = make_test_location();

  auto bool_true = std::make_shared<BooleanLiteral>(true, loc);
  EXPECT_EQ(bool_true->get_kind(), ASTNodeKind::BooleanLiteral);
  EXPECT_TRUE(bool_true->get_value());

  auto bool_false = std::make_shared<BooleanLiteral>(false, loc);
  EXPECT_EQ(bool_false->get_kind(), ASTNodeKind::BooleanLiteral);
  EXPECT_FALSE(bool_false->get_value());
}

/**
 * @test UnaryOpExprNode
 * @brief Test UnaryOpExpr node creation and methods
 */
TEST_F(ASTCoverageTest, UnaryOpExprNode) {
  auto loc = make_test_location();
  auto operand = std::make_shared<IntegerLiteral>(42, loc);

  // Test Minus operator
  auto unary_minus =
      std::make_shared<UnaryOpExpr>(UnaryOperator::Minus, operand, loc);
  EXPECT_EQ(unary_minus->get_kind(), ASTNodeKind::UnaryOp);
  EXPECT_EQ(unary_minus->get_operator(), UnaryOperator::Minus);
  EXPECT_EQ(unary_minus->get_operand(), operand);

  // Test Plus operator
  auto unary_plus =
      std::make_shared<UnaryOpExpr>(UnaryOperator::Plus, operand, loc);
  EXPECT_EQ(unary_plus->get_operator(), UnaryOperator::Plus);

  // Test Not operator
  auto bool_operand = std::make_shared<BooleanLiteral>(true, loc);
  auto unary_not =
      std::make_shared<UnaryOpExpr>(UnaryOperator::Not, bool_operand, loc);
  EXPECT_EQ(unary_not->get_operator(), UnaryOperator::Not);
  EXPECT_EQ(unary_not->get_operand(), bool_operand);
}

/**
 * @test ParenExprNode
 * @brief Test ParenExpr node creation and methods
 */
TEST_F(ASTCoverageTest, ParenExprNode) {
  auto loc = make_test_location();
  auto inner_expr = std::make_shared<IntegerLiteral>(100, loc);
  auto paren_expr = std::make_shared<ParenExpr>(inner_expr, loc);

  EXPECT_EQ(paren_expr->get_kind(), ASTNodeKind::ParenExpr);
  EXPECT_EQ(paren_expr->get_expression(), inner_expr);
}

/**
 * @test CallExprNode
 * @brief Test CallExpr node creation and methods
 */
TEST_F(ASTCoverageTest, CallExprNode) {
  auto loc = make_test_location();
  auto callee = std::make_shared<Identifier>("my_function", loc);

  std::vector<std::shared_ptr<Expression>> args;
  args.push_back(std::make_shared<IntegerLiteral>(10, loc));
  args.push_back(std::make_shared<StringLiteral>("test", loc));

  auto call_expr = std::make_shared<CallExpr>(callee, args, loc);

  EXPECT_EQ(call_expr->get_kind(), ASTNodeKind::CallExpr);
  EXPECT_EQ(call_expr->get_callee(), callee);
  EXPECT_EQ(call_expr->get_arguments().size(), 2);
  EXPECT_EQ(call_expr->get_arguments()[0], args[0]);
  EXPECT_EQ(call_expr->get_arguments()[1], args[1]);
}

/**
 * @test IndexExprNode
 * @brief Test IndexExpr node creation and methods
 */
TEST_F(ASTCoverageTest, IndexExprNode) {
  auto loc = make_test_location();
  auto object = std::make_shared<Identifier>("my_array", loc);
  auto index = std::make_shared<IntegerLiteral>(5, loc);

  auto index_expr = std::make_shared<IndexExpr>(object, index, loc);

  EXPECT_EQ(index_expr->get_kind(), ASTNodeKind::IndexExpr);
  EXPECT_EQ(index_expr->get_object(), object);
  EXPECT_EQ(index_expr->get_index(), index);
}

/**
 * @test MemberExprNode
 * @brief Test MemberExpr node creation and methods
 */
TEST_F(ASTCoverageTest, MemberExprNode) {
  auto loc = make_test_location();
  auto object = std::make_shared<Identifier>("my_struct", loc);

  auto member_expr = std::make_shared<MemberExpr>(object, "field_name", loc);

  EXPECT_EQ(member_expr->get_kind(), ASTNodeKind::MemberExpr);
  EXPECT_EQ(member_expr->get_object(), object);
  EXPECT_EQ(member_expr->get_member(), "field_name");
}

/**
 * @test ExprStmtNode
 * @brief Test ExprStmt node creation and methods
 */
TEST_F(ASTCoverageTest, ExprStmtNode) {
  auto loc = make_test_location();
  auto expr = std::make_shared<IntegerLiteral>(42, loc);

  auto expr_stmt = std::make_shared<ExprStmt>(expr, loc);

  EXPECT_EQ(expr_stmt->get_kind(), ASTNodeKind::ExprStmt);
  EXPECT_EQ(expr_stmt->get_expression(), expr);
}

/**
 * @test ReturnStmtNode
 * @brief Test ReturnStmt node creation and methods
 */
TEST_F(ASTCoverageTest, ReturnStmtNode) {
  auto loc = make_test_location();
  auto value = std::make_shared<IntegerLiteral>(123, loc);

  auto return_stmt = std::make_shared<ReturnStmt>(value, loc);

  EXPECT_EQ(return_stmt->get_kind(), ASTNodeKind::ReturnStmt);
  EXPECT_EQ(return_stmt->get_value(), value);
}

/**
 * @test IfStmtNode
 * @brief Test IfStmt node creation and methods
 */
TEST_F(ASTCoverageTest, IfStmtNode) {
  auto loc = make_test_location();
  auto condition = std::make_shared<BooleanLiteral>(true, loc);
  auto then_branch = std::make_shared<BlockStmt>(loc);
  auto else_branch = std::make_shared<BlockStmt>(loc);

  auto if_stmt =
      std::make_shared<IfStmt>(condition, then_branch, else_branch, loc);

  EXPECT_EQ(if_stmt->get_kind(), ASTNodeKind::IfStmt);
  EXPECT_EQ(if_stmt->get_condition(), condition);
  EXPECT_EQ(if_stmt->get_then_branch(), then_branch);
  EXPECT_EQ(if_stmt->get_else_branch(), else_branch);
}

/**
 * @test ParameterNode
 * @brief Test Parameter node creation and methods
 */
TEST_F(ASTCoverageTest, ParameterNode) {
  auto loc = make_test_location();

  // For simplicity, using nullptr for type (optional)
  auto param = std::make_shared<Parameter>("param_name", nullptr, loc);

  EXPECT_EQ(param->get_name(), "param_name");
  EXPECT_EQ(param->get_type(), nullptr);
}

/**
 * @test FunctionDeclNode
 * @brief Test FunctionDecl node creation and methods
 */
TEST_F(ASTCoverageTest, FunctionDeclNode) {
  auto loc = make_test_location();

  std::vector<std::shared_ptr<Parameter>> params;
  params.push_back(std::make_shared<Parameter>("x", nullptr, loc));
  params.push_back(std::make_shared<Parameter>("y", nullptr, loc));

  auto body = std::make_shared<BlockStmt>(loc);
  body->add_statement(std::make_shared<ReturnStmt>(
      std::make_shared<IntegerLiteral>(0, loc), loc));

  auto func_decl =
      std::make_shared<FunctionDecl>("my_function", params, nullptr, body, loc);

  EXPECT_EQ(func_decl->get_kind(), ASTNodeKind::FunctionDecl);
  EXPECT_EQ(func_decl->get_name(), "my_function");
  EXPECT_EQ(func_decl->get_parameters().size(), 2);
  EXPECT_EQ(func_decl->get_parameters()[0]->get_name(), "x");
  EXPECT_EQ(func_decl->get_parameters()[1]->get_name(), "y");
  EXPECT_EQ(func_decl->get_return_type(), nullptr);
  EXPECT_EQ(func_decl->get_body(), body);
}

/**
 * @test StructFieldNode
 * @brief Test StructField node creation and methods
 */
TEST_F(ASTCoverageTest, StructFieldNode) {
  auto loc = make_test_location();

  auto field = std::make_shared<StructField>("field_name", nullptr, loc);

  EXPECT_EQ(field->get_kind(), ASTNodeKind::StructField);
  EXPECT_EQ(field->get_name(), "field_name");
  EXPECT_EQ(field->get_type(), nullptr);
}

/**
 * @test StructDeclNode
 * @brief Test StructDecl node creation and methods
 */
TEST_F(ASTCoverageTest, StructDeclNode) {
  auto loc = make_test_location();

  std::vector<std::shared_ptr<StructField>> fields;
  fields.push_back(std::make_shared<StructField>("x", nullptr, loc));
  fields.push_back(std::make_shared<StructField>("y", nullptr, loc));
  fields.push_back(std::make_shared<StructField>("name", nullptr, loc));

  auto struct_decl = std::make_shared<StructDecl>("Point", fields, loc);

  EXPECT_EQ(struct_decl->get_kind(), ASTNodeKind::StructDecl);
  EXPECT_EQ(struct_decl->get_name(), "Point");
  EXPECT_EQ(struct_decl->get_fields().size(), 3);
  EXPECT_EQ(struct_decl->get_fields()[0]->get_name(), "x");
  EXPECT_EQ(struct_decl->get_fields()[1]->get_name(), "y");
  EXPECT_EQ(struct_decl->get_fields()[2]->get_name(), "name");
}

/**
 * @test VarDeclNode
 * @brief Test VarDecl node creation and methods
 */
TEST_F(ASTCoverageTest, VarDeclNode) {
  auto loc = make_test_location();
  auto initializer = std::make_shared<IntegerLiteral>(999, loc);

  auto var_decl =
      std::make_shared<VarDecl>("my_var", nullptr, initializer, loc);

  EXPECT_EQ(var_decl->get_kind(), ASTNodeKind::VarDecl);
  EXPECT_EQ(var_decl->get_name(), "my_var");
  EXPECT_EQ(var_decl->get_type_annotation(), nullptr);
  EXPECT_EQ(var_decl->get_initializer(), initializer);
}

/**
 * @test ProgramNode
 * @brief Test Program node with multiple declarations
 */
TEST_F(ASTCoverageTest, ProgramNode) {
  auto loc = make_test_location();
  auto program = std::make_shared<Program>(loc);

  EXPECT_EQ(program->get_kind(), ASTNodeKind::Program);
  EXPECT_EQ(program->get_declarations().size(), 0);

  // Add variable declaration
  auto var_decl = std::make_shared<VarDecl>(
      "x", nullptr, std::make_shared<IntegerLiteral>(10, loc), loc);
  program->add_declaration(var_decl);

  // Add function declaration
  std::vector<std::shared_ptr<Parameter>> params;
  auto body = std::make_shared<BlockStmt>(loc);
  auto func_decl =
      std::make_shared<FunctionDecl>("test_fn", params, nullptr, body, loc);
  program->add_declaration(func_decl);

  // Add struct declaration
  std::vector<std::shared_ptr<StructField>> fields;
  auto struct_decl = std::make_shared<StructDecl>("TestStruct", fields, loc);
  program->add_declaration(struct_decl);

  EXPECT_EQ(program->get_declarations().size(), 3);
}

/**
 * @test BlockStmtNode
 * @brief Test BlockStmt node with multiple statements
 */
TEST_F(ASTCoverageTest, BlockStmtNode) {
  auto loc = make_test_location();
  auto block = std::make_shared<BlockStmt>(loc);

  EXPECT_EQ(block->get_kind(), ASTNodeKind::BlockStmt);
  EXPECT_EQ(block->get_statements().size(), 0);

  // Add expression statement
  auto expr_stmt = std::make_shared<ExprStmt>(
      std::make_shared<IntegerLiteral>(42, loc), loc);
  block->add_statement(expr_stmt);

  // Add return statement
  auto return_stmt = std::make_shared<ReturnStmt>(
      std::make_shared<BooleanLiteral>(true, loc), loc);
  block->add_statement(return_stmt);

  EXPECT_EQ(block->get_statements().size(), 2);
}

/**
 * @test TypeNodeInheritance
 * @brief Test Type base class functionality
 */
TEST_F(ASTCoverageTest, TypeNodeInheritance) {
  auto loc = make_test_location();

  // Type is abstract, but we can test through Expression which sets/gets type
  auto expr = std::make_shared<IntegerLiteral>(42, loc);

  EXPECT_EQ(expr->get_type(), nullptr);

  // Note: We can't instantiate Type directly as it would need a concrete
  // subclass This test ensures the type getter/setter mechanism works
}

/**
 * @test AllBinaryOperators
 * @brief Test all binary operator types
 */
TEST_F(ASTCoverageTest, AllBinaryOperators) {
  auto loc = make_test_location();
  auto left = std::make_shared<IntegerLiteral>(10, loc);
  auto right = std::make_shared<IntegerLiteral>(20, loc);

  // Arithmetic operators
  auto add_op =
      std::make_shared<BinaryOpExpr>(BinaryOperator::Add, left, right, loc);
  EXPECT_EQ(add_op->get_operator(), BinaryOperator::Add);

  auto sub_op =
      std::make_shared<BinaryOpExpr>(BinaryOperator::Sub, left, right, loc);
  EXPECT_EQ(sub_op->get_operator(), BinaryOperator::Sub);

  auto mul_op =
      std::make_shared<BinaryOpExpr>(BinaryOperator::Mul, left, right, loc);
  EXPECT_EQ(mul_op->get_operator(), BinaryOperator::Mul);

  auto div_op =
      std::make_shared<BinaryOpExpr>(BinaryOperator::Div, left, right, loc);
  EXPECT_EQ(div_op->get_operator(), BinaryOperator::Div);

  auto mod_op =
      std::make_shared<BinaryOpExpr>(BinaryOperator::Mod, left, right, loc);
  EXPECT_EQ(mod_op->get_operator(), BinaryOperator::Mod);

  // Comparison operators
  auto eq_op =
      std::make_shared<BinaryOpExpr>(BinaryOperator::Eq, left, right, loc);
  EXPECT_EQ(eq_op->get_operator(), BinaryOperator::Eq);

  auto ne_op =
      std::make_shared<BinaryOpExpr>(BinaryOperator::Ne, left, right, loc);
  EXPECT_EQ(ne_op->get_operator(), BinaryOperator::Ne);

  auto lt_op =
      std::make_shared<BinaryOpExpr>(BinaryOperator::Lt, left, right, loc);
  EXPECT_EQ(lt_op->get_operator(), BinaryOperator::Lt);

  auto le_op =
      std::make_shared<BinaryOpExpr>(BinaryOperator::Le, left, right, loc);
  EXPECT_EQ(le_op->get_operator(), BinaryOperator::Le);

  auto gt_op =
      std::make_shared<BinaryOpExpr>(BinaryOperator::Gt, left, right, loc);
  EXPECT_EQ(gt_op->get_operator(), BinaryOperator::Gt);

  auto ge_op =
      std::make_shared<BinaryOpExpr>(BinaryOperator::Ge, left, right, loc);
  EXPECT_EQ(ge_op->get_operator(), BinaryOperator::Ge);

  // Logical operators
  auto bool_left = std::make_shared<BooleanLiteral>(true, loc);
  auto bool_right = std::make_shared<BooleanLiteral>(false, loc);

  auto and_op = std::make_shared<BinaryOpExpr>(BinaryOperator::And, bool_left,
                                               bool_right, loc);
  EXPECT_EQ(and_op->get_operator(), BinaryOperator::And);

  auto or_op = std::make_shared<BinaryOpExpr>(BinaryOperator::Or, bool_left,
                                              bool_right, loc);
  EXPECT_EQ(or_op->get_operator(), BinaryOperator::Or);
}

/**
 * @test BaseClassPolymorphism
 * @brief Test AST node base class polymorphism
 */
TEST_F(ASTCoverageTest, BaseClassPolymorphism) {
  auto loc = make_test_location();

  // Test Expression inheritance
  std::shared_ptr<Expression> expr1 = std::make_shared<IntegerLiteral>(10, loc);
  std::shared_ptr<Expression> expr2 = std::make_shared<FloatLiteral>(3.14, loc);
  std::shared_ptr<Expression> expr3 = std::make_shared<Identifier>("var", loc);

  EXPECT_EQ(expr1->get_kind(), ASTNodeKind::IntegerLiteral);
  EXPECT_EQ(expr2->get_kind(), ASTNodeKind::FloatLiteral);
  EXPECT_EQ(expr3->get_kind(), ASTNodeKind::Identifier);

  // Test Statement inheritance
  std::shared_ptr<Statement> stmt1 = std::make_shared<ExprStmt>(expr1, loc);
  std::shared_ptr<Statement> stmt2 = std::make_shared<ReturnStmt>(expr2, loc);
  std::shared_ptr<Statement> stmt3 = std::make_shared<BlockStmt>(loc);

  EXPECT_EQ(stmt1->get_kind(), ASTNodeKind::ExprStmt);
  EXPECT_EQ(stmt2->get_kind(), ASTNodeKind::ReturnStmt);
  EXPECT_EQ(stmt3->get_kind(), ASTNodeKind::BlockStmt);

  // Test Declaration inheritance
  std::shared_ptr<Declaration> decl1 =
      std::make_shared<VarDecl>("x", nullptr, nullptr, loc);
  std::vector<std::shared_ptr<StructField>> fields;
  std::shared_ptr<Declaration> decl2 =
      std::make_shared<StructDecl>("S", fields, loc);

  EXPECT_EQ(decl1->get_kind(), ASTNodeKind::VarDecl);
  EXPECT_EQ(decl2->get_kind(), ASTNodeKind::StructDecl);
}
