/**
 * @file test_token_preprocessor.cpp
 * @brief Token 预处理器测试套件
 * @author BegoniaHe
 */

#include "czc/token_preprocessor/token_preprocessor.hpp"
#include "czc/lexer/lexer.hpp"
#include <iostream>
#include <cassert>
#include <vector>

/**
 * @brief 创建测试上下文
 * @return 分析上下文对象
 */
static AnalysisContext make_test_context()
{
    static std::string empty_filename = "";
    static std::string empty_source = "";
    return AnalysisContext(empty_filename, empty_source, nullptr);
}

/**
 * @brief 打印分析结果
 * @param literal 字面量字符串
 * @param info 科学计数法信息
 */
void print_analysis(const std::string &literal, const ScientificNotationInfo &info)
{
    std::cout << "Literal: " << literal << std::endl;
    std::cout << "  Mantissa: " << info.mantissa << std::endl;
    std::cout << "  Exponent: " << info.exponent << std::endl;
    std::cout << "  Has decimal: " << (info.has_decimal_point ? "yes" : "no") << std::endl;
    std::cout << "  Decimal digits: " << info.decimal_digits << std::endl;
    std::cout << "  Inferred type: " << inferred_type_to_string(info.inferred_type) << std::endl;
    std::cout << "  Normalized: " << info.normalized_value << std::endl;
}

/**
 * @brief 测试负指数推断为 FLOAT
 */
void test_negative_exponent()
{
    std::cout << "\n=== Test: Negative Exponent ===" << std::endl;

    auto info1 = ScientificNotationAnalyzer::analyze("1e-10", nullptr, make_test_context());
    assert(info1.has_value());
    assert(info1->inferred_type == InferredNumericType::FLOAT);
    std::cout << "1e-10 -> FLOAT" << std::endl;

    auto info2 = ScientificNotationAnalyzer::analyze("3.14e-5", nullptr, make_test_context());
    assert(info2.has_value());
    assert(info2->inferred_type == InferredNumericType::FLOAT);
    std::cout << "3.14e-5 -> FLOAT" << std::endl;

    auto info3 = ScientificNotationAnalyzer::analyze("123e-1", nullptr, make_test_context());
    assert(info3.has_value());
    assert(info3->inferred_type == InferredNumericType::FLOAT);
    std::cout << "123e-1 -> FLOAT" << std::endl;
}

/**
 * @brief 测试无小数点的整数形式
 */
void test_integer_form()
{
    std::cout << "\n=== Test: Integer Form (No Decimal Point) ===" << std::endl;

    // 小指数，应该是 INT64
    auto info1 = ScientificNotationAnalyzer::analyze("1e10", nullptr, make_test_context());
    assert(info1.has_value());
    assert(info1->inferred_type == InferredNumericType::INT64);
    std::cout << "1e10 -> INT64" << std::endl;

    auto info2 = ScientificNotationAnalyzer::analyze("123e5", nullptr, make_test_context());
    assert(info2.has_value());
    assert(info2->inferred_type == InferredNumericType::INT64);
    std::cout << "123e5 -> INT64" << std::endl;

    // 大指数，应该溢出为 FLOAT
    auto info3 = ScientificNotationAnalyzer::analyze("1e100", nullptr, make_test_context());
    assert(info3.has_value());
    assert(info3->inferred_type == InferredNumericType::FLOAT);
    std::cout << "1e100 -> FLOAT (overflow)" << std::endl;

    // 边界情况：接近 INT64 最大值
    auto info4 = ScientificNotationAnalyzer::analyze("9e18", nullptr, make_test_context());
    assert(info4.has_value());
    assert(info4->inferred_type == InferredNumericType::INT64);
    std::cout << "9e18 -> INT64 (near limit)" << std::endl;

    auto info5 = ScientificNotationAnalyzer::analyze("1e19", nullptr, make_test_context());
    assert(info5.has_value());
    assert(info5->inferred_type == InferredNumericType::FLOAT);
    std::cout << "1e19 -> FLOAT (overflow)" << std::endl;
}

/**
 * @brief 测试有小数点的情况 - 小数位数 > 指数
 */
