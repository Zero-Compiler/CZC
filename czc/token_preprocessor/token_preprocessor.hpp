/**
 * @file token_preprocessor.hpp
 * @brief 定义了 `TokenPreprocessor`，用于在语法分析前对 Token 流进行细化。
 * @author BegoniaHe
 * @date 2025-11-05
 */

#ifndef CZC_TOKEN_PREPROCESSOR_HPP
#define CZC_TOKEN_PREPROCESSOR_HPP

#include "czc/diagnostics/diagnostic_reporter.hpp"
#include "czc/lexer/token.hpp"
#include "error_collector.hpp"
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace czc {
namespace token_preprocessor {

/**
 * @brief `int64_t` 的最大值约为 9e18，因此整数部分超过 18 位可能溢出。
 * @details 用于在类型推断时快速检查潜在的整数溢出。
 */
constexpr int MAX_I64_MAGNITUDE = 18;
/**
 * @brief IEEE 754 双精度浮点数 (double) 的最大指数约为 10^308。
 * @details 用于在解析阶段检测潜在的浮点数溢出。
 */
constexpr int MAX_F64_MAGNITUDE = 308;

/**
 * @brief
 *   表示根据科学计数法字面量的形式和值推断出的最终数值类型。
 */
enum class InferredNumericType {
  INT64, // 推断为 64 位整数。
  FLOAT  // 推断为 64 位浮点数。
};

/**
 * @brief 存储对科学计数法字面量进行详细分析后提取出的信息。
 * @property {数据成员} 这是一个纯数据结构 (POD-like)。
 */
struct ScientificNotationInfo {
  // 原始字符串, e.g., "1.5e10"。
  std::string original_literal;
  // 尾数部分, e.g., "1.5"。
  std::string mantissa;
  // 指数部分, e.g., 10。
  int64_t exponent;
  // 尾数是否包含小数点。
  bool has_decimal_point;
  // 小数点后的有效位数（去除尾随0后）。
  size_t decimal_digits;
  // 根据规则推断出的类型。
  InferredNumericType inferred_type;
  // 规范化后的值（用于后续计算）。
  std::string normalized_value;
};

/**
 * @brief 封装了执行分析所需的上下文信息。
 * @details
 *   将这些信息打包在一起，可以简化向分析函数传递参数的过程。
 */
struct AnalysisContext {
  // 当前分析的文件名。
  const std::string &filename;
  // 当前文件的完整源码内容。
  const std::string &source_content;
  // 用于报告错误的收集器实例。
  TPErrorCollector *error_collector;

  /**
   * @brief 构造一个新的分析上下文。
   *
   * @param[in] fname     文件名（第一个字符串参数）。
   * @param[in] source    源码内容（第二个字符串参数）。
   * @param[in] collector (可选) 错误收集器指针。
   *
   * @warning 前两个参数都是字符串引用，容易混淆。
   *          正确顺序为: 文件名, 源码内容, 错误收集器。
   */
  AnalysisContext(const std::string &fname, const std::string &source,
                  TPErrorCollector *collector = nullptr)
      : filename(fname), source_content(source), error_collector(collector) {}
};

/**
 * @brief 提供用于分析科学计数法字面量的静态工具函数集。
 * @details
 *   此类封装了处理科学计数法字面量（如 `1.23e-10`）的所有复杂逻辑。
 *   由于词法分析器在扫描阶段无法确定一个科学计数法数字最终应该是整数还是浮点数，
 *   也无法进行溢出检查，因此这些职责被委托给了这个分析器。它负责：
 *   1.  **分解**: 将字面量分解为尾数和指数。
 *   2.  **类型推断**: 根据尾数是否包含小数点以及指数的大小，推断其应为 `INT64`
 * 还是 `FLOAT`。
 *   3.  **溢出检查**: 估算数值的数量级，以判断其是否超出 `int64_t` 或 `double`
 * 的表示范围。
 *
 * @note 此类是无状态的，所有方法均为静态，不应被实例化。
 */
class ScientificNotationAnalyzer {
public:
  /**
   * @brief 对科学计数法字面量进行完整分析。
   * @param[in] literal 要分析的字面量字符串。
   * @param[in] token   与字面量关联的 Token，用于错误报告。
   * @param[in] context 当前的分析上下文。
   * @return 如果分析成功，返回包含详细信息的 `ScientificNotationInfo`；
   *         如果发生错误（如格式错误、溢出），则返回 `std::nullopt`。
   */
  static std::optional<ScientificNotationInfo>
  analyze(const std::string &literal, const lexer::Token *token,
          const AnalysisContext &context);

private:
  /**
   * @brief 从字面量中解析出尾数和指数部分。
   * @param[in]  literal  字面量字符串。
   * @param[out] mantissa 解析出的尾数。
   * @param[out] exponent 解析出的指数。
   * @return 如果解析成功，返回 `true`。
   */
  static bool parse_components(const std::string &literal,
                               std::string &mantissa, int64_t &exponent);

  /**
   * @brief 去除小数部分的尾随零。
   * @param[in] decimal_part 小数部分字符串。
   * @return 去除尾随零后的字符串。
   */
  static std::string trim_trailing_zeros(const std::string &decimal_part);

  /**
   * @brief 计算去除尾随零后的小数位数。
   * @param[in] mantissa 尾数字符串。
   * @return 小数位数。
   */
  static size_t count_decimal_digits(const std::string &mantissa);

  /**
   * @brief 根据规则推断数值类型（INT64 或 FLOAT）。
   * @param[in] info    已部分填充的科学计数法信息。
   * @param[in] token   关联的 Token。
   * @param[in] context 分析上下文。
   * @return 推断出的数值类型。
   */
  static InferredNumericType infer_type(const ScientificNotationInfo &info,
                                        const lexer::Token *token,
                                        const AnalysisContext &context);

  /**
   * @brief 检查一个潜在的整数值是否在 `int64_t` 的表示范围内。
   * @param[in] mantissa 尾数字符串。
   * @param[in] exponent 指数。
   * @param[in] token    关联的 Token。
   * @param[in] context  分析上下文。
   * @return 如果值适合 `int64_t`，则返回 `true`。
   */
  static bool fits_in_int64(const std::string &mantissa, int64_t exponent,
                            const lexer::Token *token,
                            const AnalysisContext &context);

  /**
   * @brief 计算数值的“数量级”（大致的位数）。
   * @param[in] mantissa 尾数字符串。
   * @param[in] exponent 指数。
   * @param[in] token    关联的 Token。
   * @param[in] context  分析上下文。
   * @return 返回计算出的数量级。
   */
  static std::optional<int64_t>
  calculate_magnitude(const std::string &mantissa, int64_t exponent,
                      const lexer::Token *token,
                      const AnalysisContext &context);

  /**
   * @brief 报告一个数值溢出错误。
   * @param[in] token    关联的 Token。
   * @param[in] mantissa 尾数字符串。
   * @param[in] exponent 指数。
   * @param[in] context  分析上下文。
   */
  static void report_overflow(const lexer::Token *token,
                              const std::string &mantissa, int64_t exponent,
                              const AnalysisContext &context);
};

/**
 * @brief 在语法分析前对 Token 流进行分析、转换和细化。
 * @details
 *   此预处理器是介于词法分析和语法分析之间的一个重要阶段。它的主要职责是
 *   处理那些在词法分析阶段无法完全确定的语法单元。目前，其核心功能是
 *   **科学计数法字面量的类型推断**。
 *
 *   词法分析器会将所有科学计数法形式的数字（例如 `1.23e10`）统一识别为
 *   临时的 `ScientificExponent` 类型。此预处理器随后会遍历 Token 流，
 *   利用 `ScientificNotationAnalyzer` 对这些字面量进行深度分析，最终将
 *   它们的类型精确地转换为 `Integer` 或
 * `Float`，并在此过程中捕获数值溢出等错误。
 *
 * @property {设计} 这种分离的设计使得词法分析器可以保持简单和高速，将复杂的
 *   数值分析逻辑解耦到这个专门的模块中。
 * @property {线程安全} 非线程安全。
 */
class TokenPreprocessor {
private:
  // 用于收集在预处理期间遇到的所有错误。
  TPErrorCollector error_collector;

  /**
   * @brief 辅助函数，用于在错误收集器中记录一个新错误。
   * @param[in] code  错误的诊断代码。
   * @param[in] token 发生错误的 Token。
   * @param[in] args  (可选) 格式化错误消息所需的参数。
   */
  void report_error(diagnostics::DiagnosticCode code, const lexer::Token *token,
                    const std::vector<std::string> &args = {});

public:
  /**
   * @brief 默认构造函数。
   */
  TokenPreprocessor() = default;

  /**
   * @brief 处理一个完整的 Token 列表。
   * @details
   *   遍历输入的 Token 向量，当遇到 `ScientificExponent` 类型的 Token 时，
   *   调用 `process_scientific_token` 对其进行处理和转换。
   * @param[in] tokens         从词法分析器获得的原始 Token 列表。
   * @param[in] filename       源代码的文件名。
   * @param[in] source_content 完整的源代码内容。
   * @return 返回经过处理和类型调整的 Token 列表。
   */
  std::vector<lexer::Token> process(const std::vector<lexer::Token> &tokens,
                                    const std::string &filename,
                                    const std::string &source_content);

  /**
   * @brief 分析并转换单个科学计数法 Token。
   * @details
   *   使用 ScientificNotationAnalyzer 对 Token 的值进行分析，推断其类型
   *   （INT64 或 FLOAT），并检查是否存在溢出。
   * @param[in] token          一个类型为 `ScientificExponent` 的 Token。
   * @param[in] filename       源代码的文件名。
   * @param[in] source_content 完整的源代码内容。
   * @return 返回类型被更新为 `Integer` 或 `Float` 的新 Token。
   */
  lexer::Token process_scientific_token(const lexer::Token &token,
                                        const std::string &filename,
                                        const std::string &source_content);

  /**
   * @brief 获取对内部错误收集器的只读访问权限。
   * @return 对 TPErrorCollector 对象的常量引用。
   */
  const TPErrorCollector &get_errors() const { return error_collector; }

private:
  /**
   * @brief 将内部的 InferredNumericType 映射到词法分析器的 TokenType。
   * @param[in] type 推断出的数值类型。
   * @return 对应的 `lexer::TokenType` (`Integer` 或 `Float`)。
   */
  static lexer::TokenType inferred_type_to_token_type(InferredNumericType type);
};

/**
 * @brief 将 InferredNumericType 转换为字符串表示。
 * @param[in] type 推断的数值类型。
 * @return 类型的字符串表示（"INT64" 或 "FLOAT"）。
 */
std::string inferred_type_to_string(InferredNumericType type);

} // namespace token_preprocessor
} // namespace czc

#endif // CZC_TOKEN_PREPROCESSOR_HPP
