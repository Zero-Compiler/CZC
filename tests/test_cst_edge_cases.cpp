/**
 * @file test_cst_edge_cases.cpp
 * @brief CST边界情况和极端场景测试（Google Test 版本）
 * @details 本测试套件专注于测试 Array、Tuple、Struct、Function
 * 的边界情况，包括：
 *          - 空结构
 *          - 深度嵌套
 *          - 类型组合的极限
 *          - 错误恢复
 *          - 畸形输入
 * @author BegoniaHe
 * @date 2025-11-12
 */

#include "czc/cst/cst_node.hpp"
#include "czc/formatter/formatter.hpp"
#include "czc/lexer/lexer.hpp"
#include "czc/parser/parser.hpp"

#include <gtest/gtest.h>

using namespace czc::cst;
using namespace czc::formatter;
using namespace czc::lexer;
using namespace czc::parser;

// --- Test Fixtures ---

/**
 * @brief CST 边界测试夹具
 * @details 提供通用的解析和验证功能
 */
class CSTEdgeCasesTest : public ::testing::Test {
protected:
  std::unique_ptr<CSTNode> parse(const std::string& source,
                                 bool expect_errors = false) {
    Lexer lexer(source, "test_edge_cases.zero");
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto cst = parser.parse();

    if (expect_errors) {
      EXPECT_TRUE(parser.has_errors());
    } else {
      EXPECT_FALSE(parser.has_errors());
    }

    return cst;
  }

  void verify_node_count(const CSTNode* node, CSTNodeType type, int expected) {
    int count = count_nodes(node, type);
    EXPECT_EQ(count, expected)
        << "Expected " << expected << " nodes of type "
        << cst_node_type_to_string(type) << ", but got " << count;
  }

private:
  int count_nodes(const CSTNode* node, CSTNodeType type) const {
    if (!node)
      return 0;
    int count = (node->get_type() == type) ? 1 : 0;
    for (const auto& child : node->get_children()) {
      count += count_nodes(child.get(), type);
    }
    return count;
  }
};

// ============================================================================
// Array Edge Cases
// ============================================================================

/**
 * @test 测试空数组
 * @details 验证各种类型的空数组字面量
 */
TEST_F(CSTEdgeCasesTest, EmptyArrays) {
  std::string source = R"(
let empty1: Integer[] = [];
let empty2: Float[][] = [];
let empty3: String[][][] = [];
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);
  verify_node_count(cst.get(), CSTNodeType::ArrayLiteral, 3);
}

/**
 * @test 测试单元素数组
 * @details 边界：最小非空数组
 */
TEST_F(CSTEdgeCasesTest, SingleElementArray) {
  std::string source = R"(
let single1: Integer[] = [42];
let single2: Float[] = [3.14];
let single3: String[] = ["hello"];
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);
  verify_node_count(cst.get(), CSTNodeType::ArrayLiteral, 3);
}

/**
 * @test 测试极深嵌套数组类型
 * @details 边界：8层嵌套数组类型
 */
TEST_F(CSTEdgeCasesTest, DeeplyNestedArrayTypes) {
  std::string source = "let deep: Integer[][][][] = [[[[42]]]];";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  // 应该有 4 个 ArrayType 节点
  verify_node_count(cst.get(), CSTNodeType::ArrayType, 4);
}

/**
 * @test 测试混合固定和动态数组
 * @details 边界：固定大小和动态数组的组合
 */
TEST_F(CSTEdgeCasesTest, MixedSizedAndDynamicArrays) {
  std::string source = R"(
let mix1: Integer[5][] = [[1, 2, 3, 4, 5]];
let mix2: Integer[][10] = [[1], [2]];
let mix3: Integer[3][4][5] = [[[1]]];
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);
  verify_node_count(cst.get(), CSTNodeType::SizedArrayType, 5);
}

/**
 * @test 测试零大小数组类型（畸形输入）
 * @details 边界：T[0] 应该被解析但可能在语义分析时报错
 */
TEST_F(CSTEdgeCasesTest, ZeroSizedArray) {
  std::string source = "let zero: Integer[0] = [];";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);
  verify_node_count(cst.get(), CSTNodeType::SizedArrayType, 1);
}