void test_decimal_greater_than_exponent()
{
    std::cout << "\n=== Test: Decimal Digits > Exponent ===" << std::endl;

    // 3.14e5: 小数位数=2, 指数=5, 2 < 5, 但有小数位，检查是否能转为整数
    auto info1 = ScientificNotationAnalyzer::analyze("3.14e5", nullptr, make_test_context());
    assert(info1.has_value());
    // 3.14e5 = 314000 (可以转为整数)
    assert(info1->inferred_type == InferredNumericType::INT64);
    std::cout << "3.14e5 -> INT64" << std::endl;

    // 3.14159e2: 小数位数=5, 指数=2, 5 > 2 -> FLOAT
    auto info2 = ScientificNotationAnalyzer::analyze("3.14159e2", nullptr, make_test_context());
    assert(info2.has_value());
    assert(info2->inferred_type == InferredNumericType::FLOAT);
    std::cout << "3.14159e2 -> FLOAT (decimal_digits > exponent)" << std::endl;

    // 1.5e1: 小数位数=1, 指数=1, 1 = 1, 可以转为整数 15
    auto info3 = ScientificNotationAnalyzer::analyze("1.5e1", nullptr, make_test_context());
    assert(info3.has_value());
    assert(info3->inferred_type == InferredNumericType::INT64);
    std::cout << "1.5e1 -> INT64" << std::endl;
}

/**
 * @brief 测试尾随零的处理
 */
void test_trailing_zeros()
{
    std::cout << "\n=== Test: Trailing Zeros ===" << std::endl;

    // 1.500e3: 去除尾随零后小数位数=1, 指数=3, 1 < 3 -> INT64
    auto info1 = ScientificNotationAnalyzer::analyze("1.500e3", nullptr, make_test_context());
    assert(info1.has_value());
    assert(info1->decimal_digits == 1); // "1.5"
    assert(info1->inferred_type == InferredNumericType::INT64);
    std::cout << "1.500e3 -> INT64 (trailing zeros removed)" << std::endl;

    // 2.0000e2: 去除尾随零后小数位数=0, 指数=2 -> INT64
    auto info2 = ScientificNotationAnalyzer::analyze("2.0000e2", nullptr, make_test_context());
    assert(info2.has_value());
    assert(info2->decimal_digits == 0);
    assert(info2->inferred_type == InferredNumericType::INT64);
    std::cout << "2.0000e2 -> INT64 (all zeros removed)" << std::endl;

    // 1.234000e10: 去除尾随零后小数位数=3, 指数=10, 3 < 10 -> INT64
    auto info3 = ScientificNotationAnalyzer::analyze("1.234000e10", nullptr, make_test_context());
    assert(info3.has_value());
    assert(info3->decimal_digits == 3);
    assert(info3->inferred_type == InferredNumericType::INT64);
    std::cout << "1.234000e10 -> INT64" << std::endl;
}

/**
 * @brief 测试边界情况
 */
void test_edge_cases()
{
    std::cout << "\n=== Test: Edge Cases ===" << std::endl;

    // 零指数
    auto info1 = ScientificNotationAnalyzer::analyze("5e0", nullptr, make_test_context());
    assert(info1.has_value());
    assert(info1->inferred_type == InferredNumericType::INT64);
    std::cout << "5e0 -> INT64" << std::endl;

    // 小数点但无小数位
    auto info2 = ScientificNotationAnalyzer::analyze("5.e2", nullptr, make_test_context());
    assert(info2.has_value());
    assert(info2->decimal_digits == 0);
    assert(info2->inferred_type == InferredNumericType::INT64);
    std::cout << "5.e2 -> INT64" << std::endl;

    // 大写 E
    auto info3 = ScientificNotationAnalyzer::analyze("1.23E4", nullptr, make_test_context());
    assert(info3.has_value());
    assert(info3->inferred_type == InferredNumericType::INT64);
    std::cout << "1.23E4 -> INT64" << std::endl;

    // 正号指数
    auto info4 = ScientificNotationAnalyzer::analyze("2.5e+3", nullptr, make_test_context());
    assert(info4.has_value());
    assert(info4->inferred_type == InferredNumericType::INT64);
    std::cout << "2.5e+3 -> INT64" << std::endl;
}

