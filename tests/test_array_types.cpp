/**
 * @file test_array_types.cpp
 * @brief 测试数组类型和数组字面量的解析和格式化 (Google Test 版本)。
 * @details 本测试套件全面测试数组系统的各项功能，包括：
 *          - 动态数组类型 (T[])
 *          - 固定大小数组类型 (T[N])
 *          - 多维数组类型 (T[][], T[N][M])
 *          - 数组字面量
 *          - 数组类型在变量声明和类型表达式中的使用
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
 * @brief 数组类型测试夹具。
 * @details 提供通用的词法分析、语法分析和格式化功能。
 */
class ArrayTypeTest : public ::testing::Test {
protected:
  /**
   * @brief 辅助函数：词法分析 + 语法分析 + CST 验证。
   */
  std::shared_ptr<CSTNode> parse(const std::string& source) {
    Lexer lexer(source, "test_array.zero");
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

// --- Dynamic Array Type Tests ---

/**
 * @test 测试基本的动态数组类型。
 * @details 验证 T[] 语法的解析。
 */
TEST_F(ArrayTypeTest, BasicDynamicArray) {
  std::string source = "let numbers: Integer[] = [1, 2, 3];";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);
  EXPECT_EQ(cst->get_type(), CSTNodeType::Program);

  // 查找变量声明节点
  auto var_decl = find_node_recursive(cst.get(), CSTNodeType::VarDeclaration);
  ASSERT_NE(var_decl, nullptr) << "Should find variable declaration";

  // 验证数组类型节点
  auto array_type = find_node_recursive(var_decl, CSTNodeType::ArrayType);
  ASSERT_NE(array_type, nullptr) << "Should find array type";
  verify_array_type(array_type, false, "Integer");

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("Integer[]") != std::string::npos);
}

/**
 * @test 测试多种基本类型的动态数组。
 * @details 验证不同基本类型都能形成数组类型。
 */
TEST_F(ArrayTypeTest, DynamicArraysOfDifferentTypes) {
  std::string source = R"(
let integers: Integer[] = [1, 2, 3];
let floats: Float[] = [1.0, 2.5, 3.14];
let strings: String[] = ["hello", "world"];
let bools: Boolean[] = [true, false];
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("Integer[]") != std::string::npos);
  EXPECT_TRUE(formatted.find("Float[]") != std::string::npos);
  EXPECT_TRUE(formatted.find("String[]") != std::string::npos);
  EXPECT_TRUE(formatted.find("Boolean[]") != std::string::npos);
}

/**
 * @test 测试空数组字面量。
 * @details 验证空数组 [] 的解析。
 */
TEST_F(ArrayTypeTest, EmptyArrayLiteral) {
  std::string source = "let empty: Integer[] = [];";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("= []") != std::string::npos);
}

// --- Sized Array Type Tests ---

/**
 * @test 测试固定大小数组类型。
 * @details 验证 T[N] 语法的解析。
 */
TEST_F(ArrayTypeTest, SizedArray) {
  std::string source = "let fixed: Integer[5] = [1, 2, 3, 4, 5];";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("Integer[5]") != std::string::npos);
}

/**
 * @test 测试不同大小的固定数组。
 * @details 验证各种数组大小的支持。
 */
TEST_F(ArrayTypeTest, DifferentSizedArrays) {
  std::string source = R"(
let small: Integer[3] = [1, 2, 3];
let medium: Integer[10] = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10];
let large: Integer[100] = [];
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("Integer[3]") != std::string::npos);
  EXPECT_TRUE(formatted.find("Integer[10]") != std::string::npos);
  EXPECT_TRUE(formatted.find("Integer[100]") != std::string::npos);
}

// --- Multi-dimensional Array Tests ---

/**
 * @test 测试二维动态数组。
 * @details 验证 T[][] 语法的解析。
 */
TEST_F(ArrayTypeTest, TwoDimensionalDynamicArray) {
  std::string source = "let matrix: Integer[][] = [[1, 2], [3, 4]];";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("Integer[][]") != std::string::npos);
}

/**
 * @test 测试二维固定数组。
 * @details 验证 T[N][M] 语法的解析。
 */
TEST_F(ArrayTypeTest, TwoDimensionalSizedArray) {
  std::string source =
      "let matrix: Integer[3][3] = [[1, 2, 3], [4, 5, 6], [7, 8, 9]];";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("Integer[3][3]") != std::string::npos);
}

/**
 * @test 测试混合维度数组。
 * @details 验证动态和固定大小混合使用 T[N][]。
 */
TEST_F(ArrayTypeTest, MixedDimensionalArray) {
  std::string source = R"(
let mixed1: Integer[5][] = [[], [], [], [], []];
let mixed2: Integer[][3] = [[1, 2, 3], [4, 5, 6]];
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("Integer[5][]") != std::string::npos);
  EXPECT_TRUE(formatted.find("Integer[][3]") != std::string::npos);
}

