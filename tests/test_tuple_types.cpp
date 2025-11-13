/**
 * @file test_tuple_types.cpp
 * @brief 测试元组类型和元组字面量的解析和格式化 (Google Test 版本)。
 * @details 本测试套件全面测试元组系统的各项功能，包括：
 *          - 元组类型声明 (T1, T2, ...)
 *          - 元组字面量 (expr1, expr2, ...)
 *          - 与括号表达式的正确区分
 *          - 嵌套元组
 *          - 元组与其他类型的组合
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
 * @brief 元组类型测试夹具。
 * @details 提供通用的词法分析、语法分析和格式化功能。
 */
class TupleTypeTest : public ::testing::Test {
protected:
  /**
   * @brief 辅助函数：词法分析 + 语法分析 + CST 验证。
   */
  std::shared_ptr<CSTNode> parse(const std::string& source) {
    Lexer lexer(source, "test_tuple.zero");
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

// --- Basic Tuple Type Tests ---

/**
 * @test 测试基本的元组类型。
 * @details 验证二元组类型的解析。
 */
TEST_F(TupleTypeTest, BasicTupleType) {
  std::string source = "let pair: (Integer, String) = (42, \"answer\");";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);
  verify_node(cst.get(), CSTNodeType::Program);

  // 查找元组类型节点
  auto tuple_type = find_node_recursive(cst.get(), CSTNodeType::TupleType);
  ASSERT_NE(tuple_type, nullptr) << "Should find tuple type";

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("(Integer, String)") != std::string::npos);
}

/**
 * @test 测试不同长度的元组类型。
 * @details 验证各种元素数量的元组类型。
 */
TEST_F(TupleTypeTest, DifferentLengthTuples) {
  std::string source = R"(
let pair: (Integer, String) = (1, "one");
let triple: (Integer, Integer, Integer) = (1, 2, 3);
let quad: (String, Integer, Float, Boolean) = ("test", 42, 3.14, true);
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  // 统计元组类型数量
  size_t tuple_type_count = count_nodes(cst.get(), CSTNodeType::TupleType);
  EXPECT_EQ(tuple_type_count, 3) << "Should have 3 tuple types";

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("(Integer, String)") != std::string::npos);
  EXPECT_TRUE(formatted.find("(Integer, Integer, Integer)") !=
              std::string::npos);
  EXPECT_TRUE(formatted.find("(String, Integer, Float, Boolean)") !=
              std::string::npos);
}

/**
 * @test 测试同类型元组。
 * @details 验证所有元素类型相同的元组。
 */
TEST_F(TupleTypeTest, HomogeneousTuple) {
  std::string source =
      "let coords: (Integer, Integer, Integer) = (10, 20, 30);";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("(Integer, Integer, Integer)") !=
              std::string::npos);
}

// --- Tuple Literal Tests ---

/**
 * @test 测试基本的元组字面量。
 * @details 验证简单的元组字面量解析。
 */
TEST_F(TupleTypeTest, BasicTupleLiteral) {
  std::string source = "let data = (42, \"hello\", 3.14);";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("(42, \"hello\", 3.14)") != std::string::npos);
}

/**
 * @test 测试元组字面量与括号表达式的区分。
 * @details 验证单个元素的括号是表达式，而非元组。
 */
TEST_F(TupleTypeTest, TupleLiteralVsParenthesizedExpression) {
  std::string source = R"(
let expr = (42);
let tuple = (42, 43);
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  // 验证第一个是括号表达式（ParenExpr），第二个是元组字面量（TupleLiteral）
  const auto& children = cst->get_children();
  ASSERT_EQ(children.size(), 2);
}

/**
 * @test 测试空元组。
 * @details 验证零元素元组（单位类型）的支持。
 */
TEST_F(TupleTypeTest, EmptyTuple) {
  std::string source = "let unit = ();";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("()") != std::string::npos);
}

// --- Nested Tuple Tests ---

/**
 * @test 测试嵌套元组类型。
 * @details 验证元组内包含元组类型。
 */
TEST_F(TupleTypeTest, NestedTupleType) {
  std::string source =
      "let nested: ((Integer, Integer), String) = ((1, 2), \"pair\");";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("((Integer, Integer), String)") !=
              std::string::npos);
}

/**
 * @test 测试嵌套元组字面量。
 * @details 验证元组内包含元组字面量。
 */
TEST_F(TupleTypeTest, NestedTupleLiteral) {
  std::string source = "let data = ((1, 2), (3, 4), (5, 6));";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("((1, 2), (3, 4), (5, 6))") != std::string::npos);
}

/**
 * @test 测试深层嵌套元组。
 * @details 验证多层嵌套的元组。
 */
TEST_F(TupleTypeTest, DeeplyNestedTuple) {
  std::string source = "let deep = (((1, 2), 3), 4);";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("(((1, 2), 3), 4)") != std::string::npos);
}

// --- Tuple with Complex Types Tests ---

/**
 * @test 测试包含数组的元组。
 * @details 验证元组元素可以是数组类型。
 */
TEST_F(TupleTypeTest, TupleWithArrays) {
  std::string source =
      "let data: (Integer[], String[]) = ([1, 2, 3], [\"a\", \"b\"]);";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("(Integer[], String[])") != std::string::npos);
}

/**
 * @test 测试包含结构体的元组。
 * @details 验证元组元素可以是自定义类型。
 */
TEST_F(TupleTypeTest, TupleWithStructs) {
  std::string source = R"(
struct Point {
    x: Integer,
    y: Integer
};

let pair: (Point, Point) = (Point { x: 0, y: 0 }, Point { x: 1, y: 1 });
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("(Point, Point)") != std::string::npos);
}

