/**
 * @file test_cst.cpp
 * @brief CST（具体语法树）节点测试
 * @details 测试CST节点的创建、子节点管理、Token关联等核心功能
 * @author BegoniaHe
 * @date 2025-01-18
 *
 * @test_module CST测试模块
 * @test_coverage czc::cst::CSTNode类的所有公共接口
 */

#include "czc/cst/cst_node.hpp"
#include "czc/parser/parser.hpp"
#include "czc/lexer/lexer.hpp"
#include "czc/lexer/token.hpp"
#include <gtest/gtest.h>
#include <unordered_set>

using namespace czc;
using namespace czc::cst;
using namespace czc::utils;
using namespace czc::lexer;

/**
 * @brief CST节点测试基类
 * @details 提供CST节点测试所需的公共功能和辅助方法
 */
class CSTNodeTest : public ::testing::Test {
protected:
  /**
   * @brief 创建测试用的源码位置对象。
   * @details 创建一个默认的源码位置对象，用于测试CST节点的位置信息。
   * @return 返回一个SourceLocation对象，表示test.zero文件的第1行第1列
   */
  SourceLocation make_test_location() {
    return SourceLocation("test.zero", 1, 1);
  }
};

/**
 * @test BasicNodeCreation
 * @brief 测试基本CST节点的创建
 * @details
 *   验证目标：
 *   1. 可以成功创建Program、ExprStmt等不同类型的CST节点
 *   2. 创建后节点类型正确
 *   3. 节点的源码位置信息被正确保存
 */
TEST_F(CSTNodeTest, BasicNodeCreation) {
  auto loc = make_test_location();
  
  // 测试创建Program节点
  auto program = std::make_unique<CSTNode>(CSTNodeType::Program, loc);
  EXPECT_EQ(program->get_type(), CSTNodeType::Program);
  
  // 测试创建ExprStmt节点
  auto stmt = std::make_unique<CSTNode>(CSTNodeType::ExprStmt, loc);
  ASSERT_NE(stmt, nullptr);
  EXPECT_EQ(stmt->get_type(), CSTNodeType::ExprStmt);
  
  // 测试创建BinaryExpr节点
  auto expr = std::make_unique<CSTNode>(CSTNodeType::BinaryExpr, loc);
  ASSERT_NE(expr, nullptr);
  EXPECT_EQ(expr->get_type(), CSTNodeType::BinaryExpr);
}

/**
 * @test AddChildren
 * @brief 测试向CST节点添加子节点
 * @details
 *   验证目标：
 *   1. 可以向父节点添加多个子节点
 *   2. 子节点数量正确
 *   3. 子节点类型和顺序正确
 */
TEST_F(CSTNodeTest, AddChildren) {
  auto loc = make_test_location();
  auto parent = std::make_unique<CSTNode>(CSTNodeType::Program, loc);
  
  // 添加多个子节点
  parent->add_child(std::make_unique<CSTNode>(CSTNodeType::ExprStmt, loc));
  parent->add_child(std::make_unique<CSTNode>(CSTNodeType::ExprStmt, loc));
  parent->add_child(std::make_unique<CSTNode>(CSTNodeType::BinaryExpr, loc));
  
  const auto& children = parent->get_children();
  EXPECT_EQ(children.size(), 3);
  EXPECT_EQ(children[0]->get_type(), CSTNodeType::ExprStmt);
  EXPECT_EQ(children[1]->get_type(), CSTNodeType::ExprStmt);
  EXPECT_EQ(children[2]->get_type(), CSTNodeType::BinaryExpr);
}

/**
 * @test AssociateToken
 * @brief 测试将Token关联到CST节点
 * @details
 *   验证目标：
 *   1. 可以将Token对象关联到CST节点
 *   2. 可以正确获取关联的Token
 *   3. Token的类型和值正确保存
 */