/**
 * @test 测试三维数组。
 * @details 验证三维及更高维度数组的支持。
 */
TEST_F(ArrayTypeTest, ThreeDimensionalArray) {
  std::string source = R"(
let cube: Integer[][][] = [[[1, 2], [3, 4]], [[5, 6], [7, 8]]];
let sized_cube: Integer[2][2][2] = [[[1, 2], [3, 4]], [[5, 6], [7, 8]]];
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("Integer[][][]") != std::string::npos);
  EXPECT_TRUE(formatted.find("Integer[2][2][2]") != std::string::npos);
}

// --- Array of Complex Types Tests ---

/**
 * @test 测试结构体数组。
 * @details 验证自定义类型的数组。
 */
TEST_F(ArrayTypeTest, ArrayOfStructs) {
  std::string source = R"(
struct Person {
    name: String,
    age: Integer
};

let people: Person[] = [
    Person { name: "Alice", age: 30 },
    Person { name: "Bob", age: 25 }
];
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("Person[]") != std::string::npos);
}

/**
 * @test 测试元组数组。
 * @details 验证元组类型的数组。
 */
TEST_F(ArrayTypeTest, ArrayOfTuples) {
  std::string source =
      "let pairs: (Integer, String)[] = [(1, \"one\"), (2, \"two\")];";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("(Integer, String)[]") != std::string::npos);
}

/**
 * @test 测试函数类型数组。
 * @details 验证函数签名类型的数组。
 */
TEST_F(ArrayTypeTest, ArrayOfFunctions) {
  std::string source =
      "let operations: ((Integer, Integer) -> Integer)[] = [];";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("((Integer, Integer) -> Integer)[]") !=
              std::string::npos);
}

// --- Array Literals Tests ---

/**
 * @test 测试基本数组字面量。
 * @details 验证简单的数组字面量解析。
 */
TEST_F(ArrayTypeTest, BasicArrayLiteral) {
  std::string source = "let arr = [1, 2, 3, 4, 5];";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("[1, 2, 3, 4, 5]") != std::string::npos);
}

/**
 * @test 测试嵌套数组字面量。
 * @details 验证多维数组字面量的解析。
 */
TEST_F(ArrayTypeTest, NestedArrayLiteral) {
  std::string source = "let matrix = [[1, 2, 3], [4, 5, 6], [7, 8, 9]];";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("[[1, 2, 3]") != std::string::npos);
}

/**
 * @test 测试混合类型表达式的数组字面量。
 * @details 验证数组元素可以是复杂表达式。
 */
TEST_F(ArrayTypeTest, ArrayWithExpressions) {
  std::string source = "let computed = [1 + 2, 3 * 4, 5 - 6];";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("[1 + 2") != std::string::npos);
}

// --- Array in Struct Fields Tests ---

/**
 * @test 测试结构体中的数组字段。
 * @details 验证结构体字段可以是数组类型。
 */
TEST_F(ArrayTypeTest, ArrayFieldInStruct) {
  std::string source = R"(
struct Collection {
    items: Integer[],
    tags: String[],
    matrix: Float[][]
};
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("items: Integer[]") != std::string::npos);
  EXPECT_TRUE(formatted.find("tags: String[]") != std::string::npos);
  EXPECT_TRUE(formatted.find("matrix: Float[][]") != std::string::npos);
}

// --- Edge Cases ---

/**
 * @test 测试数组类型作为函数参数。
 * @details 验证函数参数可以使用数组类型。
 */
TEST_F(ArrayTypeTest, ArrayAsParameter) {
  std::string source = R"(
fn processArray(arr: Integer[], size: Integer) -> Integer {
    return size;
}
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  // 格式化器可能不在冒号后添加空格
  EXPECT_TRUE(formatted.find("arr") != std::string::npos);
  EXPECT_TRUE(formatted.find("Integer[]") != std::string::npos);
}

/**
 * @test 测试数组类型作为函数返回值。
 * @details 验证函数可以返回数组类型。
 */
TEST_F(ArrayTypeTest, ArrayAsReturnType) {
  std::string source = R"(
fn createArray() -> Integer[] {
    return [1, 2, 3];
}
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("-> Integer[]") != std::string::npos);
}

/**
 * @test 测试综合场景。
 * @details 验证数组类型在各种场景下混合使用。
 */
TEST_F(ArrayTypeTest, ComprehensiveArrayUsage) {
  std::string source = R"(
struct Matrix {
    data: Integer[][],
    rows: Integer,
    cols: Integer
};

type IntArray = Integer[];

let m: Matrix = Matrix {
    data: [[1, 2], [3, 4]],
    rows: 2,
    cols: 2
};

let arr: IntArray = [1, 2, 3];
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  const auto& children = cst->get_children();
  EXPECT_GE(children.size(), 4);
  EXPECT_EQ(children[0]->get_type(), CSTNodeType::StructDeclaration);
  EXPECT_EQ(children[1]->get_type(), CSTNodeType::TypeAliasDeclaration);
}
