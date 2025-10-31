#include "czc/lexer/lexer.hpp"
#include <iostream>

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
