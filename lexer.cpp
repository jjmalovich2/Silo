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

    // 1. Skip line comments (// ...)
    if (current == '/' && pos + 1 < src.length() && src[pos + 1] == '/') {
        while (pos < src.length() && src[pos] != '\n') pos++;
        return nextToken();
    }

    // 2. F-strings: f"..."
    if (current == 'f' && pos + 1 < src.length() && src[pos + 1] == '"') {
        pos += 2; // skip f and opening "
        std::string raw;
        while (pos < src.length() && src[pos] != '"') {
            if (src[pos] == '\\' && pos + 1 < src.length() && src[pos+1] == '"') {
                raw += '"';
                pos += 2;
            } else {
                raw += src[pos];
                pos++;
            }
        }
        if (pos < src.length()) pos++; // skip closing "
        return {TokenType::FStringLiteral, raw};
    }

    // 3. Identifiers & Keywords
    if (isalpha(static_cast<unsigned char>(current)) || current == '_') {
        std::string result;
        while (pos < src.length() && (isalnum(static_cast<unsigned char>(src[pos])) || src[pos] == '_')) {
            result += src[pos];
            pos++;
        }

        // Type keywords
        if (result == "int")    return {TokenType::TypeInt,    "int"};
        if (result == "string") return {TokenType::TypeString, "string"};
        if (result == "float")  return {TokenType::TypeFloat,  "float"};
        if (result == "double") return {TokenType::TypeFloat,  "double"};
        if (result == "bool")   return {TokenType::TypeBool,   "bool"};
        if (result == "void")   return {TokenType::Void,       "void"};

        // Boolean literals
        if (result == "true")  return {TokenType::True,  "true"};
        if (result == "false") return {TokenType::False, "false"};

        // Control flow
        if (result == "return") return {TokenType::Return,   "return"};
        if (result == "if")     return {TokenType::If,       "if"};
        if (result == "else")   return {TokenType::Else,     "else"};
        if (result == "while")  return {TokenType::While,    "while"};
        if (result == "for")    return {TokenType::For,      "for"};
        if (result == "do")     return {TokenType::DoWhile,  "do"};

        // Memory & casting
        if (result == "free")        return {TokenType::Free,       "free"};
        if (result == "cast")        return {TokenType::Cast,       "cast"};
        if (result == "static_cast") return {TokenType::StaticCast, "static_cast"};

        // Class-related
        if (result == "class")       return {TokenType::Class,       "class"};
        if (result == "constructor") return {TokenType::Constructor, "constructor"};
        if (result == "private")     return {TokenType::Private,     "private"};
        if (result == "protected")   return {TokenType::Protected,   "protected"};
        if (result == "global")      return {TokenType::Global,      "global"};
        if (result == "self")        return {TokenType::Self,        "self"};

        // Struct & const
        if (result == "struct") return {TokenType::Struct, "struct"};
        if (result == "const")  return {TokenType::Const,  "const"};

        return {TokenType::Identifier, result};
    }

    // 4. Numbers (int or float)
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

    // 5. Regular strings: "..."
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

    // 6. Symbols (multi-char first)
    pos++;
    switch (current) {
        case '=':
            if (peek() == '=') { pos++; return {TokenType::EqualEqual,  "=="}; }
            return {TokenType::Equals, "="};
        case '!':
            if (peek() == '=') { pos++; return {TokenType::NotEqual,    "!="}; }
            return {TokenType::Bang, "!"};
        case '<':
            if (peek() == '=') { pos++; return {TokenType::LessEqual,   "<="}; }
            return {TokenType::LessThan, "<"};
        case '>':
            if (peek() == '=') { pos++; return {TokenType::GreaterEqual,">="}; }
            return {TokenType::GreaterThan, ">"};
        case '&':
            if (peek() == '&') { pos++; return {TokenType::AndAnd,      "&&"}; }
            return {TokenType::Ampersand, "&"};
        case '|':
            if (peek() == '|') { pos++; return {TokenType::OrOr,        "||"}; }
            return {TokenType::EndOfFile, ""}; // bare | not supported
        case '+':
            if (peek() == '+') { pos++; return {TokenType::PlusPlus,    "++"}; }
            return {TokenType::Plus, "+"};
        case '-':
            if (peek() == '-') { pos++; return {TokenType::MinusMinus,  "--"}; }
            if (peek() == '>') { pos++; return {TokenType::Arrow,       "->"}; }
            return {TokenType::Minus, "-"};
        case '~': return {TokenType::Tilde,        "~"};
        case '.': return {TokenType::Dot,          "."};
        case ';': return {TokenType::Semicolon,    ";"};
        case '(': return {TokenType::LeftParen,    "("};
        case ')': return {TokenType::RightParen,   ")"};
        case '{': return {TokenType::LeftBrace,    "{"};
        case '}': return {TokenType::RightBrace,   "}"};
        case '[': return {TokenType::LeftBracket,  "["};
        case ']': return {TokenType::RightBracket, "]"};
        case ',': return {TokenType::Comma,        ","};
        case '*': return {TokenType::Asterisk,     "*"};
        case '@': return {TokenType::At,           "@"};
        case '/': return {TokenType::Slash,        "/"};
        case '%': return {TokenType::Percent,      "%"};
        default:  return {TokenType::EndOfFile,    ""};
    }
}