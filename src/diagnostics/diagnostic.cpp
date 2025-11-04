/**
 * @file diagnostic.cpp
 * @brief 诊断系统实现
 * @author BegoniaHe
 */

#include "czc/diagnostics/diagnostic.hpp"
#include "czc/diagnostics/color.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <filesystem>
#include <cstdlib>

using namespace czc::diagnostics;

/**
 * @brief I18nMessages 构造函数
 * @param locale 区域设置标识符
 */
I18nMessages::I18nMessages(const std::string &locale)
    : current_locale(locale)
{
    if (!load_from_file(locale))
    {
        load_from_file("en_US");
    }
}

/**
 * @brief 从文件加载国际化消息
 * @param locale 区域设置标识符
 * @return 如果加载成功返回 true, 否则返回 false
 */
bool I18nMessages::load_from_file(const std::string &locale)
{
    std::vector<std::string> search_paths;

    // 检查环境变量 ZERO_LOCALE_PATH
    const char *env_path = std::getenv("ZERO_LOCALE_PATH");
    if (env_path != nullptr && env_path[0] != '\0')
    {
        std::string base_path(env_path);
        if (!base_path.empty() && base_path.back() == '/')
        {
            base_path.pop_back();
        }
        search_paths.push_back(base_path + "/" + locale + "/diagnostics.toml");
    }

    search_paths.push_back("locales/" + locale + "/diagnostics.toml");
    search_paths.push_back("../locales/" + locale + "/diagnostics.toml");
    search_paths.push_back("../../locales/" + locale + "/diagnostics.toml");

    std::string filepath;
    for (const auto &path : search_paths)
    {
        if (std::filesystem::exists(path))
        {
            filepath = path;
            break;
        }
    }

    if (filepath.empty())
    {
        return false;
    }

    std::ifstream file(filepath);
    if (!file.is_open())
    {
        return false;
    }

    messages.clear();

    std::string line;
    std::string current_code;
    MessageTemplate current_template;

    while (std::getline(file, line))
    {
        if (line.empty() || line[0] == '#')
            continue;

        // 解析 [L0001] 格式的代码
        if (line[0] == '[')
        {
            if (!current_code.empty())
            {
                messages[current_code] = current_template;
                current_template = MessageTemplate{};
            }

            size_t end = line.find(']');
            if (end != std::string::npos)
            {
                current_code = line.substr(1, end - 1);
            }
            continue;
        }

        // 解析键值对
        size_t eq_pos = line.find('=');
        if (eq_pos == std::string::npos)
            continue;

        std::string key = line.substr(0, eq_pos);
        std::string value = line.substr(eq_pos + 1);

        // 去除前后空格和引号
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);

        value.erase(0, value.find_first_not_of(" \t\""));
        value.erase(value.find_last_not_of(" \t\"") + 1);

        if (key == "message")
        {
            current_template.message = value;
        }
        else if (key == "help")
        {
            current_template.help = value;
        }
        else if (key == "source")
        {
            current_template.source = value;
        }
    }

    if (!current_code.empty())
    {
        messages[current_code] = current_template;
    }

    return !messages.empty();
}

/**
 * @brief 设置当前区域
 * @param locale 区域设置标识符
 */
void I18nMessages::set_locale(const std::string &locale)
{
    current_locale = locale;
    if (!load_from_file(locale))
    {
        load_from_file("en_US");
    }
}

/**
 * @brief 获取诊断代码对应的消息模板
 * @param code 诊断代码
 * @return 消息模板的常量引用
 */
const MessageTemplate &I18nMessages::get_message(DiagnosticCode code) const
{
    std::string code_str = diagnostic_code_to_string(code);
    auto it = messages.find(code_str);

    if (it == messages.end())
    {
        static MessageTemplate unknown{"unknown error", "", "system"};
        return unknown;
    }

    return it->second;
}

/**
 * @brief 格式化诊断消息
 * @param code 诊断代码
 * @param args 格式化参数
 * @return 格式化后的消息字符串
 */
