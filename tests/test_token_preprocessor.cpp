/**
 * @file test_token_preprocessor.cpp
 * @brief Token 预处理器测试套件（使用 Google Test 框架）。
 * @details 本测试套件专门测试科学记数法分析器的类型推断逻辑，
 *          验证其能够根据小数位数、指数大小等因素正确判断数值类型
 *          应该是整数（INT64）还是浮点数（FLOAT）。
 * @author BegoniaHe
 * @date 2025-11-11
 */

#include "czc/lexer/lexer.hpp"
#include "czc/token_preprocessor/token_preprocessor.hpp"

#include <vector>

#include <gtest/gtest.h>

using namespace czc::lexer;
using namespace czc::token_preprocessor;

// --- 测试夹具 ---

/**
 * @brief Token 预处理器测试夹具。
 * @details 提供辅助方法用于创建测试所需的分析上下文。
 */
class TokenPreprocessorTest : public ::testing::Test {
protected:
  /**
   * @brief 创建一个空的分析上下文用于测试。
   * @return 返回一个初始化的 AnalysisContext 对象。
   */
  static AnalysisContext make_test_context() {
    static std::string empty_filename = "";
    static std::string empty_source = "";
    return AnalysisContext(empty_filename, empty_source, nullptr);
  }
};

// --- 负指数测试 ---

/**
 * @brief 测试负指数的科学记数法。
 * @details 负指数总是表示小于 1 的小数，因此类型应推断为 FLOAT。
 */
TEST_F(TokenPreprocessorTest, NegativeExponent) {
  // 1e-10 = 0.0000000001，必须是浮点数
  auto info1 = ScientificNotationAnalyzer::analyze("1e-10", nullptr,
                                                   make_test_context());
  ASSERT_TRUE(info1.has_value());
  EXPECT_EQ(info1->inferred_type, InferredNumericType::FLOAT);

  // 3.14e-5 = 0.0000314，必须是浮点数
  auto info2 = ScientificNotationAnalyzer::analyze("3.14e-5", nullptr,
                                                   make_test_context());
  ASSERT_TRUE(info2.has_value());
  EXPECT_EQ(info2->inferred_type, InferredNumericType::FLOAT);
}

// --- 整数形式测试 ---

/**
 * @brief 测试小指数的科学记数法（整数形式）。
 * @details 当指数较小且结果可以用 INT64 表示时，类型应推断为 INT64。
 *          例如 1e10 = 10,000,000,000，在 INT64 范围内。
 */
TEST_F(TokenPreprocessorTest, IntegerFormSmallExponent) {
  auto info =
      ScientificNotationAnalyzer::analyze("1e10", nullptr, make_test_context());
  ASSERT_TRUE(info.has_value());
  EXPECT_EQ(info->inferred_type, InferredNumericType::INT64);
}

/**
 * @brief 测试大指数的科学记数法（溢出到浮点数）。
 * @details 当指数过大导致结果超出 INT64 范围时，类型应推断为 FLOAT。
 *          例如 1e100 远超 INT64 最大值（约 9e18）。
 */
TEST_F(TokenPreprocessorTest, IntegerFormLargeExponent) {
  auto info = ScientificNotationAnalyzer::analyze("1e100", nullptr,
                                                  make_test_context());
  ASSERT_TRUE(info.has_value());
  EXPECT_EQ(info->inferred_type, InferredNumericType::FLOAT);
}

// --- 小数位数大于指数的测试 ---

/**
 * @brief 测试小数位数大于指数的情况。
 * @details 当小数部分的位数大于指数时，结果必然包含小数部分，
 *          因此类型应推断为 FLOAT。
 *          例如 3.14159e2：小数部分 5 位 > 指数 2，结果为 314.159。
 */
TEST_F(TokenPreprocessorTest, DecimalGreaterThanExponent) {
  auto info = ScientificNotationAnalyzer::analyze("3.14159e2", nullptr,
                                                  make_test_context());
  ASSERT_TRUE(info.has_value());
  EXPECT_EQ(info->inferred_type, InferredNumericType::FLOAT);
}

// --- 尾随零测试 ---

/**
 * @brief 测试尾随零的处理（保留 1 位有效小数）。
 * @details 尾随零会被忽略。例如 1.500e3 的有效小数位数为 1，
 *          1 < 3，因此类型应推断为 INT64。
 */