TEST_F(CSTNodeTest, AssociateToken) {
  auto loc = make_test_location();
  auto node = std::make_unique<CSTNode>(CSTNodeType::IntegerLiteral, loc);
  
  // 创建Token并关联到节点
  Token tok(TokenType::Integer, "42", 1, 1);
  node->set_token(tok);
  
  // 验证Token正确关联
  const auto& associated_token = node->get_token();
  ASSERT_TRUE(associated_token.has_value());
  EXPECT_EQ(associated_token->token_type, TokenType::Integer);
  EXPECT_EQ(associated_token->value, "42");
}

/**
 * @test EmptyChildrenList
 * @brief 测试CST节点的空子节点列表
 * @details
 *   验证目标：
 *   1. 新创建的节点子节点列表为空
 *   2. get_children()返回的引用有效
 */
TEST_F(CSTNodeTest, EmptyChildrenList) {
  auto loc = make_test_location();
  auto node = std::make_unique<CSTNode>(CSTNodeType::Program, loc);
  
  const auto& children = node->get_children();
  EXPECT_TRUE(children.empty());
  EXPECT_EQ(children.size(), 0);
}

/**
 * @test LocationInfo
 * @brief 测试CST节点的位置信息
 * @details
 *   验证目标：
 *   1. 节点能正确保存源码位置信息
 *   2. get_location()返回正确的位置
 */
TEST_F(CSTNodeTest, LocationInfo) {
  SourceLocation loc("test.zero", 5, 10);
  
  auto node = std::make_unique<CSTNode>(CSTNodeType::BinaryExpr, loc);
  const auto& node_loc = node->get_location();
  
  EXPECT_EQ(node_loc.filename, "test.zero");
  EXPECT_EQ(node_loc.line, 5);
  EXPECT_EQ(node_loc.column, 10);
}

/**
 * @test AllNodeTypes
 * @brief 测试所有CST节点类型的创建
 * @details
 *   验证目标：
 *   1. 可以创建所有定义的CST节点类型
 *   2. 每种类型的节点都能正确保存其类型信息
 */
TEST_F(CSTNodeTest, AllNodeTypes) {
  auto loc = make_test_location();
  
  std::vector<CSTNodeType> all_types = {
    CSTNodeType::Program,
    CSTNodeType::VarDeclaration,
    CSTNodeType::FnDeclaration,
    CSTNodeType::ReturnStmt,
    CSTNodeType::IfStmt,
    CSTNodeType::WhileStmt,
    CSTNodeType::BlockStmt,
    CSTNodeType::ExprStmt,
    CSTNodeType::BinaryExpr,
    CSTNodeType::UnaryExpr,
    CSTNodeType::CallExpr,
    CSTNodeType::IndexExpr,
    CSTNodeType::MemberExpr,
    CSTNodeType::AssignExpr,
    CSTNodeType::IndexAssignExpr,
    CSTNodeType::ArrayLiteral,
    CSTNodeType::IntegerLiteral,
    CSTNodeType::FloatLiteral,
    CSTNodeType::StringLiteral,
    CSTNodeType::BooleanLiteral,
    CSTNodeType::Identifier,
    CSTNodeType::ParenExpr,
    CSTNodeType::TypeAnnotation,
    CSTNodeType::ArrayType,
    CSTNodeType::Parameter,
    CSTNodeType::ParameterList,
    CSTNodeType::ArgumentList,
    CSTNodeType::StatementList,
    CSTNodeType::Operator,
    CSTNodeType::Delimiter,
    CSTNodeType::Comment
  };
  
  for (const auto& type : all_types) {
    auto node = std::make_unique<CSTNode>(type, loc);
    EXPECT_EQ(node->get_type(), type);
  }
}

/**
 * @test NodeTypeToString
 * @brief 测试CST节点类型转字符串功能
 * @details
 *   验证目标：
 *   1. cst_node_type_to_string()能正确转换节点类型
 *   2. 返回的字符串非空且有意义
 */
TEST_F(CSTNodeTest, NodeTypeToString) {
  std::string program_str = cst_node_type_to_string(CSTNodeType::Program);
  EXPECT_FALSE(program_str.empty());
  
  std::string stmt_str = cst_node_type_to_string(CSTNodeType::ExprStmt);
  EXPECT_FALSE(stmt_str.empty());
  
  std::string expr_str = cst_node_type_to_string(CSTNodeType::BinaryExpr);
  EXPECT_FALSE(expr_str.empty());
  
  // 不同类型应该有不同的字符串
  EXPECT_NE(program_str, stmt_str);
  EXPECT_NE(stmt_str, expr_str);
}

