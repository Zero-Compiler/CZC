/**
 * @file format_visitor.hpp
 * @brief 定义了格式化访问者接口，用于实现访问者模式格式化 CST 节点。
 * @author BegoniaHe
 * @date 2025-11-11
 */

#ifndef CZC_FORMAT_VISITOR_HPP
#define CZC_FORMAT_VISITOR_HPP

#include "czc/cst/cst_node.hpp"

#include <string>

namespace czc {
namespace formatter {

/**
 * @brief CST 节点格式化访问者接口。
 * @details
 *   采用访问者模式设计，每种 CST 节点类型对应一个 visit 方法。
 *   相比巨大的 switch-case，访问者模式符合开闭原则，更易于扩展和维护。
 */
class FormatVisitor {
public:
  virtual ~FormatVisitor() = default;

  // --- 程序结构 ---
  virtual std::string visit_program(const cst::CSTNode* node) = 0;

  // --- 声明 ---

  /**
   * @brief 访问变量声明节点。
   * @param[in] node 变量声明节点。
   * @return 格式化后的字符串。
   * @details
   *   变量声明用于在当前作用域中创建新的变量绑定。
   *   支持 let（不可变）和 var（可变）两种声明方式。
   *   格式化时需要处理关键字、标识符、可选的类型注解、初始化表达式和分号。
   *   关键字与标识符之间、等号两侧都需要添加空格。
   *   如果存在行内注释，需要在分号后添加两个空格再跟注释。
   * @note 格式: let/var name: type = expr;
   * @example
   *   let x = 10;
   *   var count: Integer = 0;
   *   let message = "Hello";
   *   var total = a + b;  // 计算总和
   */
  virtual std::string visit_var_declaration(const cst::CSTNode* node) = 0;

  /**
   * @brief 访问函数声明节点。
   * @param[in] node 函数声明节点。
   * @return 格式化后的字符串。
   * @details
   *   函数声明定义了一个可重用的代码块，包含名称、参数列表、可选的返回类型和函数体。
   *   格式化时需要处理多个组成部分：
   *   - fn 关键字后需要添加空格
   *   - 函数名紧跟参数列表的括号（括号前不加空格）
   *   - 参数列表中的参数用逗号和空格分隔
   *   - 可选的返回类型箭头（->）两侧添加空格
   *   - 函数体的左花括号前添加空格
   *   - 函数体内的语句遵循代码块的缩进规则
   * @note 格式: fn name(params) -> type { body }
   * @example
   *   fn add(a: Integer, b: Integer) -> Integer {
   *     return a + b;
   *   }
   *
   *   fn greet(name: String) {
   *     print("Hello, " + name);
   *   }
   *
   *   fn calculate(x: Float, y: Float, op: String) -> Float {
   *     if (op == "+") {
   *       return x + y;
   *     }
   *     return x - y;
   *   }
   */
  virtual std::string visit_fn_declaration(const cst::CSTNode* node) = 0;

  // --- 语句 ---

  /**
   * @brief 访问返回语句节点。
   * @param[in] node 返回语句节点。
   * @return 格式化后的字符串。
   * @details
   *   返回语句用于从函数中返回值并终止函数执行。
   * @note 格式: return expr;
   * @example
   *   return 42;
   *   return a + b;
   *   return calculate(x, y);
   */
  virtual std::string visit_return_stmt(const cst::CSTNode* node) = 0;

  /**
   * @brief 访问条件语句节点。
   * @param[in] node 条件语句节点。
   * @return 格式化后的字符串。
   * @details
   *   条件语句根据条件表达式的真假值来决定执行哪个代码块。
   *   格式化时需要注意括号前的空格、花括号的位置等细节。
   * @note 格式: if (condition) { statements } [else { statements }]
   * @example
   *   if (x > 0) {
   *     print(x);
   *   }
   *
   *   if (a == b) {
   *     return true;
   *   } else {
   *     return false;
   *   }
   */
  virtual std::string visit_if_stmt(const cst::CSTNode* node) = 0;

