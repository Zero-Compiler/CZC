/**
 * @file lexer.hpp
 * @brief Lexer 主类，门面模式协调各扫描器。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-29
 *
 * @details
 *   Lexer 是词法分析器的主入口，采用门面模式协调各扫描器。
 *   提供两种工作模式：
 *   - 基础模式: 跳过空白和注释，仅返回有意义的 Token
 *   - Trivia 模式: 保留空白和注释作为 Token 的 trivia 附件
 *
 *   设计特点：
 *   - 单遍扫描，O(n) 时间复杂度
 *   - 延迟错误收集，允许一次扫描报告所有错误
 *   - 组合优于继承，各扫描器独立实现
 *   - 支持多文件并发（不同文件使用不同 Lexer 实例）
 */

#ifndef CZC_LEXER_LEXER_HPP
#define CZC_LEXER_LEXER_HPP

#include "czc/common/config.hpp"

#include "czc/lexer/char_scanner.hpp"
#include "czc/lexer/comment_scanner.hpp"
#include "czc/lexer/ident_scanner.hpp"
#include "czc/lexer/lexer_error.hpp"
#include "czc/lexer/number_scanner.hpp"
#include "czc/lexer/scanner.hpp"
#include "czc/lexer/source_manager.hpp"
#include "czc/lexer/source_reader.hpp"
#include "czc/lexer/string_scanner.hpp"
#include "czc/lexer/token.hpp"

#include <span>
#include <vector>

namespace czc::lexer {

/**
 * @brief Lexer 主类。
 *
 * @details
 *   词法分析器的门面类，对外提供统一接口。
 *   内部协调多个专门的扫描器完成词法分析。
 *
 * @note 不可拷贝，可移动
 */
class Lexer {
public:
  /**
   * @brief 构造函数：接受 SourceManager 引用和 BufferID。
   *
   * @param sm SourceManager 引用
   * @param buffer 源码缓冲区 ID
   */
  explicit Lexer(SourceManager &sm, BufferID buffer);

  // 不可拷贝
  Lexer(const Lexer &) = delete;
  Lexer &operator=(const Lexer &) = delete;

  // 可移动（移动赋值因引用成员而删除）
  Lexer(Lexer &&) noexcept = default;
  Lexer &operator=(Lexer &&) noexcept = delete;

  ~Lexer() = default;

  /**
   * @brief 获取下一个 Token（基础模式）。
   *
   * @details
   *   跳过空白和注释，仅返回有意义的 Token。
   *   到达文件末尾时返回 TOKEN_EOF。
   *
   * @return 下一个 Token
   */
  [[nodiscard]] Token nextToken();

  /**
   * @brief 对整个源码进行词法分析（基础模式）。
   *
   * @details
   *   返回所有 Token，包括最后的 TOKEN_EOF。
   *
   * @return Token 列表
   */
  [[nodiscard]] std::vector<Token> tokenize();

  /**
   * @brief 获取下一个 Token（Trivia 模式）。
   *
   * @details
   *   保留空白和注释作为 Token 的 trivia 附件。
   *   用于 IDE/格式化器/语义高亮等高级工具。
   *
   * @return 下一个 Token（含 trivia）
   */
  [[nodiscard]] Token nextTokenWithTrivia();

  /**
   * @brief 对整个源码进行词法分析（Trivia 模式）。
   *
   * @details
   *   返回所有 Token，每个 Token 都带有相应的 trivia。
   *
   * @return Token 列表（含 trivia）
   */
  [[nodiscard]] std::vector<Token> tokenizeWithTrivia();

  /**
   * @brief 获取所有错误。
   *
   * @return 错误列表的 span 视图
   */
  [[nodiscard]] std::span<const LexerError> errors() const noexcept;

  /**
   * @brief 检查是否有错误。
   *
   * @return 若有错误返回 true
   */
  [[nodiscard]] bool hasErrors() const noexcept;

  /**
   * @brief 获取 SourceManager 引用。
   *
   * @return SourceManager 引用
   */
  [[nodiscard]] SourceManager &sourceManager() noexcept { return sm_; }

  /**
   * @brief 获取 SourceManager 常量引用。
   *
   * @return SourceManager 常量引用
   */
  [[nodiscard]] const SourceManager &sourceManager() const noexcept {
    return sm_;
  }

private:
  SourceManager &sm_;     ///< 源码管理器引用
  SourceReader reader_;   ///< 源码读取器
  ErrorCollector errors_; ///< 错误收集器

  // 扫描器实例
  IdentScanner identScanner_;     ///< 标识符扫描器
  NumberScanner numberScanner_;   ///< 数字扫描器
  StringScanner stringScanner_;   ///< 字符串扫描器
  CommentScanner commentScanner_; ///< 注释扫描器
  CharScanner charScanner_;       ///< 字符扫描器

  /**
   * @brief 跳过空白字符。
   */
  void skipWhitespace();

  /**
   * @brief 跳过空白和注释。
   */
  void skipWhitespaceAndComments();

  /**
   * @brief 收集前置 Trivia。
   *
   * @return Trivia 列表
   */
  [[nodiscard]] std::vector<Trivia> collectLeadingTrivia();

  /**
   * @brief 收集后置 Trivia。
   *
   * @return Trivia 列表
   */
  [[nodiscard]] std::vector<Trivia> collectTrailingTrivia();

  /**
   * @brief 内部扫描单个 Token。
   *
   * @return 扫描得到的 Token
   */
  [[nodiscard]] Token scanToken();

  /**
   * @brief 扫描未知字符。
   *
   * @param ctx 扫描上下文
   * @return Unknown Token
   */
  [[nodiscard]] Token scanUnknown(ScanContext &ctx);

  /**
   * @brief 规范化换行符（\r\n -> \\n）。(这里多了一个反斜杠,防止被解析)
   *
   * @details
   *   在 advance 时自动处理，将 Windows 风格换行转换为 Unix 风格。
   */
  void normalizeNewlines();
};

} // namespace czc::lexer

#endif // CZC_LEXER_LEXER_HPP
