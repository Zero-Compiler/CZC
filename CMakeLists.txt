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

# 核心库：libczc
add_library(czc STATIC
    src/lexer/token.cpp
    src/lexer/lexer_error.cpp
    src/lexer/lexer.cpp
)

target_include_directories(czc PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# CLI 工具
add_executable(czc-cli
    cli/main.cpp
)

target_link_libraries(czc-cli PRIVATE czc)

# 安装规则
install(TARGETS czc czc-cli
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/czc
    DESTINATION include
)

# 测试
option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