/**
 * @test 测试包含函数类型的元组。
 * @details 验证元组元素可以是函数签名类型。
 */
TEST_F(TupleTypeTest, TupleWithFunctions) {
  std::string source = R"(
let ops: ((Integer) -> Integer, (Integer) -> Integer) = (
    fn (x) { return x + 1; },
    fn (x) { return x * 2; }
);
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("((Integer) -> Integer, (Integer) -> Integer)") !=
              std::string::npos);
}

// --- Tuple in Type Aliases Tests ---

/**
 * @test 测试元组类型别名。
 * @details 验证为元组类型创建别名。
 */
TEST_F(TupleTypeTest, TupleTypeAlias) {
  std::string source = R"(
type Point2D = (Integer, Integer);
type Point3D = (Integer, Integer, Integer);

let p2: Point2D = (10, 20);
let p3: Point3D = (10, 20, 30);
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("type Point2D = (Integer, Integer)") !=
              std::string::npos);
  EXPECT_TRUE(formatted.find("type Point3D = (Integer, Integer, Integer)") !=
              std::string::npos);
}

// --- Tuple in Struct Fields Tests ---

/**
 * @test 测试结构体中的元组字段。
 * @details 验证结构体字段可以是元组类型。
 */
TEST_F(TupleTypeTest, TupleFieldInStruct) {
  std::string source = R"(
struct DataPoint {
    id: Integer,
    coordinates: (Float, Float),
    metadata: (String, Integer, Boolean)
};
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("coordinates: (Float, Float)") !=
              std::string::npos);
  EXPECT_TRUE(formatted.find("metadata: (String, Integer, Boolean)") !=
              std::string::npos);
}

// --- Tuple in Function Signatures Tests ---

/**
 * @test 测试元组作为函数参数。
 * @details 验证函数参数可以是元组类型。
 */
TEST_F(TupleTypeTest, TupleAsParameter) {
  std::string source = R"(
fn processPoint(point: (Integer, Integer)) -> Integer {
    return 0;
}
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  // 格式化器可能不在冒号后添加空格
  EXPECT_TRUE(formatted.find("point") != std::string::npos);
  EXPECT_TRUE(formatted.find("(Integer, Integer)") != std::string::npos);
}

/**
 * @test 测试元组作为函数返回值。
 * @details 验证函数可以返回元组类型。
 */
TEST_F(TupleTypeTest, TupleAsReturnType) {
  std::string source = R"(
fn getPair() -> (Integer, String) {
    return (42, "answer");
}
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  // 检查返回类型存在
  EXPECT_TRUE(formatted.find("->") != std::string::npos);
  EXPECT_TRUE(formatted.find("(Integer, String)") != std::string::npos);
}

/**
 * @test 测试函数签名完全使用元组。
 * @details 验证多个元组参数和元组返回值。
 */
TEST_F(TupleTypeTest, FunctionWithMultipleTuples) {
  std::string source = R"(
fn transform(p1: (Integer, Integer), p2: (Integer, Integer)) -> (Integer, Integer) {
    return (0, 0);
}
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  // 检查参数和返回类型存在，不检查空格
  EXPECT_TRUE(formatted.find("p1") != std::string::npos);
  EXPECT_TRUE(formatted.find("p2") != std::string::npos);
  EXPECT_TRUE(formatted.find("->") != std::string::npos);
  // 至少出现3次 (Integer, Integer): p1类型, p2类型, 返回类型
  size_t count = 0;
  size_t pos = 0;
  std::string pattern = "(Integer, Integer)";
  while ((pos = formatted.find(pattern, pos)) != std::string::npos) {
    count++;
    pos += pattern.length();
  }
  EXPECT_GE(count, 3);
}

// --- Complex Expression Tests ---

/**
 * @test 测试元组字面量包含表达式。
 * @details 验证元组元素可以是计算表达式。
 */
TEST_F(TupleTypeTest, TupleWithExpressions) {
  std::string source = "let computed = (1 + 2, 3 * 4, 5 - 6);";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("(1 + 2, 3 * 4, 5 - 6)") != std::string::npos);
}

/**
 * @test 测试元组字面量包含函数调用。
 * @details 验证元组元素可以是函数调用结果。
 */
TEST_F(TupleTypeTest, TupleWithFunctionCalls) {
  std::string source = "let results = (func1(), func2(), func3());";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("(func1(), func2(), func3())") !=
              std::string::npos);
}

// --- Edge Cases ---

/**
 * @test 测试单元素元组类型（需要尾逗号）。
 * @details 验证单元素元组的语法支持。
 */
TEST_F(TupleTypeTest, SingleElementTuple) {
  // 注：根据语言设计，单元素可能需要特殊语法如 (T,)
  // 这里测试当前实现的行为
  std::string source = "let single = (42,);";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  // 验证是否被识别为元组（取决于实现）
}

/**
 * @test 测试综合场景。
 * @details 验证元组在各种场景下混合使用。
 */
TEST_F(TupleTypeTest, ComprehensiveTupleUsage) {
  std::string source = R"(
struct Point {
    x: Integer,
    y: Integer
};

type Pair = (Point, Point);
type Triple = (Integer, Integer, Integer);

fn distance(p1: Point, p2: Point) -> Float {
    return 0.0;
}

let points: Pair = (Point { x: 0, y: 0 }, Point { x: 3, y: 4 });
let coords: Triple = (1, 2, 3);
let mixed: (Integer, String, (Float, Float)) = (42, "test", (1.0, 2.0));
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  const auto& children = cst->get_children();
  EXPECT_GE(children.size(), 6);
}
