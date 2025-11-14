cmake_minimum_required(VERSION 3.15)
project(CZC VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# Fetch tomlplusplus library
include(FetchContent)
FetchContent_Declare(
    tomlplusplus
    GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
    GIT_TAG        v3.4.0
)
FetchContent_MakeAvailable(tomlplusplus)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -pedantic)
    if(ENABLE_COVERAGE)
        add_compile_options(--coverage -O0 -g)
        add_link_options(--coverage)
    endif()
endif()

add_library(czc STATIC
    # Lexer module (词法分析器)
    src/lexer/token.cpp
    src/lexer/lexer.cpp
    src/lexer/lexer_number.cpp
    src/lexer/lexer_string.cpp
    src/lexer/lexer_operators.cpp
    src/lexer/utf8_handler.cpp
    
    # Token preprocessor (Token 预处理器)
    src/token_preprocessor/token_preprocessor.cpp
    
    # Diagnostics (诊断系统)
    src/diagnostics/diagnostic_code.cpp
    src/diagnostics/diagnostic.cpp
    
    # Utilities (工具类)
    src/utils/source_tracker.cpp
    src/utils/file_collector.cpp
    
    # CST module (具体语法树)
    src/cst/cst_node.cpp
    
    # Parser module (语法分析器)
    src/parser/parser.cpp
    src/parser/parser_decl.cpp
    src/parser/parser_type.cpp
    src/parser/parser_stmt.cpp
    src/parser/parser_expr.cpp
    
    # Formatter module (代码格式化器)
    src/formatter/formatter.cpp
    src/formatter/formatter_decl.cpp
    src/formatter/formatter_stmt.cpp
    src/formatter/formatter_expr.cpp
    src/formatter/formatter_type.cpp
    
    # AST module (抽象语法树)
    src/ast/ast_builder.cpp
    src/ast/ast_visitor.cpp
)

target_include_directories(czc PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(czc PUBLIC tomlplusplus::tomlplusplus)

add_executable(czc-cli
    cli/main.cpp
)

target_link_libraries(czc-cli PRIVATE czc)

install(TARGETS czc czc-cli
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY czc/
    DESTINATION include/czc
    FILES_MATCHING PATTERN "*.hpp"
)

option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

option(BUILD_BENCHMARKS "Build benchmarks" OFF)
if(BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()