TEST_F(TokenPreprocessorTest, TrailingZerosOneDecimal) {
  auto info = ScientificNotationAnalyzer::analyze("1.500e3", nullptr,
                                                  make_test_context());
  ASSERT_TRUE(info.has_value());
  EXPECT_EQ(info->decimal_digits, 1);
  EXPECT_EQ(info->inferred_type, InferredNumericType::INT64);
}

/**
 * @brief 测试尾随零的处理（小数部分全为零）。
 * @details 当小数部分全为零时，有效小数位数为 0，
 *          结果可表示为整数，类型应推断为 INT64。
 *          例如 2.0000e2 = 200。
 */
TEST_F(TokenPreprocessorTest, TrailingZerosNoDecimal) {
  auto info = ScientificNotationAnalyzer::analyze("2.0000e2", nullptr,
                                                  make_test_context());
  ASSERT_TRUE(info.has_value());
  EXPECT_EQ(info->decimal_digits, 0);
  EXPECT_EQ(info->inferred_type, InferredNumericType::INT64);
}

// --- 边界情况测试 ---

/**
 * @brief 测试零指数的科学记数法。
 * @details 零指数表示数值本身不变。例如 5e0 = 5，
 *          类型应推断为 INT64。
 */
TEST_F(TokenPreprocessorTest, ZeroExponent) {
  auto info =
      ScientificNotationAnalyzer::analyze("5e0", nullptr, make_test_context());
  ASSERT_TRUE(info.has_value());
  EXPECT_EQ(info->inferred_type, InferredNumericType::INT64);
}

/**
 * @brief 测试小数点后无数字的情况。
 * @details 例如 5.e2 = 500，小数部分为空，
 *          有效小数位数为 0，类型应推断为 INT64。
 */
TEST_F(TokenPreprocessorTest, DecimalPointNoFraction) {
  auto info =
      ScientificNotationAnalyzer::analyze("5.e2", nullptr, make_test_context());
  ASSERT_TRUE(info.has_value());
  EXPECT_EQ(info->decimal_digits, 0);
  EXPECT_EQ(info->inferred_type, InferredNumericType::INT64);
}

/**
 * @brief 测试大写 E 的科学记数法。
 * @details 验证分析器支持大小写 e/E。例如 1.23E4 = 12300。
 */
TEST_F(TokenPreprocessorTest, UppercaseE) {
  auto info = ScientificNotationAnalyzer::analyze("1.23E4", nullptr,
                                                  make_test_context());
  ASSERT_TRUE(info.has_value());
  EXPECT_EQ(info->inferred_type, InferredNumericType::INT64);
}

/**
 * @brief 测试显式正指数的科学记数法。
 * @details 验证分析器支持 e+n 格式。例如 2.5e+3 = 2500。
 */
TEST_F(TokenPreprocessorTest, ExplicitPositiveExponent) {
  auto info = ScientificNotationAnalyzer::analyze("2.5e+3", nullptr,
                                                  make_test_context());
  ASSERT_TRUE(info.has_value());
  EXPECT_EQ(info->inferred_type, InferredNumericType::INT64);
}

// --- Token 流处理测试 ---

/**
 * @brief 测试完整 Token 流的处理。
 * @details 验证预处理器能够正确处理包含多个科学记数法字面量的 Token 流，
 *          并根据分析结果修改 Token 类型（Integer 或 Float）。
 */
TEST_F(TokenPreprocessorTest, TokenStreamProcessing) {
  std::string code = "let a = 1e10; let b = 3.14e-5; let c = 1.5e2;";
  Lexer lexer(code);
  auto tokens = lexer.tokenize();

  TokenPreprocessor preprocessor;
  auto processed = preprocessor.process(tokens, "<test>", code);

  bool found_1e10 = false;
  bool found_314e5 = false;
  bool found_15e2 = false;

  for (const auto& token : processed) {
    if (token.value == "1e10") {
      // 1e10 应被推断为整数
      EXPECT_EQ(token.token_type, TokenType::Integer);
      found_1e10 = true;
    } else if (token.value == "3.14e-5") {
      // 3.14e-5 应被推断为浮点数
      EXPECT_EQ(token.token_type, TokenType::Float);
      found_314e5 = true;
    } else if (token.value == "1.5e2") {
      // 1.5e2 = 150，应被推断为整数
      EXPECT_EQ(token.token_type, TokenType::Integer);
      found_15e2 = true;
    }
  }

  // 确保所有目标 Token 都被找到并处理
  EXPECT_TRUE(found_1e10);
  EXPECT_TRUE(found_314e5);
  EXPECT_TRUE(found_15e2);
}

