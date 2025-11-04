/**
 * @file error_collector.hpp
 * @brief 词法分析错误收集器类定义
 * @author BegoniaHe
 * @date 2025-11-04
 */

#ifndef CZC_LEX_ERROR_COLLECTOR_HPP
#define CZC_LEX_ERROR_COLLECTOR_HPP

#include "czc/diagnostics/diagnostic_code.hpp"
#include "czc/utils/source_location.hpp"
#include <vector>
#include <string>

/**
 * @brief 代表一个在词法分析阶段检测到的错误。
 * @details
 *   此结构体用于封装词法错误的所有相关信息，包括错误代码、
 *   发生错误的确切源码位置以及格式化错误消息所需的任何参数。
 */
struct LexerError
{
    /// @brief 标识错误类型的唯一代码。
    DiagnosticCode code;
    /// @brief 错误在源代码中的位置。
    SourceLocation location;
    /// @brief (可选) 用于生成详细错误消息的参数。
    std::vector<std::string> args;

    /**
     * @brief 构造一个新的词法错误记录。
     * @param[in] c 诊断代码。
     * @param[in] loc 源码位置。
     * @param[in] arguments (可选) 消息参数列表。
     */
    LexerError(DiagnosticCode c, const SourceLocation &loc,
               const std::vector<std::string> &arguments = {})
        : code(c), location(loc), args(arguments) {}
};

/**
 * @brief 收集并管理在词法分析过程中产生的所有错误。
 * @details
 *   此类提供了一个中心化的机制Í来记录词法分析器遇到的所有问题。
 *   词法分析器在检测到错误时，会调用 `add` 方法来记录错误，而不是立即中止。
 *   这允许词法分析过程继续进行，从而一次性报告多个错误。
 * @note 此类不是线程安全的。
 */
class LexErrorCollector
{
private:
    /// @brief 存储所有已报告的词法错误的列表。
    std::vector<LexerError> errors;

public:
    /**
     * @brief 向收集中添加一个新的词法错误。
     * @param[in] code 错误的诊断代码。
     * @param[in] loc 错误在源代码中的位置。
     * @param[in] args (可选) 用于格式化错误消息的参数。
     */
    void add(DiagnosticCode code, const SourceLocation &loc,
             const std::vector<std::string> &args = {});

    /**
     * @brief 获取所有已收集的错误。
     * @return 返回对内部错误列表的常量引用。
     */
    const std::vector<LexerError> &get_errors() const { return errors; }

    /**
     * @brief 检查是否收集到了任何错误。
     * @return 如果错误列表不为空，则返回 true。
     */
    bool has_errors() const { return !errors.empty(); }

    /**
     * @brief 清空所有已收集的错误。
     */
    void clear() { errors.clear(); }

    /**
     * @brief 获取当前收集到的错误总数。
     * @return 错误列表的大小。
     */
    size_t count() const { return errors.size(); }
};

#endif // CZC_LEX_ERROR_COLLECTOR_HPP