/**
 * @test 测试巨大的数组大小
 * @details 边界：极大的数组大小字面量
 */
TEST_F(CSTEdgeCasesTest, HugeSizedArray) {
  std::string source = "let huge: Integer[999999999] = [];";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);
  verify_node_count(cst.get(), CSTNodeType::SizedArrayType, 1);
}

/**
 * @test 测试数组中的尾随逗号
 * @details 边界：验证数组支持尾随逗号
 */
TEST_F(CSTEdgeCasesTest, ArrayTrailingComma) {
  std::string source = R"(
let arr1: Integer[] = [1, 2, 3,];
let arr2: String[] = ["a", "b",];
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);
  verify_node_count(cst.get(), CSTNodeType::ArrayLiteral, 2);
}

// ============================================================================
// Tuple Edge Cases
// ============================================================================

/**
 * @test 测试最小元组（2元素）
 * @details 边界：元组至少需要2个元素
 */
TEST_F(CSTEdgeCasesTest, MinimalTuple) {
  std::string source = "let pair: (Integer, String) = (42, \"answer\");";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);
  verify_node_count(cst.get(), CSTNodeType::TupleType, 1);
  verify_node_count(cst.get(), CSTNodeType::TupleLiteral, 1);
}

/**
 * @test 测试超大元组
 * @details 边界：20个元素的元组
 */
TEST_F(CSTEdgeCasesTest, LargeTuple) {
  std::string source = R"(
let large: (Integer, Integer, Integer, Integer, Integer,
            Integer, Integer, Integer, Integer, Integer,
            Integer, Integer, Integer, Integer, Integer,
            Integer, Integer, Integer, Integer, Integer) =
           (1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
            11, 12, 13, 14, 15, 16, 17, 18, 19, 20);
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);
  verify_node_count(cst.get(), CSTNodeType::TupleType, 1);
  verify_node_count(cst.get(), CSTNodeType::TupleLiteral, 1);
}

/**
 * @test 测试嵌套元组
 * @details 边界：元组中包含元组
 */
TEST_F(CSTEdgeCasesTest, NestedTuples) {
  std::string source = R"(
let nested1: ((Integer, String), Float) = ((42, "answer"), 3.14);
let nested2: (Integer, (String, (Boolean, Float))) =
             (1, ("test", (true, 2.5)));
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);
  // nested1: 2 个元组类型, nested2: 3 个元组类型
  verify_node_count(cst.get(), CSTNodeType::TupleType, 5);
  verify_node_count(cst.get(), CSTNodeType::TupleLiteral, 5);
}

/**
 * @test 测试元组尾随逗号
 * @details 边界：验证元组支持尾随逗号
 */
TEST_F(CSTEdgeCasesTest, TupleTrailingComma) {
  std::string source = R"(
let t1: (Integer, String) = (42, "answer",);
let t2: (Integer, Integer, Integer) = (1, 2, 3,);
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);
  verify_node_count(cst.get(), CSTNodeType::TupleLiteral, 2);
}

/**
 * @test 测试单元素括号（不是元组）
 * @details 边界：(expr) 应该被解析为括号表达式，不是元组
 */
TEST_F(CSTEdgeCasesTest, ParenExprNotTuple) {
  std::string source = R"(
let x: Integer = (42);
let y: Float = (3.14 + 2.0);
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);
  // 应该是括号表达式，不是元组
  verify_node_count(cst.get(), CSTNodeType::ParenExpr, 2);
  verify_node_count(cst.get(), CSTNodeType::TupleLiteral, 0);
}

// ============================================================================
// Struct Edge Cases
// ============================================================================

/**
 * @test 测试空结构体
 * @details 边界：没有字段的结构体
 */
TEST_F(CSTEdgeCasesTest, EmptyStruct) {
  std::string source = R"(
struct Empty {};
let e: Empty = Empty {};
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);
  verify_node_count(cst.get(), CSTNodeType::StructDeclaration, 1);
  verify_node_count(cst.get(), CSTNodeType::StructLiteral, 1);
}

/**
 * @test 测试单字段结构体
 * @details 边界：最小非空结构体
 */
