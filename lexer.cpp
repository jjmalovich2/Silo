#include "silo.h"
#include <cctype>

Lexer::Lexer(const std::string& source) : src(source), pos(0) {}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    while (true) {
        Token t = nextToken();
        if (t.type == TokenType::EndOfFile) break;
        tokens.push_back(t);
    }
    tokens.push_back({TokenType::EndOfFile, ""});
    return tokens;
}

void Lexer::skipWhitespace() {
    while (pos < src.length() && isspace(static_cast<unsigned char>(src[pos]))) {
        pos++;
    }
}

char Lexer::peek() {
    if (pos < src.length()) return src[pos];
    return '\0';
}

Token Lexer::nextToken() {
    skipWhitespace();

    if (pos >= src.length()) return {TokenType::EndOfFile, ""};

    char current = src[pos];

    // 1. Skip Comments (// ...)
    if (current == '/' && pos + 1 < src.length() && src[pos + 1] == '/') {
        while (pos < src.length() && src[pos] != '\n') {
            pos++;
        }
        return nextToken(); 
    }

    // 2. Identifiers & Keywords
    if (isalpha(static_cast<unsigned char>(current))) {
        std::string result;
        while (pos < src.length() && (isalnum(static_cast<unsigned char>(src[pos])) || src[pos] == '_')) {
            result += src[pos];
            pos++;
        }

        if (result == "int") return {TokenType::TypeInt, "int"};
        if (result == "string") return {TokenType::TypeString, "string"};
        if (result == "float") return {TokenType::TypeFloat, "float"};
        if (result == "double") return {TokenType::TypeFloat, "double"};
        if (result == "bool") return {TokenType::TypeBool, "bool"};
        if (result == "true") return {TokenType::True, "true"};
        if (result == "false") return {TokenType::False, "false"};
        if (result == "return") return {TokenType::Return, "return"};
        if (result == "free") return {TokenType::Free, "free"};
        if (result == "cast") return {TokenType::Cast, "cast"};
        if (result == "static_cast") return {TokenType::StaticCast, "static_cast"};
        if (result == "if") return {TokenType::If, "if"};
        if (result == "else") return {TokenType::Else, "else"};
        if (result == "while") return {TokenType::While, "while"};
        
        // "print" is handled as a standard identifier in the Parser, so we don't need a specific token for it here.
        return {TokenType::Identifier, result};
    }

    // 3. Numbers
    if (isdigit(static_cast<unsigned char>(current))) {
        std::string result;
        bool hasDecimal = false;
        while (pos < src.length() && (isdigit(static_cast<unsigned char>(src[pos])) || src[pos] == '.')) {
            if (src[pos] == '.') {
                if (hasDecimal) break; 
                hasDecimal = true;
            }
            result += src[pos];
            pos++;
        }
        return {TokenType::Number, result};
    }

    // 4. Strings
    if (current == '"') {
        pos++;
        std::string result;
        while (pos < src.length() && src[pos] != '"') {
            result += src[pos];
            pos++;
        }
        if (pos < src.length()) pos++;
        return {TokenType::StringLiteral, result};
    }

    // 5. Symbols
    pos++;
    switch (current) {
        case '=': 
            if (peek() == '=') { pos++; return {TokenType::EqualEqual, "=="}; }
            return {TokenType::Equals, "="};
        case '!':
            if (peek() == '=') { pos++; return {TokenType::NotEqual, "!="}; }
            return {TokenType::Bang, "!"};
        case '<':
            if (peek() == '=') { pos++; return {TokenType::LessEqual, "<="}; }
            return {TokenType::LessThan, "<"};
        case '>':
            if (peek() == '=') { pos++; return {TokenType::GreaterEqual, ">="}; }
            return {TokenType::GreaterThan, ">"};
        case '&':
            if (peek() == '&') { pos++; return {TokenType::AndAnd, "&&"}; }
            return {TokenType::Ampersand, "&"};
        case '|':
            if (peek() == '|') { pos++; return {TokenType::OrOr, "||"}; }
            return {TokenType::EndOfFile, ""};
        case ';': return {TokenType::Semicolon, ";"};
        case '(': return {TokenType::LeftParen, "("};
        case ')': return {TokenType::RightParen, ")"};
        case '{': return {TokenType::LeftBrace, "{"};
        case '}': return {TokenType::RightBrace, "}"};
        case '[': return {TokenType::LeftBracket, "["};
        case ']': return {TokenType::RightBracket, "]"};
        case ',': return {TokenType::Comma, ","};
        case '*': return {TokenType::Asterisk, "*"};
        case '@': return {TokenType::At, "@"};
        case '+': return {TokenType::Plus, "+"};
        case '-': return {TokenType::Minus, "-"};
        case '/': return {TokenType::Slash, "/"};
        case '%': return {TokenType::Percent, "%"};
        default:  return {TokenType::EndOfFile, ""};
    }
}