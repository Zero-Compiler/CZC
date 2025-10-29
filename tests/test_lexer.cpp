#include "../lexer/mod.hpp"
#include <iostream>
#include <cassert>
#include <vector>

void print_token(const Token &token)
{
    std::cout << "Token(" << token_type_to_string(token.token_type)
              << ", \"" << token.value << "\")" << std::endl;
}

void test_integers()
{
    std::cout << "\n=== Test: Integers ===" << std::endl;
    Lexer lexer("123 456 789");
    auto tokens = lexer.tokenize();

    assert(tokens.size() == 4); // 3 integers + EOF
    assert(tokens[0].token_type == TokenType::Integer);
    assert(tokens[0].value == "123");
    assert(tokens[1].token_type == TokenType::Integer);
    assert(tokens[1].value == "456");
    assert(tokens[2].token_type == TokenType::Integer);
    assert(tokens[2].value == "789");
    assert(tokens[3].token_type == TokenType::EndOfFile);

    std::cout << "Integer test passed" << std::endl;
}

void test_floats()
{
    std::cout << "\n=== Test: Floats ===" << std::endl;
    Lexer lexer("3.14 2.71828 0.5");
    auto tokens = lexer.tokenize();

    assert(tokens.size() == 4); // 3 floats + EOF
    assert(tokens[0].token_type == TokenType::Float);
    assert(tokens[0].value == "3.14");
    assert(tokens[1].token_type == TokenType::Float);
    assert(tokens[1].value == "2.71828");
    assert(tokens[2].token_type == TokenType::Float);
    assert(tokens[2].value == "0.5");

    std::cout << "Float test passed" << std::endl;
}

void test_strings()
{
    std::cout << "\n=== Test: Strings ===" << std::endl;
    Lexer lexer("\"hello\" \"world\" \"test string\"");
    auto tokens = lexer.tokenize();

    assert(tokens.size() == 4); // 3 strings + EOF
    assert(tokens[0].token_type == TokenType::String);
    assert(tokens[0].value == "hello");
    assert(tokens[1].token_type == TokenType::String);
    assert(tokens[1].value == "world");
    assert(tokens[2].token_type == TokenType::String);
    assert(tokens[2].value == "test string");

    std::cout << "String test passed" << std::endl;
}

void test_keywords()
{
    std::cout << "\n=== Test: Keywords ===" << std::endl;
    Lexer lexer("let var fn return if else while for in true false");
    auto tokens = lexer.tokenize();

    assert(tokens.size() == 12); // 11 keywords + EOF
    assert(tokens[0].token_type == TokenType::Let);
    assert(tokens[1].token_type == TokenType::Var);
    assert(tokens[2].token_type == TokenType::Fn);
    assert(tokens[3].token_type == TokenType::Return);
    assert(tokens[4].token_type == TokenType::If);
    assert(tokens[5].token_type == TokenType::Else);
    assert(tokens[6].token_type == TokenType::While);
    assert(tokens[7].token_type == TokenType::For);
    assert(tokens[8].token_type == TokenType::In);
    assert(tokens[9].token_type == TokenType::True);
    assert(tokens[10].token_type == TokenType::False);

    std::cout << "Keyword test passed" << std::endl;
}

void test_identifiers()
{
    std::cout << "\n=== Test: Identifiers ===" << std::endl;
    Lexer lexer("foo bar baz_123 _private MyClass");
    auto tokens = lexer.tokenize();

    assert(tokens.size() == 6); // 5 identifiers + EOF
    assert(tokens[0].token_type == TokenType::Identifier);
    assert(tokens[0].value == "foo");
    assert(tokens[1].token_type == TokenType::Identifier);
    assert(tokens[1].value == "bar");
    assert(tokens[2].token_type == TokenType::Identifier);
    assert(tokens[2].value == "baz_123");
    assert(tokens[3].token_type == TokenType::Identifier);
    assert(tokens[3].value == "_private");
    assert(tokens[4].token_type == TokenType::Identifier);
    assert(tokens[4].value == "MyClass");

    std::cout << "Identifier test passed" << std::endl;
}

