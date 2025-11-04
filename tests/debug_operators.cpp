/**
 * @file debug_operators.cpp
 * @brief 调试运算符词法分析
 * @author BegoniaHe
 */

#include "czc/lexer/lexer.hpp"
#include <iostream>

/**
 * @brief 主函数入口
 * @return 程序退出码
 */
int main()
{
    Lexer lexer("+ - * / % = == ! != < <= > >= && ||");
    auto tokens = lexer.tokenize();

    std::cout << "Total tokens: " << tokens.size() << std::endl;
    for (size_t i = 0; i < tokens.size(); i++)
    {
        std::cout << i << ": " << token_type_to_string(tokens[i].token_type)
                  << " (" << tokens[i].value << ")" << std::endl;
    }

    return 0;
}
