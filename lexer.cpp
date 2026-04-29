#include "silo.h"
#include <cctype>
#include <stdexcept>

Lexer::Lexer(const std::string& source) : src(source), pos(0) {}

char Lexer::peek() {
    // look at current char without moving
    return (pos < src.size()) ? src[pos] : '\0';
}

void Lexer::skipWhitespace() {
    // ignore spaces, tabs, and // comments
    while (pos < src.size()) {
        if (std::isspace((unsigned char)src[pos])) {
            pos++;
        } else if (src[pos] == '/' && pos + 1 < src.size() && src[pos + 1] == '/') {
            while (pos < src.size() && src[pos] != '\n') pos++;
        } else {
            break;
        }
    }
}

Token Lexer::nextToken() {
    // figure out what the next token is
    skipWhitespace();
    if (pos >= src.size()) return {TokenType::EndOfFile, ""};

    char c = src[pos];

    // handle f-strings
    if (c == 'f' && pos + 1 < src.size() && src[pos + 1] == '"') {
        pos += 2;
        std::string val;
        while (pos < src.size() && src[pos] != '"') {
            if (src[pos] == '\\' && pos + 1 < src.size()) {
                pos++;
                switch (src[pos]) {
                    case 'n':  val += '\n'; break;
                    case 't':  val += '\t'; break;
                    case '"':  val += '"';  break;
                    case '\\': val += '\\'; break;
                    default:   val += src[pos]; break;
                }
                pos++;
            } else {
                val += src[pos++];
            }
        }
        if (pos < src.size()) pos++;
        return {TokenType::FStringLiteral, val};
    }

    // regular double-quoted string
    if (c == '"') {
        pos++;
        std::string val;
        while (pos < src.size() && src[pos] != '"') {
            if (src[pos] == '\\' && pos + 1 < src.size()) {
                pos++;
                switch (src[pos]) {
                    case 'n':  val += '\n'; break;
                    case 't':  val += '\t'; break;
                    case '"':  val += '"';  break;
                    case '\\': val += '\\'; break;
                    default:   val += src[pos]; break;
                }
                pos++;
            } else {
                val += src[pos++];
            }
        }
        if (pos < src.size()) pos++;
        return {TokenType::StringLiteral, val};
    }

    // integers or floats
    if (std::isdigit((unsigned char)c)) {
        std::string val;
        bool hasDot = false;
        while (pos < src.size() &&
               (std::isdigit((unsigned char)src[pos]) || (src[pos] == '.' && !hasDot))) {
            if (src[pos] == '.') hasDot = true;
            val += src[pos++];
        }
        return {TokenType::Number, val};
    }

    // check if it matches a reserved word
    if (std::isalpha((unsigned char)c) || c == '_') {
        std::string val;
        while (pos < src.size() &&
               (std::isalnum((unsigned char)src[pos]) || src[pos] == '_')) {
            val += src[pos++];
        }
        // turn the value into a token, probably a better way to do 
        // this but this works and im not gonna change it
        if (val == "int")         return {TokenType::TypeInt,     val};
        if (val == "string")      return {TokenType::TypeString,  val};
        if (val == "float")       return {TokenType::TypeFloat,   val};
        if (val == "double")      return {TokenType::TypeFloat,   val};
        if (val == "bool")        return {TokenType::TypeBool,    val};
        if (val == "void")        return {TokenType::Void,        val};
        if (val == "return")      return {TokenType::Return,      val};
        if (val == "if")          return {TokenType::If,          val};
        if (val == "else")        return {TokenType::Else,        val};
        if (val == "while")       return {TokenType::While,       val};
        if (val == "for")         return {TokenType::For,         val};
        if (val == "do")          return {TokenType::DoWhile,     val};
        if (val == "break")       return {TokenType::Break,       val};
        if (val == "continue")    return {TokenType::Continue,    val};
        if (val == "true")        return {TokenType::True,        val};
        if (val == "false")       return {TokenType::False,       val};
        if (val == "cast")        return {TokenType::Cast,        val};
        if (val == "static_cast") return {TokenType::StaticCast,  val};
        if (val == "free")        return {TokenType::Free,        val};
        if (val == "class")       return {TokenType::Class,       val};
        if (val == "constructor") return {TokenType::Constructor, val};
        if (val == "private")     return {TokenType::Private,     val};
        if (val == "protected")   return {TokenType::Protected,   val};
        if (val == "global")      return {TokenType::Global,      val};
        if (val == "self")        return {TokenType::Self,        val};
        if (val == "struct")      return {TokenType::Struct,      val};
        if (val == "const")       return {TokenType::Const,       val};
        return {TokenType::Identifier, val};
    }

    pos++;

    // single-char tokens and multi-char operators
    switch (c) {
        case '+':
            if (pos < src.size() && src[pos] == '=') { pos++; return {TokenType::PlusEquals,  "+="}; }
            if (pos < src.size() && src[pos] == '+') { pos++; return {TokenType::PlusPlus,     "++"}; }
            return {TokenType::Plus, "+"};
        case '-':
            if (pos < src.size() && src[pos] == '=') { pos++; return {TokenType::MinusEquals,  "-="}; }
            if (pos < src.size() && src[pos] == '-') { pos++; return {TokenType::MinusMinus,    "--"}; }
            if (pos < src.size() && src[pos] == '>') { pos++; return {TokenType::Arrow,         "->"}; }
            return {TokenType::Minus, "-"};
        case '*':
            if (pos < src.size() && src[pos] == '=') { pos++; return {TokenType::TimesEquals,  "*="}; }
            return {TokenType::Asterisk, "*"};
        case '/':
            if (pos < src.size() && src[pos] == '=') { pos++; return {TokenType::DivEquals,    "/="}; }
            return {TokenType::Slash, "/"};
        case '%':
            if (pos < src.size() && src[pos] == '=') { pos++; return {TokenType::ModEquals,    "%="}; }
            return {TokenType::Percent, "%"};
        case '=':
            if (pos < src.size() && src[pos] == '=') { pos++; return {TokenType::EqualEqual,   "=="}; }
            return {TokenType::Equals, "="};
        case '!':
            if (pos < src.size() && src[pos] == '=') { pos++; return {TokenType::NotEqual,     "!="}; }
            return {TokenType::Bang, "!"};
        case '<':
            if (pos < src.size() && src[pos] == '=') { pos++; return {TokenType::LessEqual,    "<="}; }
            return {TokenType::LessThan, "<"};
        case '>':
            if (pos < src.size() && src[pos] == '=') { pos++; return {TokenType::GreaterEqual, ">="}; }
            return {TokenType::GreaterThan, ">"};
        case '&':
            if (pos < src.size() && src[pos] == '&') { pos++; return {TokenType::AndAnd, "&&"}; }
            return {TokenType::Ampersand, "&"};
        case '|':
            if (pos < src.size() && src[pos] == '|') { pos++; return {TokenType::OrOr, "||"}; }
            return nextToken();
        case '(': return {TokenType::LeftParen,    "("};
        case ')': return {TokenType::RightParen,   ")"};
        case '{': return {TokenType::LeftBrace,    "{"};
        case '}': return {TokenType::RightBrace,   "}"};
        case '[': return {TokenType::LeftBracket,  "["};
        case ']': return {TokenType::RightBracket, "]"};
        case ';': return {TokenType::Semicolon,    ";"};
        case ',': return {TokenType::Comma,        ","};
        case '.': return {TokenType::Dot,          "."};
        case '~': return {TokenType::Tilde,        "~"};
        case '@': return {TokenType::At,           "@"};
        default:  return nextToken(); // skip unknown chars
    }
}

std::vector<Token> Lexer::tokenize() {
    // keep grabbing tokens until EOF
    std::vector<Token> tokens;
    while (true) {
        Token t = nextToken();
        if (t.type == TokenType::EndOfFile) break;
        tokens.push_back(t);
    }
    tokens.push_back({TokenType::EndOfFile, ""});
    return tokens;
}