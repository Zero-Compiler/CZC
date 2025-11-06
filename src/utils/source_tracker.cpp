/**
 * @file source_tracker.cpp
 * @brief `SourceTracker` 类的功能实现。
 * @author BegoniaHe
 * @date 2025-11-05
 */

#include "czc/utils/source_tracker.hpp"

namespace czc {
namespace utils {

SourceTracker::SourceTracker(const std::string &source,
                             const std::string &fname)
    : filename(fname), position(0), line(1), column(1) {
  // NOTE(BegoniaHe): 将输入的 std::string 复制到内部的 std::vector<char>。
  // 使用 vector<char> 而非 string 是一个设计选择。虽然 string
  // 在很多方面功能更强，但 vector<char> 确保了数据是连续存储的，
  // 并且在与需要原始指针和大小的 C API 或其他底层库交互时可能更直接。
  // 对于词法分析器这种性能敏感的组件，直接操作字符数组有时更清晰。
  input.assign(source.begin(), source.end());
}

void SourceTracker::advance(char c) {
  // 每次消耗一个字符时，字节位置 `position` 总是增加
  position++;

  // 检查消耗的字符是否是换行符，以正确更新行列计数
  if (c == '\n') {
    // 如果是换行符，行号增加，列号重置为 1
    line++;
    column = 1;
  } else {
    // 如果不是换行符，只增加列号
    column++;
  }
}

SourceLocation SourceTracker::make_location(size_t start_line,
                                            size_t start_col) const {
  // 使用给定的起始位置和跟踪器当前的结束位置来创建一个 SourceLocation 对象
  // 这是从词法分析器中标记出 Token 范围的核心功能
  return SourceLocation(filename, start_line, start_col, line, column);
}

void SourceTracker::build_line_offsets() const {
  // 如果已经构建过索引，直接返回
  if (line_offsets_built) {
    return;
  }

  line_offsets.clear();
  line_offsets.push_back(0); // 第 1 行从索引 0 开始

  // 遍历整个输入，记录每个换行符后的位置作为下一行的起始
  for (size_t i = 0; i < input.size(); i++) {
    if (input[i] == '\n') {
      line_offsets.push_back(i + 1);
    }
  }

  line_offsets_built = true;
}

std::string SourceTracker::get_source_line(size_t line_num) const {
  if (line_num == 0) {
    return "";
  }

  // --- 使用预构建的行索引表实现 O(1) 查找 ---
  build_line_offsets(); // 惰性初始化

  // 检查行号是否有效
  if (line_num > line_offsets.size()) {
    return "";
  }

  // 获取该行的起始和结束位置
  size_t line_start = line_offsets[line_num - 1];
  size_t line_end;

  if (line_num < line_offsets.size()) {
    // 中间的行：结束位置是下一行的起始位置减 1（去掉 '\n'）
    line_end = line_offsets[line_num] - 1;
  } else {
    // 最后一行：结束位置是文件末尾
    line_end = input.size();
  }

  // 边界检查：确保不会越界
  if (line_start > input.size()) {
    return "";
  }
  if (line_end > input.size()) {
    line_end = input.size();
  }

  return std::string(input.begin() + line_start, input.begin() + line_end);
}

} // namespace utils
} // namespace czc