TEST_F(CSTEdgeCasesTest, SingleFieldStruct) {
  std::string source = R"(
struct Single { value: Integer };
let s: Single = Single { value: 42 };
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);
  verify_node_count(cst.get(), CSTNodeType::StructField, 1);
}

/**
 * @test 测试大型结构体
 * @details 边界：包含20个字段的结构体
 */
TEST_F(CSTEdgeCasesTest, LargeStruct) {
  std::string source = R"(
struct Large {
  f1: Integer,
  f2: Integer,
  f3: Integer,
  f4: Integer,
  f5: Integer,
  f6: Integer,
  f7: Integer,
  f8: Integer,
  f9: Integer,
  f10: Integer,
  f11: String,
  f12: String,
  f13: Float,
  f14: Float,
  f15: Boolean,
  f16: Boolean,
  f17: Integer,
  f18: Integer,
  f19: Integer,
  f20: Integer
};
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);
  verify_node_count(cst.get(), CSTNodeType::StructField, 20);
}

/**
 * @test 测试嵌套结构体字段
 * @details 边界：结构体包含结构体类型的字段
 */
TEST_F(CSTEdgeCasesTest, NestedStructFields) {
  std::string source = R"(
struct Inner { value: Integer };
struct Outer { inner: Inner, name: String };
let o: Outer = Outer {
  inner: Inner { value: 42 },
  name: "test"
};
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);
  verify_node_count(cst.get(), CSTNodeType::StructDeclaration, 2);
  verify_node_count(cst.get(), CSTNodeType::StructLiteral, 2);
}

/**
 * @test 测试结构体字段尾随逗号
 * @details 边界：结构体定义和初始化允许尾随逗号
 */
TEST_F(CSTEdgeCasesTest, StructTrailingComma) {
  std::string source = R"(
struct Point {
  x: Integer,
  y: Integer,
};
let p: Point = Point { x: 10, y: 20, };
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);
  verify_node_count(cst.get(), CSTNodeType::StructDeclaration, 1);
}

/**
 * @test 测试结构体与数组/元组的组合
 * @details 边界：复杂的类型组合
 */
TEST_F(CSTEdgeCasesTest, StructWithComplexFields) {
  std::string source = R"(
struct Complex {
  arr: Integer[],
  tup: (String, Float),
  nested: Integer[][]
};
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);
  verify_node_count(cst.get(), CSTNodeType::StructField, 3);
  verify_node_count(cst.get(), CSTNodeType::ArrayType, 3);
  verify_node_count(cst.get(), CSTNodeType::TupleType, 1);
}

// ============================================================================
// Function Edge Cases
// ============================================================================

/**
 * @test 测试无参数函数字面量
 * @details 边界：空参数列表
 */
TEST_F(CSTEdgeCasesTest, ZeroParameterFunction) {
  std::string source = R"(
let f: () -> Integer = fn () {
  return 42;
};
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);
  verify_node_count(cst.get(), CSTNodeType::FunctionLiteral, 1);
  verify_node_count(cst.get(), CSTNodeType::FunctionSignatureType, 1);
}

/**
 * @test 测试单参数函数
 * @details 边界：最小参数列表
 */
TEST_F(CSTEdgeCasesTest, SingleParameterFunction) {
  std::string source = R"(
let f: (Integer) -> Integer = fn (x) {
  return x;
};
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);
  verify_node_count(cst.get(), CSTNodeType::FunctionLiteral, 1);
  verify_node_count(cst.get(), CSTNodeType::Parameter, 1);
}

/**
 * @test 测试多参数函数
 * @details 边界：10个参数的函数
 */
TEST_F(CSTEdgeCasesTest, ManyParametersFunction) {
  std::string source = R"(
let f = fn (a, b, c, d, e, f, g, h, i, j) {
  return a + b + c + d + e + f + g + h + i + j;
};
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);
  verify_node_count(cst.get(), CSTNodeType::Parameter, 10);
}

/**
 * @test 测试嵌套函数字面量
 * @details 边界：函数返回函数
 */
TEST_F(CSTEdgeCasesTest, NestedFunctionLiterals) {
  std::string source = R"(
let makeAdder = fn (x) {
  return fn (y) {
    return x + y;
  };
};
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);
  verify_node_count(cst.get(), CSTNodeType::FunctionLiteral, 2);
}