// --- 实际数值验证测试 ---

/**
 * @brief 验证 1e10 的实际数值和类型推断。
 * @details 1e10 = 10,000,000,000，在 INT64 范围内，
 *          类型应推断为 INT64。
 */
TEST_F(TokenPreprocessorTest, ActualValue1e10) {
  auto info =
      ScientificNotationAnalyzer::analyze("1e10", nullptr, make_test_context());
  ASSERT_TRUE(info.has_value());
  EXPECT_EQ(info->inferred_type, InferredNumericType::INT64);
}

/**
 * @brief 验证 1.5e2 的实际数值和类型推断。
 * @details 1.5e2 = 150，可精确表示为整数，
 *          类型应推断为 INT64。
 */
TEST_F(TokenPreprocessorTest, ActualValue1Point5e2) {
  auto info = ScientificNotationAnalyzer::analyze("1.5e2", nullptr,
                                                  make_test_context());
  ASSERT_TRUE(info.has_value());
  EXPECT_EQ(info->inferred_type, InferredNumericType::INT64);
}

/**
 * @brief 验证 3.14159e2 的实际数值和类型推断。
 * @details 3.14159e2 = 314.159，包含小数部分，
 *          类型应推断为 FLOAT。
 */
TEST_F(TokenPreprocessorTest, ActualValue3Point14159e2) {
  auto info = ScientificNotationAnalyzer::analyze("3.14159e2", nullptr,
                                                  make_test_context());
  ASSERT_TRUE(info.has_value());
  EXPECT_EQ(info->inferred_type, InferredNumericType::FLOAT);
}

// --- 新增测试：极端情况和特殊场景 ---

/**
 * @brief 测试非常大的指数（接近浮点数上限）。
 * @details 验证分析器对接近 IEEE 754 双精度浮点数极限的指数的处理。
 */
TEST_F(TokenPreprocessorTest, VeryLargeExponent) {
  // 1e308 接近 double 的最大值
  auto info = ScientificNotationAnalyzer::analyze("1e308", nullptr,
                                                  make_test_context());
  ASSERT_TRUE(info.has_value());
  EXPECT_EQ(info->inferred_type, InferredNumericType::FLOAT);
}

/**
 * @brief 测试非常小的负指数（接近浮点数下限）。
 * @details 验证分析器对接近 IEEE 754 双精度浮点数最小正值的指数的处理。
 */
TEST_F(TokenPreprocessorTest, VerySmallNegativeExponent) {
  // 1e-308 接近 double 的最小正值
  auto info = ScientificNotationAnalyzer::analyze("1e-308", nullptr,
                                                  make_test_context());
  ASSERT_TRUE(info.has_value());
  EXPECT_EQ(info->inferred_type, InferredNumericType::FLOAT);
}

/**
 * @brief 测试整数边界附近的科学记数法。
 * @details 验证分析器对接近 INT64 最大值的科学记数法的处理。
 *          INT64 最大值约为 9.22e18。
 */
TEST_F(TokenPreprocessorTest, NearInt64Boundary) {
  // 9e18 应在 INT64 范围内
  auto info1 =
      ScientificNotationAnalyzer::analyze("9e18", nullptr, make_test_context());
  ASSERT_TRUE(info1.has_value());
  EXPECT_EQ(info1->inferred_type, InferredNumericType::INT64);

  // 1e19 超出 INT64 范围，应推断为浮点数
  auto info2 =
      ScientificNotationAnalyzer::analyze("1e19", nullptr, make_test_context());
  ASSERT_TRUE(info2.has_value());
  EXPECT_EQ(info2->inferred_type, InferredNumericType::FLOAT);
}

/**
 * @brief 测试多位尾随零的处理。
 * @details 验证分析器能够正确去除尾随零并计算有效小数位数。
 */
TEST_F(TokenPreprocessorTest, MultipleTrailingZeros) {
  // 1.2340000e4 的有效小数位数为 3（去除尾随零后为 1.234）
  auto info = ScientificNotationAnalyzer::analyze("1.2340000e4", nullptr,
                                                  make_test_context());
  ASSERT_TRUE(info.has_value());
  // 3 < 4，因此应推断为 INT64
  EXPECT_EQ(info->inferred_type, InferredNumericType::INT64);
}

/**
 * @brief 测试前导零加小数的处理。
 * @details 验证分析器对类似 0.001e3 这样的数值的处理。
 */