/**
 * @test NestedStructure
 * @brief 测试CST节点的嵌套结构
 * @details
 *   验证目标：
 *   1. 可以构建多层嵌套的CST树结构
 *   2. 父子关系正确建立
 *   3. 可以通过引用访问子节点
 */
TEST_F(CSTNodeTest, NestedStructure) {
  auto loc = make_test_location();
  
  // 创建三层嵌套结构: Program -> BlockStmt -> ExprStmt -> BinaryExpr
  auto program = std::make_unique<CSTNode>(CSTNodeType::Program, loc);
  auto block = std::make_unique<CSTNode>(CSTNodeType::BlockStmt, loc);
  auto stmt = std::make_unique<CSTNode>(CSTNodeType::ExprStmt, loc);
  auto expr = std::make_unique<CSTNode>(CSTNodeType::BinaryExpr, loc);
  
  // 构建嵌套关系
  stmt->add_child(std::move(expr));
  block->add_child(std::move(stmt));
  program->add_child(std::move(block));
  
  // 验证结构
  EXPECT_EQ(program->get_children().size(), 1);
  const auto* block_ref = program->get_children()[0].get();
  EXPECT_EQ(block_ref->get_type(), CSTNodeType::BlockStmt);
  EXPECT_EQ(block_ref->get_children().size(), 1);
  
  const auto* stmt_ref = block_ref->get_children()[0].get();
  EXPECT_EQ(stmt_ref->get_type(), CSTNodeType::ExprStmt);
  EXPECT_EQ(stmt_ref->get_children().size(), 1);
  
  const auto* expr_ref = stmt_ref->get_children()[0].get();
  EXPECT_EQ(expr_ref->get_type(), CSTNodeType::BinaryExpr);
}

/**
 * @test RealParserCST
 * @brief 测试Parser生成的真实CST
 * @details
 *   验证目标：
 *   1. Parser能成功解析简单代码并生成CST
 *   2. 生成的CST结构正确
 *   3. CST根节点类型为Program
 */
TEST_F(CSTNodeTest, RealParserCST) {
  const char* source = "let x: int = 42;";
  lexer::Lexer lexer(source);
  auto tokens = lexer.tokenize();
  
  parser::Parser parser(tokens);
  auto cst = parser.parse();
  
  ASSERT_NE(cst, nullptr);
  EXPECT_EQ(cst->get_type(), CSTNodeType::Program);
  EXPECT_FALSE(cst->get_children().empty());
}

/**
 * @test BinaryExpressionCST
 * @brief 测试二元表达式的CST结构
 * @details
 *   验证目标：
 *   1. 可以为二元表达式创建完整的CST结构
 *   2. 左右操作数和运算符正确关联
 */
TEST_F(CSTNodeTest, BinaryExpressionCST) {
  auto loc = make_test_location();
  
  // 创建二元表达式: 1 + 2
  auto binary = std::make_unique<CSTNode>(CSTNodeType::BinaryExpr, loc);
  auto left = std::make_unique<CSTNode>(CSTNodeType::IntegerLiteral, loc);
  auto right = std::make_unique<CSTNode>(CSTNodeType::IntegerLiteral, loc);
  
  Token left_tok(TokenType::Integer, "1", 1, 1);
  Token right_tok(TokenType::Integer, "2", 1, 5);
  
  left->set_token(left_tok);
  right->set_token(right_tok);
  
  binary->add_child(std::move(left));
  binary->add_child(std::move(right));
  
  EXPECT_EQ(binary->get_children().size(), 2);
  EXPECT_EQ(binary->get_children()[0]->get_token()->value, "1");
  EXPECT_EQ(binary->get_children()[1]->get_token()->value, "2");
}