void test_operators()
{
    std::cout << "\n=== Test: Operators ===" << std::endl;
    Lexer lexer("+ - * / % = == ! != < <= > >= && ||");
    auto tokens = lexer.tokenize();

    assert(tokens.size() == 16); // 15 operators + EOF
    assert(tokens[0].token_type == TokenType::Plus);
    assert(tokens[1].token_type == TokenType::Minus);
    assert(tokens[2].token_type == TokenType::Star);
    assert(tokens[3].token_type == TokenType::Slash);
    assert(tokens[4].token_type == TokenType::Percent);
    assert(tokens[5].token_type == TokenType::Equal);
    assert(tokens[6].token_type == TokenType::EqualEqual);
    assert(tokens[7].token_type == TokenType::Bang);
    assert(tokens[8].token_type == TokenType::BangEqual);
    assert(tokens[9].token_type == TokenType::Less);
    assert(tokens[10].token_type == TokenType::LessEqual);
    assert(tokens[11].token_type == TokenType::Greater);
    assert(tokens[12].token_type == TokenType::GreaterEqual);
    assert(tokens[13].token_type == TokenType::And);
    assert(tokens[14].token_type == TokenType::Or);

    std::cout << "Operator test passed" << std::endl;
}

void test_delimiters()
{
    std::cout << "\n=== Test: Delimiters ===" << std::endl;
    Lexer lexer("( ) { } [ ] , ; : . ..");
    auto tokens = lexer.tokenize();

    assert(tokens.size() == 12); // 11 delimiters + EOF
    assert(tokens[0].token_type == TokenType::LeftParen);
    assert(tokens[1].token_type == TokenType::RightParen);
    assert(tokens[2].token_type == TokenType::LeftBrace);
    assert(tokens[3].token_type == TokenType::RightBrace);
    assert(tokens[4].token_type == TokenType::LeftBracket);
    assert(tokens[5].token_type == TokenType::RightBracket);
    assert(tokens[6].token_type == TokenType::Comma);
    assert(tokens[7].token_type == TokenType::Semicolon);
    assert(tokens[8].token_type == TokenType::Colon);
    assert(tokens[9].token_type == TokenType::Dot);
    assert(tokens[10].token_type == TokenType::DotDot);

    std::cout << "Delimiter test passed" << std::endl;
}

void test_comments()
{
    std::cout << "\n=== Test: Comments ===" << std::endl;
    Lexer lexer("let x = 5; // this is a comment\nlet y = 10;");
    auto tokens = lexer.tokenize();

    assert(tokens.size() == 11); // let x = 5 ; let y = 10 ; EOF
    assert(tokens[0].token_type == TokenType::Let);
    assert(tokens[1].token_type == TokenType::Identifier);
    assert(tokens[1].value == "x");
    assert(tokens[2].token_type == TokenType::Equal);
    assert(tokens[3].token_type == TokenType::Integer);
    assert(tokens[3].value == "5");
    assert(tokens[4].token_type == TokenType::Semicolon);
    assert(tokens[5].token_type == TokenType::Let);
    assert(tokens[6].token_type == TokenType::Identifier);
    assert(tokens[6].value == "y");
    assert(tokens[7].token_type == TokenType::Equal);
    assert(tokens[8].token_type == TokenType::Integer);
    assert(tokens[8].value == "10");
    assert(tokens[9].token_type == TokenType::Semicolon);

    std::cout << "Comment test passed" << std::endl;
}