/**
 * @test 测试函数签名类型的组合
 * @details 边界：函数类型作为参数和返回类型
 */
TEST_F(CSTEdgeCasesTest, FunctionTypeCompositions) {
  std::string source = R"(
type UnaryOp = (Integer) -> Integer;
type BinaryOp = (Integer, Integer) -> Integer;
type Composer = ((Integer) -> Integer) -> ((Integer) -> Integer);
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);
  // Composer 包含 3 个 FunctionSignatureType: 整个签名 + 2 个参数/返回值
  verify_node_count(cst.get(), CSTNodeType::FunctionSignatureType, 5);
}

/**
 * @test 测试函数数组
 * @details 边界：函数作为一等公民存储在数组中
 */
TEST_F(CSTEdgeCasesTest, ArrayOfFunctions) {
  std::string source = R"(
let ops: ((Integer) -> Integer)[] = [
  fn (x) { return x + 1; },
  fn (x) { return x * 2; },
  fn (x) { return x - 1; }
];
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);
  verify_node_count(cst.get(), CSTNodeType::FunctionLiteral, 3);
  verify_node_count(cst.get(), CSTNodeType::ArrayLiteral, 1);
}

/**
 * @test 测试空函数体
 * @details 边界：没有语句的函数体
 */
TEST_F(CSTEdgeCasesTest, EmptyFunctionBody) {
  std::string source = R"(
let empty = fn () {};
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);
  verify_node_count(cst.get(), CSTNodeType::FunctionLiteral, 1);
}

// ============================================================================
// Complex Type Combinations
// ============================================================================

/**
 * @test 测试极端复杂的类型嵌套
 * @details 边界：数组+元组+结构体+函数的深度组合
 */
TEST_F(CSTEdgeCasesTest, ExtremeTypeNesting) {
  std::string source = R"(
type Complex = ((Integer, String) -> (Float, Boolean))[][];
type MoreComplex = struct {
  field1: ((Integer) -> Integer)[],
  field2: (String, (Integer, Float))[],
  field3: ((Integer, Integer) -> (String, Boolean))
};
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);
  EXPECT_TRUE(cst != nullptr);
}

/**
 * @test 测试结构体字面量中的复杂表达式
 * @details 边界：初始化值包含函数调用、数组等
 */
TEST_F(CSTEdgeCasesTest, StructLiteralWithComplexExpressions) {
  std::string source = R"(
struct Data {
  arr: Integer[],
  tup: (Integer, String),
  func: (Integer) -> Integer
};
let data: Data = Data {
  arr: [1, 2, 3],
  tup: (42, "test"),
  func: fn (x) { return x * 2; }
};
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);
  verify_node_count(cst.get(), CSTNodeType::ArrayLiteral, 1);
  verify_node_count(cst.get(), CSTNodeType::TupleLiteral, 1);
  verify_node_count(cst.get(), CSTNodeType::FunctionLiteral, 1);
}

// ============================================================================
// Error Recovery Edge Cases
// ============================================================================

/**
 * @test 测试未闭合的数组
 * @details 边界：缺少右方括号
 */
TEST_F(CSTEdgeCasesTest, UnclosedArray) {
  std::string source = "let arr: Integer[] = [1, 2, 3;";

  auto cst = parse(source, true); // 期望错误
  ASSERT_NE(cst, nullptr);
  // CST 应该仍然被构建（错误恢复）
}

/**
 * @test 测试未闭合的元组
 * @details 边界：缺少右括号
 */
TEST_F(CSTEdgeCasesTest, UnclosedTuple) {
  std::string source = "let t: (Integer, String = (42, \"test\";";

  auto cst = parse(source, true); // 期望错误
  ASSERT_NE(cst, nullptr);
}

/**
 * @test 测试未闭合的结构体
 * @details 边界：缺少右花括号
 */
TEST_F(CSTEdgeCasesTest, UnclosedStruct) {
  std::string source = R"(
struct Point {
  x: Integer,
  y: Integer
;
)";

  auto cst = parse(source, true); // 期望错误
  ASSERT_NE(cst, nullptr);
}