/**
 * @test FunctionDeclCST
 * @brief 测试函数声明的CST结构
 * @details
 *   验证目标：
 *   1. 可以为函数声明创建CST结构
 *   2. 函数声明节点可以包含参数列表和函数体
 */
TEST_F(CSTNodeTest, FunctionDeclCST) {
  auto loc = make_test_location();
  
  auto fn_decl = std::make_unique<CSTNode>(CSTNodeType::FnDeclaration, loc);
  auto param_list = std::make_unique<CSTNode>(CSTNodeType::ParameterList, loc);
  auto body = std::make_unique<CSTNode>(CSTNodeType::BlockStmt, loc);
  
  fn_decl->add_child(std::move(param_list));
  fn_decl->add_child(std::move(body));
  
  EXPECT_EQ(fn_decl->get_type(), CSTNodeType::FnDeclaration);
  EXPECT_EQ(fn_decl->get_children().size(), 2);
  EXPECT_EQ(fn_decl->get_children()[0]->get_type(), CSTNodeType::ParameterList);
  EXPECT_EQ(fn_decl->get_children()[1]->get_type(), CSTNodeType::BlockStmt);
}

/**
 * @test MultipleStatementsCST
 * @brief 测试多个语句的CST结构
 * @details
 *   验证目标：
 *   1. Program节点可以包含多个语句
 *   2. 语句顺序正确保存
 */
TEST_F(CSTNodeTest, MultipleStatementsCST) {
  auto loc = make_test_location();
  
  auto program = std::make_unique<CSTNode>(CSTNodeType::Program, loc);
  program->add_child(std::make_unique<CSTNode>(CSTNodeType::VarDeclaration, loc));
  program->add_child(std::make_unique<CSTNode>(CSTNodeType::ExprStmt, loc));
  program->add_child(std::make_unique<CSTNode>(CSTNodeType::ReturnStmt, loc));
  
  EXPECT_EQ(program->get_children().size(), 3);
  EXPECT_EQ(program->get_children()[0]->get_type(), CSTNodeType::VarDeclaration);
  EXPECT_EQ(program->get_children()[1]->get_type(), CSTNodeType::ExprStmt);
  EXPECT_EQ(program->get_children()[2]->get_type(), CSTNodeType::ReturnStmt);
}

/**
 * @test ControlFlowCST
 * @brief 测试控制流语句的CST结构
 * @details
 *   验证目标：
 *   1. 可以创建if和while等控制流语句的CST
 *   2. 控制流语句可以包含条件和代码块
 */
TEST_F(CSTNodeTest, ControlFlowCST) {
  auto loc = make_test_location();
  
  // 创建if语句
  auto if_stmt = std::make_unique<CSTNode>(CSTNodeType::IfStmt, loc);
  auto condition = std::make_unique<CSTNode>(CSTNodeType::BinaryExpr, loc);
  auto then_block = std::make_unique<CSTNode>(CSTNodeType::BlockStmt, loc);
  
  if_stmt->add_child(std::move(condition));
  if_stmt->add_child(std::move(then_block));
  
  EXPECT_EQ(if_stmt->get_type(), CSTNodeType::IfStmt);
  EXPECT_EQ(if_stmt->get_children().size(), 2);
  
  // 创建while语句
  auto while_stmt = std::make_unique<CSTNode>(CSTNodeType::WhileStmt, loc);
  auto loop_cond = std::make_unique<CSTNode>(CSTNodeType::BinaryExpr, loc);
  auto loop_body = std::make_unique<CSTNode>(CSTNodeType::BlockStmt, loc);
  
  while_stmt->add_child(std::move(loop_cond));
  while_stmt->add_child(std::move(loop_body));
  
  EXPECT_EQ(while_stmt->get_type(), CSTNodeType::WhileStmt);
  EXPECT_EQ(while_stmt->get_children().size(), 2);
}

/**
 * @test EmptyProgramCST
 * @brief 测试空程序的CST
 * @details
 *   验证目标：
 *   1. 可以创建不包含任何语句的Program节点
 *   2. 空Program节点的子节点列表为空
 */