void test_complex_expression()
{
    std::cout << "\n=== Test: Complex Expression ===" << std::endl;
    Lexer lexer("fn add(a, b) { return a + b; }");
    auto tokens = lexer.tokenize();

    assert(tokens.size() == 15); // fn add ( a , b ) { return a + b ; } EOF
    assert(tokens[0].token_type == TokenType::Fn);
    assert(tokens[1].token_type == TokenType::Identifier);
    assert(tokens[1].value == "add");
    assert(tokens[2].token_type == TokenType::LeftParen);
    assert(tokens[3].token_type == TokenType::Identifier);
    assert(tokens[4].token_type == TokenType::Comma);
    assert(tokens[5].token_type == TokenType::Identifier);
    assert(tokens[6].token_type == TokenType::RightParen);
    assert(tokens[7].token_type == TokenType::LeftBrace);
    assert(tokens[8].token_type == TokenType::Return);
    assert(tokens[9].token_type == TokenType::Identifier);
    assert(tokens[10].token_type == TokenType::Plus);
    assert(tokens[11].token_type == TokenType::Identifier);
    assert(tokens[12].token_type == TokenType::Semicolon);
    assert(tokens[13].token_type == TokenType::RightBrace);

    std::cout << "Complex expression test passed" << std::endl;
}

void test_if_statement()
{
    std::cout << "\n=== Test: If Statement ===" << std::endl;
    Lexer lexer("if x > 10 { return true; } else { return false; }");
    auto tokens = lexer.tokenize();

    assert(tokens[0].token_type == TokenType::If);
    assert(tokens[2].token_type == TokenType::Greater);
    assert(tokens[5].token_type == TokenType::Return);
    assert(tokens[6].token_type == TokenType::True);
    assert(tokens[9].token_type == TokenType::Else);
    assert(tokens[12].token_type == TokenType::False);

    std::cout << "If statement test passed" << std::endl;
}

void test_array_range()
{
    std::cout << "\n=== Test: Array and Range ===" << std::endl;
    Lexer lexer("for i in 0..10 { arr[i] = i * 2; }");
    auto tokens = lexer.tokenize();

    assert(tokens[0].token_type == TokenType::For);
    assert(tokens[2].token_type == TokenType::In);
    assert(tokens[4].token_type == TokenType::DotDot);
    assert(tokens[8].token_type == TokenType::LeftBracket);
    assert(tokens[10].token_type == TokenType::RightBracket);

    std::cout << "Array and range test passed" << std::endl;
}

void test_whitespace_handling()
{
    std::cout << "\n=== Test: Whitespace Handling ===" << std::endl;
    Lexer lexer("   let   x   =   5   ;   ");
    auto tokens = lexer.tokenize();

    assert(tokens.size() == 6); // let x = 5 ; EOF
    assert(tokens[0].token_type == TokenType::Let);
    assert(tokens[1].token_type == TokenType::Identifier);
    assert(tokens[2].token_type == TokenType::Equal);
    assert(tokens[3].token_type == TokenType::Integer);
    assert(tokens[4].token_type == TokenType::Semicolon);

    std::cout << "Whitespace handling test passed" << std::endl;
}

void test_empty_input()
{
    std::cout << "\n=== Test: Empty Input ===" << std::endl;
    Lexer lexer("");
    auto tokens = lexer.tokenize();

    assert(tokens.size() == 1); // Only EndOfFile
    assert(tokens[0].token_type == TokenType::EndOfFile);

    std::cout << "Empty input test passed" << std::endl;
}

void test_escaped_strings()
{
    std::cout << "\n=== Test: Escaped Strings ===" << std::endl;
    Lexer lexer("\"hello\\nworld\" \"test\\\"quote\"");
    auto tokens = lexer.tokenize();

    assert(tokens.size() == 3); // 2 strings + EOF
    assert(tokens[0].token_type == TokenType::String);
    assert(tokens[0].value == "hello\nworld");
    assert(tokens[1].token_type == TokenType::String);
    assert(tokens[1].value == "test\"quote");

    std::cout << "Escaped string test passed" << std::endl;
}

int main()
{
    std::cout << "Running Lexer Tests..." << std::endl;
    std::cout << "======================" << std::endl;

    try
    {
        test_integers();
        test_floats();
        test_strings();
        test_keywords();
        test_identifiers();
        test_operators();
        test_delimiters();
        test_comments();
        test_complex_expression();
        test_if_statement();
        test_array_range();
        test_whitespace_handling();
        test_empty_input();
        test_escaped_strings();

        std::cout << "\n======================" << std::endl;
        std::cout << "All tests passed!" << std::endl;
        std::cout << "======================" << std::endl;

        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "\nTest failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
