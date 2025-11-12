/**
 * @file benchmark_parser.cpp
 * @brief Parser performance benchmarks
 * @author BegoniaHe
 * @date 2025-11-11
 */

#include "czc/lexer/lexer.hpp"
#include "czc/parser/parser.hpp"
#include <benchmark/benchmark.h>
#include <sstream>

using namespace czc::lexer;
using namespace czc::parser;

// Helper function to generate source code
std::string generate_function_source(size_t num_functions) {
  std::ostringstream oss;
  for (size_t i = 0; i < num_functions; ++i) {
    oss << "fn func" << i << "() {\n";
    oss << "  let x = " << i << ";\n";
    oss << "  let y = x + 1;\n";
    oss << "  return y;\n";
    oss << "}\n\n";
  }
  return oss.str();
}

// Benchmark: Parse small program (10 functions)
static void BM_Parser_SmallProgram(benchmark::State &state) {
  std::string source = generate_function_source(10);

  for (auto _ : state) {
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parse();
    benchmark::DoNotOptimize(ast);
  }
  state.SetItemsProcessed(state.iterations() * 10);
}
BENCHMARK(BM_Parser_SmallProgram);

// Benchmark: Parse medium program (100 functions)
static void BM_Parser_MediumProgram(benchmark::State &state) {
  std::string source = generate_function_source(100);

  for (auto _ : state) {
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parse();
    benchmark::DoNotOptimize(ast);
  }
  state.SetItemsProcessed(state.iterations() * 100);
}
BENCHMARK(BM_Parser_MediumProgram);

// Benchmark: Parse expressions
static void BM_Parser_Expressions(benchmark::State &state) {
  std::ostringstream oss;
  for (int i = 0; i < 50; ++i) {
    oss << "let expr" << i << " = (a + b) * (c - d) / e;\n";
  }
  std::string source = oss.str();

  for (auto _ : state) {
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parse();
    benchmark::DoNotOptimize(ast);
  }
}
BENCHMARK(BM_Parser_Expressions);

BENCHMARK_MAIN();