TEST_F(CSTNodeTest, EmptyProgramCST) {
  auto loc = make_test_location();
  auto program = std::make_unique<CSTNode>(CSTNodeType::Program, loc);
  
  EXPECT_EQ(program->get_type(), CSTNodeType::Program);
  EXPECT_TRUE(program->get_children().empty());
}

/**
 * @test CSTWithComments
 * @brief 测试包含注释的CST
 * @details
 *   验证目标：
 *   1. CST可以包含Comment类型的节点
 *   2. 注释节点可以与其他节点共存
 */
TEST_F(CSTNodeTest, CSTWithComments) {
  auto loc = make_test_location();
  
  auto program = std::make_unique<CSTNode>(CSTNodeType::Program, loc);
  
  // 添加注释节点
  auto comment = std::make_unique<CSTNode>(CSTNodeType::Comment, loc);
  Token comment_tok(TokenType::Comment, "// This is a comment", 1, 1);
  comment->set_token(comment_tok);
  
  program->add_child(std::move(comment));
  program->add_child(std::make_unique<CSTNode>(CSTNodeType::ExprStmt, loc));
  
  EXPECT_EQ(program->get_children().size(), 2);
  EXPECT_EQ(program->get_children()[0]->get_type(), CSTNodeType::Comment);
  EXPECT_EQ(program->get_children()[1]->get_type(), CSTNodeType::ExprStmt);
}

/**
 * @test MakeCSTNodeWithLocation
 * @brief 测试make_cst_node辅助函数（使用SourceLocation）
 * @details
 *   验证目标：
 *   1. make_cst_node辅助函数能正确创建节点
 *   2. 创建的节点类型正确
 *   3. 位置信息正确保存
 */
TEST_F(CSTNodeTest, MakeCSTNodeWithLocation) {
  SourceLocation loc("helper_test.zero", 10, 20);
  
  auto node = make_cst_node(CSTNodeType::VarDeclaration, loc);
  
  ASSERT_NE(node, nullptr);
  EXPECT_EQ(node->get_type(), CSTNodeType::VarDeclaration);
  EXPECT_EQ(node->get_location().filename, "helper_test.zero");
  EXPECT_EQ(node->get_location().line, 10);
  EXPECT_EQ(node->get_location().column, 20);
  EXPECT_TRUE(node->get_children().empty());
}

/**
 * @test MakeCSTNodeWithToken
 * @brief 测试make_cst_node辅助函数（使用Token）
 * @details
 *   验证目标：
 *   1. make_cst_node能从Token创建节点
 *   2. Token信息正确关联到节点
 *   3. 位置信息从Token正确提取
 */
TEST_F(CSTNodeTest, MakeCSTNodeWithToken) {
  Token tok(TokenType::Identifier, "myVar", 15, 25);
  
  auto node = make_cst_node(CSTNodeType::Identifier, tok);
  
  ASSERT_NE(node, nullptr);
  EXPECT_EQ(node->get_type(), CSTNodeType::Identifier);
  EXPECT_EQ(node->get_location().line, 15);
  EXPECT_EQ(node->get_location().column, 25);
  
  const auto& token_opt = node->get_token();
  ASSERT_TRUE(token_opt.has_value());
  EXPECT_EQ(token_opt->token_type, TokenType::Identifier);
  EXPECT_EQ(token_opt->value, "myVar");
}

/**
 * @test AllCSTNodeTypeStrings
 * @brief 测试所有CST节点类型的字符串转换
 * @details
 *   验证目标：
 *   1. 所有CST节点类型都有对应的字符串表示
 *   2. 字符串转换不返回"Unknown"
 *   3. 每个类型的字符串都是唯一的
 */
