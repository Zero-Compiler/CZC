cmake_minimum_required(VERSION 3.15)

# Fetch Google Benchmark
include(FetchContent)
FetchContent_Declare(
  googlebenchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG v1.8.3
)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googlebenchmark)

# Lexer benchmark
add_executable(benchmark_lexer
    benchmark_lexer.cpp
)

target_link_libraries(benchmark_lexer PRIVATE czc benchmark::benchmark_main)

# Parser benchmark
add_executable(benchmark_parser
    benchmark_parser.cpp
)

target_link_libraries(benchmark_parser PRIVATE czc benchmark::benchmark_main)
