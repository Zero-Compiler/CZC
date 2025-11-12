/**
 * @file benchmark_lexer.cpp
 * @brief Lexer performance benchmarks
 * @author BegoniaHe
 * @date 2025-11-11
 */

#include "czc/lexer/lexer.hpp"
#include <benchmark/benchmark.h>
#include <sstream>

using namespace czc::lexer;

// Helper function to generate source code of specific size
std::string generate_source(size_t num_lines) {
  std::ostringstream oss;
  for (size_t i = 0; i < num_lines; ++i) {
    oss << "let x" << i << " = " << i << " + " << (i * 2) << ";\n";
  }
  return oss.str();
}

// Benchmark: Small file (100 lines)
static void BM_Lexer_SmallFile(benchmark::State &state) {
  std::string source = generate_source(100);
  for (auto _ : state) {
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    benchmark::DoNotOptimize(tokens);
  }
  state.SetItemsProcessed(state.iterations() * 100);
}
BENCHMARK(BM_Lexer_SmallFile);

// Benchmark: Medium file (1000 lines)
static void BM_Lexer_MediumFile(benchmark::State &state) {
  std::string source = generate_source(1000);
  for (auto _ : state) {
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    benchmark::DoNotOptimize(tokens);
  }
  state.SetItemsProcessed(state.iterations() * 1000);
}
BENCHMARK(BM_Lexer_MediumFile);

// Benchmark: Large file (10000 lines)
static void BM_Lexer_LargeFile(benchmark::State &state) {
  std::string source = generate_source(10000);
  for (auto _ : state) {
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    benchmark::DoNotOptimize(tokens);
  }
  state.SetItemsProcessed(state.iterations() * 10000);
}
BENCHMARK(BM_Lexer_LargeFile);

// Benchmark: String processing
static void BM_Lexer_Strings(benchmark::State &state) {
  std::ostringstream oss;
  for (int i = 0; i < 100; ++i) {
    oss << "let s" << i << " = \"This is a test string number " << i
        << "\";\n";
  }
  std::string source = oss.str();

  for (auto _ : state) {
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    benchmark::DoNotOptimize(tokens);
  }
}
BENCHMARK(BM_Lexer_Strings);

// Benchmark: Number processing
static void BM_Lexer_Numbers(benchmark::State &state) {
  std::ostringstream oss;
  for (int i = 0; i < 100; ++i) {
    oss << "let n" << i << " = " << (i * 3.14159) << "e" << (i % 10) << ";\n";
  }
  std::string source = oss.str();

  for (auto _ : state) {
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    benchmark::DoNotOptimize(tokens);
  }
}
BENCHMARK(BM_Lexer_Numbers);

// Benchmark: UTF-8 identifiers
static void BM_Lexer_UTF8(benchmark::State &state) {
  std::ostringstream oss;
  for (int i = 0; i < 100; ++i) {
    oss << "let 变量" << i << " = " << i << ";\n";
    oss << "let переменная" << i << " = " << (i * 2) << ";\n";
  }
  std::string source = oss.str();

  for (auto _ : state) {
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    benchmark::DoNotOptimize(tokens);
  }
}
BENCHMARK(BM_Lexer_UTF8);

BENCHMARK_MAIN();