/**
 * @brief 测试 Token 流处理
 */
void test_token_processing()
{
    std::cout << "\n=== Test: Token Stream Processing ===" << std::endl;

    // 创建测试代码
    std::string code = "let a = 1e10; let b = 3.14e-5; let c = 1.5e2;";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();

    // 处理 Token 流
    auto processed = TokenPreprocessor::process(tokens, "<test>", code, nullptr);

    // 验证处理结果
    for (const auto &token : processed)
    {
        if (token.value == "1e10")
        {
            assert(token.token_type == TokenType::Integer);
            std::cout << "1e10 processed as Integer" << std::endl;
        }
        else if (token.value == "3.14e-5")
        {
            assert(token.token_type == TokenType::Float);
            std::cout << "3.14e-5 processed as Float" << std::endl;
        }
        else if (token.value == "1.5e2")
        {
            assert(token.token_type == TokenType::Integer);
            std::cout << "1.5e2 processed as Integer" << std::endl;
        }
    }
}

/**
 * @brief 测试详细分析输出
 */
void test_detailed_analysis()
{
    std::cout << "\n=== Test: Detailed Analysis Output ===" << std::endl;

    std::vector<std::string> test_cases = {
        "1e10",
        "3.14e-5",
        "1.5e2",
        "2.718e0",
        "9.999e18",
        "1.23456789e10"};

    for (const auto &literal : test_cases)
    {
        auto info = ScientificNotationAnalyzer::analyze(literal, nullptr, make_test_context());
        if (info.has_value())
        {
            std::cout << "\n";
            print_analysis(literal, info.value());
        }
    }
}

/**
 * @brief 测试实际数值验证
 */
void test_actual_values()
{
    std::cout << "\n=== Test: Actual Value Validation ===" << std::endl;

    // 1e10 = 10,000,000,000 (可以用 INT64 表示)
    auto info1 = ScientificNotationAnalyzer::analyze("1e10", nullptr, make_test_context());
    assert(info1.has_value());
    assert(info1->inferred_type == InferredNumericType::INT64);
    std::cout << "1e10 = 10,000,000,000 (INT64)" << std::endl;

    // 1.5e2 = 150 (可以用 INT64 表示)
    auto info2 = ScientificNotationAnalyzer::analyze("1.5e2", nullptr, make_test_context());
    assert(info2.has_value());
    assert(info2->inferred_type == InferredNumericType::INT64);
    std::cout << "1.5e2 = 150 (INT64)" << std::endl;

    // 3.14159e2 = 314.159 (不是整数，需要 FLOAT)
    auto info3 = ScientificNotationAnalyzer::analyze("3.14159e2", nullptr, make_test_context());
    assert(info3.has_value());
    assert(info3->inferred_type == InferredNumericType::FLOAT);
    std::cout << "3.14159e2 = 314.159 (FLOAT)" << std::endl;

    // 2.5e-3 = 0.0025 (FLOAT)
    auto info4 = ScientificNotationAnalyzer::analyze("2.5e-3", nullptr, make_test_context());
    assert(info4.has_value());
    assert(info4->inferred_type == InferredNumericType::FLOAT);
    std::cout << "2.5e-3 = 0.0025 (FLOAT)" << std::endl;
}

/**
 * @brief 主函数入口
 * @return 程序退出码
 */
int main()
{
    std::cout << "===================================" << std::endl;
    std::cout << "Token Preprocessor Tests" << std::endl;
    std::cout << "===================================" << std::endl;

    try
    {
        test_negative_exponent();
        test_integer_form();
        test_decimal_greater_than_exponent();
        test_trailing_zeros();
        test_edge_cases();
        test_token_processing();
        test_actual_values();
        test_detailed_analysis();

        std::cout << "\n===================================" << std::endl;
        std::cout << "All tests passed! ✓" << std::endl;
        std::cout << "===================================" << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "\n===================================" << std::endl;
        std::cerr << "Test failed: " << e.what() << std::endl;
        std::cerr << "===================================" << std::endl;
        return 1;
    }

    return 0;
}
