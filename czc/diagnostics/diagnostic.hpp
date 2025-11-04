/**
 * @file diagnostic.hpp
 * @brief 诊断系统核心类定义
 * @author BegoniaHe
 * @date 2025-11-04
 */

#ifndef CZC_DIAGNOSTIC_HPP
#define CZC_DIAGNOSTIC_HPP

#include "czc/utils/source_location.hpp"
#include "diagnostic_code.hpp"
#include "diagnostic_reporter.hpp"
#include <memory>
#include <unordered_map>
#include <vector>

namespace czc {
namespace diagnostics {

/**
 * @brief 存储单条诊断消息的国际化模板。
 * @details
 *   此结构体定义了本地化诊断消息所需的所有静态文本，包括消息格式、
 *   可选的帮助文本和来源信息。
 */
struct MessageTemplate {
  // 消息格式化字符串，例如 "invalid character '{0}'"。
  // 占位符 {0}, {1}, ... 将被动态参数替换。
  std::string message;
  // (可选) 补充的帮助信息或修复建议。
  std::string help;
  // (可选) 消息来源，例如定义该诊断的模块或标准。
  std::string source;
};

/**
 * @brief 管理和提供国际化（i18n）的诊断消息。
 * @details
 *   此类负责根据当前的语言环境（locale）加载对应的消息模板文件（.toml），
 *   并提供接口来获取和格式化诊断消息。
 * @property {线程安全} 非线程安全。在多线程环境中需由调用者外部加锁。
 */
class I18nMessages {
private:
  // 当前设置的语言环境字符串，例如 "en_US"。
  std::string current_locale;
  // 存储从 .toml 文件加载的所有诊断消息模板。
  // 键是诊断代码的字符串表示（如 "L0001"）。
  std::unordered_map<std::string, MessageTemplate> messages;

  /**
   * @brief 从指定的 .toml 文件加载特定语言环境的消息。
   * @param[in] locale 要加载的语言环境标识符，例如 "en_US"。
   * @return 如果文件成功加载并解析，则返回 `true`，否则返回 `false`。
   */
  bool load_from_file(const std::string &locale);

public:
  /**
   * @brief 构造并初始化一个国际化消息管理器。
   * @param[in] locale 初始的语言环境，默认为 "en_US"。
   */
  I18nMessages(const std::string &locale = "en_US");

  /**
   * @brief 切换当前的语言环境。
   * @details
   *   如果新的语言环境与当前不同，将尝试从对应的 .toml 文件加载新的消息模板。
   * @param[in] locale 新的语言环境标识符。
   */
  void set_locale(const std::string &locale);

  /**
   * @brief 根据诊断代码获取对应的消息模板。
   * @param[in] code 唯一的诊断代码。
   * @return 返回一个指向消息模板的常量引用。
   * @exception std::runtime_error 如果找不到与 `code` 对应的消息模板。
   */
  const MessageTemplate &get_message(DiagnosticCode code) const;

  /**
   * @brief 格式化一条诊断消息。
   * @details
   *   使用诊断代码对应的消息模板，并将模板中的占位符（如 {0}, {1}）
   *   替换为 `args` 中提供的实际参数。
   * @param[in] code 诊断代码。
   * @param[in] args 用于替换消息模板中占位符的字符串列表。
   * @return 返回格式化后的完整消息字符串。
   */
  std::string format_message(DiagnosticCode code,
                             const std::vector<std::string> &args) const;
};

/**
 * @brief 代表一个具体的诊断事件（如错误或警告）。
 * @details
 *   此类是不可变对象，封装了诊断事件的所有信息，包括其严重级别、
 *   唯一的诊断代码、在源代码中的位置以及格式化消息所需的参数。
 */
class Diagnostic {
private:
  // 诊断的严重级别（例如：警告、错误）。
  DiagnosticLevel level;
  // 唯一的诊断标识代码。
  DiagnosticCode code;
  // 诊断在源代码中的精确位置（文件、行、列）。
  utils::SourceLocation location;
  // 用于格式化诊断消息的动态参数列表。
  std::vector<std::string> args;
  // 发生诊断的源代码行内容，用于在报告中显示上下文。
  std::string source_line;

public:
  /**
   * @brief 构造一个新的诊断对象。
   * @param[in] lvl 诊断的严重级别。
   * @param[in] c 唯一的诊断代码。
   * @param[in] loc 源代码中的位置。
   * @param[in] arguments (可选) 格式化消息所需的参数。
   */
  Diagnostic(DiagnosticLevel lvl, DiagnosticCode c,
             const utils::SourceLocation &loc,
             const std::vector<std::string> &arguments = {})
      : level(lvl), code(c), location(loc), args(arguments) {}

