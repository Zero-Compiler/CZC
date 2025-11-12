/**
 * @file debug_operators.cpp
 * @brief 一个简单的调试工具，用于测试运算符的词法分析。
 * @author BegoniaHe
 * @date 2025-11-11
 */

#include "czc/lexer/lexer.hpp"

#include <iostream>

/**
 * @brief 程序主入口。
 * @details
 *   此程序对一个包含所有支持的运算符的字符串进行词法分析，
 *   并将其生成的 Token 序列打印到标准输出，以供手动验证。
 * @return 总是返回 0。
 */
int main() {
  using namespace czc::lexer;

  // 创建一个包含所有运算符的测试字符串。
  Lexer lexer("+ - * / % = == ! != < <= > >= && ||");
  auto tokens = lexer.tokenize();

  // 打印所有生成的 Token。
  std::cout << "Total tokens: " << tokens.size() << std::endl;
  for (size_t i = 0; i < tokens.size(); i++) {
    std::cout << i << ": " << token_type_to_string(tokens[i].token_type) << " ("
              << tokens[i].value << ")" << std::endl;
  }

  return 0;
}
