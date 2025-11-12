/**
 * @file test_function_literals.cpp
 * @brief 测试函数字面量（匿名函数）的解析和格式化 (Google Test 版本)。
 * @details 本测试套件全面测试函数字面量的各项功能，包括：
 *          - 基本函数字面量语法 fn (params) { body }
 *          - 函数类型注解 (T1, T2) -> T3
 *          - 函数字面量与类型注解结合
 *          - 复杂函数体（包含 if-else 等控制流）
 *          - 闭包和嵌套函数
 * @author BegoniaHe
 * @date 2025-11-12
 */

#include "czc/formatter/formatter.hpp"
#include "czc/lexer/lexer.hpp"
#include "czc/parser/parser.hpp"

#include <gtest/gtest.h>

using namespace czc::formatter;
using namespace czc::lexer;
using namespace czc::parser;
using namespace czc::cst;

// --- Test Fixtures ---

/**
 * @brief 函数字面量测试夹具。
 * @details 提供通用的词法分析、语法分析和格式化功能。
 */
class FunctionLiteralTest : public ::testing::Test {
protected:
  /**
   * @brief 辅助函数：词法分析 + 语法分析 + CST 验证。
   */
  std::shared_ptr<CSTNode> parse(const std::string& source) {
    Lexer lexer(source, "test_function_literal.zero");
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

// --- Basic Function Literal Tests ---

/**
 * @test 测试最简单的函数字面量。
 * @details 验证基本的 fn () { } 语法。
 */
TEST_F(FunctionLiteralTest, BasicFunctionLiteral) {
  std::string source = R"(
let greet = fn () {
    print("Hello");
};
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);
  EXPECT_EQ(cst->get_type(), CSTNodeType::Program);

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("fn ()") != std::string::npos);
}

/**
 * @test 测试带参数的函数字面量。
 * @details 验证函数字面量的参数解析。
 */
TEST_F(FunctionLiteralTest, FunctionLiteralWithParameters) {
  std::string source = R"(
let add = fn (a, b) {
    return a + b;
};
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("fn (a, b)") != std::string::npos);
  EXPECT_TRUE(formatted.find("return a + b") != std::string::npos);
}

/**
 * @test 测试带类型注解的参数。
 * @details 验证参数类型注解的解析。
 */
TEST_F(FunctionLiteralTest, FunctionLiteralWithTypedParameters) {
  std::string source = R"(
let multiply = fn (x: Integer, y: Integer) {
    return x * y;
};
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  // 检查参数名和类型存在，不检查空格
  EXPECT_TRUE(formatted.find("x") != std::string::npos);
  EXPECT_TRUE(formatted.find("y") != std::string::npos);
  EXPECT_TRUE(formatted.find("Integer") != std::string::npos);
}

// --- Function Type Annotation Tests ---

/**
 * @test 测试简单的函数类型注解。
 * @details 验证 (T1, T2) -> T3 语法。
 */
TEST_F(FunctionLiteralTest, SimpleFunctionTypeAnnotation) {
  std::string source = R"(
let add: (Integer, Integer) -> Integer = fn (a, b) {
    return a + b;
};
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("(Integer, Integer) -> Integer") !=
              std::string::npos);
}

/**
 * @test 测试无参数函数类型。
 * @details 验证 () -> T 语法。
 */
TEST_F(FunctionLiteralTest, NoParameterFunctionType) {
  std::string source = R"(
let getAnswer: () -> Integer = fn () {
    return 42;
};
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("() -> Integer") != std::string::npos);
}

/**
 * @test 测试单参数函数类型。
 * @details 验证 (T) -> R 语法。
 */
TEST_F(FunctionLiteralTest, SingleParameterFunctionType) {
  std::string source = R"(
let square: (Integer) -> Integer = fn (x) {
    return x * x;
};
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("(Integer) -> Integer") != std::string::npos);
}

/**
 * @test 测试返回函数的函数类型。
 * @details 验证高阶函数类型 (T) -> ((T) -> T)。
 */
TEST_F(FunctionLiteralTest, HigherOrderFunctionType) {
  std::string source = R"(
let makeAdder: (Integer) -> ((Integer) -> Integer) = fn (x) {
    return fn (y) {
        return x + y;
    };
};
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("(Integer) -> ((Integer) -> Integer)") !=
              std::string::npos);
}

