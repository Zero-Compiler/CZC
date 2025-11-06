/**
 * @file test_comments.cpp
 * @brief 测试注释功能
 * @author BegoniaHe
 * @date 2025-11-06
 */

#include "czc/formatter/formatter.hpp"
#include "czc/lexer/lexer.hpp"
#include "czc/parser/parser.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

using namespace czc::lexer;
using namespace czc::parser;
using namespace czc::formatter;
using namespace czc::cst;

int main() {
  // 读取测试文件
  std::ifstream file("examples/test_comments.zero");
  if (!file.is_open()) {
    std::cerr << "Failed to open test file" << std::endl;
    return 1;
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  std::string source = buffer.str();

  std::cout << "=== 原始代码 ===" << std::endl;
  std::cout << source << std::endl;

  // 词法分析
  std::cout << "\n=== 词法分析结果 ===" << std::endl;
  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  for (const auto &token : tokens) {
    std::cout << token_type_to_string(token.token_type) << ": \"" << token.value
              << "\" "
              << "(Line " << token.line << ", Col " << token.column << ")"
              << std::endl;
  }

  std::cout << "\n总共生成了 " << tokens.size() << " 个 Token" << std::endl;

  // 统计注释数量
  int comment_count = 0;
  for (const auto &token : tokens) {
    if (token.token_type == TokenType::Comment) {
      comment_count++;
    }
  }
  std::cout << "其中注释有 " << comment_count << " 个" << std::endl;

  // 语法分析
  std::cout << "\n=== 语法分析（构建CST） ===" << std::endl;
  Parser parser(tokens);
  auto cst = parser.parse();

  if (!cst) {
    std::cerr << "解析失败" << std::endl;
    return 1;
  }

  std::cout << "CST 根节点类型: " << cst_node_type_to_string(cst->get_type())
            << std::endl;
  std::cout << "CST 子节点数量: " << cst->get_children().size() << std::endl;

  // 统计CST中的注释节点
  int cst_comment_count = 0;
  for (const auto &child : cst->get_children()) {
    if (child->get_type() == CSTNodeType::Comment) {
      cst_comment_count++;
    }
  }
  std::cout << "CST 中的注释节点: " << cst_comment_count << " 个" << std::endl;

  // 格式化
  std::cout << "\n=== 格式化输出 ===" << std::endl;
  FormatOptions options;
  Formatter formatter(options);
  std::string formatted = formatter.format(cst.get());

  std::cout << formatted << std::endl;

  std::cout << "\n=== 测试完成 ===" << std::endl;

  return 0;
}