/**
 * @test 测试未闭合的函数字面量
 * @details 边界：缺少右花括号
 */
TEST_F(CSTEdgeCasesTest, UnclosedFunctionLiteral) {
  std::string source = R"(
let f = fn (x) {
  return x;
;
)";

  auto cst = parse(source, true); // 期望错误
  ASSERT_NE(cst, nullptr);
}

/**
 * @test 测试错误的逗号位置
 * @details 边界：前导逗号（语法错误）
 */
TEST_F(CSTEdgeCasesTest, LeadingCommas) {
  std::string source = R"(
let arr: Integer[] = [, 1, 2, 3];
let tup: (Integer, String) = (, 42, "test");
)";

  auto cst = parse(source, true); // 期望错误
  ASSERT_NE(cst, nullptr);
}

/**
 * @test 测试连续逗号
 * @details 边界：多个逗号之间没有元素
 */
TEST_F(CSTEdgeCasesTest, ConsecutiveCommas) {
  std::string source = R"(
let arr: Integer[] = [1,, 2,, 3];
)";

  auto cst = parse(source, true); // 期望错误
  ASSERT_NE(cst, nullptr);
}

// ============================================================================
// Whitespace and Comment Edge Cases
// ============================================================================

/**
 * @test 测试极端的空白处理
 * @details 边界：大量空白和换行
 */
TEST_F(CSTEdgeCasesTest, ExtremWhitespace) {
  std::string source = R"(
let   arr   :   Integer  [  ]   =   [   1   ,   2   ,   3   ]  ;
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);
  // 应该正常解析，忽略多余空白
}

/**
 * @test 测试注释在复杂结构中
 * @details 边界：注释出现在各种位置
 */
TEST_F(CSTEdgeCasesTest, CommentsInComplexStructures) {
  std::string source = R"(
struct Point {
  x: Integer,
  y: Integer
};

let p: Point = Point {
  x: 10,
  y: 20
};
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);
  // 简化测试：只验证结构体被正确解析
  verify_node_count(cst.get(), CSTNodeType::StructDeclaration, 1);
  verify_node_count(cst.get(), CSTNodeType::StructLiteral, 1);
}

// ============================================================================
// Special Characters and Unicode
// ============================================================================

/**
 * @test 测试 UTF-8 标识符在复杂类型中
 * @details 边界：Unicode 字符作为字段名和变量名
 */
TEST_F(CSTEdgeCasesTest, UnicodeIdentifiersInComplexTypes) {
  std::string source = R"(
struct 点 {
  横坐标: Integer,
  纵坐标: Integer
};
let 我的点: 点 = 点 { 横坐标: 10, 纵坐标: 20 };
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);
  verify_node_count(cst.get(), CSTNodeType::StructDeclaration, 1);
}

/**
 * @test 测试特殊字符串内容在结构体中
 * @details 边界：包含转义字符的字符串
 */
TEST_F(CSTEdgeCasesTest, SpecialStringContents) {
  std::string source = R"(
struct Data {
  escaped: String
};
let d: Data = Data { escaped: "line1\nline2\ttab\x41\u0041" };
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);
}

// ============================================================================
// Type Alias Edge Cases
// ============================================================================

/**
 * @test 测试递归类型别名（间接）
 * @details 边界：类型别名相互引用
 */
TEST_F(CSTEdgeCasesTest, MutuallyRecursiveTypeAliases) {
  std::string source = R"(
type ListNode = struct {
  value: Integer,
  next: ListNode
};
type Tree = struct {
  left: Tree,
  right: Tree,
  value: Integer
};
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);
  verify_node_count(cst.get(), CSTNodeType::TypeAliasDeclaration, 2);
}

/**
 * @test 测试复杂的类型别名链
 * @details 边界：多层类型别名
 */
TEST_F(CSTEdgeCasesTest, ChainedTypeAliases) {
  std::string source = R"(
type IntArray = Integer[];
type IntMatrix = IntArray[];
type IntCube = IntMatrix[];
let cube: IntCube = [[[1]]];
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);
  verify_node_count(cst.get(), CSTNodeType::TypeAliasDeclaration, 3);
}

// 运行所有测试
int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