TEST_F(TokenPreprocessorTest, LeadingZeroWithDecimal) {
  // 0.001e3 = 1，应推断为 INT64
  auto info = ScientificNotationAnalyzer::analyze("0.001e3", nullptr,
                                                  make_test_context());
  ASSERT_TRUE(info.has_value());
  EXPECT_EQ(info->inferred_type, InferredNumericType::INT64);

  // 0.001e2 = 0.1，应推断为 FLOAT
  auto info2 = ScientificNotationAnalyzer::analyze("0.001e2", nullptr,
                                                   make_test_context());
  ASSERT_TRUE(info2.has_value());
  EXPECT_EQ(info2->inferred_type, InferredNumericType::FLOAT);
}

// --- 新增测试：错误处理和边界情况 ---

/**
 * @brief 测试无效科学记数法格式的处理。
 * @details 验证分析器能够正确拒绝格式错误的输入。
 */
TEST_F(TokenPreprocessorTest, InvalidScientificFormat) {
  // 缺少指数部分
  auto info1 =
      ScientificNotationAnalyzer::analyze("1.5e", nullptr, make_test_context());
  EXPECT_FALSE(info1.has_value());

  // 缺少尾数部分
  auto info2 =
      ScientificNotationAnalyzer::analyze("e10", nullptr, make_test_context());
  EXPECT_FALSE(info2.has_value());

  // 没有 e 或 E
  auto info3 = ScientificNotationAnalyzer::analyze("123.456", nullptr,
                                                   make_test_context());
  EXPECT_FALSE(info3.has_value());
}

/**
 * @brief 测试指数部分包含非法字符的处理。
 * @details 验证分析器能够检测指数部分的非法格式。
 */
TEST_F(TokenPreprocessorTest, InvalidExponentFormat) {
  // 指数包含字母
  auto info1 = ScientificNotationAnalyzer::analyze("1.5eabc", nullptr,
                                                   make_test_context());
  EXPECT_FALSE(info1.has_value());

  // 指数包含特殊字符
  auto info2 = ScientificNotationAnalyzer::analyze("1.5e@10", nullptr,
                                                   make_test_context());
  EXPECT_FALSE(info2.has_value());
}

/**
 * @brief 测试 TokenPreprocessor 的完整处理流程。
 * @details 验证预处理器能够正确处理包含多种 Token 类型的完整流。
 */
TEST_F(TokenPreprocessorTest, FullProcessingPipeline) {
  std::string code = "let x = 1e5; let y = 2.5e-3; let z = 3.14;";
  Lexer lexer(code);
  auto tokens = lexer.tokenize();

  TokenPreprocessor preprocessor;
  auto processed = preprocessor.process(tokens, "<test>", code);

  // 验证错误收集器没有错误
  EXPECT_FALSE(preprocessor.get_errors().has_errors());

  // 验证 Token 流长度合理
  EXPECT_GT(processed.size(), 0);
}

/**
 * @brief 测试空 Token 流的处理。
 * @details 验证预处理器能够安全处理空输入。
 */
TEST_F(TokenPreprocessorTest, EmptyTokenStream) {
  std::vector<Token> empty_tokens;

  TokenPreprocessor preprocessor;
  auto processed = preprocessor.process(empty_tokens, "<test>", "");

  EXPECT_EQ(processed.size(), 0);
  EXPECT_FALSE(preprocessor.get_errors().has_errors());
}

/**
 * @brief 测试不包含科学记数法的 Token 流。
 * @details 验证预处理器对非科学记数法 Token 的透传。
 */
TEST_F(TokenPreprocessorTest, NoScientificNotation) {
  std::string code = "let x = 123; let y = 456.789;";
  Lexer lexer(code);
  auto tokens = lexer.tokenize();

  TokenPreprocessor preprocessor;
  auto processed = preprocessor.process(tokens, "<test>", code);

  // Token 数量应该不变
  EXPECT_EQ(processed.size(), tokens.size());
  EXPECT_FALSE(preprocessor.get_errors().has_errors());
}

/**
 * @brief 测试混合类型的 Token 流处理。
 * @details 验证预处理器能够在复杂的 Token 流中正确识别和处理科学记数法。
 */
TEST_F(TokenPreprocessorTest, MixedTokenTypes) {
  std::string code = "fn calc() { return 1.5e2 + 100 - 2.0e-1; }";
  Lexer lexer(code);
  auto tokens = lexer.tokenize();

  TokenPreprocessor preprocessor;
  auto processed = preprocessor.process(tokens, "<test>", code);

  // 查找并验证科学记数法 Token 被正确转换
  int integer_count = 0;
  int float_count = 0;

  for (const auto& token : processed) {
    if (token.token_type == TokenType::Integer) {
      integer_count++;
    } else if (token.token_type == TokenType::Float) {
      float_count++;
    }
  }

  // 应该至少有一些数字 Token 被处理
  EXPECT_GT(integer_count + float_count, 0);
}

