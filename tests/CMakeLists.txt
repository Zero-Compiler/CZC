cmake_minimum_required(VERSION 3.15)

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

add_executable(test_parser
    test_parser.cpp
)

target_link_libraries(test_parser PRIVATE czc)

add_executable(test_formatter
    test_formatter.cpp
)

target_link_libraries(test_formatter PRIVATE czc)

add_executable(test_source_tracker_performance
    test_source_tracker_performance.cpp
)

target_link_libraries(test_source_tracker_performance PRIVATE czc)

add_executable(test_comments
    test_comments.cpp
)

target_link_libraries(test_comments PRIVATE czc)

add_test(NAME LexerTest COMMAND test_lexer)
add_test(NAME TokenPreprocessorTest COMMAND test_token_preprocessor)
add_test(NAME ParserTest COMMAND test_parser)
