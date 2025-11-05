/**
 * @file test_token_preprocessor.cpp
 * @brief Token 预处理器测试套件。
 * @author BegoniaHe
 * @date 2025-11-05
 */

#include "czc/lexer/lexer.hpp"
#include "czc/token_preprocessor/token_preprocessor.hpp"
#include <cassert>
#include <iostream>
#include <vector>

using namespace czc::lexer;
using namespace czc::token_preprocessor;

// Helper to create an empty analysis context for tests.
static AnalysisContext make_test_context() {
  static std::string empty_filename = "";
  static std::string empty_source = "";
  return AnalysisContext(empty_filename, empty_source, nullptr);
}

// Helper to print analysis results for debugging.
void print_analysis(const std::string &literal,
                    const ScientificNotationInfo &info) {
  std::cout << "Literal: " << literal << std::endl;
  std::cout << "  Mantissa: " << info.mantissa << std::endl;
  std::cout << "  Exponent: " << info.exponent << std::endl;
  std::cout << "  Has decimal: " << (info.has_decimal_point ? "yes" : "no")
            << std::endl;
  std::cout << "  Decimal digits: " << info.decimal_digits << std::endl;
  std::cout << "  Inferred type: "
            << inferred_type_to_string(info.inferred_type) << std::endl;
  std::cout << "  Normalized: " << info.normalized_value << std::endl;
}

// --- Test Cases ---

// Tests that negative exponents always result in FLOAT.
void test_negative_exponent() {
  std::cout << "\n=== Test: Negative Exponent ===" << std::endl;

  auto info1 = ScientificNotationAnalyzer::analyze("1e-10", nullptr,
                                                   make_test_context());
  assert(info1.has_value() &&
         info1->inferred_type == InferredNumericType::FLOAT);
  std::cout << "1e-10 -> FLOAT" << std::endl;

  auto info2 = ScientificNotationAnalyzer::analyze("3.14e-5", nullptr,
                                                   make_test_context());
  assert(info2.has_value() &&
         info2->inferred_type == InferredNumericType::FLOAT);
  std::cout << "3.14e-5 -> FLOAT" << std::endl;
}

// Tests literals without a decimal point.
void test_integer_form() {
  std::cout << "\n=== Test: Integer Form (No Decimal Point) ===" << std::endl;

  // Small exponent, should be INT64.
  auto info1 =
      ScientificNotationAnalyzer::analyze("1e10", nullptr, make_test_context());
  assert(info1.has_value() &&
         info1->inferred_type == InferredNumericType::INT64);
  std::cout << "1e10 -> INT64" << std::endl;

  // Large exponent, should overflow to FLOAT.
  auto info3 = ScientificNotationAnalyzer::analyze("1e100", nullptr,
                                                   make_test_context());
  assert(info3.has_value() &&
         info3->inferred_type == InferredNumericType::FLOAT);
  std::cout << "1e100 -> FLOAT (overflow)" << std::endl;
}

// Tests literals where the number of decimal digits is greater than the
// exponent.
void test_decimal_greater_than_exponent() {
  std::cout << "\n=== Test: Decimal Digits > Exponent ===" << std::endl;

  // 3.14159e2: 5 decimal digits > exponent 2 -> FLOAT
  auto info2 = ScientificNotationAnalyzer::analyze("3.14159e2", nullptr,
                                                   make_test_context());
  assert(info2.has_value() &&
         info2->inferred_type == InferredNumericType::FLOAT);
  std::cout << "3.14159e2 -> FLOAT" << std::endl;
}

// Tests that trailing zeros in the mantissa are handled correctly.
void test_trailing_zeros() {
  std::cout << "\n=== Test: Trailing Zeros ===" << std::endl;

  // 1.500e3: decimal_digits becomes 1, 1 < 3 -> INT64
  auto info1 = ScientificNotationAnalyzer::analyze("1.500e3", nullptr,
                                                   make_test_context());
  assert(info1.has_value() && info1->decimal_digits == 1 &&
         info1->inferred_type == InferredNumericType::INT64);
  std::cout << "1.500e3 -> INT64" << std::endl;

  // 2.0000e2: decimal_digits becomes 0 -> INT64
  auto info2 = ScientificNotationAnalyzer::analyze("2.0000e2", nullptr,
                                                   make_test_context());
  assert(info2.has_value() && info2->decimal_digits == 0 &&
         info2->inferred_type == InferredNumericType::INT64);
  std::cout << "2.0000e2 -> INT64" << std::endl;
}

