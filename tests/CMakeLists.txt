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
add_executable(test_lexer
    test_lexer.cpp
)
target_link_libraries(test_lexer PRIVATE czc)

add_executable(debug_operators
    debug_operators.cpp
)
target_link_libraries(debug_operators PRIVATE czc)

add_executable(test_token_preprocessor
    test_token_preprocessor.cpp
)
target_link_libraries(test_token_preprocessor PRIVATE czc)

add_executable(test_error_recovery
    test_error_recovery.cpp
)
target_link_libraries(test_error_recovery PRIVATE czc)

add_executable(test_synthetic_tokens
    test_synthetic_tokens.cpp
)
target_link_libraries(test_synthetic_tokens PRIVATE czc)

add_executable(test_parser
    test_parser.cpp
)
target_link_libraries(test_parser PRIVATE czc)

add_executable(test_formatter
    test_formatter.cpp
)
target_link_libraries(test_formatter PRIVATE czc)

add_executable(test_comments
    test_comments.cpp
)
target_link_libraries(test_comments PRIVATE czc)

# Tests using Google Test framework - link gtest_main
add_executable(test_lexer_gtest
    test_lexer_gtest.cpp
)
target_link_libraries(test_lexer_gtest PRIVATE czc gtest_main)

# Performance test - separate from regular tests, can be run manually
add_executable(test_source_tracker_performance
    test_source_tracker_performance.cpp
)
target_link_libraries(test_source_tracker_performance PRIVATE czc)

# 注意：test_utf8_edge_cases 暂时禁用，因为它使用了不存在的 API
# add_executable(test_utf8_edge_cases
#     test_utf8_edge_cases.cpp
# )
# 
# target_link_libraries(test_utf8_edge_cases PRIVATE czc)

# Register tests
# Use add_test instead of gtest_discover_tests to avoid running tests during build
add_test(NAME test_lexer COMMAND test_lexer)
add_test(NAME test_lexer_gtest COMMAND test_lexer_gtest)
add_test(NAME test_token_preprocessor COMMAND test_token_preprocessor)
add_test(NAME test_parser COMMAND test_parser)
add_test(NAME test_error_recovery COMMAND test_error_recovery)
add_test(NAME test_formatter COMMAND test_formatter)
# add_test(NAME test_utf8_edge_cases COMMAND test_utf8_edge_cases)
# Note: test_source_tracker_performance is not added to regular tests due to high resource usage


