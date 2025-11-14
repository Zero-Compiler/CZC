/**
 * @file test_struct_type_alias.cpp
 * @brief 测试结构体声明、类型别名和结构体字面量的解析和格式化 (Google Test
 * 版本)。
 * @details 本测试套件全面测试结构体系统的各项功能，包括：
 *          - 结构体声明及字段定义
 *          - 类型别名（包括联合、交集、否定等复杂类型表达式）
 *          - 结构体字面量的解析与消歧
 *          - 与 if 语句代码块的正确区分
 * @author BegoniaHe
 * @date 2025-11-12
 */

#include "czc/formatter/formatter.hpp"
#include "czc/lexer/lexer.hpp"
#include "czc/parser/parser.hpp"

#include "test_helpers.hpp"
#include <gtest/gtest.h>

using namespace czc::formatter;
using namespace czc::lexer;
using namespace czc::parser;
using namespace czc::cst;
using namespace czc::test;

// --- Test Fixtures ---

/**
 * @brief 结构体测试夹具。
 * @details 提供通用的词法分析、语法分析和格式化功能。
 */
class StructTest : public ::testing::Test {
protected:
  /**
   * @brief 辅助函数：词法分析 + 语法分析 + CST 验证。
   */
  std::shared_ptr<CSTNode> parse(const std::string& source) {
    Lexer lexer(source, "test_struct.zero");
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    return parser.parse();
  }

  /**
   * @brief 辅助函数：格式化 CST 并返回结果字符串。
   */
  std::string format(std::shared_ptr<CSTNode> cst) {
    Formatter formatter;
    return formatter.format(cst.get());
  }
};

// --- Basic Struct Tests ---

/**
 * @test 测试基本的结构体声明。
 * @details 验证包含多个字段的结构体能正确解析。
 */
TEST_F(StructTest, BasicStructDeclaration) {
  std::string source = R"(
struct Person {
    name: String,
    age: Integer
};
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);
  EXPECT_EQ(cst->get_type(), CSTNodeType::Program);

  // 验证有一个子节点（结构体声明）
  const auto& children = cst->get_children();
  ASSERT_EQ(children.size(), 1);

  auto struct_node = children[0].get();
  EXPECT_EQ(struct_node->get_type(), CSTNodeType::StructDeclaration);

  // 使用辅助函数进行结构化验证
  verify_struct_declaration(struct_node, "Person", 2);

  // 验证格式化输出
  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("struct Person") != std::string::npos);
  EXPECT_TRUE(formatted.find("name: String") != std::string::npos);
  EXPECT_TRUE(formatted.find("age: Integer") != std::string::npos);
}

/**
 * @test 测试空结构体声明。
 * @details 验证没有字段的结构体能正确解析。
 */
TEST_F(StructTest, EmptyStruct) {
  std::string source = "struct Empty {};";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);
  EXPECT_EQ(cst->get_type(), CSTNodeType::Program);

  const auto& children = cst->get_children();
  ASSERT_EQ(children.size(), 1);

  auto struct_node = children[0].get();
  EXPECT_EQ(struct_node->get_type(), CSTNodeType::StructDeclaration);

  // 使用辅助函数验证空结构体
  verify_struct_declaration(struct_node, "Empty", 0);
}

/**
 * @test 测试包含多种类型字段的结构体。
 * @details 验证结构体字段可以使用各种类型表达式。
 */
TEST_F(StructTest, StructWithComplexTypes) {
  std::string source = R"(
struct DataPoint {
    id: Integer,
    value: Float,
    label: String,
    active: Boolean,
    tags: String[],
    coordinates: (Float, Float)
};
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("id: Integer") != std::string::npos);
  EXPECT_TRUE(formatted.find("tags: String[]") != std::string::npos);
  EXPECT_TRUE(formatted.find("coordinates: (Float, Float)") !=
              std::string::npos);
}

// --- Type Alias Tests ---

/**
 * @test 测试简单类型别名。
 * @details 验证基本的类型别名声明。
 */
TEST_F(StructTest, SimpleTypeAlias) {
  std::string source = "type User = Person;";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  const auto& children = cst->get_children();
  ASSERT_EQ(children.size(), 1);
  EXPECT_EQ(children[0]->get_type(), CSTNodeType::TypeAliasDeclaration);

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("type User = Person") != std::string::npos);
}

/**
 * @test 测试联合类型别名。
 * @details 验证使用 | 操作符的联合类型。
 */
TEST_F(StructTest, UnionTypeAlias) {
  std::string source = "type StringOrInt = String | Integer;";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("String | Integer") != std::string::npos);
}

/**
 * @test 测试交集类型别名。
 * @details 验证使用 & 操作符的交集类型。
 */