  /**
   * @brief 设置与此诊断相关的源代码行。
   * @param[in] line 包含诊断位置的完整源代码行。
   */
  void set_source_line(const std::string &line) { source_line = line; }

  /**
   * @brief 获取诊断的严重级别。
   * @return 返回诊断级别枚举值。
   */
  DiagnosticLevel get_level() const { return level; }

  /**
   * @brief 获取唯一的诊断代码。
   * @return 返回诊断代码枚举值。
   */
  DiagnosticCode get_code() const { return code; }

  /**
   * @brief 获取诊断在源代码中的位置。
   * @return 返回对 SourceLocation 对象的常量引用。
   */
  const utils::SourceLocation &get_location() const { return location; }

  /**
   * @brief 获取用于格式化消息的参数列表。
   * @return 返回对字符串向量的常量引用。
   */
  const std::vector<std::string> &get_args() const { return args; }

  /**
   * @brief 获取相关的源代码行。
   * @return 返回对源代码行字符串的常量引用。
   */
  const std::string &get_source_line() const { return source_line; }

  /**
   * @brief 将此诊断格式化为人类可读的字符串。
   * @details
   *   此方法使用提供的 I18nMessages 管理器来生成本地化的消息，
   *   并可以应用 ANSI 颜色代码以增强可读性。
   * @param[in] i18n 用于消息本地化的国际化管理器。
   * @param[in] use_color 如果为 true，则在输出中使用 ANSI 颜色代码。
   * @return 返回格式化后的完整诊断报告字符串。
   */
  std::string format(const I18nMessages &i18n, bool use_color = true) const;
};

/**
 * @brief 编译器中处理所有诊断信息的中心枢纽。
 * @details
 *   此类实现了 IDiagnosticReporter 接口，负责收集、计数和最终报告
 *   在编译过程中产生的所有诊断（错误和警告）。
 *   它还管理 I18nMessages 实例以支持多语言输出。
 * @property {线程安全} 非线程安全。在多线程环境中需由调用者外部加锁。
 * @property {生命周期}
 *   DiagnosticEngine 应在所有可能报告诊断的组件（如 Lexer, Parser）
 *   的生命周期内保持有效。
 */
class DiagnosticEngine : public IDiagnosticReporter {
private:
  // 存储所有已报告的诊断信息的列表。
  std::vector<std::shared_ptr<Diagnostic>> diagnostics;
  // 指向国际化消息管理器的共享指针。
  std::shared_ptr<I18nMessages> i18n;
  // 记录已报告的错误总数。
  size_t error_count = 0;
  // 记录已报告的警告总数。
  size_t warning_count = 0;

public:
  /**
   * @brief 构造一个新的诊断引擎。
   * @param[in] locale 初始的语言环境，默认为 "en_US"。
   */
  DiagnosticEngine(const std::string &locale = "en_US");

  /**
   * @brief 更改诊断引擎的语言环境。
   * @param[in] locale 新的语言环境标识符。
   */
  void set_locale(const std::string &locale) { i18n->set_locale(locale); }

  /**
   * @brief 报告一个新的诊断事件。
   * @details
   *   这是 IDiagnosticReporter 接口的实现。此方法会接收一个诊断对象，
   *   根据其级别更新错误或警告计数，并将其存储以备后续报告。
   * @param[in] diag 要报告的诊断对象的共享指针。
   */
  void report(std::shared_ptr<Diagnostic> diag) override;

  /**
   * @brief 检查是否报告了任何错误。
   * @details 这是 IDiagnosticReporter 接口的实现。
   * @return 如果 `error_count` 大于 0，则返回 `true`。
   */
  bool has_errors() const override { return error_count > 0; }

  /**
   * @brief 将所有收集到的诊断信息打印到标准输出。
   * @param[in] use_color 如果为 true，则使用 ANSI 颜色代码进行打印。
   */
  void print_all(bool use_color = true) const;

  /**
   * @brief 获取对内部 I18nMessages 管理器的访问权限。
   * @return 对 I18nMessages 对象的常量引用。
   */
  const I18nMessages &get_i18n() const { return *i18n; }
};

} // namespace diagnostics
} // namespace czc

#endif // CZC_DIAGNOSTIC_HPP
