/**
 * @file format_options.hpp
 * @brief 定义了 `FormatOptions` 类，用于配置代码格式化选项。
 * @author BegoniaHe
 * @date 2025-11-06
 */

#ifndef CZC_FORMAT_OPTIONS_HPP
#define CZC_FORMAT_OPTIONS_HPP

#include <cstddef>

namespace czc {
namespace formatter {

/**
 * @brief 定义了代码格式化器使用的缩进风格。
 */
enum class IndentStyle {
  SPACES, // 使用空格缩进
  TABS    // 使用制表符缩进
};

/**
 * @brief 封装了控制代码格式化行为的所有选项。
 * @details 此结构体用于向 `Formatter` 提供详细的配置，例如缩进宽度、
 *          最大行长等。这是一个纯数据结构。
 */
struct FormatOptions {
  // 缩进风格 (空格或制表符)
  IndentStyle indent_style;
  // 缩进宽度 (如果使用空格)
  size_t indent_width;
  // 最大行长度，用于未来的自动换行功能
  size_t max_line_length;
  // 是否在函数调用的括号前添加空格
  bool space_before_paren;
  // 是否在逗号后添加空格
  bool space_after_comma;
  // 是否在左大括号前换行
  bool newline_before_brace;

  /**
   * @brief 构造一个具有推荐默认值的 FormatOptions 实例。
   */
  FormatOptions()
      : indent_style(IndentStyle::SPACES), indent_width(4), max_line_length(80),
        space_before_paren(true), space_after_comma(true),
        newline_before_brace(false) {}

  /**
   * @brief 构造一个完全自定义的 FormatOptions 实例。
   * @param[in] style 缩进风格。
   * @param[in] width 缩进宽度。
   * @param[in] max_len 最大行长度。
   * @param[in] space_paren 括号前是否添加空格。
   * @param[in] space_comma 逗号后是否添加空格。
   * @param[in] newline_brace 大括号前是否换行。
   */
  FormatOptions(IndentStyle style, size_t width, size_t max_len,
                bool space_paren, bool space_comma, bool newline_brace)
      : indent_style(style), indent_width(width), max_line_length(max_len),
        space_before_paren(space_paren), space_after_comma(space_comma),
        newline_before_brace(newline_brace) {}
};

} // namespace formatter
} // namespace czc

#endif // CZC_FORMAT_OPTIONS_HPP