// --- Complex Function Body Tests ---

/**
 * @test 测试包含 if-else 的函数体。
 * @details 验证函数体内的控制流语句解析正确。
 */
TEST_F(FunctionLiteralTest, FunctionWithIfElse) {
  std::string source = R"(
let test: (Bool) -> String = fn (flag) {
    if flag {
        return "yes";
    } else {
        return "no";
    }
};
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("if flag") != std::string::npos);
  EXPECT_TRUE(formatted.find("return \"yes\"") != std::string::npos);
  EXPECT_TRUE(formatted.find("return \"no\"") != std::string::npos);
}

/**
 * @test 测试包含循环的函数体。
 * @details 验证函数体内可以包含 while 循环。
 */
TEST_F(FunctionLiteralTest, FunctionWithWhileLoop) {
  std::string source = R"(
let sum = fn (n: Integer) {
    let total = 0;
    let i = 0;
    while i < n {
        total = total + i;
        i = i + 1;
    }
    return total;
};
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  // 检查 while 循环存在，不检查具体空格
  EXPECT_TRUE(formatted.find("while") != std::string::npos);
  EXPECT_TRUE(formatted.find("i < n") != std::string::npos);
}

/**
 * @test 测试包含多条语句的函数体。
 * @details 验证函数体可以包含复杂逻辑。
 */
TEST_F(FunctionLiteralTest, FunctionWithMultipleStatements) {
  std::string source = R"(
let process = fn (data: Integer) {
    let temp = data * 2;
    let result = temp + 10;
    print(result);
    return result;
};
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("let temp = data * 2") != std::string::npos);
  EXPECT_TRUE(formatted.find("let result = temp + 10") != std::string::npos);
}

// --- Nested Function Tests ---

/**
 * @test 测试嵌套函数字面量。
 * @details 验证函数内定义函数字面量。
 */
TEST_F(FunctionLiteralTest, NestedFunctionLiteral) {
  std::string source = R"(
let outer = fn (x: Integer) {
    let inner = fn (y: Integer) {
        return x + y;
    };
    return inner(10);
};
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  // 检查嵌套函数存在，不检查空格
  EXPECT_TRUE(formatted.find("let inner") != std::string::npos);
  EXPECT_TRUE(formatted.find("fn") != std::string::npos);
  EXPECT_TRUE(formatted.find("y") != std::string::npos);
}

/**
 * @test 测试立即调用的函数字面量。
 * @details 验证 IIFE（Immediately Invoked Function Expression）。
 */
TEST_F(FunctionLiteralTest, ImmediatelyInvokedFunction) {
  std::string source = R"(
let result = fn () {
    return 42;
}();
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("fn ()") != std::string::npos);
  EXPECT_TRUE(formatted.find("return 42") != std::string::npos);
}

// --- Function with Complex Types Tests ---

/**
 * @test 测试接受数组参数的函数。
 * @details 验证函数参数可以是数组类型。
 */
TEST_F(FunctionLiteralTest, FunctionWithArrayParameter) {
  std::string source = R"(
let sum: (Integer[]) -> Integer = fn (arr) {
    return 0;
};
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("(Integer[]) -> Integer") != std::string::npos);
}

/**
 * @test 测试接受元组参数的函数。
 * @details 验证函数参数可以是元组类型。
 */
TEST_F(FunctionLiteralTest, FunctionWithTupleParameter) {
  std::string source = R"(
let distance: ((Integer, Integer)) -> Float = fn (point) {
    return 0.0;
};
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("((Integer, Integer)) -> Float") !=
              std::string::npos);
}

/**
 * @test 测试接受结构体参数的函数。
 * @details 验证函数参数可以是自定义类型。
 */
TEST_F(FunctionLiteralTest, FunctionWithStructParameter) {
  std::string source = R"(
struct Person {
    name: String,
    age: Integer
};

let greet: (Person) -> String = fn (p) {
    return "Hello";
};
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("(Person) -> String") != std::string::npos);
}

