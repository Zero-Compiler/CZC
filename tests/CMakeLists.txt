cmake_minimum_required(VERSION 3.15)

# Try to find system Google Test first, otherwise fetch it
find_package(GTest QUIET)

if(NOT GTest_FOUND)
  # Fetch Google Test only if not found on system
  include(FetchContent)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
  )
  # For Windows: Prevent overriding the parent project's compiler/linker settings
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)
endif()

# Enable Google Test integration
include(GoogleTest)

# Tests with custom main() - do NOT link gtest_main
add_executable(debug_operators
    debug_operators.cpp
)
target_link_libraries(debug_operators PRIVATE czc)

# Tests using Google Test framework - LINK gtest_main
add_executable(test_lexer_gtest
    test_lexer_gtest.cpp
)
target_link_libraries(test_lexer_gtest PRIVATE czc GTest::gtest_main)
gtest_discover_tests(test_lexer_gtest)

add_executable(test_token_preprocessor
    test_token_preprocessor.cpp
)
target_link_libraries(test_token_preprocessor PRIVATE czc GTest::gtest_main)
gtest_discover_tests(test_token_preprocessor)

add_executable(test_parser
    test_parser.cpp
)
target_link_libraries(test_parser PRIVATE czc GTest::gtest_main)
gtest_discover_tests(test_parser)

add_executable(test_error_recovery
    test_error_recovery.cpp
)
target_link_libraries(test_error_recovery PRIVATE czc GTest::gtest_main)
gtest_discover_tests(test_error_recovery)

add_executable(test_formatter
    test_formatter.cpp
)
target_link_libraries(test_formatter PRIVATE czc GTest::gtest_main)
gtest_discover_tests(test_formatter)

add_executable(test_comments
    test_comments.cpp
)
target_link_libraries(test_comments PRIVATE czc GTest::gtest_main)
gtest_discover_tests(test_comments)

add_executable(test_else_if
    test_else_if.cpp
)
target_link_libraries(test_else_if PRIVATE czc GTest::gtest_main)
gtest_discover_tests(test_else_if)

add_executable(test_synthetic_tokens
    test_synthetic_tokens.cpp
)
target_link_libraries(test_synthetic_tokens PRIVATE czc GTest::gtest_main)
gtest_discover_tests(test_synthetic_tokens)

add_executable(test_utf8_edge_cases
    test_utf8_edge_cases.cpp
)
target_link_libraries(test_utf8_edge_cases PRIVATE czc GTest::gtest_main)
gtest_discover_tests(test_utf8_edge_cases)

add_executable(test_source_tracker_performance
    test_source_tracker_performance.cpp
)
target_link_libraries(test_source_tracker_performance PRIVATE czc GTest::gtest_main)
gtest_discover_tests(test_source_tracker_performance)

add_executable(test_cst
    test_cst.cpp
)
target_link_libraries(test_cst PRIVATE czc GTest::gtest_main)
gtest_discover_tests(test_cst)

# Tests for new features (structs, arrays, tuples, function literals)
add_executable(test_struct_type_alias
    test_struct_type_alias.cpp
)
target_link_libraries(test_struct_type_alias PRIVATE czc GTest::gtest_main)
gtest_discover_tests(test_struct_type_alias)

add_executable(test_array_types
    test_array_types.cpp
)
target_link_libraries(test_array_types PRIVATE czc GTest::gtest_main)
gtest_discover_tests(test_array_types)

add_executable(test_tuple_types
    test_tuple_types.cpp
)
target_link_libraries(test_tuple_types PRIVATE czc GTest::gtest_main)
gtest_discover_tests(test_tuple_types)

add_executable(test_function_literals
    test_function_literals.cpp
)
target_link_libraries(test_function_literals PRIVATE czc GTest::gtest_main)
gtest_discover_tests(test_function_literals)

add_executable(test_cst_edge_cases
    test_cst_edge_cases.cpp
)
target_link_libraries(test_cst_edge_cases PRIVATE czc GTest::gtest_main)
gtest_discover_tests(test_cst_edge_cases)

# AST tests
add_executable(test_ast
    test_ast.cpp
)
target_link_libraries(test_ast PRIVATE czc GTest::gtest_main)
gtest_discover_tests(test_ast)

# AST coverage tests (additional tests to improve function coverage)
add_executable(test_ast_coverage
    test_ast_coverage.cpp
)
target_link_libraries(test_ast_coverage PRIVATE czc GTest::gtest_main)
gtest_discover_tests(test_ast_coverage)

