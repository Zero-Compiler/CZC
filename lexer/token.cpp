#include "token.hpp"

// Token constructor
Token::Token(TokenType type, const std::string &val, size_t line, size_t column)
    : token_type(type), value(val), line(line), column(column) {}

// Helper function to get keyword token type
std::optional<TokenType> get_keyword(const std::string &word)
{
    if (word == "let")
        return TokenType::Let;
    if (word == "var")
        return TokenType::Var;
    if (word == "fn")
        return TokenType::Fn;
    if (word == "return")
        return TokenType::Return;
    if (word == "if")
        return TokenType::If;
    if (word == "else")
        return TokenType::Else;
    if (word == "while")
        return TokenType::While;
    if (word == "for")
        return TokenType::For;
    if (word == "in")
        return TokenType::In;
    if (word == "true")
        return TokenType::True;
    if (word == "false")
        return TokenType::False;
    return std::nullopt;
}

// Helper function to convert TokenType to string representation
std::string token_type_to_string(TokenType type)
{
    switch (type)
    {
    case TokenType::Integer:
        return "Integer";
    case TokenType::Float:
        return "Float";
    case TokenType::String:
        return "String";
    case TokenType::Identifier:
        return "Identifier";
    case TokenType::Let:
        return "Let";
    case TokenType::Var:
        return "Var";
    case TokenType::Fn:
        return "Fn";
    case TokenType::Return:
        return "Return";
    case TokenType::If:
        return "If";
    case TokenType::Else:
        return "Else";
    case TokenType::While:
        return "While";
    case TokenType::For:
        return "For";
    case TokenType::In:
        return "In";
    case TokenType::True:
        return "True";
    case TokenType::False:
        return "False";
    case TokenType::Plus:
        return "Plus";
    case TokenType::Minus:
        return "Minus";
    case TokenType::Star:
        return "Star";
    case TokenType::Slash:
        return "Slash";
    case TokenType::Percent:
        return "Percent";
    case TokenType::Equal:
        return "Equal";
    case TokenType::EqualEqual:
        return "EqualEqual";
    case TokenType::Bang:
        return "Bang";
    case TokenType::BangEqual:
        return "BangEqual";
    case TokenType::Less:
        return "Less";
    case TokenType::LessEqual:
        return "LessEqual";
    case TokenType::Greater:
        return "Greater";
    case TokenType::GreaterEqual:
        return "GreaterEqual";
    case TokenType::And:
        return "And";
    case TokenType::Or:
        return "Or";
    case TokenType::LeftParen:
        return "LeftParen";
    case TokenType::RightParen:
        return "RightParen";
    case TokenType::LeftBrace:
        return "LeftBrace";
    case TokenType::RightBrace:
        return "RightBrace";
    case TokenType::LeftBracket:
        return "LeftBracket";
    case TokenType::RightBracket:
        return "RightBracket";
    case TokenType::Comma:
        return "Comma";
    case TokenType::Semicolon:
        return "Semicolon";
    case TokenType::Colon:
        return "Colon";
    case TokenType::Dot:
        return "Dot";
    case TokenType::DotDot:
        return "DotDot";
    case TokenType::EndOfFile:
        return "EOF";
    case TokenType::Unknown:
        return "Unknown";
    default:
        return "Unknown";
    }
}
