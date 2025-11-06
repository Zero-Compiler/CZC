cmake_minimum_required(VERSION 3.15)
project(CZC VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

add_library(czc STATIC
    src/lexer/token.cpp
    src/lexer/lexer.cpp
    src/lexer/utf8_handler.cpp
    src/lexer/error_collector.cpp
    src/token_preprocessor/token_preprocessor.cpp
    src/token_preprocessor/error_collector.cpp
    src/diagnostics/diagnostic_code.cpp
    src/diagnostics/diagnostic.cpp
    src/utils/source_tracker.cpp
    src/utils/file_collector.cpp
    src/cst/cst_node.cpp
    src/cst/error_collector.cpp
    src/parser/parser.cpp
    src/parser/error_collector.cpp
    src/formatter/formatter.cpp
    src/formatter/error_collector.cpp
)

target_include_directories(czc PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

add_executable(czc-cli
    cli/main.cpp
)

target_link_libraries(czc-cli PRIVATE czc)

install(TARGETS czc czc-cli
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/czc
    DESTINATION include
)

option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
