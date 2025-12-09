cmake_minimum_required(VERSION 3.20)
project(czc VERSION 0.0.1 LANGUAGES CXX)

# C++23 标准
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 生成 compile_commands.json（用于 clang-tidy）
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ============================================================================
# 覆盖率选项
# ============================================================================
option(ENABLE_COVERAGE "Enable code coverage" OFF)

if(ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        # Clang 使用 source-based coverage
        add_compile_options(-fprofile-instr-generate -fcoverage-mapping)
        add_link_options(-fprofile-instr-generate -fcoverage-mapping)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        # GCC 使用 gcov
        add_compile_options(--coverage -fprofile-arcs -ftest-coverage)
        add_link_options(--coverage)
    endif()
endif()

# macOS: 确保 clang-tidy 能找到系统头文件
if(APPLE)
    execute_process(
        COMMAND xcrun --show-sdk-path
        OUTPUT_VARIABLE MACOS_SDK_PATH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(MACOS_SDK_PATH)
        add_compile_options(-isysroot ${MACOS_SDK_PATH})
    endif()
endif()

# ============================================================================
# 第三方依赖
# ============================================================================
include(FetchContent)

# CLI11 - 命令行解析
FetchContent_Declare(
    cli11
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
    GIT_TAG        v2.6.1
)

# glaze - JSON 序列化库
FetchContent_Declare(
    glaze
    GIT_REPOSITORY https://github.com/stephenberry/glaze.git
    GIT_TAG        v6.1.0
)

# tomlplusplus - TOML 配置文件解析
FetchContent_Declare(
    tomlplusplus
    GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
    GIT_TAG        v3.4.0
)

# GoogleTest - 单元测试框架
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.17.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# cmark - Markdown 解析库
FetchContent_Declare(
    cmark
    GIT_REPOSITORY https://github.com/commonmark/cmark.git
    GIT_TAG        0.31.1
)
# 禁用 cmark 测试
set(CMARK_TESTS OFF CACHE BOOL "" FORCE)
set(CMARK_SHARED OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(cli11 glaze tomlplusplus googletest cmark)

# ============================================================================
# 包含目录
# ============================================================================
include_directories(${CMAKE_SOURCE_DIR}/include)

# ============================================================================
# Diag 库（诊断系统）
# ============================================================================
set(DIAG_SOURCES
    src/diag/span.cpp
    src/diag/error_code.cpp
    src/diag/message.cpp
    src/diag/i18n.cpp
    src/diag/diagnostic.cpp
    src/diag/diag_builder.cpp
    src/diag/diag_context.cpp
    src/diag/emitters/ansi_renderer.cpp
    src/diag/emitters/text_emitter.cpp
    src/diag/emitters/json_emitter.cpp
)

add_library(czc_diag STATIC ${DIAG_SOURCES})
target_include_directories(czc_diag PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(czc_diag 
    PUBLIC cmark
    PUBLIC tomlplusplus::tomlplusplus
    PUBLIC glaze::glaze
)

# ============================================================================
# Lexer 库
# ============================================================================
set(LEXER_SOURCES
    src/lexer/source_manager.cpp
    src/lexer/source_reader.cpp
    src/lexer/token.cpp
    src/lexer/utf8.cpp
    src/lexer/scanner.cpp
    src/lexer/ident_scanner.cpp
    src/lexer/number_scanner.cpp
    src/lexer/string_scanner.cpp
    src/lexer/comment_scanner.cpp
    src/lexer/char_scanner.cpp
    src/lexer/lexer_error.cpp
    src/lexer/lexer.cpp
    src/lexer/lexer_error_codes.cpp
    src/lexer/lexer_source_locator.cpp
)

# 查找 ICU 库（用于 Unicode 支持）
# macOS Homebrew ICU 路径提示
if(APPLE)
    set(ICU_ROOT "/opt/homebrew/opt/icu4c")
    list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew/opt/icu4c")
endif()
find_package(ICU COMPONENTS uc REQUIRED)

add_library(czc_lexer STATIC ${LEXER_SOURCES})
target_include_directories(czc_lexer PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(czc_lexer 
    PUBLIC ICU::uc
    PUBLIC czc_diag
)

# ============================================================================
# CLI 库
# ============================================================================
set(CLI_SOURCES
    src/cli/cli.cpp
    src/cli/context.cpp
    src/cli/driver.cpp
    src/cli/phases/lexer_phase.cpp
    src/cli/output/text_formatter.cpp
    src/cli/output/json_formatter.cpp
    src/cli/commands/lex_command.cpp
    src/cli/commands/version_command.cpp
)

add_library(czc_cli STATIC ${CLI_SOURCES})
target_link_libraries(czc_cli 
    PUBLIC czc_lexer 
    PUBLIC CLI11::CLI11
    PUBLIC glaze::glaze
    PUBLIC tomlplusplus::tomlplusplus
)
target_include_directories(czc_cli PUBLIC ${CMAKE_SOURCE_DIR}/include)

# ============================================================================
# 可执行文件
# ============================================================================
add_executable(czc apps/czc/main.cpp)
target_link_libraries(czc PRIVATE czc_cli)

# ============================================================================
# 编译器警告选项
# ============================================================================
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(czc_lexer PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(czc_cli PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(czc PRIVATE -Wall -Wextra -Wpedantic)
elseif(MSVC)
    target_compile_options(czc_lexer PRIVATE /W4)
    target_compile_options(czc_cli PRIVATE /W4)
    target_compile_options(czc PRIVATE /W4)
endif()

# ============================================================================
# 测试
# ============================================================================
enable_testing()

# ============================================================================
# Lexer 单元测试
# ============================================================================
set(LEXER_UNITTEST_SOURCES
    tests/lexer/unittest/source_manager_test.cpp
    tests/lexer/unittest/source_reader_test.cpp
    tests/lexer/unittest/token_test.cpp
    tests/lexer/unittest/lexer_test.cpp
    tests/lexer/unittest/ident_scanner_test.cpp
    tests/lexer/unittest/number_scanner_test.cpp
    tests/lexer/unittest/string_scanner_test.cpp
    tests/lexer/unittest/comment_scanner_test.cpp
    tests/lexer/unittest/char_scanner_test.cpp
    tests/lexer/unittest/utf8_test.cpp
    tests/lexer/unittest/lexer_error_test.cpp
    tests/lexer/unittest/scanner_test.cpp
)

# 覆盖率模式下直接编译源文件到测试中
if(ENABLE_COVERAGE)
    add_executable(lexer_unittest ${LEXER_UNITTEST_SOURCES} ${LEXER_SOURCES})
    target_include_directories(lexer_unittest PRIVATE ${CMAKE_SOURCE_DIR}/include)
    target_link_libraries(lexer_unittest 
        PRIVATE GTest::gtest_main
        PRIVATE ICU::uc
        PRIVATE czc_diag
    )
else()
    add_executable(lexer_unittest ${LEXER_UNITTEST_SOURCES})
    target_link_libraries(lexer_unittest 
        PRIVATE czc_lexer
        PRIVATE GTest::gtest_main
    )
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(lexer_unittest PRIVATE -Wall -Wextra -Wpedantic)
elseif(MSVC)
    target_compile_options(lexer_unittest PRIVATE /W4)
endif()

include(GoogleTest)
gtest_discover_tests(lexer_unittest)

# ============================================================================
# Lexer 集成测试
# ============================================================================
set(LEXER_INTEGRATION_TEST_SOURCES
    tests/lexer/lexer_integration_test.cpp
)

add_executable(lexer_integration_tests ${LEXER_INTEGRATION_TEST_SOURCES})
target_link_libraries(lexer_integration_tests 
    PRIVATE czc_cli
    PRIVATE GTest::gtest_main
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(lexer_integration_tests PRIVATE -Wall -Wextra -Wpedantic)
elseif(MSVC)
    target_compile_options(lexer_integration_tests PRIVATE /W4)
endif()

gtest_discover_tests(lexer_integration_tests)

# ============================================================================
# Diag 单元测试
# ============================================================================
set(DIAG_UNITTEST_SOURCES
    tests/diag/unittest/i18n_test.cpp
    tests/diag/unittest/diag_context_test.cpp
)

add_executable(diag_unittest ${DIAG_UNITTEST_SOURCES})
target_link_libraries(diag_unittest 
    PRIVATE czc_diag
    PRIVATE GTest::gtest_main
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(diag_unittest PRIVATE -Wall -Wextra -Wpedantic)
elseif(MSVC)
    target_compile_options(diag_unittest PRIVATE /W4)
endif()

gtest_discover_tests(diag_unittest)

# ============================================================================
# CLI 单元测试
# ============================================================================
set(CLI_UNITTEST_SOURCES
    tests/cli/unittest/context_test.cpp
    tests/cli/unittest/driver_test.cpp
    tests/cli/unittest/formatter_test.cpp
)

add_executable(cli_unittest ${CLI_UNITTEST_SOURCES})
target_link_libraries(cli_unittest 
    PRIVATE czc_cli
    PRIVATE GTest::gtest_main
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(cli_unittest PRIVATE -Wall -Wextra -Wpedantic)
elseif(MSVC)
    target_compile_options(cli_unittest PRIVATE /W4)
endif()

gtest_discover_tests(cli_unittest)

# ============================================================================
# CLI 集成测试
# ============================================================================
set(CLI_INTEGRATION_TEST_SOURCES
    tests/cli/cli_integration_test.cpp
)

add_executable(cli_integration_tests ${CLI_INTEGRATION_TEST_SOURCES})
target_link_libraries(cli_integration_tests 
    PRIVATE czc_cli
    PRIVATE GTest::gtest_main
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(cli_integration_tests PRIVATE -Wall -Wextra -Wpedantic)
elseif(MSVC)
    target_compile_options(cli_integration_tests PRIVATE /W4)
endif()

gtest_discover_tests(cli_integration_tests)