TEST_F(CSTNodeTest, AllCSTNodeTypeStrings) {
  std::vector<CSTNodeType> all_types = {
    CSTNodeType::Program,
    CSTNodeType::VarDeclaration,
    CSTNodeType::FnDeclaration,
    CSTNodeType::ReturnStmt,
    CSTNodeType::IfStmt,
    CSTNodeType::WhileStmt,
    CSTNodeType::BlockStmt,
    CSTNodeType::ExprStmt,
    CSTNodeType::BinaryExpr,
    CSTNodeType::UnaryExpr,
    CSTNodeType::CallExpr,
    CSTNodeType::IndexExpr,
    CSTNodeType::MemberExpr,
    CSTNodeType::AssignExpr,
    CSTNodeType::IndexAssignExpr,
    CSTNodeType::ArrayLiteral,
    CSTNodeType::IntegerLiteral,
    CSTNodeType::FloatLiteral,
    CSTNodeType::StringLiteral,
    CSTNodeType::BooleanLiteral,
    CSTNodeType::Identifier,
    CSTNodeType::ParenExpr,
    CSTNodeType::TypeAnnotation,
    CSTNodeType::ArrayType,
    CSTNodeType::Parameter,
    CSTNodeType::ParameterList,
    CSTNodeType::ArgumentList,
    CSTNodeType::StatementList,
    CSTNodeType::Operator,
    CSTNodeType::Delimiter,
    CSTNodeType::Comment
  };
  
  std::unordered_set<std::string> seen_strings;
  
  for (const auto& type : all_types) {
    std::string type_str = cst_node_type_to_string(type);
    
    // 确保不是Unknown
    EXPECT_NE(type_str, "Unknown");
    
    // 确保字符串非空
    EXPECT_FALSE(type_str.empty());
    
    // 确保字符串唯一
    EXPECT_EQ(seen_strings.count(type_str), 0) 
      << "Duplicate string for type: " << type_str;
    seen_strings.insert(type_str);
  }
  
  // 验证所有类型都被测试
  EXPECT_EQ(seen_strings.size(), all_types.size());
}

/**
 * @test DeepNestedStructure
 * @brief 测试深层嵌套的CST结构
 * @details
 *   验证目标：
 *   1. 支持任意深度的嵌套
 *   2. 深层嵌套不会导致错误
 *   3. 可以正确访问深层子节点
 */
TEST_F(CSTNodeTest, DeepNestedStructure) {
  auto loc = make_test_location();
  
  // 创建5层嵌套结构
  auto root = std::make_unique<CSTNode>(CSTNodeType::Program, loc);
  auto level1 = std::make_unique<CSTNode>(CSTNodeType::BlockStmt, loc);
  auto level2 = std::make_unique<CSTNode>(CSTNodeType::IfStmt, loc);
  auto level3 = std::make_unique<CSTNode>(CSTNodeType::BlockStmt, loc);
  auto level4 = std::make_unique<CSTNode>(CSTNodeType::ExprStmt, loc);
  auto level5 = std::make_unique<CSTNode>(CSTNodeType::BinaryExpr, loc);
  
  level4->add_child(std::move(level5));
  level3->add_child(std::move(level4));
  level2->add_child(std::move(level3));
  level1->add_child(std::move(level2));
  root->add_child(std::move(level1));
  
  // 验证深层访问
  EXPECT_EQ(root->get_children().size(), 1);
  const auto* l1 = root->get_children()[0].get();
  EXPECT_EQ(l1->get_type(), CSTNodeType::BlockStmt);
  
  const auto* l2 = l1->get_children()[0].get();
  EXPECT_EQ(l2->get_type(), CSTNodeType::IfStmt);
  
  const auto* l3 = l2->get_children()[0].get();
  EXPECT_EQ(l3->get_type(), CSTNodeType::BlockStmt);
  
  const auto* l4 = l3->get_children()[0].get();
  EXPECT_EQ(l4->get_type(), CSTNodeType::ExprStmt);
  
  const auto* l5 = l4->get_children()[0].get();
  EXPECT_EQ(l5->get_type(), CSTNodeType::BinaryExpr);
}

/**
 * @test MultipleTokenAssociations
 * @brief 测试多个节点的Token关联
 * @details
 *   验证目标：
 *   1. 多个节点可以独立关联不同的Token
 *   2. Token关联互不干扰
 *   3. 每个节点的Token信息独立保存
 */
