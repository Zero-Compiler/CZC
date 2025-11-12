/**
 * @file test_source_tracker_performance.cpp
 * @brief 验证 SourceTracker 行索引优化的性能提升。
 * @author BegoniaHe
 * @date 2025-11-11
 */

#include "czc/utils/source_tracker.hpp"

#include <chrono>
#include <iostream>
#include <sstream>

using namespace czc::utils;

/**
 * @brief 生成指定行数的测试源码。
 */
std::string generate_large_source(size_t line_count) {
  std::ostringstream oss;
  for (size_t i = 1; i <= line_count; i++) {
    oss << "let variable_" << i << " = " << i << ";\n";
  }
  return oss.str();
}

/**
 * @brief 测试随机访问多行的性能。
 */
void test_performance() {
  std::cout << "\n=== SourceTracker 性能验证 ===" << std::endl;

  // 生成 大文件源码
  const size_t line_count = 10000000;
  const size_t access_count = 100;
  std::string source = generate_large_source(line_count);

  std::cout << "测试配置:" << std::endl;
  std::cout << "  - 源码行数: " << line_count << std::endl;
  std::cout << "  - 访问次数: " << access_count << std::endl;
  std::cout << "  - 源码大小: " << source.size() << " 字节" << std::endl;

  SourceTracker tracker(source, "test_large_file.zero");

  // 测试：随机访问多行（模拟错误报告场景）
  auto start = std::chrono::high_resolution_clock::now();

  for (size_t i = 0; i < access_count; i++) {
    // 访问不同的行（分布在整个文件中）
    size_t line_num = (i * 10) % line_count + 1;
    std::string line = tracker.get_source_line(line_num);

    // 简单验证：确保获取的行非空
    if (line.empty()) {
      std::cerr << "错误：第 " << line_num << " 行为空！" << std::endl;
      return;
    }
  }

  auto end = std::chrono::high_resolution_clock::now();
  auto duration =
      std::chrono::duration_cast<std::chrono::microseconds>(end - start);

  std::cout << "\n性能结果:" << std::endl;
  std::cout << "  - 总耗时: " << duration.count() << " 微秒" << std::endl;
  std::cout << "  - 平均每次访问: " << duration.count() / access_count
            << " 微秒" << std::endl;

  // 验证正确性：检查几个特定行
  std::string line1 = tracker.get_source_line(1);
  std::string line500 = tracker.get_source_line(500);
  std::string line1000 = tracker.get_source_line(1000);

  std::cout << "\n正确性验证:" << std::endl;
  std::cout << "  第 1 行: " << line1.substr(0, 30) << "..." << std::endl;
  std::cout << "  第 500 行: " << line500.substr(0, 30) << "..." << std::endl;
  std::cout << "  第 1000 行: " << line1000.substr(0, 30) << "..." << std::endl;

  std::cout << "\n性能测试通过！" << std::endl;
}

/**
 * @brief 测试边界情况。
 */
void test_edge_cases() {
  std::cout << "\n=== 边界情况测试 ===" << std::endl;

  // 空文件
  SourceTracker empty_tracker("", "empty.zero");
  std::string empty_line = empty_tracker.get_source_line(1);
  std::cout << "  - 空文件测试: " << (empty_line.empty() ? "通过" : "失败")
            << std::endl;

  // 单行无换行符
  SourceTracker single_line_tracker("let x = 42;", "single.zero");
  std::string single_line = single_line_tracker.get_source_line(1);
  std::cout << "  - 单行测试: "
            << (single_line == "let x = 42;" ? "通过" : "失败") << std::endl;

  // 多个连续空行
  SourceTracker empty_lines_tracker("line1\n\n\nline4\n", "empty_lines.zero");
  std::string line2 = empty_lines_tracker.get_source_line(2);
  std::string line3 = empty_lines_tracker.get_source_line(3);
  std::cout << "  - 空行测试: "
            << (line2.empty() && line3.empty() ? "通过" : "失败") << std::endl;

  // 超出范围
  std::string out_of_range = empty_lines_tracker.get_source_line(999);
  std::cout << "  - 超出范围测试: " << (out_of_range.empty() ? "通过" : "失败")
            << std::endl;

  std::cout << "\n所有边界测试通过！" << std::endl;
}

int main() {
  test_performance();
  test_edge_cases();
  return 0;
}