// Tests various edge cases.
void test_edge_cases() {
  std::cout << "\n=== Test: Edge Cases ===" << std::endl;

  // Zero exponent.
  auto info1 =
      ScientificNotationAnalyzer::analyze("5e0", nullptr, make_test_context());
  assert(info1.has_value() &&
         info1->inferred_type == InferredNumericType::INT64);
  std::cout << "5e0 -> INT64" << std::endl;

  // Decimal point with no fractional part.
  auto info2 =
      ScientificNotationAnalyzer::analyze("5.e2", nullptr, make_test_context());
  assert(info2.has_value() && info2->decimal_digits == 0 &&
         info2->inferred_type == InferredNumericType::INT64);
  std::cout << "5.e2 -> INT64" << std::endl;

  // Uppercase 'E'.
  auto info3 = ScientificNotationAnalyzer::analyze("1.23E4", nullptr,
                                                   make_test_context());
  assert(info3.has_value() &&
         info3->inferred_type == InferredNumericType::INT64);
  std::cout << "1.23E4 -> INT64" << std::endl;

  // Explicit positive exponent.
  auto info4 = ScientificNotationAnalyzer::analyze("2.5e+3", nullptr,
                                                   make_test_context());
  assert(info4.has_value() &&
         info4->inferred_type == InferredNumericType::INT64);
  std::cout << "2.5e+3 -> INT64" << std::endl;
}

// Tests the full preprocessing pipeline on a stream of tokens.
void test_token_processing() {
  std::cout << "\n=== Test: Token Stream Processing ===" << std::endl;

  std::string code = "let a = 1e10; let b = 3.14e-5; let c = 1.5e2;";
  Lexer lexer(code);
  auto tokens = lexer.tokenize();

  TokenPreprocessor preprocessor;
  auto processed = preprocessor.process(tokens, "<test>", code);

  for (const auto &token : processed) {
    if (token.value == "1e10") {
      assert(token.token_type == TokenType::Integer);
      std::cout << "1e10 processed as Integer" << std::endl;
    } else if (token.value == "3.14e-5") {
      assert(token.token_type == TokenType::Float);
      std::cout << "3.14e-5 processed as Float" << std::endl;
    } else if (token.value == "1.5e2") {
      assert(token.token_type == TokenType::Integer);
      std::cout << "1.5e2 processed as Integer" << std::endl;
    }
  }
}

// Tests the detailed analysis output for manual verification.
void test_detailed_analysis() {
  std::cout << "\n=== Test: Detailed Analysis Output ===" << std::endl;

  std::vector<std::string> test_cases = {
      "1e10", "3.14e-5", "1.5e2", "2.718e0", "9.999e18", "1.23456789e10"};

  for (const auto &literal : test_cases) {
    auto info = ScientificNotationAnalyzer::analyze(literal, nullptr,
                                                    make_test_context());
    if (info.has_value()) {
      std::cout << "\n";
      print_analysis(literal, info.value());
    }
  }
}

// Tests specific values to ensure correct type inference.
void test_actual_values() {
  std::cout << "\n=== Test: Actual Value Validation ===" << std::endl;

  // 1e10 = 10,000,000,000 (fits in INT64)
  auto info1 =
      ScientificNotationAnalyzer::analyze("1e10", nullptr, make_test_context());
  assert(info1.has_value() &&
         info1->inferred_type == InferredNumericType::INT64);
  std::cout << "1e10 = 10,000,000,000 (INT64)" << std::endl;

  // 1.5e2 = 150 (fits in INT64)
  auto info2 = ScientificNotationAnalyzer::analyze("1.5e2", nullptr,
                                                   make_test_context());
  assert(info2.has_value() &&
         info2->inferred_type == InferredNumericType::INT64);
  std::cout << "1.5e2 = 150 (INT64)" << std::endl;

  // 3.14159e2 = 314.159 (is a float)
  auto info3 = ScientificNotationAnalyzer::analyze("3.14159e2", nullptr,
                                                   make_test_context());
  assert(info3.has_value() &&
         info3->inferred_type == InferredNumericType::FLOAT);
  std::cout << "3.14159e2 = 314.159 (FLOAT)" << std::endl;
}

/**
 * @brief Main entry point for the test suite.
 * @return 0 on success, 1 on failure.
 */
int main() {
  std::cout << "===================================" << std::endl;
  std::cout << "Token Preprocessor Tests" << std::endl;
  std::cout << "===================================" << std::endl;

  try {
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
  } catch (const std::exception &e) {
    std::cerr << "\n===================================" << std::endl;
    std::cerr << "Test failed: " << e.what() << std::endl;
    std::cerr << "===================================" << std::endl;
    return 1;
  }

  return 0;
}