std::string I18nMessages::format_message(DiagnosticCode code,
                                         const std::vector<std::string> &args) const
{
    const auto &tmpl = get_message(code);
    std::string result = tmpl.message;

    // 替换占位符 {0}, {1}, ...
    for (size_t i = 0; i < args.size(); ++i)
    {
        std::string placeholder = "{" + std::to_string(i) + "}";
        size_t pos = 0;
        while ((pos = result.find(placeholder, pos)) != std::string::npos)
        {
            result.replace(pos, placeholder.length(), args[i]);
            pos += args[i].length();
        }
    }

    return result;
}

/**
 * @brief 格式化诊断信息
 * @param i18n 国际化消息管理器
 * @param use_color 是否使用颜色
 * @return 格式化后的诊断字符串
 */
std::string Diagnostic::format(const I18nMessages &i18n, bool use_color) const
{
    std::ostringstream oss;
    const auto &tmpl = i18n.get_message(code);

    // 1. 错误头：error[XXXXXX]: message (from: XXXX)
    if (use_color)
    {
        oss << Color::Bold;
        oss << (level == DiagnosticLevel::Error ? Color::Red : level == DiagnosticLevel::Warning ? Color::Yellow
                                                                                                 : Color::Cyan);
    }

    oss << (level == DiagnosticLevel::Error ? "error" : level == DiagnosticLevel::Warning ? "warning"
                                                                                          : "note");

    oss << "[" << diagnostic_code_to_string(code) << "]";
    if (use_color)
        oss << Color::Reset;

    oss << ": ";
    if (use_color)
        oss << Color::Bold;
    oss << i18n.format_message(code, args);
    if (use_color)
        oss << Color::Reset;

    oss << " (from: " << tmpl.source << ")";
    oss << "\n";

    if (!location.filename.empty())
    {
        if (use_color)
            oss << Color::Blue << Color::Bold;
        oss << "  --> ";
        if (use_color)
            oss << Color::Reset;
        oss << location.filename << ":" << location.line << ":" << location.column << "\n";
    }

    if (!source_line.empty())
    {
        int line_width = std::to_string(location.line).length();
        if (line_width < 3)
            line_width = 3;

        if (use_color)
            oss << Color::Blue << Color::Bold;
        oss << std::string(line_width, ' ') << " |\n";
        oss << std::setw(line_width) << location.line << " | ";
        if (use_color)
            oss << Color::Reset;
        oss << source_line << "\n";

        if (use_color)
            oss << Color::Blue << Color::Bold;
        oss << std::string(line_width, ' ') << " | ";
        if (use_color)
            oss << Color::Reset;

        size_t spaces = location.column > 0 ? location.column - 1 : 0;
        oss << std::string(spaces, ' ');

        if (use_color)
            oss << Color::Red << Color::Bold;
        size_t length = location.end_column > location.column ? location.end_column - location.column : 1;
        oss << std::string(length, '^');
        if (use_color)
            oss << Color::Reset;
        oss << "\n";
    }

    // 4. Help
    if (!tmpl.help.empty())
    {
        if (use_color)
            oss << Color::Cyan << Color::Bold;
        oss << "   = help: ";
        if (use_color)
            oss << Color::Reset;
        oss << tmpl.help << "\n";
    }

    return oss.str();
}

/**
 * @brief DiagnosticEngine 构造函数
 * @param locale 区域设置标识符
 */
DiagnosticEngine::DiagnosticEngine(const std::string &locale)
    : i18n(std::make_shared<I18nMessages>(locale)) {}

/**
 * @brief 报告一个诊断信息
 * @param diag 诊断对象的共享指针
 */
void DiagnosticEngine::report(std::shared_ptr<Diagnostic> diag)
{
    if (!diag)
        return;

    if (diag->get_level() == DiagnosticLevel::Error ||
        diag->get_level() == DiagnosticLevel::Fatal)
    {
        error_count++;
    }
    else if (diag->get_level() == DiagnosticLevel::Warning)
    {
        warning_count++;
    }

    diagnostics.push_back(diag);
}

/**
 * @brief 打印所有诊断信息
 * @param use_color 是否使用颜色
 */
void DiagnosticEngine::print_all(bool use_color) const
{
    for (const auto &diag : diagnostics)
    {
        std::cerr << diag->format(*i18n, use_color);
    }

    if (error_count > 0)
    {
        std::cerr << "\nerror: aborting due to " << error_count
                  << " previous error" << (error_count > 1 ? "s" : "") << "\n";
    }
}
