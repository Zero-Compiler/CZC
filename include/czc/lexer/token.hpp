#ifndef CZC_LEXER_TOKEN_HPP
#define CZC_LEXER_TOKEN_HPP

#include <string>
#include <optional>

enum class TokenType
{
    // 字面量
    Integer,
    Float,
    String,
    Identifier,
    ScientificExponent,

    // 关键字
    Let,
    Var,
    Fn,
    Return,
    If,
    Else,
    While,
    For,
    In,
    True,
    False,

    // 运算符
    Plus,    // +
    Minus,   // -
    Star,    // *
    Slash,   // /
    Percent, // %

    // 赋值与比较运算符
    Equal,        // =
    PlusEqual,    // +=
    MinusEqual,   // -=
    StarEqual,    // *=
    PercentEqual, // %=
    SlashEqual,   // /=
    EqualEqual,   // ==
    BangEqual,    // !=
    Less,         // <
    LessEqual,   // <=
    Greater,      // >
    GreaterEqual, // >=

    // 逻辑运算符
    And,  // &&
    Or,   // ||
    Bang, // !

    // 分隔符
    LeftParen,    // (
    RightParen,   // )
    LeftBrace,    // {
    RightBrace,   // }
    LeftBracket,  // [
    RightBracket, // ]
    Comma,        // ,
    Semicolon,    // ;
    Colon,        // :
    Dot,          // .
    DotDot,       // ..

    // 特殊
    EndOfFile,
    Unknown,
};

class Token
{
public:
    TokenType token_type;
    std::string value;
    size_t line;
    size_t column;

    Token(TokenType type, const std::string &val, size_t line = 0, size_t column = 0);
};

// Helper function to get keyword token type
std::optional<TokenType> get_keyword(const std::string &word);

// Helper function to convert TokenType to string representation
std::string token_type_to_string(TokenType type);

#endif // CZC_LEXER_TOKEN_HPP