TEST_F(CSTNodeTest, MultipleTokenAssociations) {
  auto loc = make_test_location();
  
  // 创建多个节点并关联不同的Token
  auto node1 = std::make_unique<CSTNode>(CSTNodeType::IntegerLiteral, loc);
  auto node2 = std::make_unique<CSTNode>(CSTNodeType::StringLiteral, loc);
  auto node3 = std::make_unique<CSTNode>(CSTNodeType::Identifier, loc);
  
  Token tok1(TokenType::Integer, "123", 1, 1);
  Token tok2(TokenType::String, "hello", 2, 5);
  Token tok3(TokenType::Identifier, "var", 3, 10);
  
  node1->set_token(tok1);
  node2->set_token(tok2);
  node3->set_token(tok3);
  
  // 验证每个节点的Token独立
  EXPECT_EQ(node1->get_token()->value, "123");
  EXPECT_EQ(node2->get_token()->value, "hello");
  EXPECT_EQ(node3->get_token()->value, "var");
  
  EXPECT_EQ(node1->get_token()->token_type, TokenType::Integer);
  EXPECT_EQ(node2->get_token()->token_type, TokenType::String);
  EXPECT_EQ(node3->get_token()->token_type, TokenType::Identifier);
}

/**
 * @test CSTNodeCopySemantics
 * @brief 测试CST节点的移动语义
 * @details
 *   验证目标：
 *   1. 节点可以正确移动
 *   2. 移动后原节点被清空
 *   3. 目标节点获得所有数据
 */
TEST_F(CSTNodeTest, CSTNodeMoveSemantics) {
  auto loc = make_test_location();
  
  auto parent = std::make_unique<CSTNode>(CSTNodeType::Program, loc);
  auto child1 = std::make_unique<CSTNode>(CSTNodeType::ExprStmt, loc);
  auto child2 = std::make_unique<CSTNode>(CSTNodeType::VarDeclaration, loc);
  
  // 添加子节点前记录地址
  auto* child1_ptr = child1.get();
  auto* child2_ptr = child2.get();
  
  // 移动子节点到父节点
  parent->add_child(std::move(child1));
  parent->add_child(std::move(child2));
  
  // 验证子节点已被移动
  EXPECT_EQ(child1.get(), nullptr);
  EXPECT_EQ(child2.get(), nullptr);
  
  // 验证父节点拥有子节点
  EXPECT_EQ(parent->get_children().size(), 2);
  EXPECT_EQ(parent->get_children()[0].get(), child1_ptr);
  EXPECT_EQ(parent->get_children()[1].get(), child2_ptr);
}

/**
 * @test LocationInfoPreservation
 * @brief 测试位置信息在节点操作中的保留
 * @details
 *   验证目标：
 *   1. 添加子节点不影响父节点位置信息
 *   2. 关联Token不影响位置信息
 *   3. 位置信息在整个生命周期中保持不变
 */
TEST_F(CSTNodeTest, LocationInfoPreservation) {
  SourceLocation parent_loc("test.zero", 10, 15);
  SourceLocation child_loc("test.zero", 11, 20);
  
  auto parent = std::make_unique<CSTNode>(CSTNodeType::Program, parent_loc);
  auto child = std::make_unique<CSTNode>(CSTNodeType::ExprStmt, child_loc);
  
  // 记录初始位置
  EXPECT_EQ(parent->get_location().line, 10);
  EXPECT_EQ(parent->get_location().column, 15);
  
  // 添加子节点
  parent->add_child(std::move(child));
  
  // 验证父节点位置未改变
  EXPECT_EQ(parent->get_location().line, 10);
  EXPECT_EQ(parent->get_location().column, 15);
  
  // 验证子节点位置保留
  EXPECT_EQ(parent->get_children()[0]->get_location().line, 11);
  EXPECT_EQ(parent->get_children()[0]->get_location().column, 20);
  
  // 关联Token
  Token tok(TokenType::Integer, "42", 5, 5);
  parent->set_token(tok);
  
  // 验证位置仍然保持原值（不被Token覆盖）
  EXPECT_EQ(parent->get_location().line, 10);
  EXPECT_EQ(parent->get_location().column, 15);
}