  /**
   * @brief 访问循环语句节点。
   * @param[in] node 循环语句节点。
   * @return 格式化后的字符串。
   * @details
   *   while 循环在条件为真时重复执行代码块。
   *   格式化规则与 if 语句类似，需要处理条件表达式的括号和代码块的花括号。
   * @note 格式: while (condition) { statements }
   * @example
   *   while (i < 10) {
   *     i = i + 1;
   *     print(i);
   *   }
   */
  virtual std::string visit_while_stmt(const cst::CSTNode* node) = 0;

  /**
   * @brief 访问代码块语句节点。
   * @param[in] node 代码块节点。
   * @return 格式化后的字符串。
   * @details
   *   代码块是由花括号包围的语句序列，定义了一个新的作用域。
   *   格式化时需要管理缩进级别：进入代码块时增加缩进，退出时减少缩进。
   *   左花括号后换行，右花括号前减少缩进并独占一行。
   * @note 格式: { statements }
   * @example
   *   {
   *     let x = 10;
   *     let y = 20;
   *     print(x + y);
   *   }
   */
  virtual std::string visit_block_stmt(const cst::CSTNode* node) = 0;

  /**
   * @brief 访问表达式语句节点。
   * @param[in] node 表达式语句节点。
   * @return 格式化后的字符串。
   * @details
   *   表达式语句是单独成行的表达式，通常是函数调用、赋值或其他有副作用的表达式。
   *   格式化时需要在行首添加正确的缩进，并在行末添加分号和换行符。
   *   需要处理行内注释的情况。
   * @note 格式: expression;
   * @example
   *   print("Hello");
   *   a = b + c;
   *   calculate(x, y);
   *   arr[0] = 42;  // 行内注释
   */
  virtual std::string visit_expr_stmt(const cst::CSTNode* node) = 0;

  // --- 表达式 ---

  /**
   * @brief 访问二元表达式节点。
   * @param[in] node 二元表达式节点。
   * @return 格式化后的字符串。
   * @details
   *   二元表达式由两个操作数和一个中缀运算符组成。
   *   格式化时需要在运算符两侧添加空格以提高可读性。
   *   支持算术运算符（+, -, *, /, %）、比较运算符（==, !=, <, <=, >, >=）
   *   和逻辑运算符（&&, ||）。
   * @note 格式: left operator right
   * @example
   *   a + b
   *   x * y - z
   *   count >= 10
   *   isValid && isActive
   */
  virtual std::string visit_binary_expr(const cst::CSTNode* node) = 0;

  /**
   * @brief 访问一元表达式节点。
   * @param[in] node 一元表达式节点。
   * @return 格式化后的字符串。
   * @details
   *   一元表达式由一个运算符和一个操作数组成。
   *   运算符通常紧贴操作数，不添加额外空格。
   *   支持逻辑非（!）和负号（-）运算符。
   * @note 格式: operator operand
   * @example
   *   !flag
   *   -value
   *   !isEnabled
   */
  virtual std::string visit_unary_expr(const cst::CSTNode* node) = 0;

  /**
   * @brief 访问函数调用表达式节点。
   * @param[in] node 函数调用节点。
   * @return 格式化后的字符串。
   * @details
   *   函数调用表达式由被调用者（通常是标识符或成员表达式）和参数列表组成。
   *   格式化时保持函数名与左括号之间无空格，参数之间用逗号和空格分隔。
   * @note 格式: callee(arg1, arg2, ...)
   * @example
   *   print("Hello")
   *   calculate(x, y, z)
   *   math.sqrt(16)
   */
  virtual std::string visit_call_expr(const cst::CSTNode* node) = 0;

  /**
   * @brief 访问索引访问表达式节点。
   * @param[in] node 索引访问节点。
   * @return 格式化后的字符串。
   * @details
   *   索引访问用于访问数组或类似容器的元素。
   *   格式化时方括号紧贴对象和索引表达式，不添加额外空格。
   * @note 格式: object[index]
   * @example
   *   arr[0]
   *   matrix[i][j]
   *   data[count - 1]
   */
  virtual std::string visit_index_expr(const cst::CSTNode* node) = 0;