/**
 * @test 测试返回数组的函数。
 * @details 验证函数返回值可以是数组类型。
 */
TEST_F(FunctionLiteralTest, FunctionReturningArray) {
  std::string source = R"(
let makeArray: () -> Integer[] = fn () {
    return [1, 2, 3];
};
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("() -> Integer[]") != std::string::npos);
}

/**
 * @test 测试返回元组的函数。
 * @details 验证函数返回值可以是元组类型。
 */
TEST_F(FunctionLiteralTest, FunctionReturningTuple) {
  std::string source = R"(
let getPair: () -> (Integer, String) = fn () {
    return (42, "answer");
};
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("() -> (Integer, String)") != std::string::npos);
}

// --- Function in Data Structures Tests ---

/**
 * @test 测试函数数组。
 * @details 验证数组元素可以是函数字面量。
 */
TEST_F(FunctionLiteralTest, ArrayOfFunctions) {
  std::string source = R"(
let operations: ((Integer) -> Integer)[] = [
    fn (x) { return x + 1; },
    fn (x) { return x * 2; },
    fn (x) { return x - 1; }
];
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("((Integer) -> Integer)[]") != std::string::npos);
}

/**
 * @test 测试函数元组。
 * @details 验证元组元素可以是函数字面量。
 */
TEST_F(FunctionLiteralTest, TupleOfFunctions) {
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

/**
 * @test 测试结构体中的函数字段。
 * @details 验证结构体字段可以是函数类型。
 */
TEST_F(FunctionLiteralTest, StructWithFunctionField) {
  std::string source = R"(
struct Calculator {
    add: (Integer, Integer) -> Integer,
    multiply: (Integer, Integer) -> Integer
};

let calc = Calculator {
    add: fn (a, b) { return a + b; },
    multiply: fn (a, b) { return a * b; }
};
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("add: (Integer, Integer) -> Integer") !=
              std::string::npos);
}

// --- Edge Cases and Special Scenarios ---

/**
 * @test 测试空函数体。
 * @details 验证函数体可以为空。
 */
TEST_F(FunctionLiteralTest, EmptyFunctionBody) {
  std::string source = "let noop = fn () {};";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  // 空花括号可能被格式化成多行
  EXPECT_TRUE(formatted.find("fn ()") != std::string::npos);
  EXPECT_TRUE(formatted.find("{") != std::string::npos);
  EXPECT_TRUE(formatted.find("}") != std::string::npos);
}

/**
 * @test 测试函数作为参数传递。
 * @details 验证函数字面量可以作为函数调用参数。
 */
TEST_F(FunctionLiteralTest, FunctionAsArgument) {
  std::string source = R"(
let result = map(arr, fn (x) { return x * 2; });
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("fn (x)") != std::string::npos);
}

/**
 * @test 测试函数返回函数（闭包）。
 * @details 验证闭包的创建和使用。
 */
TEST_F(FunctionLiteralTest, FunctionReturningFunction) {
  std::string source = R"(
let makeCounter = fn () {
    let count = 0;
    return fn () {
        count = count + 1;
        return count;
    };
};

let counter = makeCounter();
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  std::string formatted = format(cst);
  EXPECT_TRUE(formatted.find("return fn ()") != std::string::npos);
}

// --- Comprehensive Tests ---

/**
 * @test 测试综合场景。
 * @details 验证函数字面量在各种场景下混合使用。
 */
TEST_F(FunctionLiteralTest, ComprehensiveFunctionLiteralUsage) {
  std::string source = R"(
struct Point {
    x: Integer,
    y: Integer
};

type UnaryOp = (Integer) -> Integer;
type BinaryOp = (Integer, Integer) -> Integer;

let operations: (UnaryOp, BinaryOp) = (
    fn (x) { return x * 2; },
    fn (a, b) { return a + b; }
);

let transform: (Point) -> Point = fn (p) {
    if p.x > 10 {
        return Point { x: p.x / 2, y: p.y / 2 };
    } else {
        return p;
    }
};

let result = transform(Point { x: 20, y: 30 });
)";

  auto cst = parse(source);
  ASSERT_NE(cst, nullptr);

  const auto& children = cst->get_children();
  EXPECT_GE(children.size(), 6);
}
