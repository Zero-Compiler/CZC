cmake_minimum_required(VERSION 3.15)

add_executable(test_lexer
    test_lexer.cpp
)

target_link_libraries(test_lexer PRIVATE czc)

add_executable(debug_operators
    debug_operators.cpp
)

target_link_libraries(debug_operators PRIVATE czc)

add_test(NAME LexerTest COMMAND test_lexer)
