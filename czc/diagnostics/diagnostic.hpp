/**
 * @file diagnostic.hpp
 * @brief 诊断系统核心类定义
 * @author BegoniaHe
 */

#ifndef CZC_DIAGNOSTIC_HPP
#define CZC_DIAGNOSTIC_HPP

#include "diagnostic_code.hpp"
#include "diagnostic_reporter.hpp"
#include <unordered_map>
#include <vector>
#include <memory>

/**
 * @brief 消息模板结构
 */
struct MessageTemplate
{
    std::string message; ///< 消息内容
    std::string help;    ///< 帮助信息
    std::string source;  ///< 来源信息
};

/**
 * @brief 国际化消息管理器
 */
class I18nMessages
{
private:
    std::string current_locale;                                ///< 当前语言环境
    std::unordered_map<std::string, MessageTemplate> messages; ///< 消息模板映射表

    /**
     * @brief 从文件加载消息
     * @param locale 语言环境代码
     * @return 加载是否成功
     */
    bool load_from_file(const std::string &locale);

public:
    /**
     * @brief 构造函数
     * @param locale 语言环境代码，默认为 "en_US"
     */
    I18nMessages(const std::string &locale = "en_US");

    /**
     * @brief 设置语言环境
     * @param locale 语言环境代码
     */
    void set_locale(const std::string &locale);

    /**
     * @brief 获取消息模板
     * @param code 诊断代码
     * @return 消息模板引用
     */
    const MessageTemplate &get_message(DiagnosticCode code) const;

    /**
     * @brief 格式化消息
     * @param code 诊断代码
     * @param args 消息参数列表
     * @return 格式化后的消息字符串
     */
    std::string format_message(DiagnosticCode code,
                               const std::vector<std::string> &args) const;
};

/**
 * @brief 诊断信息类
 */
class Diagnostic
{
private:
    DiagnosticLevel level;         ///< 诊断级别
    DiagnosticCode code;           ///< 诊断代码
    SourceLocation location;       ///< 源码位置
    std::vector<std::string> args; ///< 消息参数
    std::string source_line;       ///< 源码行内容

public:
    /**
     * @brief 构造函数
     * @param lvl 诊断级别
     * @param c 诊断代码
     * @param loc 源码位置
     * @param arguments 消息参数列表
     */
    Diagnostic(DiagnosticLevel lvl,
               DiagnosticCode c,
               const SourceLocation &loc,
               const std::vector<std::string> &arguments = {})
        : level(lvl), code(c), location(loc), args(arguments) {}

    /**
     * @brief 设置源码行
     * @param line 源码行内容
     */
    void set_source_line(const std::string &line) { source_line = line; }

    /**
     * @brief 获取诊断级别
     * @return 诊断级别
     */
    DiagnosticLevel get_level() const { return level; }

    /**
     * @brief 获取诊断代码
     * @return 诊断代码
     */
    DiagnosticCode get_code() const { return code; }

    /**
     * @brief 获取源码位置
     * @return 源码位置引用
     */
    const SourceLocation &get_location() const { return location; }

    /**
     * @brief 获取消息参数
     * @return 消息参数列表引用
     */
    const std::vector<std::string> &get_args() const { return args; }

    /**
     * @brief 获取源码行
     * @return 源码行内容引用
     */
    const std::string &get_source_line() const { return source_line; }

    /**
     * @brief 格式化输出诊断信息
     * @param i18n 国际化消息管理器
     * @param use_color 是否使用颜色输出，默认为 true
     * @return 格式化后的诊断信息字符串
     */
    std::string format(const I18nMessages &i18n, bool use_color = true) const;
};

/**
 * @brief 诊断引擎类
 */
class DiagnosticEngine : public IDiagnosticReporter
{
private:
    std::vector<std::shared_ptr<Diagnostic>> diagnostics; ///< 诊断信息列表
    std::shared_ptr<I18nMessages> i18n;                   ///< 国际化消息管理器
    size_t error_count = 0;                               ///< 错误计数
    size_t warning_count = 0;                             ///< 警告计数

public:
    /**
     * @brief 构造函数
     * @param locale 语言环境代码，默认为 "en_US"
     */
    DiagnosticEngine(const std::string &locale = "en_US");

    /**
     * @brief 设置语言环境
     * @param locale 语言环境代码
     */
    void set_locale(const std::string &locale) { i18n->set_locale(locale); }

    /**
     * @brief 报告诊断信息（实现 IDiagnosticReporter 接口）
     * @param diag 诊断信息智能指针
     */
    void report(std::shared_ptr<Diagnostic> diag) override;

    /**
     * @brief 检查是否有错误（实现 IDiagnosticReporter 接口）
     * @return 如果有错误返回 true，否则返回 false
     */
    bool has_errors() const override { return error_count > 0; }

    /**
     * @brief 打印所有诊断信息
     * @param use_color 是否使用颜色输出，默认为 true
     */
    void print_all(bool use_color = true) const;

    /**
     * @brief 获取国际化消息管理器
     * @return 国际化消息管理器引用
     */
    const I18nMessages &get_i18n() const { return *i18n; }
};

#endif // CZC_DIAGNOSTIC_HPP
