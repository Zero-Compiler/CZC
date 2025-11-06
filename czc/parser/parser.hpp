/**
 * @file parser.hpp
 * @brief 定义了 `Parser` 类，负责将 Token 流转换为具体语法树（CST）。
 * @author BegoniaHe
 * @date 2025-11-05
 */

#ifndef CZC_PARSER_HPP
#define CZC_PARSER_HPP

#include "czc/cst/cst_node.hpp"
#include "czc/diagnostics/diagnostic_reporter.hpp"
#include "czc/lexer/token.hpp"
#include "error_collector.hpp"
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace czc {
namespace parser {

/**
 * @brief 负责将 Token 流转换为具体语法树（CST）的语法分析器。
 * @details
 *   此类采用经典的 **递归下降（Recursive Descent）** 算法实现。它消费
 *   由词法分析器生成的 Token 序列，并根据语言的语法规则构建一棵
 *   能够精确反映源码结构（包括所有标点和关键字）的具体语法树（CST）。
 *   选择生成 CST 而非 AST，是为了更好地支持源码格式化、精确的错误恢复
 *   以及未来可能的 IDE 集成功能。
 *
 * @property {设计} 这是一个有状态的解析器，通过内部索引 `current` 跟踪
 *   其在 Token 流中的处理进度。
 * @property {线程安全} 非线程安全。每个 `Parser` 实例都应由单个线程独占使用。
 */
class Parser {
public:
  /**
   * @brief 构造一个语法分析器。
   * @param[in] tokens Token 序列。
   */
  explicit Parser(const std::vector<lexer::Token> &tokens);

  /**
   * @brief 解析 Token 流并生成 CST。
   * @return 解析成功返回程序根节点，失败返回 nullptr。
   */
  std::unique_ptr<cst::CSTNode> parse();

  /**
   * @brief 获取解析过程中收集的所有错误。
   * @return 错误列表的常量引用。
   */
  const std::vector<ParserError> &get_errors() const {
    return error_collector.get_errors();
  }

  /**
   * @brief 检查是否有解析错误。
   * @return 如果有错误返回 true，否则返回 false。
   */
  bool has_errors() const { return error_collector.has_errors(); }

private:
  // --- Token 流管理 ---

  /**
   * @brief 获取当前 Token。
   * @return 当前 Token，如果到达末尾则返回 EOF Token。
   */
  lexer::Token current_token() const;

  /**
   * @brief 向前查看指定偏移量的 Token。
   * @param[in] offset 偏移量（0 表示当前 Token）。
   * @return 指定位置的 Token，如果超出范围则返回 EOF Token。
   */
  lexer::Token peek(size_t offset = 0) const;

  /**
   * @brief 前进到下一个 Token。
   * @return 前进前的当前 Token。
   */
  lexer::Token advance();

  /**
   * @brief 检查当前 Token 是否为指定类型。
   * @param[in] type 要检查的 Token 类型。
   * @return 如果匹配返回 true，否则返回 false。
   */
  bool check(lexer::TokenType type) const;

  /**
   * @brief 尝试匹配并消费当前 Token。
   * @param[in] types 允许的 Token 类型列表。
   * @return 如果匹配并消费了某个类型返回 true，否则返回 false。
   */
  bool match_token(const std::vector<lexer::TokenType> &types);

  /**
   * @brief 消费一个指定类型的 Token，如果不匹配则报错。
   * @param[in] type 期望的 Token 类型。
   * @param[in] message 错误消息。
   * @return 如果成功返回消费的 Token，否则返回 nullopt。
   */
  std::optional<lexer::Token> consume(lexer::TokenType type);

  // --- 错误处理 ---

  /**
   * @brief 记录一个解析错误。
   * @param[in] code 错误的诊断代码。
   * @param[in] location 错误位置。
   * @param[in] args 用于格式化错误消息的参数。
   */
  void report_error(diagnostics::DiagnosticCode code,
                    const utils::SourceLocation &location,
                    const std::vector<std::string> &args = {});

  /**
   * @brief 从当前 Token 创建源码位置。
   * @return 源码位置对象。
   */
  utils::SourceLocation make_location() const;

  // --- 语法规则解析方法 ---

  /**
   * @brief 解析声明（变量声明或函数声明）。
   * @return 声明节点。
   */
  std::unique_ptr<cst::CSTNode> declaration();

  /**
   * @brief 解析变量声明。
   * @details 语法：(let|var) identifier [: type] [= expression] ;
   * @return 变量声明节点。
   */
  std::unique_ptr<cst::CSTNode> var_declaration();

  /**
   * @brief 解析函数声明。
   * @details 语法：fn identifier ( parameters ) [-> type] { statements }
   * @return 函数声明节点。
   */
  std::unique_ptr<cst::CSTNode> fn_declaration();

  /**
   * @brief 解析类型注解。
   * @details 语法：int64 | float64 | string | bool | void | null | [type]
   * @return 类型注解节点。
   */
  std::unique_ptr<cst::CSTNode> parse_type();

  /**
   * @brief 解析语句。
   * @return 语句节点。
   */
  std::unique_ptr<cst::CSTNode> statement();

  /**
   * @brief 解析返回语句。
   * @details 语法：return [expression] ;
   * @return 返回语句节点。
   */
  std::unique_ptr<cst::CSTNode> return_statement();

  /**
   * @brief 解析条件语句。
   * @details 语法：if expression { statements } [else { statements }]
   * @return 条件语句节点。
   */
  std::unique_ptr<cst::CSTNode> if_statement();

  /**
   * @brief 解析循环语句。
   * @details 语法：while expression { statements }
   * @return 循环语句节点。
   */
  std::unique_ptr<cst::CSTNode> while_statement();

  /**
   * @brief 解析代码块。
   * @details 语法：{ statements }
   * @return 代码块节点。
   */
  std::unique_ptr<cst::CSTNode> block_statement();

  /**
   * @brief 解析表达式语句。
   * @details 语法：expression ;
   * @return 表达式语句节点。
   */
  std::unique_ptr<cst::CSTNode> expression_statement();

  // --- 表达式解析（按优先级递减） ---

  /**
   * @brief 解析表达式（入口）。
   * @return 表达式节点。
   */
  std::unique_ptr<cst::CSTNode> expression();

  /**
   * @brief 解析赋值表达式。
   * @details 语法：(identifier | index) = assignment
   * @return 赋值表达式节点。
   */
  std::unique_ptr<cst::CSTNode> assignment();

  /**
   * @brief 解析逻辑或表达式。
   * @details 语法：and (|| and)*
   * @return 逻辑或表达式节点。
   */
  std::unique_ptr<cst::CSTNode> logical_or();

  /**
   * @brief 解析逻辑与表达式。
   * @details 语法：equality (&& equality)*
   * @return 逻辑与表达式节点。
   */
  std::unique_ptr<cst::CSTNode> logical_and();

  /**
   * @brief 解析相等性表达式。
   * @details 语法：comparison ((== | !=) comparison)*
   * @return 相等性表达式节点。
   */
  std::unique_ptr<cst::CSTNode> equality();

  /**
   * @brief 解析比较表达式。
   * @details 语法：term ((< | <= | > | >=) term)*
   * @return 比较表达式节点。
   */
  std::unique_ptr<cst::CSTNode> comparison();

  /**
   * @brief 解析加减表达式。
   * @details 语法：factor ((+ | -) factor)*
   * @return 加减表达式节点。
   */
  std::unique_ptr<cst::CSTNode> term();

  /**
   * @brief 解析乘除取模表达式。
   * @details 语法：unary ((* | / | %) unary)*
   * @return 乘除取模表达式节点。
   */
  std::unique_ptr<cst::CSTNode> factor();

  /**
   * @brief 解析一元表达式。
   * @details 语法：(! | -) unary | call
   * @return 一元表达式节点。
   */
  std::unique_ptr<cst::CSTNode> unary();

  /**
   * @brief 解析函数调用和索引访问。
   * @details 语法：primary (( arguments ) | [ expression ])*
   * @return 调用或索引表达式节点。
   */
  std::unique_ptr<cst::CSTNode> call();

  /**
   * @brief 解析基本表达式。
   * @details
   *   语法：integer | float | string | boolean | identifier |
   *         ( expression ) | [ elements ]
   * @return 基本表达式节点。
   */
  std::unique_ptr<cst::CSTNode> primary();

  // --- 成员变量 ---

  // 从词法分析器接收到的、需要解析的 Token 序列。
  std::vector<lexer::Token> tokens;

  // 指向 `tokens` 向量中当前正在处理的 Token 的索引。
  size_t current;

  // 用于收集在语法分析期间遇到的所有语法错误。
  ParserErrorCollector error_collector;
};

} // namespace parser
} // namespace czc

#endif // CZC_PARSER_HPP