  /**
   * @brief 访问成员访问表达式节点。
   * @param[in] node 成员访问节点。
   * @return 格式化后的字符串。
   * @details
   *   成员访问用于访问对象的属性或方法。
   *   格式化时点号两侧不添加空格，保持紧凑格式。
   * @note 格式: object.member
   * @example
   *   person.name
   *   math.pi
   *   config.settings.timeout
   */
  virtual std::string visit_member_expr(const cst::CSTNode* node) = 0;

  /**
   * @brief 访问赋值表达式节点。
   * @param[in] node 赋值表达式节点。
   * @return 格式化后的字符串。
   * @details
   *   赋值表达式将右侧的值赋给左侧的变量。
   *   格式化时在等号两侧添加空格以提高可读性。
   *   支持简单赋值和复合赋值运算符。
   * @note 格式: lvalue = rvalue
   * @example
   *   x = 10
   *   name = "Alice"
   *   result = a + b
   */
  virtual std::string visit_assign_expr(const cst::CSTNode* node) = 0;

  /**
   * @brief 访问索引赋值表达式节点。
   * @param[in] node 索引赋值节点。
   * @return 格式化后的字符串。
   * @details
   *   索引赋值用于修改数组或容器中特定位置的元素值。
   *   格式化时方括号紧贴对象和索引，等号两侧添加空格。
   * @note 格式: object[index] = value
   * @example
   *   arr[0] = 42
   *   matrix[i][j] = 0
   *   data[key] = "value"
   */
  virtual std::string visit_index_assign_expr(const cst::CSTNode* node) = 0;

  /**
   * @brief 访问数组字面量节点。
   * @param[in] node 数组字面量节点。
   * @return 格式化后的字符串。
   * @details
   *   数组字面量是用方括号包围的元素列表。
   *   格式化时元素之间用逗号分隔，逗号后根据配置选项添加空格。
   *   方括号与内容之间不添加空格。
   * @note 格式: [elem1, elem2, elem3, ...]
   * @example
   *   [1, 2, 3]
   *   ["a", "b", "c"]
   *   [x + 1, y * 2, z - 3]
   */
  virtual std::string visit_array_literal(const cst::CSTNode* node) = 0;

  /**
   * @brief 访问括号表达式节点。
   * @param[in] node 括号表达式节点。
   * @return 格式化后的字符串。
   * @details
   *   括号表达式用于改变运算优先级或提高代码可读性。
   *   格式化时保持括号内外的表达式格式，括号与内容之间不添加空格。
   * @note 格式: (expression)
   * @example
   *   (a + b)
   *   (x * y) + z
   *   ((a + b) * c)
   */
  virtual std::string visit_paren_expr(const cst::CSTNode* node) = 0;

  // --- 字面量 ---
  virtual std::string visit_integer_literal(const cst::CSTNode* node) = 0;
  virtual std::string visit_float_literal(const cst::CSTNode* node) = 0;
  virtual std::string visit_string_literal(const cst::CSTNode* node) = 0;
  virtual std::string visit_boolean_literal(const cst::CSTNode* node) = 0;
  virtual std::string visit_identifier(const cst::CSTNode* node) = 0;

  // --- 类型 ---
  virtual std::string visit_type_annotation(const cst::CSTNode* node) = 0;
  virtual std::string visit_array_type(const cst::CSTNode* node) = 0;

  // --- 参数和列表 ---
  virtual std::string visit_parameter(const cst::CSTNode* node) = 0;
  virtual std::string visit_parameter_list(const cst::CSTNode* node) = 0;
  virtual std::string visit_argument_list(const cst::CSTNode* node) = 0;
  virtual std::string visit_statement_list(const cst::CSTNode* node) = 0;

  // --- 符号 ---
  virtual std::string visit_operator(const cst::CSTNode* node) = 0;
  virtual std::string visit_delimiter(const cst::CSTNode* node) = 0;
  virtual std::string visit_comment(const cst::CSTNode* node) = 0;
};

} // namespace formatter
} // namespace czc

#endif // CZC_FORMAT_VISITOR_HPP