/**
 * @brief 测试小数点位置的各种情况。
 * @details 验证分析器对小数点在不同位置的处理。
 */
TEST_F(TokenPreprocessorTest, DecimalPointPositions) {
  // 小数点在开头
  auto info1 =
      ScientificNotationAnalyzer::analyze(".5e2", nullptr, make_test_context());
  if (info1.has_value()) {
    EXPECT_TRUE(info1->has_decimal_point);
  }

  // 小数点在中间
  auto info2 = ScientificNotationAnalyzer::analyze("1.5e2", nullptr,
                                                   make_test_context());
  ASSERT_TRUE(info2.has_value());
  EXPECT_TRUE(info2->has_decimal_point);

  // 没有小数点
  auto info3 =
      ScientificNotationAnalyzer::analyze("15e2", nullptr, make_test_context());
  ASSERT_TRUE(info3.has_value());
  EXPECT_FALSE(info3->has_decimal_point);
}

/**
 * @brief 测试指数符号的各种形式。
 * @details 验证分析器支持 e 和 E 两种形式。
 */
TEST_F(TokenPreprocessorTest, ExponentNotationVariants) {
  // 小写 e
  auto info1 = ScientificNotationAnalyzer::analyze("1.5e10", nullptr,
                                                   make_test_context());
  ASSERT_TRUE(info1.has_value());
  EXPECT_EQ(info1->exponent, 10);

  // 大写 E
  auto info2 = ScientificNotationAnalyzer::analyze("1.5E10", nullptr,
                                                   make_test_context());
  ASSERT_TRUE(info2.has_value());
  EXPECT_EQ(info2->exponent, 10);

  // 显式正号
  auto info3 = ScientificNotationAnalyzer::analyze("1.5e+10", nullptr,
                                                   make_test_context());
  ASSERT_TRUE(info3.has_value());
  EXPECT_EQ(info3->exponent, 10);

  // 负号
  auto info4 = ScientificNotationAnalyzer::analyze("1.5e-10", nullptr,
                                                   make_test_context());
  ASSERT_TRUE(info4.has_value());
  EXPECT_EQ(info4->exponent, -10);
}

/**
 * @brief 测试 inferred_type_to_string 函数。
 * @details 验证类型到字符串的转换功能。
 */
TEST_F(TokenPreprocessorTest, TypeToStringConversion) {
  std::string int_str = inferred_type_to_string(InferredNumericType::INT64);
  std::string float_str = inferred_type_to_string(InferredNumericType::FLOAT);

  EXPECT_FALSE(int_str.empty());
  EXPECT_FALSE(float_str.empty());
  EXPECT_NE(int_str, float_str);
}

/**
 * @brief 测试规范化值的生成。
 * @details 验证分析器生成的规范化值格式正确。
 */
TEST_F(TokenPreprocessorTest, NormalizedValueGeneration) {
  auto info = ScientificNotationAnalyzer::analyze("1.5e10", nullptr,
                                                  make_test_context());
  ASSERT_TRUE(info.has_value());
  EXPECT_FALSE(info->normalized_value.empty());
  // 规范化值应该包含 'e'
  EXPECT_NE(info->normalized_value.find('e'), std::string::npos);
}

/**
 * @brief 测试原始字面量的保存。
 * @details 验证分析器保存了原始输入字符串。
 */
TEST_F(TokenPreprocessorTest, OriginalLiteralPreservation) {
  std::string original = "1.23e-45";
  auto info = ScientificNotationAnalyzer::analyze(original, nullptr,
                                                  make_test_context());
  ASSERT_TRUE(info.has_value());
  EXPECT_EQ(info->original_literal, original);
}

/**
 * @brief 测试尾数部分的提取。
 * @details 验证分析器正确提取尾数部分。
 */
TEST_F(TokenPreprocessorTest, MantissaExtraction) {
  auto info = ScientificNotationAnalyzer::analyze("12.34e5", nullptr,
                                                  make_test_context());
  ASSERT_TRUE(info.has_value());
  EXPECT_EQ(info->mantissa, "12.34");
  EXPECT_EQ(info->exponent, 5);
}