TEST_F(StructTest, IntersectionTypeAlias) {
  std::string source = "type Admin = User & Permissions;";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("User & Permissions") != std::string::npos);
}

/**
 * @test 测试否定类型别名。
 * @details 验证使用 ~ 操作符的否定类型。
 */
TEST_F(StructTest, NegationTypeAlias) {
  std::string source = "type NotNull = ~Null;";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("~Null") != std::string::npos);
}

/**
 * @test 测试复杂类型表达式别名。
 * @details 验证多种类型操作符组合使用。
 */
TEST_F(StructTest, ComplexTypeExpression) {
  std::string source = "type Complex = (String | Integer) & ~Null;";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("String | Integer") != std::string::npos);
  EXPECT_TRUE(formatted.find("~Null") != std::string::npos);
}

// --- Struct Literal Tests ---

/**
 * @test 测试基本结构体字面量。
 * @details 验证结构体字面量的解析。
 */
TEST_F(StructTest, BasicStructLiteral) {
  std::string source = R"(
let p = Person { name: "Alice", age: 30 };
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("Person {") != std::string::npos);
  EXPECT_TRUE(formatted.find("name: \"Alice\"") != std::string::npos);
  EXPECT_TRUE(formatted.find("age: 30") != std::string::npos);
}

/**
 * @test 测试空结构体字面量。
 * @details 验证 {} 被正确识别为空结构体字面量。
 */
TEST_F(StructTest, EmptyStructLiteral) {
  std::string source = "let e = Empty {};";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  // 格式化器会将空花括号拆成两行
  EXPECT_TRUE(formatted.find("Empty {") != std::string::npos);
}

/**
 * @test 测试嵌套结构体字面量。
 * @details 验证结构体字面量可以嵌套使用。
 */
TEST_F(StructTest, NestedStructLiteral) {
  std::string source = R"(
let data = Wrapper {
    inner: Person { name: "Bob", age: 25 },
    count: 1
};
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("Wrapper {") != std::string::npos);
  EXPECT_TRUE(formatted.find("Person {") != std::string::npos);
}

// --- Disambiguation Tests ---

/**
 * @test 测试结构体字面量与 if 语句的消歧。
 * @details 验证 if 语句的代码块不会被误识别为结构体字面量。
 */
TEST_F(StructTest, StructLiteralVsIfStatement) {
  std::string source = R"(
let flag = true;
if flag {
    return "yes";
}
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  const auto& children = cst->get_children();
  ASSERT_EQ(children.size(), 2);
  EXPECT_EQ(children[0]->get_type(), CSTNodeType::VarDeclaration);
  EXPECT_EQ(children[1]->get_type(), CSTNodeType::IfStmt);
}

/**
 * @test 测试小写类型名的结构体字面量。
 * @details 验证小写类型名也能正确解析（通过 field: 模式识别）。
 */
TEST_F(StructTest, LowercaseStructLiteral) {
  std::string source = R"(
struct point {
    x: Integer,
    y: Integer
};

let p = point { x: 10, y: 20 };
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("struct point") != std::string::npos);
  EXPECT_TRUE(formatted.find("point {") != std::string::npos);
}

/**
 * @test 测试结构体字面量与 if-else 的消歧。
 * @details 验证复杂的 if-else 语句不会与结构体字面量混淆。
 */
TEST_F(StructTest, StructLiteralWithIfElse) {
  std::string source = R"(
let result = process { data: value };
if condition {
    action1();
} else {
    action2();
}
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  const auto& children = cst->get_children();
  ASSERT_GE(children.size(), 2);
  EXPECT_EQ(children[1]->get_type(), CSTNodeType::IfStmt);
}

/**
 * @test 测试在表达式中使用结构体字面量。
 * @details 验证结构体字面量可以作为函数参数。
 */
TEST_F(StructTest, StructLiteralInExpression) {
  std::string source = R"(
let result = processData(Person { name: "Charlie", age: 35 });
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("processData(Person {") != std::string::npos);
}

// --- Mixed Feature Tests ---

/**
 * @test 测试综合场景。
 * @details 验证结构体、类型别名、字面量混合使用。
 */
TEST_F(StructTest, ComprehensiveMixedFeatures) {
  std::string source = R"(
struct Person {
    name: String,
    age: Integer
};

type User = Person;

let user: User = Person { name: "Dave", age: 40 };

if user {
    print("User exists");
}
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  const auto& children = cst->get_children();
  EXPECT_GE(children.size(), 4);
  EXPECT_EQ(children[0]->get_type(), CSTNodeType::StructDeclaration);
  EXPECT_EQ(children[1]->get_type(), CSTNodeType::TypeAliasDeclaration);
  EXPECT_EQ(children[2]->get_type(), CSTNodeType::VarDeclaration);
  EXPECT_EQ(children[3]->get_type(), CSTNodeType::IfStmt);
}
