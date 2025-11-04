/**
 * @file token.hpp
 * @brief Token 类型和类定义
 * @author BegoniaHe
 */

#ifndef CZC_LEXER_TOKEN_HPP
#define CZC_LEXER_TOKEN_HPP

#include <string>
#include <optional>

/**
 * @brief Token 类型枚举
 */
enum class TokenType
{
    // 字面量
    Integer,            ///< 整数字面量
    Float,              ///< 浮点数字面量
    String,             ///< 字符串字面量
    Identifier,         ///< 标识符
    ScientificExponent, ///< 科学计数法字面量

    // 关键字
    Let,    ///< let 关键字
    Var,    ///< var 关键字
    Fn,     ///< fn 关键字
    Return, ///< return 关键字
    If,     ///< if 关键字
    Else,   ///< else 关键字
    While,  ///< while 关键字
    For,    ///< for 关键字
    In,     ///< in 关键字
    True,   ///< true 关键字
    False,  ///< false 关键字

    // 运算符
    Plus,    ///< +
    Minus,   ///< -
    Star,    ///< *
    Slash,   ///< /
    Percent, ///< %

    // 赋值与比较运算符
    Equal,        ///< =
    PlusEqual,    ///< +=
    MinusEqual,   ///< -=
    StarEqual,    ///< *=
    PercentEqual, ///< %=
    SlashEqual,   ///< /=
    EqualEqual,   ///< ==
    BangEqual,    ///< !=
    Less,         ///< <
    LessEqual,    ///< <=
    Greater,      ///< >
    GreaterEqual, ///< >=

    // 逻辑运算符
    And,  ///< &&
    Or,   ///< ||
    Bang, ///< !

    // 分隔符
    LeftParen,    ///< (
    RightParen,   ///< )
    LeftBrace,    ///< {
    RightBrace,   ///< }
    LeftBracket,  ///< [
    RightBracket, ///< ]
    Comma,        ///< ,
    Semicolon,    ///< ;
    Colon,        ///< :
    Dot,          ///< .
    DotDot,       ///< ..

    // 特殊
    EndOfFile, ///< 文件结束
    Unknown,   ///< 未知 Token
};

/**
 * @brief Token 类
 */
class Token
{
public:
    TokenType token_type; ///< Token 类型
    std::string value;    ///< Token 值
    size_t line;          ///< 行号
    size_t column;        ///< 列号

    /**
     * @brief 构造函数
     * @param type Token 类型
     * @param val Token 值
     * @param line 行号，默认为 0
     * @param column 列号，默认为 0
     */
    Token(TokenType type, const std::string &val, size_t line = 0, size_t column = 0);
};

/**
 * @brief 获取关键字对应的 Token 类型
 * @param word 关键字字符串
 * @return Token 类型的可选值
 */
std::optional<TokenType> get_keyword(const std::string &word);

/**
 * @brief 将 Token 类型转换为字符串表示
 * @param type Token 类型
 * @return Token 类型的字符串表示
 */
std::string token_type_to_string(TokenType type);

#endif // CZC_LEXER_TOKEN_HPP
