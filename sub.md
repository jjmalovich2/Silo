# AP CSP Create Task - Program Code

## Anthropic's CLI ClaudeCode was used throughout development, used to fix bugs and optimize parts of the Abstract Syntax Tree. Other CLI AI Agents, like Kimi K2.6, were also used, but barely.

## Part 1: Custom Language Script (The Application)
This script demonstrates the required logic (List, Procedure, Algorithm) running on my custom interpreter.
```cpp
#include <math>

int scores[] = [84, 86, 75, 96, 84, 92, 71, 64, 68, 77];

int countPassing(int threshold) {
    int count = 0;
    
    for (int i = 0; i < scores.size(); i++) {
        if (scores[i] >= threshold) {
            count = count + 1;
        }
    }
    return count;
}

string name = input("What is your name? ");
int minPassing = input<int>("What is the minimum passing score (int)? ");
int* minPassing_ptr = @minPassing;

int totalPassing = countPassing(minPassing);

print(f"Name: {name}");
print(f"Passing Grade: {minPassing}");
print(f"MemAddr of minPassing: {minPassing_ptr}");
print(f"Total Passing Grades: {totalPassing}");
print(f"MemAddr of totalPassing: {@totalPassing}");
print(f"Average Grade: {avg_int(scores)} (imported from 'math' library)");
print("Freed Memory by Deleting 'scores' array from Values Table");
free scores;
```

## Math Standard Library created in pure Silo: (math.sl)
```cpp
double abs(double val) {
    if (val < 0) { return val - val - val; }
    return val;
}

double clamp(double val, double lo, double hi) {
    if (val < lo) { return lo; }
    if (val > hi) { return hi; }
    return val;
}

double exp(double bs, int ex) {
    if (ex == 0) { return 1; }
    double result = 1.0;
    if (ex > 0) {
        for (int i = 0; i < ex; i++) {
            result = result * bs;
        }
        return result;
    } else {
        int posEx = 0 - ex;
        for (int i = 0; i < posEx; i++) {
            result = result * bs;
        }
        return 1 / result;
    }
    return 0;
}

double sqrt(double val) {
    if (val < 0) {
        print("Cannot sqrt negative number");
        return -1;
    }
    if (val == 0) { return 0; }
    double guess = val / 2.0;
    double prev_guess = 0.0;
    while (abs(guess - prev_guess) >= 0.00001) {
        prev_guess = guess;
        guess = 0.5 * (guess + val / guess);
    }
    return guess;
}

double max(double n1, double n2) {
    if (n1 >= n2) { return n1; }
    return n2;
}

double min(double n1, double n2) {
    if (n1 <= n2) { return n1; }
    return n2;
}

double round(double val, int dec) {
    double factor = exp(10, dec);
    double shifted = val * factor + 0.5;
    return shifted / factor;
}

int floor(double v) {
    int f = v;
    return f;
}

int ceil(double v) {
    int f = v;
    if (v - f > 0) { return f + 1; }
    return f;
}

int sign(double v) {
    if (v < 0) { return -1; }
    if (v == 0) { return 0; }
    return 1;
}

bool isEven(int n) {
    return (n % 2 == 0);
}

bool isOdd(int n) {
    return !isEven(n);
}

double log(double x) {
    if (x <= 0) { return 0; }
    double guess = 1.0;
    double prev = 0.0;
    for (int i = 0; i < 100; i++) {
        prev = guess;
        guess = guess - 1.0 + x / exp(Math.E, cast<int>(guess));
        if (abs(guess - prev) < 0.00001) { return guess; }
    }
    return guess;
}

double log10(double x) {
    return log(x) / log(10);
}

int factorial(int n) {
    int result = 1;
    for (int i = 1; i <= n; i++) {
        result = result * i;
    }
    return result;
}

double toRadians(double deg) {
    return deg * (Math.PI / 180.0);
}

double sin(double x) {
    double result = 0.0;
    double term = x;
    int s = 1;
    for (int i = 1; i < 20; i++) {
        result = result + s * term;
        term = term * x * x / ((2 * i) * (2 * i + 1));
        s = s - s - s;
    }
    return result;
}

double cos(double x) {
    double result = 1.0;
    double term = 1.0;
    int s = 0 - 1;
    for (int i = 1; i < 20; i++) {
        term = term * x * x / ((2 * i - 1) * (2 * i));
        result = result + s * term;
        s = s - s - s;
    }
    return result;
}

double tan(double x) {
    return sin(x) / cos(x);
}

int gcd(int a, int b) {
    if (a < 0) { a = abs(a); }
    if (b < 0) { b = abs(b); }
    while (b != 0) {
        int temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

int lcm(int a, int b) {
    return (a / gcd(a, b)) * b;
}

double lerp(double a, double b, double t) {
    return a + t * (b - a);
}

double avg_int(int[] arr) {
    int result = 0;

    for (int i = 0; i < arr.size(); i++) {
        result = result + arr[i];
    }
    result = result / arr.size();
    return result;
}
```

---

## Part 2: Interpreter Source Code (The Engine)

### silo.h
```cpp
#ifndef SILO_H
#define SILO_H

#include <string>
#include <vector>
#include <map>
#include <set>
#include <memory>
#include <iostream>
#include <functional>
// Token types the lexer spits out

enum TokenType {
    // Keywords
    TypeInt, TypeString, TypeFloat, TypeBool,
    Return, Print, If, Else, While, For, DoWhile,
    Break, Continue,
    True, False,
    Cast, StaticCast, Free,
    Class, Constructor, Private, Protected, Global, Void,
    Self,
    Struct, Const,

    // Identifiers & Literals
    Identifier, Number, StringLiteral, FStringLiteral,

    // Operators & Symbols
    Equals,         // =
    Plus,           // +
    Minus,          // -
    Asterisk,       // *
    Slash,          // /
    Percent,        // %
    PlusPlus,       // ++
    MinusMinus,     // --
    PlusEquals,     // +=
    MinusEquals,    // -=
    TimesEquals,    // *=
    DivEquals,      // /=
    ModEquals,      // %=
    LeftParen,      // (
    RightParen,     // )
    LeftBrace,      // {
    RightBrace,     // }
    LeftBracket,    // [
    RightBracket,   // ]
    Semicolon,      // ;
    Comma,          // ,
    Dot,            // .
    Tilde,          // ~
    Arrow,          // ->
    LessThan,       // <
    GreaterThan,    // >
    EqualEqual,     // ==
    NotEqual,       // !=
    LessEqual,      // <=
    GreaterEqual,   // >=
    AndAnd,         // &&
    OrOr,           // ||
    Bang,           // !
    Ampersand,      // &
    At,             // @

    EndOfFile
};

struct Token {
    TokenType type;
    std::string value;
};

// Forward declarations
struct ASTNode;
struct ExprNode;
class BlockNode;

struct FieldDef {
    // field access level, type, default value, const flag
    std::string access;   // "private", "protected", "global", "public"
    std::string type;
    std::string value;
    bool isConst = false;
    std::shared_ptr<ExprNode> initExpr; // stored for deferred evaluation at instantiation
};

struct MethodDef {
    // return type, params, body, ctor bindings, parent ctor info
    std::string returnType;
    std::vector<std::pair<std::string, std::string>> params;
    std::shared_ptr<BlockNode> body;
    std::vector<std::string> constructorBindings;
    std::string parentConstructorClass;
    std::vector<std::string> parentConstructorArgs;
    std::string ownerClass;
};

struct RuntimeValue {
    // can be a variable, function, class instance, or array
    std::string type;
    std::string value;
    bool isConst = false;
    std::vector<std::string> arrayElements;

    std::vector<std::pair<std::string, std::string>> params;
    std::shared_ptr<ASTNode> body;

    std::string parentClass;
    std::string instanceOf;
    std::map<std::string, FieldDef>  fields;
    std::map<std::string, MethodDef> methods;
};

extern std::map<std::string, RuntimeValue> SYMBOL_TABLE;
extern std::string CURRENT_CLASS;
extern std::string CURRENT_INSTANCE;

void printSymbolTable();
void clear();

struct ASTNode {
    // base class for all statements
    virtual ~ASTNode() = default;
    virtual void execute() = 0;
};

struct ExprNode : public ASTNode {
    // base class for all expressions
    virtual std::string evaluate() const = 0;
    virtual std::string getExprType() const { return "unknown"; }
    void execute() override { evaluate(); }
};

// EXPRESSION NODES

class NumberLiteralNode : public ExprNode {
    std::string value;
public:
    NumberLiteralNode(const std::string& val);
    std::string evaluate() const override;
};

class StringLiteralNode : public ExprNode {
    std::string value;
public:
    StringLiteralNode(const std::string& val);
    std::string evaluate() const override;
};

class BooleanLiteralNode : public ExprNode {
    bool value;
public:
    BooleanLiteralNode(bool val);
    std::string evaluate() const override;
};

class FStringNode : public ExprNode {
public:
    std::vector<std::pair<bool, std::string>> parts;
    FStringNode(std::vector<std::pair<bool, std::string>> p);
    std::string evaluate() const override;
};

class VariableNode : public ExprNode {
    std::string name;
public:
    VariableNode(const std::string& n);
    std::string evaluate() const override;
    std::string getName() const { return name; }
};

class BinaryOpNode : public ExprNode {
    std::string op;
    std::unique_ptr<ExprNode> left;
    std::unique_ptr<ExprNode> right;
public:
    BinaryOpNode(std::string op, std::unique_ptr<ExprNode> l, std::unique_ptr<ExprNode> r);
    std::string evaluate() const override;
};

class PostfixOpNode : public ExprNode {
    std::string varName;
    std::string op;
public:
    PostfixOpNode(const std::string& name, const std::string& op);
    std::string evaluate() const override;
};

class AssignExprNode : public ExprNode {
    std::string varName;
    std::unique_ptr<ExprNode> value;
public:
    AssignExprNode(const std::string& name, std::unique_ptr<ExprNode> val);
    std::string evaluate() const override;
};

class CastOrRefNode : public ExprNode {
    std::string operation;
    std::string targetVar;
public:
    CastOrRefNode(const std::string& op, const std::string& var);
    std::string evaluate() const override;
};

class CastExprNode : public ExprNode {
    std::string targetType;
    std::unique_ptr<ExprNode> expr;
public:
    CastExprNode(const std::string& t, std::unique_ptr<ExprNode> e);
    std::string evaluate() const override;
};

class ArrayAccessNode : public ExprNode {
    std::string arrayName;
    std::unique_ptr<ExprNode> indexExpr;
public:
    ArrayAccessNode(const std::string& name, std::unique_ptr<ExprNode> idx);
    std::string evaluate() const override;
};

class ArrayAssignNode : public ExprNode {
    std::string arrayName;
    std::unique_ptr<ExprNode> indexExpr;
    std::unique_ptr<ExprNode> valueExpr;
public:
    ArrayAssignNode(const std::string& name, std::unique_ptr<ExprNode> idx, std::unique_ptr<ExprNode> val);
    std::string evaluate() const override;
};

class ArrayCompoundAssignNode : public ExprNode {
    std::string arrayName;
    std::unique_ptr<ExprNode> indexExpr;
    std::string op;
    std::unique_ptr<ExprNode> valueExpr;
public:
    ArrayCompoundAssignNode(const std::string& name, std::unique_ptr<ExprNode> idx, std::string op, std::unique_ptr<ExprNode> val);
    std::string evaluate() const override;
};

class FunctionCallNode : public ExprNode {
    std::string funcName;
    std::vector<std::unique_ptr<ExprNode>> args;
    std::string templateType; // optional type parameter, e.g. input<int>
public:
    FunctionCallNode(const std::string& name, std::vector<std::unique_ptr<ExprNode>> a,
                     const std::string& t = "");
    std::string evaluate() const override;
    std::string getExprType() const override;
};

class SelfAccessNode : public ExprNode {
    std::string memberName;
    bool isCall;
    std::vector<std::unique_ptr<ExprNode>> callArgs;
public:
    SelfAccessNode(const std::string& member, bool isCall,
                   std::vector<std::unique_ptr<ExprNode>> args = {});
    std::string evaluate() const override;
};

class MemberAccessNode : public ExprNode {
    std::string instanceName;
    std::string memberName;
    bool isCall;
    std::vector<std::unique_ptr<ExprNode>> callArgs;
public:
    MemberAccessNode(const std::string& inst, const std::string& member,
                     bool isCall, std::vector<std::unique_ptr<ExprNode>> args = {});
    std::string evaluate() const override;
    std::string getExprType() const override;
};

class MemberAssignNode : public ExprNode {
    std::string instanceName;
    std::string fieldName;
    std::unique_ptr<ExprNode> value;
public:
    MemberAssignNode(const std::string& inst, const std::string& field,
                     std::unique_ptr<ExprNode> val);
    std::string evaluate() const override;
};

// STATEMENT NODES

class BlockNode : public ASTNode {
    // just a list of statements
public:
    std::vector<std::unique_ptr<ASTNode>> statements;
    void execute() override;
};

class VarDeclarationNode : public ASTNode {
    std::string baseType;
    bool isPointer;
    bool isConst;
    std::string identifier;
    std::unique_ptr<ExprNode> initializer;
public:
    VarDeclarationNode(const std::string& t, bool p, bool c,
                       const std::string& id, std::unique_ptr<ExprNode> init);
    void execute() override;
};

class ArrayDeclarationNode : public ASTNode {
    std::string type;
    std::string name;
    int size;
    std::vector<std::unique_ptr<ExprNode>> initializers;
public:
    ArrayDeclarationNode(const std::string& t, const std::string& n, int s,
                         std::vector<std::unique_ptr<ExprNode>> inits = {});
    void execute() override;
};

class ArrayLiteralNode : public ExprNode {
    std::vector<std::unique_ptr<ExprNode>> elements;
public:
    ArrayLiteralNode(std::vector<std::unique_ptr<ExprNode>> elems);
    std::string evaluate() const override;
    const std::vector<std::unique_ptr<ExprNode>>& getElements() const { return elements; }
};

class ArrayReassignNode : public ExprNode {
    std::string arrayName;
    std::vector<std::unique_ptr<ExprNode>> elements;
public:
    ArrayReassignNode(const std::string& name, std::vector<std::unique_ptr<ExprNode>> elems);
    std::string evaluate() const override;
};

class RetypeNode : public ASTNode {
    std::string newType;
    std::string targetVar;
public:
    RetypeNode(const std::string& t, const std::string& v);
    void execute() override;
};

class PrintNode : public ASTNode {
    std::unique_ptr<ExprNode> expression;
public:
    PrintNode(std::unique_ptr<ExprNode> expr);
    void execute() override;
};

class ReturnNode : public ASTNode {
    std::unique_ptr<ExprNode> value;
public:
    ReturnNode(std::unique_ptr<ExprNode> v);
    void execute() override;
};

class FreeNode : public ASTNode {
    std::string identifier;
public:
    FreeNode(const std::string& id);
    void execute() override;
};

class BreakNode : public ASTNode {
public:
    void execute() override;
};

class ContinueNode : public ASTNode {
public:
    void execute() override;
};

class IfNode : public ASTNode {
    std::unique_ptr<ExprNode> condition;
    std::unique_ptr<BlockNode> thenBlock;
    std::vector<std::pair<std::unique_ptr<ExprNode>, std::unique_ptr<BlockNode>>> elseIfBlocks;
    std::unique_ptr<BlockNode> elseBlock;
public:
    IfNode(std::unique_ptr<ExprNode> cond, std::unique_ptr<BlockNode> thenB);
    void addElseIf(std::unique_ptr<ExprNode> cond, std::unique_ptr<BlockNode> block);
    void setElse(std::unique_ptr<BlockNode> block);
    void execute() override;
};

class WhileNode : public ASTNode {
    std::unique_ptr<ExprNode> condition;
    std::unique_ptr<BlockNode> body;
public:
    WhileNode(std::unique_ptr<ExprNode> cond, std::unique_ptr<BlockNode> b);
    void execute() override;
};

class DoWhileNode : public ASTNode {
    std::unique_ptr<ExprNode> condition;
    std::unique_ptr<BlockNode> body;
public:
    DoWhileNode(std::unique_ptr<ExprNode> cond, std::unique_ptr<BlockNode> b);
    void execute() override;
};

class ForNode : public ASTNode {
    std::unique_ptr<ASTNode> init;
    std::unique_ptr<ExprNode> condition;
    std::unique_ptr<ExprNode> increment;
    std::unique_ptr<BlockNode> body;
public:
    ForNode(std::unique_ptr<ASTNode> init, std::unique_ptr<ExprNode> cond,
            std::unique_ptr<ExprNode> inc, std::unique_ptr<BlockNode> b);
    void execute() override;
};

class FunctionDefNode : public ASTNode {
    std::string returnType;
    std::string name;
    std::vector<std::pair<std::string, std::string>> params;
    std::shared_ptr<BlockNode> body;
public:
    FunctionDefNode(const std::string& rt, const std::string& n,
                    const std::vector<std::pair<std::string, std::string>> p,
                    std::shared_ptr<BlockNode> b);
    void execute() override;
};

class ClassDefNode : public ASTNode {
    std::string className;
    std::string parentName;
    std::map<std::string, FieldDef>  fields;
    std::map<std::string, MethodDef> methods;
public:
    ClassDefNode(const std::string& name, const std::string& parent,
                 std::map<std::string, FieldDef> f,
                 std::map<std::string, MethodDef> m);
    void execute() override;
};

class StructDefNode : public ASTNode {
    std::string structName;
    std::map<std::string, FieldDef> fields;
public:
    StructDefNode(const std::string& name, std::map<std::string, FieldDef> f);
    void execute() override;
};

class InstanceCreateNode : public ASTNode {
    std::string className;
    std::string instanceName;
    std::vector<std::unique_ptr<ExprNode>> args;
public:
    InstanceCreateNode(const std::string& cls, const std::string& inst,
                       std::vector<std::unique_ptr<ExprNode>> a);
    void execute() override;
};

class MemberAccessStatement : public ASTNode {
    std::unique_ptr<MemberAccessNode> node;
public:
    MemberAccessStatement(std::unique_ptr<MemberAccessNode> n);
    void execute() override;
};

// LEXER & PARSER

class Lexer {
    // chops source code into tokens
    std::string src;
    size_t pos;
    void skipWhitespace();
    Token nextToken();
    char peek();
public:
    Lexer(const std::string& source);
    std::vector<Token> tokenize();
};

class Parser {
    // builds the AST from tokens
    std::vector<Token> tokens;
    size_t position;

    Token peek();
    Token advance();
    Token consume(TokenType type, const std::string& err);
    std::string parseTypeName();

public:
    Parser(const std::vector<Token>& toks);

    std::unique_ptr<ExprNode> parsePrimary();
    std::unique_ptr<ExprNode> parsePostfix();
    std::unique_ptr<ExprNode> parseTerm();
    std::unique_ptr<ExprNode> parseExpression();
    std::unique_ptr<ExprNode> parseComparison();
    std::unique_ptr<ExprNode> parseLogicalAnd();
    std::unique_ptr<ExprNode> parseLogicalOr();

    std::unique_ptr<BlockNode> parseBlock();
    std::unique_ptr<ASTNode>   parseStatement();
    std::unique_ptr<ASTNode>   parseClassDef();
    std::unique_ptr<ASTNode>   parseStructDef();
};

#endif
```

### lexer.cpp
```cpp
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
```

### AST.cpp
```cpp
#include "silo.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <cmath>
#include <stdexcept>

// GLOBALS

std::map<std::string, RuntimeValue> SYMBOL_TABLE;
// tracks every variable, function, class, and instance
std::string CURRENT_CLASS    = "";
std::string CURRENT_INSTANCE = "";

// CONTROL-FLOW EXCEPTIONS

struct ReturnException   { std::string value; };
// thrown when return is called
struct BreakException    {};
// thrown to break out of a loop
struct ContinueException {};
// thrown to skip to the next loop iteration

// HELPERS

void printSymbolTable() {
    // debug: dump everything in the symbol table
    std::cout << "\n=== SYMBOL TABLE DUMP ===\n";
    if (SYMBOL_TABLE.empty()) {
        std::cout << "(Empty)\n";
    } else {
        for (const auto& pair : SYMBOL_TABLE) {
            const std::string& name = pair.first;
            const RuntimeValue& val = pair.second;
            std::cout << "Name: " << name << " | Type: " << val.type;
            if (!val.instanceOf.empty())
                std::cout << " | Instance of: " << val.instanceOf;
            else if (val.type == "class")
                std::cout << " | [Class]";
            else if (val.type == "struct")
                std::cout << " | [Struct]";
            else if (!val.params.empty() || val.body)
                std::cout << " | [Function]";
            else if (!val.arrayElements.empty()) {
                std::cout << " | Values: [";
                for (size_t i = 0; i < val.arrayElements.size(); ++i)
                    std::cout << val.arrayElements[i]
                              << (i + 1 < val.arrayElements.size() ? ", " : "");
                std::cout << "]";
            } else {
                std::cout << " | Value: " << val.value;
            }
            std::cout << "\n";
        }
    }
    std::cout << "=========================\n";
}

// ANSI escape to clear the terminal
void clear() {
    std::cout << "\033[2J\033[1;1H";
    std::cout.flush();
}

// quick check if a string is a number
static bool isNumeric(const std::string& s) {
    if (s.empty()) return false;
    bool hasDot = false;
    for (size_t i = (s[0] == '-' ? 1 : 0); i < s.size(); i++) {
        if (s[i] == '.') { if (hasDot) return false; hasDot = true; }
        else if (!isdigit((unsigned char)s[i])) return false;
    }
    return true;
}

// guess int, float, or string from a value
static std::string inferType(const std::string& val) {
    if (isNumeric(val))
        return (val.find('.') != std::string::npos) ? "float" : "int";
    return "string";
}

// clean up trailing zeros from doubles
static std::string formatNum(double v) {
    if (v == (long long)v)
        return std::to_string((long long)v);
    std::string s = std::to_string(v);
    size_t dot = s.find('.');
    if (dot != std::string::npos) {
        size_t last = s.find_last_not_of('0');
        if (last != std::string::npos && last > dot)
            s = s.substr(0, last + 1);
        else if (last == dot)
            s = s.substr(0, dot);
    }
    return s;
}

// allow some type widening (int -> float, etc.)
static bool typesCompatible(const std::string& expected, const std::string& actual) {
    if (expected == actual) return true;
    if (expected == "unknown" || actual == "unknown") return true;
    // int -> float/double widening
    if ((expected == "float" || expected == "double") && actual == "int") return true;
    // float <-> double equivalence
    if ((expected == "float" || expected == "double") && (actual == "float" || actual == "double")) return true;
    // array type compatibility
    if (expected.find("[]") != std::string::npos && actual.find("[]") != std::string::npos) return true;
    return false;
}

// GLOBAL FIELD INJECT / CAPTURE / REAPPLY

// pull global fields from a class into the symbol table
static void injectGlobals(const std::string& className) { 
    std::string cls = className;
    while (!cls.empty()) {
        auto it = SYMBOL_TABLE.find(cls);
        if (it == SYMBOL_TABLE.end()) break;
        for (auto& kv : it->second.fields)
            if (kv.second.access == "global")
                SYMBOL_TABLE[kv.first] = {kv.second.type, kv.second.value};
        cls = it->second.parentClass;
    }
}

static std::map<std::string, std::map<std::string, std::string>>
    // snapshot global field values before method call
captureGlobals(const std::string& className) {
    std::map<std::string, std::map<std::string, std::string>> updates;
    std::string cls = className;
    while (!cls.empty()) {
        auto it = SYMBOL_TABLE.find(cls);
        if (it == SYMBOL_TABLE.end()) break;
        for (auto& kv : it->second.fields) {
            if (kv.second.access == "global") {
                auto fit = SYMBOL_TABLE.find(kv.first);
                if (fit != SYMBOL_TABLE.end())
                    updates[cls][kv.first] = fit->second.value;
            }
        }
        cls = it->second.parentClass;
    }
    return updates;
}

static void reapplyGlobals(
    // write updated globals back into the class definition
    const std::map<std::string, std::map<std::string, std::string>>& updates)
{
    for (auto& clsKv : updates) {
        auto it = SYMBOL_TABLE.find(clsKv.first);
        if (it == SYMBOL_TABLE.end()) continue;
        for (auto& fieldKv : clsKv.second)
            it->second.fields[fieldKv.first].value = fieldKv.second;
    }
}

static void clearOuterLocals(const std::string& className) {
    // remove local variables that leaked out of a method
    std::set<std::string> globals;
    std::string cls = className;
    while (!cls.empty()) {
        auto it = SYMBOL_TABLE.find(cls);
        if (it == SYMBOL_TABLE.end()) break;
        for (auto& kv : it->second.fields)
            if (kv.second.access == "global") globals.insert(kv.first);
        cls = it->second.parentClass;
    }
    for (auto it = SYMBOL_TABLE.begin(); it != SYMBOL_TABLE.end(); ) {
        const std::string& type = it->second.type;
        bool keep = (type == "class" ||
                     type == "struct" ||
                     type.compare(0, 9, "instance:") == 0 ||
                     type == "function" ||
                     globals.count(it->first) > 0);
        if (!keep) it = SYMBOL_TABLE.erase(it);
        else        ++it;
    }
}

// INSTANCE FIELD ACCESS

static std::string getInstanceField(const std::string& instName,
    // read a field, respecting private/protected access
                                    const std::string& fieldName,
                                    const std::string& callerClass = "") {
    auto iit = SYMBOL_TABLE.find(instName);
    if (iit == SYMBOL_TABLE.end())
        throw std::runtime_error("Unknown instance: " + instName);

    RuntimeValue& inst = iit->second;
    std::string className = inst.instanceOf;

    auto cit = SYMBOL_TABLE.find(className);
    if (cit != SYMBOL_TABLE.end()) {
        auto fit = cit->second.fields.find(fieldName);
        if (fit != cit->second.fields.end() && fit->second.access == "global")
            return fit->second.value;
    }

    auto fit = inst.fields.find(fieldName);
    if (fit == inst.fields.end())
        throw std::runtime_error("Field '" + fieldName + "' not found on " + instName);

    FieldDef& fd = fit->second;

    if (fd.access == "private" && callerClass != className)
        throw std::runtime_error("Access error: '" + fieldName +
                                 "' is private to class " + className);

    if (fd.access == "protected") {
        bool ok = (callerClass == className);
        if (!ok && SYMBOL_TABLE.find(callerClass) != SYMBOL_TABLE.end()) {
            std::string p = SYMBOL_TABLE[callerClass].parentClass;
            while (!p.empty()) {
                if (p == className) { ok = true; break; }
                if (SYMBOL_TABLE.find(p) != SYMBOL_TABLE.end())
                    p = SYMBOL_TABLE[p].parentClass;
                else break;
            }
        }
        if (!ok)
            throw std::runtime_error("Access error: '" + fieldName +
                                     "' is protected in class " + className);
    }

    return fd.value;
}

static void setInstanceField(const std::string& instName,
    // write a field on an instance
                             const std::string& fieldName,
                             const std::string& newVal) {
    auto iit = SYMBOL_TABLE.find(instName);
    if (iit == SYMBOL_TABLE.end())
        throw std::runtime_error("Unknown instance: " + instName);

    RuntimeValue& inst = iit->second;
    std::string className = inst.instanceOf;

    auto cit = SYMBOL_TABLE.find(className);
    if (cit != SYMBOL_TABLE.end()) {
        auto fit = cit->second.fields.find(fieldName);
        if (fit != cit->second.fields.end() && fit->second.access == "global") {
            fit->second.value = newVal;
            return;
        }
    }

    auto fit = inst.fields.find(fieldName);
    if (fit == inst.fields.end())
        throw std::runtime_error("Field '" + fieldName + "' not found on " + instName);
    fit->second.value = newVal;
}

// METHOD CALL

static std::string callMethod(const std::string& instName,
    // invoke a method, handling globals and scope cleanup
                              const std::string& methodName,
                              const std::vector<std::string>& argVals,
                              const std::vector<std::string>& argTypes,
                              const std::vector<std::vector<std::string>>& argArrayElems = {},
                              const std::vector<std::string>& argOriginalNames = {}) {
    auto iit = SYMBOL_TABLE.find(instName);
    if (iit == SYMBOL_TABLE.end())
        throw std::runtime_error("Unknown instance: " + instName);

    std::string className = iit->second.instanceOf;

    MethodDef* methodPtr = nullptr;
    std::string searchClass = className;
    while (!searchClass.empty()) {
        auto it = SYMBOL_TABLE.find(searchClass);
        if (it == SYMBOL_TABLE.end()) break;
        auto mit = it->second.methods.find(methodName);
        if (mit != it->second.methods.end()) {
            methodPtr = &mit->second;
            break;
        }
        searchClass = it->second.parentClass;
    }
    if (!methodPtr)
        throw std::runtime_error("Method '" + methodName +
                                 "' not found on class " + className);

    MethodDef method = *methodPtr;

    std::string outerClass    = CURRENT_CLASS;
    std::string outerInstance = CURRENT_INSTANCE;

    injectGlobals(className);
    clearOuterLocals(className);
    auto prevSymbols = SYMBOL_TABLE;

    CURRENT_CLASS    = className;
    CURRENT_INSTANCE = instName;

    for (size_t i = 0; i < method.params.size() && i < argVals.size(); i++) {
        const std::string& pname = method.params[i].second;
        const std::string& ptype = method.params[i].first;
        if (!typesCompatible(ptype, argTypes[i])) {
            SYMBOL_TABLE     = prevSymbols;
            CURRENT_CLASS    = outerClass;
            CURRENT_INSTANCE = outerInstance;
            throw std::runtime_error("Type mismatch: param '" + pname +
                                     "' expects " + ptype + " but got " + argTypes[i]);
        }
        if (i < argArrayElems.size() && (!argArrayElems[i].empty() || ptype.find("[]") != std::string::npos)) {
            RuntimeValue arr;
            arr.type = ptype;
            arr.value = "";
            arr.arrayElements = argArrayElems[i];
            SYMBOL_TABLE[pname] = arr;
        } else {
            SYMBOL_TABLE[pname] = {ptype, argVals[i]};
        }
    }

    std::string result = "0";
    try {
        if (method.body) method.body->execute();
    } catch (const ReturnException& ret) {
        result = ret.value;
    }

    // Copy back array parameter changes to original variables
    for (size_t i = 0; i < method.params.size() && i < argOriginalNames.size(); i++) {
        if (!argOriginalNames[i].empty() && i < argArrayElems.size() && !argArrayElems[i].empty()) {
            auto pit = SYMBOL_TABLE.find(method.params[i].second);
            if (pit != SYMBOL_TABLE.end() && pit->second.type.find("[]") != std::string::npos) {
                auto vit = prevSymbols.find(argOriginalNames[i]);
                if (vit != prevSymbols.end() && vit->second.type.find("[]") != std::string::npos) {
                    prevSymbols[argOriginalNames[i]].arrayElements = pit->second.arrayElements;
                }
            }
        }
    }

    auto globalUpdates = captureGlobals(className);
    RuntimeValue updatedInst = SYMBOL_TABLE[instName];
    SYMBOL_TABLE = prevSymbols;
    SYMBOL_TABLE[instName] = updatedInst;
    reapplyGlobals(globalUpdates);

    CURRENT_CLASS    = outerClass;
    CURRENT_INSTANCE = outerInstance;

    return result;
}

// EXPRESSION STATEMENT WRAPPER

class ExpressionStatement : public ASTNode {
    // wraps an expression used as a statement
    std::unique_ptr<ExprNode> expr;
public:
    ExpressionStatement(std::unique_ptr<ExprNode> e) : expr(std::move(e)) {}
    void execute() override { expr->evaluate(); }
};

// CONSTRUCTORS

NumberLiteralNode::NumberLiteralNode(const std::string& val) : value(val) {}
StringLiteralNode::StringLiteralNode(const std::string& val) : value(val) {}
BooleanLiteralNode::BooleanLiteralNode(bool val) : value(val) {}
VariableNode::VariableNode(const std::string& n) : name(n) {}
CastOrRefNode::CastOrRefNode(const std::string& op, const std::string& var) : operation(op), targetVar(var) {}
CastExprNode::CastExprNode(const std::string& t, std::unique_ptr<ExprNode> e) : targetType(t), expr(std::move(e)) {}
ArrayAccessNode::ArrayAccessNode(const std::string& name, std::unique_ptr<ExprNode> idx) : arrayName(name), indexExpr(std::move(idx)) {}
FunctionCallNode::FunctionCallNode(const std::string& name, std::vector<std::unique_ptr<ExprNode>> a,
                                   const std::string& t)
    : funcName(name), args(std::move(a)), templateType(t) {}

std::string FunctionCallNode::getExprType() const {
    if (funcName == "input")
        return templateType.empty() ? "string" : templateType;
    if (funcName == "size" || funcName == "length" || funcName == "empty" || funcName == "resize")
        return "int";
    if (funcName == "pop" || funcName == "front" || funcName == "back" || funcName == "get" ||
        funcName == "insert" || funcName == "erase" || funcName == "delete" || funcName == "remove" ||
        funcName == "push" || funcName == "append")
        return "string";
    if (funcName == "readFile" || funcName == "writeFile" || funcName == "appendFile")
        return "string";
    if (funcName == "fileExists" || funcName == "deleteFile" || funcName == "renameFile" || funcName == "fileSize")
        return "int";
    return "unknown";
}

VarDeclarationNode::VarDeclarationNode(const std::string& t, bool p, bool c, const std::string& id, std::unique_ptr<ExprNode> init) : baseType(t), isPointer(p), isConst(c), identifier(id), initializer(std::move(init)) {}
ArrayDeclarationNode::ArrayDeclarationNode(const std::string& t, const std::string& n, int s, std::vector<std::unique_ptr<ExprNode>> inits)
    : type(t), name(n), size(s), initializers(std::move(inits)) {}
RetypeNode::RetypeNode(const std::string& t, const std::string& v) : newType(t), targetVar(v) {}
PrintNode::PrintNode(std::unique_ptr<ExprNode> expr) : expression(std::move(expr)) {}
ReturnNode::ReturnNode(std::unique_ptr<ExprNode> v) : value(std::move(v)) {}
FunctionDefNode::FunctionDefNode(const std::string& rt, const std::string& n,
                                 const std::vector<std::pair<std::string,std::string>> p,
                                 std::shared_ptr<BlockNode> b)
    : returnType(rt), name(n), params(p), body(std::move(b)) {}
FreeNode::FreeNode(const std::string& id) : identifier(id) {}
WhileNode::WhileNode(std::unique_ptr<ExprNode> cond, std::unique_ptr<BlockNode> b) : condition(std::move(cond)), body(std::move(b)) {}
DoWhileNode::DoWhileNode(std::unique_ptr<ExprNode> cond, std::unique_ptr<BlockNode> b) : condition(std::move(cond)), body(std::move(b)) {}
ForNode::ForNode(std::unique_ptr<ASTNode> init, std::unique_ptr<ExprNode> cond, std::unique_ptr<ExprNode> inc, std::unique_ptr<BlockNode> b) : init(std::move(init)), condition(std::move(cond)), increment(std::move(inc)), body(std::move(b)) {}
PostfixOpNode::PostfixOpNode(const std::string& name, const std::string& o) : varName(name), op(o) {}
AssignExprNode::AssignExprNode(const std::string& name, std::unique_ptr<ExprNode> val) : varName(name), value(std::move(val)) {}
BinaryOpNode::BinaryOpNode(std::string o, std::unique_ptr<ExprNode> l, std::unique_ptr<ExprNode> r) : op(std::move(o)), left(std::move(l)), right(std::move(r)) {}
MemberAssignNode::MemberAssignNode(const std::string& inst, const std::string& field,
                                   std::unique_ptr<ExprNode> val)
    : instanceName(inst), fieldName(field), value(std::move(val)) {}

IfNode::IfNode(std::unique_ptr<ExprNode> cond, std::unique_ptr<BlockNode> thenB)
    : condition(std::move(cond)), thenBlock(std::move(thenB)), elseBlock(nullptr) {}
void IfNode::addElseIf(std::unique_ptr<ExprNode> cond, std::unique_ptr<BlockNode> block) {
    elseIfBlocks.push_back({std::move(cond), std::move(block)});
}
void IfNode::setElse(std::unique_ptr<BlockNode> block) {
    elseBlock = std::move(block);
}

FStringNode::FStringNode(std::vector<std::pair<bool, std::string>> p) : parts(std::move(p)) {}

SelfAccessNode::SelfAccessNode(const std::string& member, bool isC,
                               std::vector<std::unique_ptr<ExprNode>> a)
    : memberName(member), isCall(isC), callArgs(std::move(a)) {}

MemberAccessNode::MemberAccessNode(const std::string& inst, const std::string& member,
                                   bool isC, std::vector<std::unique_ptr<ExprNode>> a)
    : instanceName(inst), memberName(member), isCall(isC), callArgs(std::move(a)) {}

MemberAccessStatement::MemberAccessStatement(std::unique_ptr<MemberAccessNode> n)
    : node(std::move(n)) {}

ClassDefNode::ClassDefNode(const std::string& name, const std::string& parent,
                           std::map<std::string, FieldDef> f,
                           std::map<std::string, MethodDef> m)
    : className(name), parentName(parent), fields(std::move(f)), methods(std::move(m)) {}

InstanceCreateNode::InstanceCreateNode(const std::string& cls, const std::string& inst,
                                       std::vector<std::unique_ptr<ExprNode>> a)
    : className(cls), instanceName(inst), args(std::move(a)) {}

StructDefNode::StructDefNode(const std::string& name, std::map<std::string, FieldDef> f)
    : structName(name), fields(std::move(f)) {}

// EXPRESSION EVALUATORS

std::string NumberLiteralNode::evaluate() const { return value; }
std::string StringLiteralNode::evaluate() const { return value; }
std::string BooleanLiteralNode::evaluate() const { return value ? "1" : "0"; }

std::string FStringNode::evaluate() const {
    std::string result;
    for (const auto& part : parts) {
        if (!part.first) {
            result += part.second;
        } else {
            std::string expr = part.second;
            size_t start = expr.find_first_not_of(' ');
            size_t end   = expr.find_last_not_of(' ');
            if (start != std::string::npos) expr = expr.substr(start, end - start + 1);

            try {
                Lexer miniLexer(expr);
                auto tokens = miniLexer.tokenize();
                Parser miniParser(tokens);
                auto ast = miniParser.parseLogicalOr();
                result += ast->evaluate();
            } catch (const std::exception&) {
                result += expr;
            }
        }
    }
    return result;
}

std::string VariableNode::evaluate() const {
    auto it = SYMBOL_TABLE.find(name);
    if (it != SYMBOL_TABLE.end()) return it->second.value;

    if (!CURRENT_INSTANCE.empty()) {
        auto iit = SYMBOL_TABLE.find(CURRENT_INSTANCE);
        if (iit != SYMBOL_TABLE.end()) {
            auto fit = iit->second.fields.find(name);
            if (fit != iit->second.fields.end())
                return fit->second.value;
            std::string cls = iit->second.instanceOf;
            auto cit = SYMBOL_TABLE.find(cls);
            if (cit != SYMBOL_TABLE.end()) {
                auto gfit = cit->second.fields.find(name);
                if (gfit != cit->second.fields.end() && gfit->second.access == "global")
                    return gfit->second.value;
            }
        }
    }
    throw std::runtime_error("Undefined variable: '" + name + "'");
}

std::string PostfixOpNode::evaluate() const {
    auto it = SYMBOL_TABLE.find(varName);
    if (it == SYMBOL_TABLE.end())
        throw std::runtime_error("Undefined variable: " + varName);
    std::string oldVal = it->second.value;
    double v = 0;
    try { v = std::stod(oldVal); } catch (...) {}
    if (op == "++") v += 1;
    else            v -= 1;
    it->second.value = formatNum(v);
    return oldVal;
}

std::string AssignExprNode::evaluate() const {
    auto it = SYMBOL_TABLE.find(varName);
    if (it != SYMBOL_TABLE.end() && it->second.isConst)
        throw std::runtime_error("Cannot reassign const variable: " + varName);

    // Array literal assignment: arr = [1, 2, 3];
    ArrayLiteralNode* aln = dynamic_cast<ArrayLiteralNode*>(value.get());
    if (aln != nullptr) {
        RuntimeValue* arr = nullptr;
        if (it != SYMBOL_TABLE.end() && it->second.type.find("[]") != std::string::npos) {
            arr = &it->second;
        } else if (it == SYMBOL_TABLE.end()) {
            SYMBOL_TABLE[varName] = {"auto[]", ""};
            arr = &SYMBOL_TABLE[varName];
        }
        if (arr != nullptr) {
            arr->arrayElements.clear();
            for (auto& elem : aln->getElements()) {
                arr->arrayElements.push_back(elem->evaluate());
            }
            return varName;
        }
    }

    std::string result = value->evaluate();

    // Check instance fields first when inside a method
    if (!CURRENT_INSTANCE.empty()) {
        auto iit = SYMBOL_TABLE.find(CURRENT_INSTANCE);
        if (iit != SYMBOL_TABLE.end()) {
            auto fit = iit->second.fields.find(varName);
            if (fit != iit->second.fields.end()) {
                if (fit->second.isConst)
                    throw std::runtime_error("Cannot reassign const field: " + varName);
                fit->second.value = result;
                return result;
            }
        }
    }

    if (it != SYMBOL_TABLE.end()) {
        it->second.value = result;
    } else {
        SYMBOL_TABLE[varName] = {"auto", result};
    }
    return result;
}

std::string MemberAssignNode::evaluate() const {
    std::string result = value->evaluate();
    setInstanceField(instanceName, fieldName, result);
    return result;
}

std::string CastOrRefNode::evaluate() const {
    if (operation == "@") {
        auto it = SYMBOL_TABLE.find(targetVar);
        if (it != SYMBOL_TABLE.end()) {
            const void* address = static_cast<const void*>(&it->second);
            std::stringstream ss; ss << address;
            return ss.str();
        }
        return "NULL";
    }

    std::string currentValStr = "0";
    size_t dotPos = targetVar.find('.');
    if (dotPos != std::string::npos) {
        std::string instName  = targetVar.substr(0, dotPos);
        std::string fieldName = targetVar.substr(dotPos + 1);
        auto iit = SYMBOL_TABLE.find(instName);
        if (iit != SYMBOL_TABLE.end()) {
            RuntimeValue& inst = iit->second;
            std::string cls = inst.instanceOf.empty() ? instName : inst.instanceOf;
            auto cit = SYMBOL_TABLE.find(cls);
            if (cit != SYMBOL_TABLE.end()) {
                auto fit = cit->second.fields.find(fieldName);
                if (fit != cit->second.fields.end() && fit->second.access == "global")
                    currentValStr = fit->second.value;
                else {
                    auto ifit = inst.fields.find(fieldName);
                    if (ifit != inst.fields.end())
                        currentValStr = ifit->second.value;
                }
            } else {
                auto ifit = inst.fields.find(fieldName);
                if (ifit != inst.fields.end())
                    currentValStr = ifit->second.value;
            }
        }
    } else {
        auto it = SYMBOL_TABLE.find(targetVar);
        if (it != SYMBOL_TABLE.end()) currentValStr = it->second.value;
    }

    if (operation.find("int")    != std::string::npos) {
        try { return std::to_string((int)std::stod(currentValStr)); } catch (...) { return "0"; }
    }
    if (operation.find("string") != std::string::npos) return currentValStr;
    if (operation.find("float")  != std::string::npos ||
        operation.find("double") != std::string::npos) {
        try { return formatNum(std::stod(currentValStr)); } catch (...) { return "0"; }
    }
    return currentValStr;
}

std::string CastExprNode::evaluate() const {
    std::string val = expr->evaluate();
    if (targetType.find("int")    != std::string::npos) {
        try { return std::to_string((int)std::stod(val)); } catch (...) { return "0"; }
    }
    if (targetType.find("string") != std::string::npos) return val;
    if (targetType.find("float")  != std::string::npos ||
        targetType.find("double") != std::string::npos) {
        try { return formatNum(std::stod(val)); } catch (...) { return "0"; }
    }
    return val;
}

std::string ArrayAccessNode::evaluate() const {
    auto it = SYMBOL_TABLE.find(arrayName);
    if (it == SYMBOL_TABLE.end()) throw std::runtime_error("Undefined array: " + arrayName);
    if (it->second.type.find("[]") == std::string::npos) throw std::runtime_error("Not an array: " + arrayName);
    std::string idxStr = indexExpr->evaluate();
    int idx = 0;
    try { idx = std::stoi(idxStr); } catch (...) { throw std::runtime_error("Invalid array index: " + idxStr); }
    if (idx >= 0 && idx < (int)it->second.arrayElements.size())
        return it->second.arrayElements[idx];
    throw std::runtime_error("Array index out of bounds: " + std::to_string(idx));
}

ArrayAssignNode::ArrayAssignNode(const std::string& name, std::unique_ptr<ExprNode> idx, std::unique_ptr<ExprNode> val)
    : arrayName(name), indexExpr(std::move(idx)), valueExpr(std::move(val)) {}

std::string ArrayAssignNode::evaluate() const {
    auto it = SYMBOL_TABLE.find(arrayName);
    if (it == SYMBOL_TABLE.end()) throw std::runtime_error("Undefined array: " + arrayName);
    if (it->second.type.find("[]") == std::string::npos) throw std::runtime_error("Not an array: " + arrayName);
    std::string idxStr = indexExpr->evaluate();
    int idx = 0;
    try { idx = std::stoi(idxStr); } catch (...) { throw std::runtime_error("Invalid array index: " + idxStr); }
    std::string val = valueExpr->evaluate();
    if (idx >= 0 && idx < (int)it->second.arrayElements.size()) {
        it->second.arrayElements[idx] = val;
        return val;
    }
    throw std::runtime_error("Array index out of bounds: " + std::to_string(idx));
}

ArrayCompoundAssignNode::ArrayCompoundAssignNode(const std::string& name, std::unique_ptr<ExprNode> idx, std::string op, std::unique_ptr<ExprNode> val)
    : arrayName(name), indexExpr(std::move(idx)), op(std::move(op)), valueExpr(std::move(val)) {}

std::string ArrayCompoundAssignNode::evaluate() const {
    auto it = SYMBOL_TABLE.find(arrayName);
    if (it == SYMBOL_TABLE.end()) throw std::runtime_error("Undefined array: " + arrayName);
    if (it->second.type.find("[]") == std::string::npos) throw std::runtime_error("Not an array: " + arrayName);
    std::string idxStr = indexExpr->evaluate();
    int idx = 0;
    try { idx = std::stoi(idxStr); } catch (...) { throw std::runtime_error("Invalid array index: " + idxStr); }
    if (idx < 0 || idx >= (int)it->second.arrayElements.size())
        throw std::runtime_error("Array index out of bounds: " + std::to_string(idx));
    std::string current = it->second.arrayElements[idx];
    std::string val = valueExpr->evaluate();
    std::string result;
    if ((!isNumeric(current) || !isNumeric(val)) && op == "+") {
        result = current + val;
    } else if (!isNumeric(current) || !isNumeric(val)) {
        throw std::runtime_error("Cannot apply " + op + " to non-numeric array elements");
    } else {
        double l = std::stod(current), r = std::stod(val);
        if (op == "+") result = formatNum(l + r);
        else if (op == "-") result = formatNum(l - r);
        else if (op == "*") result = formatNum(l * r);
        else if (op == "/") {
            if (r == 0) { std::cerr << "Runtime Error: Division by Zero\n"; result = "0"; }
            else result = formatNum(l / r);
        }
        else if (op == "%") result = formatNum(std::fmod(l, r));
        else throw std::runtime_error("Unknown compound operator: " + op);
    }
    it->second.arrayElements[idx] = result;
    return result;
}

std::string FunctionCallNode::evaluate() const {
    if (funcName == "input") {
        if (!args.empty()) {
            std::cout << args[0]->evaluate();
            std::cout.flush();
        }
        std::string line;
        std::getline(std::cin, line);
        if (templateType.empty()) return line; // default: string

        // convert to requested template type and return as string representation
        if (templateType.find("int") != std::string::npos) {
            try {
                int v = std::stoi(line);
                return std::to_string(v);
            } catch (...) {
                throw std::runtime_error("Input conversion error: expected int");
            }
        }
        if (templateType.find("float") != std::string::npos ||
            templateType.find("double") != std::string::npos) {
            try {
                double v = std::stod(line);
                return formatNum(v);
            } catch (...) {
                throw std::runtime_error("Input conversion error: expected float/double");
            }
        }
        if (templateType.find("bool") != std::string::npos) {
            if (line == "true") return "1";
            if (line == "false") return "0";
            try { double v = std::stod(line); return (v != 0) ? "1" : "0"; } catch (...) {
                throw std::runtime_error("Input conversion error: expected bool");
            }
        }

        // fallback: return string
        return line;
    }

    auto getArrayVarName = [&](size_t argIdx) -> std::string {
        if (argIdx >= args.size()) throw std::runtime_error("Array function missing argument " + std::to_string(argIdx));
        VariableNode* vn = dynamic_cast<VariableNode*>(args[argIdx].get());
        if (!vn) throw std::runtime_error("Array function argument " + std::to_string(argIdx) + " must be an array variable");
        return vn->getName();
    };
    auto requireArray = [&](const std::string& name) -> RuntimeValue& {
        auto it = SYMBOL_TABLE.find(name);
        if (it == SYMBOL_TABLE.end()) throw std::runtime_error("Undefined array: " + name);
        if (it->second.type.find("[]") == std::string::npos) throw std::runtime_error("Not an array: " + name);
        return it->second;
    };

    if (funcName == "push" || funcName == "append") {
        std::string arrName = getArrayVarName(0);
        RuntimeValue& arr = requireArray(arrName);
        std::string val = args.size() > 1 ? args[1]->evaluate() : "0";
        arr.arrayElements.push_back(val);
        return val;
    }
    if (funcName == "pop") {
        std::string arrName = getArrayVarName(0);
        RuntimeValue& arr = requireArray(arrName);
        if (arr.arrayElements.empty()) throw std::runtime_error("pop from empty array");
        std::string val = arr.arrayElements.back();
        arr.arrayElements.pop_back();
        return val;
    }
    if (funcName == "insert") {
        std::string arrName = getArrayVarName(0);
        RuntimeValue& arr = requireArray(arrName);
        if (args.size() < 2) throw std::runtime_error("insert requires (array, index, value)");
        int idx = std::stoi(args[1]->evaluate());
        std::string val = args.size() > 2 ? args[2]->evaluate() : "0";
        if (idx < 0) idx = 0;
        if (idx > (int)arr.arrayElements.size()) idx = arr.arrayElements.size();
        arr.arrayElements.insert(arr.arrayElements.begin() + idx, val);
        return val;
    }
    if (funcName == "erase" || funcName == "delete" || funcName == "remove") {
        std::string arrName = getArrayVarName(0);
        RuntimeValue& arr = requireArray(arrName);
        if (args.size() < 2) throw std::runtime_error("erase/delete/remove requires (array, index)");
        int idx = std::stoi(args[1]->evaluate());
        if (idx < 0 || idx >= (int)arr.arrayElements.size()) throw std::runtime_error("erase index out of bounds");
        std::string val = arr.arrayElements[idx];
        arr.arrayElements.erase(arr.arrayElements.begin() + idx);
        return val;
    }
    if (funcName == "size" || funcName == "length") {
        std::string arrName = getArrayVarName(0);
        RuntimeValue& arr = requireArray(arrName);
        return std::to_string(arr.arrayElements.size());
    }
    if (funcName == "clear") {
        std::string arrName = getArrayVarName(0);
        RuntimeValue& arr = requireArray(arrName);
        arr.arrayElements.clear();
        return "0";
    }
    if (funcName == "resize") {
        std::string arrName = getArrayVarName(0);
        RuntimeValue& arr = requireArray(arrName);
        if (args.size() < 2) throw std::runtime_error("resize requires (array, newSize)");
        int newSize = std::stoi(args[1]->evaluate());
        if (newSize < 0) newSize = 0;
        arr.arrayElements.resize(newSize, "0");
        return std::to_string(newSize);
    }
    if (funcName == "front") {
        std::string arrName = getArrayVarName(0);
        RuntimeValue& arr = requireArray(arrName);
        if (arr.arrayElements.empty()) throw std::runtime_error("front from empty array");
        return arr.arrayElements.front();
    }
    if (funcName == "back") {
        std::string arrName = getArrayVarName(0);
        RuntimeValue& arr = requireArray(arrName);
        if (arr.arrayElements.empty()) throw std::runtime_error("back from empty array");
        return arr.arrayElements.back();
    }
    if (funcName == "empty") {
        std::string arrName = getArrayVarName(0);
        RuntimeValue& arr = requireArray(arrName);
        return arr.arrayElements.empty() ? "1" : "0";
    }

    if (funcName == "readFile") {
        if (args.empty()) throw std::runtime_error("readFile requires a filename");
        std::string filename = args[0]->evaluate();
        std::ifstream file(filename);
        if (!file) throw std::runtime_error("Cannot open file for reading: " + filename);
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        return content;
    }
    if (funcName == "writeFile") {
        if (args.size() < 2) throw std::runtime_error("writeFile requires (filename, content)");
        std::string filename = args[0]->evaluate();
        std::string content = args[1]->evaluate();
        std::ofstream file(filename);
        if (!file) throw std::runtime_error("Cannot open file for writing: " + filename);
        file << content;
        return content;
    }
    if (funcName == "appendFile") {
        if (args.size() < 2) throw std::runtime_error("appendFile requires (filename, content)");
        std::string filename = args[0]->evaluate();
        std::string content = args[1]->evaluate();
        std::ofstream file(filename, std::ios::app);
        if (!file) throw std::runtime_error("Cannot open file for appending: " + filename);
        file << content;
        return content;
    }
    if (funcName == "fileExists") {
        if (args.empty()) return "0";
        std::string filename = args[0]->evaluate();
        std::ifstream file(filename);
        return file.good() ? "1" : "0";
    }
    if (funcName == "deleteFile") {
        if (args.empty()) return "0";
        std::string filename = args[0]->evaluate();
        return (std::remove(filename.c_str()) == 0) ? "1" : "0";
    }
    if (funcName == "renameFile") {
        if (args.size() < 2) throw std::runtime_error("renameFile requires (oldPath, newPath)");
        std::string oldName = args[0]->evaluate();
        std::string newName = args[1]->evaluate();
        return (std::rename(oldName.c_str(), newName.c_str()) == 0) ? "1" : "0";
    }
    if (funcName == "fileSize") {
        if (args.empty()) return "0";
        std::string filename = args[0]->evaluate();
        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        if (!file) return "0";
        return std::to_string(file.tellg());
    }

    auto it = SYMBOL_TABLE.find(funcName);
    if (it == SYMBOL_TABLE.end())
        throw std::runtime_error("Undefined function: '" + funcName + "'");

    std::vector<std::string> argValues, argTypes;
    std::vector<std::vector<std::string>> argArrayElems;
    for (auto& arg : args) {
        std::string val = arg->evaluate();
        argValues.push_back(val);
        argTypes.push_back(inferType(val));
        std::vector<std::string> elems;
        VariableNode* vn = dynamic_cast<VariableNode*>(arg.get());
        if (vn) {
            auto vit = SYMBOL_TABLE.find(vn->getName());
            if (vit != SYMBOL_TABLE.end() && vit->second.type.find("[]") != std::string::npos) {
                elems = vit->second.arrayElements;
                argTypes.back() = vit->second.type;
            }
        }
        ArrayLiteralNode* aln = dynamic_cast<ArrayLiteralNode*>(arg.get());
        if (aln) {
            for (auto& elem : aln->getElements()) {
                elems.push_back(elem->evaluate());
            }
            argTypes.back() = "auto[]";
        }
        argArrayElems.push_back(elems);
    }

    RuntimeValue func = it->second;
    auto prevScope = SYMBOL_TABLE;

    for (size_t i = 0; i < func.params.size() && i < argValues.size(); i++) {
        const std::string& ptype = func.params[i].first;
        const std::string& pname = func.params[i].second;
        if (!typesCompatible(ptype, argTypes[i])) {
            throw std::runtime_error("Type mismatch: param '" + pname +
                                     "' expects " + ptype + " but got " + argTypes[i]);
        }
        if (!argArrayElems[i].empty() || ptype.find("[]") != std::string::npos) {
            RuntimeValue arr;
            arr.type = ptype;
            arr.value = "";
            arr.arrayElements = argArrayElems[i];
            SYMBOL_TABLE[pname] = arr;
        } else {
            SYMBOL_TABLE[pname] = {ptype, argValues[i]};
        }
    }

    std::string result = "0";
    try {
        if (func.body) func.body->execute();
    } catch (const ReturnException& ret) { result = ret.value; }

    // Copy back array parameter changes to original variables
    for (size_t i = 0; i < func.params.size() && i < args.size(); i++) {
        VariableNode* vn = dynamic_cast<VariableNode*>(args[i].get());
        if (vn && (!argArrayElems[i].empty() || func.params[i].first.find("[]") != std::string::npos)) {
            auto pit = SYMBOL_TABLE.find(func.params[i].second);
            if (pit != SYMBOL_TABLE.end() && pit->second.type.find("[]") != std::string::npos) {
                auto vit = prevScope.find(vn->getName());
                if (vit != prevScope.end() && vit->second.type.find("[]") != std::string::npos) {
                    prevScope[vn->getName()].arrayElements = pit->second.arrayElements;
                }
            }
        }
    }

    SYMBOL_TABLE = prevScope;
    return result;
}

std::string BinaryOpNode::evaluate() const {
    // +, -, *, /, %, ==, !=, <, >, &&, ||
    std::string lStr = left->evaluate();
    std::string rStr = right->evaluate();

    // String concatenation
    if ((!isNumeric(lStr) || !isNumeric(rStr)) && op == "+")
        return lStr + rStr;

    // String equality — compare as strings if either side is non-numeric
    if (!isNumeric(lStr) || !isNumeric(rStr)) {
        if (op == "==") return (lStr == rStr) ? "1" : "0";
        if (op == "!=") return (lStr != rStr) ? "1" : "0";
        return "0";
    }

    double l = 0, r = 0;
    try { l = std::stod(lStr); r = std::stod(rStr); } catch (...) { return "0"; }

    if (op == "+")  return formatNum(l + r);
    if (op == "-")  return formatNum(l - r);
    if (op == "*")  return formatNum(l * r);
    if (op == "/") {
        if (r == 0) { std::cerr << "Runtime Error: Division by Zero\n"; return "0"; }
        return formatNum(l / r);
    }
    if (op == "%")  return formatNum(std::fmod(l, r));
    if (op == "==") return (l == r)  ? "1" : "0";
    if (op == "!=") return (l != r)  ? "1" : "0";
    if (op == "<")  return (l <  r)  ? "1" : "0";
    if (op == ">")  return (l >  r)  ? "1" : "0";
    if (op == "<=") return (l <= r)  ? "1" : "0";
    if (op == ">=") return (l >= r)  ? "1" : "0";
    if (op == "&&") return ((l != 0) && (r != 0)) ? "1" : "0";
    if (op == "||") return ((l != 0) || (r != 0)) ? "1" : "0";
    return "0";
}

std::string SelfAccessNode::evaluate() const {
    // self.field or self.method(...) inside a class
    if (CURRENT_INSTANCE.empty())
        throw std::runtime_error("'self' used outside of a class method");
    if (isCall) {
        std::vector<std::string> argVals, argTypes;
        std::vector<std::vector<std::string>> argArrayElems;
        std::vector<std::string> argOriginalNames;
        for (auto& a : callArgs) {
            std::string val = a->evaluate();
            argVals.push_back(val);
            argTypes.push_back(inferType(val));
            std::vector<std::string> elems;
            VariableNode* vn = dynamic_cast<VariableNode*>(a.get());
            if (vn) {
                auto vit = SYMBOL_TABLE.find(vn->getName());
                if (vit != SYMBOL_TABLE.end() && vit->second.type.find("[]") != std::string::npos) {
                    elems = vit->second.arrayElements;
                    argTypes.back() = vit->second.type;
                }
            }
            ArrayLiteralNode* aln = dynamic_cast<ArrayLiteralNode*>(a.get());
            if (aln) {
                for (auto& elem : aln->getElements()) {
                    elems.push_back(elem->evaluate());
                }
                argTypes.back() = "auto[]";
            }
            argArrayElems.push_back(elems);
            argOriginalNames.push_back(vn ? vn->getName() : "");
        }
        return callMethod(CURRENT_INSTANCE, memberName, argVals, argTypes, argArrayElems, argOriginalNames);
    }
    return getInstanceField(CURRENT_INSTANCE, memberName, CURRENT_CLASS);
}

std::string MemberAccessNode::evaluate() const {
    // instance.field or instance.method(...)
    auto iit = SYMBOL_TABLE.find(instanceName);
    if (iit == SYMBOL_TABLE.end())
        throw std::runtime_error("Unknown identifier: " + instanceName);

    RuntimeValue& inst = iit->second;

    if (inst.type == "struct") {
        auto fit = inst.fields.find(memberName);
        if (fit != inst.fields.end()) return fit->second.value;
        throw std::runtime_error("Field '" + memberName + "' not found on struct " + instanceName);
    }

    if (inst.type == "class") {
        auto fit = inst.fields.find(memberName);
        if (fit != inst.fields.end()) return fit->second.value;
        throw std::runtime_error("Field '" + memberName + "' not found on class " + instanceName);
    }

    if (inst.type.find("[]") != std::string::npos) {
        if (!isCall) {
            throw std::runtime_error("Array does not have field: " + memberName);
        }
        std::vector<std::string> argVals;
        for (auto& a : callArgs) {
            argVals.push_back(a->evaluate());
        }
        auto& arr = inst.arrayElements;
        if (memberName == "push" || memberName == "append") {
            if (argVals.size() < 1) throw std::runtime_error("push/append requires 1 argument");
            arr.push_back(argVals[0]);
            return argVals[0];
        }
        if (memberName == "pop") {
            if (arr.empty()) throw std::runtime_error("pop from empty array");
            std::string val = arr.back();
            arr.pop_back();
            return val;
        }
        if (memberName == "insert") {
            if (argVals.size() < 2) throw std::runtime_error("insert requires 2 arguments (index, value)");
            int idx = std::stoi(argVals[0]);
            if (idx < 0) idx = 0;
            if (idx > (int)arr.size()) idx = arr.size();
            arr.insert(arr.begin() + idx, argVals[1]);
            return argVals[1];
        }
        if (memberName == "erase" || memberName == "delete" || memberName == "remove") {
            if (argVals.size() < 1) throw std::runtime_error("erase/delete/remove requires 1 argument (index)");
            int idx = std::stoi(argVals[0]);
            if (idx < 0 || idx >= (int)arr.size()) throw std::runtime_error("erase index out of bounds");
            std::string val = arr[idx];
            arr.erase(arr.begin() + idx);
            return val;
        }
        if (memberName == "size" || memberName == "length") {
            return std::to_string(arr.size());
        }
        if (memberName == "clear") {
            arr.clear();
            return "0";
        }
        if (memberName == "resize") {
            if (argVals.size() < 1) throw std::runtime_error("resize requires 1 argument (new size)");
            int newSize = std::stoi(argVals[0]);
            if (newSize < 0) newSize = 0;
            arr.resize(newSize, "0");
            return std::to_string(newSize);
        }
        if (memberName == "front") {
            if (arr.empty()) throw std::runtime_error("front from empty array");
            return arr.front();
        }
        if (memberName == "back") {
            if (arr.empty()) throw std::runtime_error("back from empty array");
            return arr.back();
        }
        if (memberName == "empty") {
            return arr.empty() ? "1" : "0";
        }
        if (memberName == "get") {
            if (argVals.size() < 1) throw std::runtime_error("get requires 1 argument (index)");
            int idx = std::stoi(argVals[0]);
            if (idx < 0 || idx >= (int)arr.size()) throw std::runtime_error("get index out of bounds");
            return arr[idx];
        }
        if (memberName == "set") {
            if (argVals.size() < 2) throw std::runtime_error("set requires 2 arguments (index, value)");
            int idx = std::stoi(argVals[0]);
            if (idx < 0 || idx >= (int)arr.size()) throw std::runtime_error("set index out of bounds");
            arr[idx] = argVals[1];
            return argVals[1];
        }
        throw std::runtime_error("Unknown array method: " + memberName);
    }

    if (isCall) {
        std::vector<std::string> argVals, argTypes;
        std::vector<std::vector<std::string>> argArrayElems;
        std::vector<std::string> argOriginalNames;
        for (auto& a : callArgs) {
            std::string val = a->evaluate();
            argVals.push_back(val);
            argTypes.push_back(inferType(val));
            std::vector<std::string> elems;
            VariableNode* vn = dynamic_cast<VariableNode*>(a.get());
            if (vn) {
                auto vit = SYMBOL_TABLE.find(vn->getName());
                if (vit != SYMBOL_TABLE.end() && vit->second.type.find("[]") != std::string::npos) {
                    elems = vit->second.arrayElements;
                    argTypes.back() = vit->second.type;
                }
            }
            ArrayLiteralNode* aln = dynamic_cast<ArrayLiteralNode*>(a.get());
            if (aln) {
                for (auto& elem : aln->getElements()) {
                    elems.push_back(elem->evaluate());
                }
                argTypes.back() = "auto[]";
            }
            argArrayElems.push_back(elems);
            argOriginalNames.push_back(vn ? vn->getName() : "");
        }
        return callMethod(instanceName, memberName, argVals, argTypes, argArrayElems, argOriginalNames);
    }

    std::string className = inst.instanceOf;
    auto cit = SYMBOL_TABLE.find(className);
    if (cit != SYMBOL_TABLE.end()) {
        auto fit = cit->second.fields.find(memberName);
        if (fit != cit->second.fields.end() && fit->second.access == "global")
            return fit->second.value;
    }
    return getInstanceField(instanceName, memberName, CURRENT_CLASS);
}

std::string MemberAccessNode::getExprType() const {
    if (memberName == "size" || memberName == "length" || memberName == "empty" || memberName == "resize")
        return "int";
    if (memberName == "pop" || memberName == "front" || memberName == "back" || memberName == "get" ||
        memberName == "insert" || memberName == "erase" || memberName == "delete" || memberName == "remove" ||
        memberName == "push" || memberName == "append")
        return "string";
    return "unknown";
}

// STATEMENT EXECUTORS

void VarDeclarationNode::execute() {
    // declare a variable with optional type checking
    auto it = SYMBOL_TABLE.find(identifier);
    if (it != SYMBOL_TABLE.end() && it->second.isConst)
        throw std::runtime_error("Cannot reassign const variable: " + identifier);

    std::string val = initializer ? initializer->evaluate() : "0";

    // Type enforcement: infer initializer type and ensure compatibility with declared type
    if (!isPointer) {
        std::string initType = initializer ? initializer->getExprType() : "unknown";
        if (initType == "unknown")
            initType = inferType(val);
        // allow unknown (e.g., when value can't be inferred) to pass
        if (initType != "unknown") {
            if (!typesCompatible(baseType, initType)) {
                throw std::runtime_error("Type mismatch: variable '" + identifier + "' expects "
                                         + baseType + " but got " + initType);
            }
        }
    }

    RuntimeValue rv;
    rv.type    = baseType + (isPointer ? "*" : "");
    rv.value   = val;
    rv.isConst = isConst;
    SYMBOL_TABLE[identifier] = rv;
}

void RetypeNode::execute() {
    auto it = SYMBOL_TABLE.find(targetVar);
    if (it != SYMBOL_TABLE.end()) it->second.type = newType;
}

void ArrayDeclarationNode::execute() {
    // allocate an array with optional initial values
    RuntimeValue v; v.type = type + "[]";
    int actualSize = size;
    if (actualSize == 0 && !initializers.empty()) {
        actualSize = (int)initializers.size();
    }
    for (int i = 0; i < actualSize; i++) v.arrayElements.push_back("0");
    for (size_t i = 0; i < initializers.size() && i < v.arrayElements.size(); i++) {
        v.arrayElements[i] = initializers[i]->evaluate();
    }
    SYMBOL_TABLE[name] = v;
}

ArrayLiteralNode::ArrayLiteralNode(std::vector<std::unique_ptr<ExprNode>> elems) : elements(std::move(elems)) {}

std::string ArrayLiteralNode::evaluate() const {
    // build a string like "[1, 2, 3]"
    std::string result = "[";
    for (size_t i = 0; i < elements.size(); i++) {
        if (i > 0) result += ", ";
        result += elements[i]->evaluate();
    }
    result += "]";
    return result;
}

ArrayReassignNode::ArrayReassignNode(const std::string& name, std::vector<std::unique_ptr<ExprNode>> elems)
    : arrayName(name), elements(std::move(elems)) {}

std::string ArrayReassignNode::evaluate() const {
    // wipe and refill an existing array
    auto it = SYMBOL_TABLE.find(arrayName);
    if (it == SYMBOL_TABLE.end()) throw std::runtime_error("Undefined array: " + arrayName);
    if (it->second.type.find("[]") == std::string::npos) throw std::runtime_error("Not an array: " + arrayName);
    it->second.arrayElements.clear();
    for (auto& elem : elements) {
        it->second.arrayElements.push_back(elem->evaluate());
    }
    return arrayName;
}

void FunctionDefNode::execute() {
    // store the function in the symbol table
    RuntimeValue rv;
    rv.type   = "function";
    rv.value  = "";
    rv.params = params;
    rv.body   = body; // shared_ptr copy — body never destroyed
    SYMBOL_TABLE[name] = rv;
}

void BlockNode::execute() {
    // run every statement in order
    for (const auto& s : statements) if (s) s->execute();
}

void FreeNode::execute() {
    // remove a variable from the symbol table
    auto it = SYMBOL_TABLE.find(identifier);
    if (it != SYMBOL_TABLE.end()) SYMBOL_TABLE.erase(it);
    else std::cerr << "[!] Warning: Freeing non-existent '" << identifier << "'\n";
}

// break and continue throw their exceptions to be caught by loop nodes
void BreakNode::execute()    { throw BreakException{};    }
void ContinueNode::execute() { throw ContinueException{}; }

void IfNode::execute() {
    // if / else if / else chain
    if (condition->evaluate() != "0") { thenBlock->execute(); return; }
    for (auto& ei : elseIfBlocks)
        if (ei.first->evaluate() != "0") { ei.second->execute(); return; }
    if (elseBlock) elseBlock->execute();
}

void WhileNode::execute() {
    // standard while loop with break/continue support
    while (condition->evaluate() != "0") {
        try {
            body->execute();
        } catch (const BreakException&) {
            return;
        } catch (const ContinueException&) {
            // re-evaluate condition on next iteration
        }
    }
}

void DoWhileNode::execute() {
    // do-while loop with break/continue support
    do {
        try {
            body->execute();
        } catch (const BreakException&) {
            return;
        } catch (const ContinueException&) {
            // fall through to condition check
        }
    } while (condition->evaluate() != "0");
}

void ForNode::execute() {
    // C-style for loop with break/continue support
    if (init) init->execute();
    while (condition->evaluate() != "0") {
        try {
            body->execute();
        } catch (const BreakException&) {
            return;
        } catch (const ContinueException&) {
            if (increment) increment->evaluate(); // still run increment on continue
            continue;
        }
        if (increment) increment->evaluate();
    }
}

void ReturnNode::execute() { throw ReturnException{value ? value->evaluate() : "0"}; }
void PrintNode::execute()  { std::cout << expression->evaluate() << "\n"; }
void MemberAccessStatement::execute() { node->evaluate(); }

// CLASS DEFINITION

void ClassDefNode::execute() {
    // register a class, inheriting parent fields/methods
    RuntimeValue classVal;
    classVal.type        = "class";
    classVal.parentClass = parentName;

    if (!parentName.empty()) {
        auto pit = SYMBOL_TABLE.find(parentName);
        if (pit != SYMBOL_TABLE.end()) {
            for (const auto& kv : pit->second.fields)
                if (kv.second.access != "private")
                    classVal.fields[kv.first] = kv.second;
            classVal.methods = pit->second.methods;
        }
    }

    for (const auto& kv : fields)  classVal.fields[kv.first]  = kv.second;
    for (const auto& kv : methods) classVal.methods[kv.first] = kv.second;

    SYMBOL_TABLE[className] = std::move(classVal);
}

// STRUCT DEFINITION

void StructDefNode::execute() {
    // register a simple struct type
    RuntimeValue sv;
    sv.type   = "struct";
    sv.fields = fields;
    SYMBOL_TABLE[structName] = std::move(sv);
}

// INSTANCE CREATION

void InstanceCreateNode::execute() {
    // create an object instance and run its constructor
    auto cit = SYMBOL_TABLE.find(className);
    if (cit == SYMBOL_TABLE.end())
        throw std::runtime_error("Unknown class: " + className);

    RuntimeValue inst;
    inst.type       = "instance:" + className;
    inst.instanceOf = className;
    // Copy non-global fields; re-evaluate initExpr so defaults are fresh per instance
    for (const auto& kv : cit->second.fields) {
        if (kv.second.access != "global") {
            FieldDef fd = kv.second;
            if (fd.initExpr) fd.value = fd.initExpr->evaluate();
            inst.fields[kv.first] = fd;
        }
    }

    SYMBOL_TABLE[instanceName] = inst;

    auto ctorIt = cit->second.methods.find(className);
    if (ctorIt == cit->second.methods.end()) return;

    MethodDef ctor = ctorIt->second;

    std::vector<std::string> argVals;
    for (auto& a : args) argVals.push_back(a->evaluate());

    std::string outerClass    = CURRENT_CLASS;
    std::string outerInstance = CURRENT_INSTANCE;
    CURRENT_CLASS    = className;
    CURRENT_INSTANCE = instanceName;

    if (!ctor.parentConstructorClass.empty()) {
        std::string parentCls = ctor.parentConstructorClass;
        auto pcit = SYMBOL_TABLE.find(parentCls);
        if (pcit != SYMBOL_TABLE.end()) {
            auto pmit = pcit->second.methods.find(parentCls);
            if (pmit != pcit->second.methods.end()) {
                MethodDef parentCtor = pmit->second;

                std::vector<std::string> parentArgVals;
                for (const std::string& argName : ctor.parentConstructorArgs) {
                    bool found = false;
                    for (size_t i = 0; i < ctor.params.size(); i++) {
                        if (ctor.params[i].second == argName && i < argVals.size()) {
                            parentArgVals.push_back(argVals[i]);
                            found = true; break;
                        }
                    }
                    if (!found) parentArgVals.push_back("0");
                }

                for (size_t i = 0; i < parentCtor.constructorBindings.size()
                                   && i < parentArgVals.size(); i++) {
                    const std::string& fn = parentCtor.constructorBindings[i];
                    auto fit = SYMBOL_TABLE[instanceName].fields.find(fn);
                    if (fit != SYMBOL_TABLE[instanceName].fields.end())
                        fit->second.value = parentArgVals[i];
                    else
                        SYMBOL_TABLE[instanceName].fields[fn] = {"public", "string", parentArgVals[i]};
                }

                injectGlobals(parentCls);
                auto prevSymbols = SYMBOL_TABLE;
                for (size_t i = 0; i < parentCtor.params.size() && i < parentArgVals.size(); i++)
                    SYMBOL_TABLE[parentCtor.params[i].second] =
                        {parentCtor.params[i].first, parentArgVals[i]};
                try { if (parentCtor.body) parentCtor.body->execute(); }
                catch (const ReturnException&) {}
                auto globalUpdates = captureGlobals(parentCls);
                RuntimeValue updatedInst = SYMBOL_TABLE[instanceName];
                SYMBOL_TABLE = prevSymbols;
                SYMBOL_TABLE[instanceName] = updatedInst;
                reapplyGlobals(globalUpdates);
            }
        }
    }

    for (size_t i = 0; i < ctor.constructorBindings.size() && i < argVals.size(); i++) {
        const std::string& fn = ctor.constructorBindings[i];
        auto fit = SYMBOL_TABLE[instanceName].fields.find(fn);
        if (fit != SYMBOL_TABLE[instanceName].fields.end())
            fit->second.value = argVals[i];
    }

    injectGlobals(className);
    auto prevSymbols = SYMBOL_TABLE;
    for (size_t i = 0; i < ctor.params.size() && i < argVals.size(); i++)
        SYMBOL_TABLE[ctor.params[i].second] = {ctor.params[i].first, argVals[i]};

    try { if (ctor.body) ctor.body->execute(); }
    catch (const ReturnException&) {}

    auto globalUpdates = captureGlobals(className);
    RuntimeValue updatedInst = SYMBOL_TABLE[instanceName];
    SYMBOL_TABLE = prevSymbols;
    SYMBOL_TABLE[instanceName] = updatedInst;
    reapplyGlobals(globalUpdates);

    CURRENT_CLASS    = outerClass;
    CURRENT_INSTANCE = outerInstance;
}

// PARSER

Parser::Parser(const std::vector<Token>& toks) : tokens(toks), position(0) {}
// basic peek / advance helpers

Token Parser::peek() {
    return (position < tokens.size()) ? tokens[position] : Token{TokenType::EndOfFile, ""};
}
Token Parser::advance() {
    return (position < tokens.size()) ? tokens[position++] : Token{TokenType::EndOfFile, ""};
}
Token Parser::consume(TokenType type, const std::string& err) {
    // expect a specific token or throw
    if (peek().type != type) {
        std::stringstream ss;
        ss << err << " Got: '" << peek().value << "' at position " << position;
        throw std::runtime_error(ss.str());
    }
    return advance();
}

std::string Parser::parseTypeName() {
    // int, string, float, bool, void, or custom type, optionally[]
    Token t = peek();
    if (t.type == TokenType::TypeInt    || t.type == TokenType::TypeString ||
        t.type == TokenType::TypeFloat  || t.type == TokenType::TypeBool   ||
        t.type == TokenType::Void       || t.type == TokenType::Identifier) {
        advance();
        std::string type = t.value;
        if (peek().type == TokenType::LeftBracket) {
            advance();
            consume(TokenType::RightBracket, "]");
            type += "[]";
        }
        return type;
    }
    throw std::runtime_error("Expected type name, got: " + t.value);
}

static std::string compoundOp(TokenType t) {
    if (t == TokenType::PlusEquals)  return "+";
    if (t == TokenType::MinusEquals) return "-";
    if (t == TokenType::TimesEquals) return "*";
    if (t == TokenType::DivEquals)   return "/";
    if (t == TokenType::ModEquals)   return "%";
    return "";
}

static bool isCompound(TokenType t) {
    return t == TokenType::PlusEquals  || t == TokenType::MinusEquals ||
           t == TokenType::TimesEquals || t == TokenType::DivEquals   ||
           t == TokenType::ModEquals;
}

std::unique_ptr<ExprNode> Parser::parsePrimary() {
    // literals, variables, casts, self, parens, arrays
    Token t = peek();

    if (t.type == TokenType::Bang) {
        advance();
        auto operand = parsePrimary();
        return std::make_unique<BinaryOpNode>("==", std::move(operand),
                                              std::make_unique<NumberLiteralNode>("0"));
    }
    if (t.type == TokenType::True)          { advance(); return std::make_unique<BooleanLiteralNode>(true); }
    if (t.type == TokenType::False)         { advance(); return std::make_unique<BooleanLiteralNode>(false); }
    if (t.type == TokenType::StringLiteral) { advance(); return std::make_unique<StringLiteralNode>(t.value); }
    if (t.type == TokenType::Number)        { advance(); return std::make_unique<NumberLiteralNode>(t.value); }

    if (t.type == TokenType::FStringLiteral) {
        advance();
        std::vector<std::pair<bool, std::string>> parts;
        std::string raw = t.value;
        size_t i = 0; std::string literal;
        while (i < raw.size()) {
            if (raw[i] == '{') {
                if (!literal.empty()) { parts.push_back({false, literal}); literal.clear(); }
                i++;
                std::string expr;
                while (i < raw.size() && raw[i] != '}') expr += raw[i++];
                if (i < raw.size()) i++;
                parts.push_back({true, expr});
            } else { literal += raw[i++]; }
        }
        if (!literal.empty()) parts.push_back({false, literal});
        return std::make_unique<FStringNode>(std::move(parts));
    }

    if (t.type == TokenType::At) {
        advance();
        return std::make_unique<CastOrRefNode>("@", consume(TokenType::Identifier, "Var name").value);
    }

    if (t.type == TokenType::Cast || t.type == TokenType::StaticCast) {
        advance();
        consume(TokenType::LessThan, "<");
        std::string type;
        while (peek().type != TokenType::GreaterThan) type += advance().value;
        consume(TokenType::GreaterThan, ">");
        consume(TokenType::LeftParen, "(");
        auto inner = parseLogicalOr();
        consume(TokenType::RightParen, ")");
        return std::make_unique<CastExprNode>(type, std::move(inner));
    }

    if (t.type == TokenType::Self) {
        advance();
        consume(TokenType::Dot, "Expected '.' after self");
        std::string member = consume(TokenType::Identifier, "Expected member name").value;
        if (peek().type == TokenType::LeftParen) {
            advance();
            std::vector<std::unique_ptr<ExprNode>> args;
            if (peek().type != TokenType::RightParen) {
                do {
                    if (peek().type == TokenType::Comma) advance();
                    args.push_back(parseLogicalOr());
                } while (peek().type == TokenType::Comma);
            }
            consume(TokenType::RightParen, ")");
            return std::make_unique<SelfAccessNode>(member, true, std::move(args));
        }
        return std::make_unique<SelfAccessNode>(member, false);
    }

    if (t.type == TokenType::Identifier) {
        std::string name = advance().value;

        if (peek().type == TokenType::Dot) {
            advance();
            std::string member = consume(TokenType::Identifier, "Expected member name").value;
            if (peek().type == TokenType::LeftParen) {
                advance();
                std::vector<std::unique_ptr<ExprNode>> args;
                if (peek().type != TokenType::RightParen) {
                    do {
                        if (peek().type == TokenType::Comma) advance();
                        args.push_back(parseLogicalOr());
                    } while (peek().type == TokenType::Comma);
                }
                consume(TokenType::RightParen, ")");
                return std::make_unique<MemberAccessNode>(name, member, true, std::move(args));
            }
            return std::make_unique<MemberAccessNode>(name, member, false);
        }

        // optional template/type parameter, e.g. input<int>
        std::string templateType = "";
        if (peek().type == TokenType::LessThan) {
            size_t savedPos = position;
            advance(); // <
            if (peek().type == TokenType::TypeInt    || peek().type == TokenType::TypeFloat ||
                peek().type == TokenType::TypeString || peek().type == TokenType::TypeBool  ||
                peek().type == TokenType::Void       || peek().type == TokenType::Identifier) {
                templateType = advance().value;
                if (peek().type == TokenType::GreaterThan && position + 1 < tokens.size() &&
                    tokens[position + 1].type == TokenType::LeftParen) {
                    advance(); // >
                } else {
                    position = savedPos;
                    templateType = "";
                }
            } else {
                position = savedPos;
            }
        }
        if (peek().type == TokenType::LeftParen) {
            advance();
            std::vector<std::unique_ptr<ExprNode>> args;
            if (peek().type != TokenType::RightParen) {
                do {
                    if (peek().type == TokenType::Comma) advance();
                    args.push_back(parseLogicalOr());
                } while (peek().type == TokenType::Comma);
            }
            consume(TokenType::RightParen, ")");
            return std::make_unique<FunctionCallNode>(name, std::move(args), templateType);
        }

        if (peek().type == TokenType::LeftBracket) {
            advance();
            auto idx = parseLogicalOr();
            consume(TokenType::RightBracket, "]");
            return std::make_unique<ArrayAccessNode>(name, std::move(idx));
        }

        return std::make_unique<VariableNode>(name);
    }

    if (t.type == TokenType::LeftParen) {
        advance();
        auto expr = parseLogicalOr();
        consume(TokenType::RightParen, ")");
        return expr;
    }

    // Array literal: [expr, expr, ...]
    if (t.type == TokenType::LeftBracket) {
        advance();
        std::vector<std::unique_ptr<ExprNode>> elems;
        if (peek().type != TokenType::RightBracket) {
            do {
                if (peek().type == TokenType::Comma) advance();
                elems.push_back(parseLogicalOr());
            } while (peek().type == TokenType::Comma);
        }
        consume(TokenType::RightBracket, "]");
        return std::make_unique<ArrayLiteralNode>(std::move(elems));
    }

    return std::make_unique<NumberLiteralNode>("0");
}

std::unique_ptr<ExprNode> Parser::parsePostfix() {
    // handles x++ and x--
    auto left = parsePrimary();
    if (peek().type == TokenType::PlusPlus || peek().type == TokenType::MinusMinus) {
        std::string op = advance().value;
        VariableNode* vn = dynamic_cast<VariableNode*>(left.get());
        if (!vn) throw std::runtime_error("++ / -- can only be applied to a variable");
        return std::make_unique<PostfixOpNode>(vn->getName(), op);
    }
    return left;
}

std::unique_ptr<ExprNode> Parser::parseTerm() {
    // * / %
    auto left = parsePostfix();
    while (peek().type == TokenType::Asterisk ||
           peek().type == TokenType::Slash    ||
           peek().type == TokenType::Percent) {
        std::string op = advance().value;
        left = std::make_unique<BinaryOpNode>(op, std::move(left), parsePostfix());
    }
    return left;
}

std::unique_ptr<ExprNode> Parser::parseExpression() {
    // + -
    auto left = parseTerm();
    while (peek().type == TokenType::Plus || peek().type == TokenType::Minus) {
        std::string op = advance().value;
        left = std::make_unique<BinaryOpNode>(op, std::move(left), parseTerm());
    }
    return left;
}

std::unique_ptr<ExprNode> Parser::parseComparison() {
    // == != < > <= >=
    auto left = parseExpression();
    while (peek().type == TokenType::EqualEqual   || peek().type == TokenType::NotEqual  ||
           peek().type == TokenType::LessThan     || peek().type == TokenType::GreaterThan ||
           peek().type == TokenType::LessEqual    || peek().type == TokenType::GreaterEqual) {
        std::string op = advance().value;
        left = std::make_unique<BinaryOpNode>(op, std::move(left), parseExpression());
    }
    return left;
}

std::unique_ptr<ExprNode> Parser::parseLogicalAnd() {
    // &&
    auto left = parseComparison();
    while (peek().type == TokenType::AndAnd) {
        advance();
        left = std::make_unique<BinaryOpNode>("&&", std::move(left), parseComparison());
    }
    return left;
}

std::unique_ptr<ExprNode> Parser::parseLogicalOr() {
    // ||
    auto left = parseLogicalAnd();
    while (peek().type == TokenType::OrOr) {
        advance();
        left = std::make_unique<BinaryOpNode>("||", std::move(left), parseLogicalAnd());
    }
    return left;
}

std::unique_ptr<BlockNode> Parser::parseBlock() {
    // { ... }
    consume(TokenType::LeftBrace, "{");
    auto b = std::make_unique<BlockNode>();
    while (peek().type != TokenType::RightBrace && peek().type != TokenType::EndOfFile)
        if (auto s = parseStatement()) b->statements.push_back(std::move(s));
    consume(TokenType::RightBrace, "}");
    return b;
}

std::unique_ptr<ASTNode> Parser::parseStructDef() {
    // struct Name { fields... }
    consume(TokenType::Struct, "struct");
    std::string structName = consume(TokenType::Identifier, "struct name").value;
    consume(TokenType::LeftBrace, "{");

    std::map<std::string, FieldDef> fields;

    while (peek().type != TokenType::RightBrace && peek().type != TokenType::EndOfFile) {
        bool isConst = false;
        if (peek().type == TokenType::Const) { isConst = true; advance(); }

        std::string typeName   = parseTypeName();
        std::string memberName = consume(TokenType::Identifier, "field name").value;

        std::shared_ptr<ExprNode> initExpr = nullptr;
        if (peek().type == TokenType::Equals) {
            advance();
            initExpr = parseLogicalOr();
        }
        consume(TokenType::Semicolon, ";");

        FieldDef fd;
        fd.access   = "public";
        fd.type     = typeName;
        fd.value    = initExpr ? initExpr->evaluate() : "0";
        fd.isConst  = isConst;
        fd.initExpr = initExpr;
        fields[memberName] = fd;
    }

    consume(TokenType::RightBrace, "}");
    return std::make_unique<StructDefNode>(structName, std::move(fields));
}

std::unique_ptr<ASTNode> Parser::parseClassDef() {
    // class Name ~Parent { fields, methods, constructors }
    consume(TokenType::Class, "class");
    std::string className = consume(TokenType::Identifier, "class name").value;

    std::string parentName;
    if (peek().type == TokenType::Tilde) {
        advance();
        parentName = consume(TokenType::Identifier, "parent class name").value;
    }

    consume(TokenType::LeftBrace, "{");

    std::map<std::string, FieldDef>  fields;
    std::map<std::string, MethodDef> methods;

    while (peek().type != TokenType::RightBrace && peek().type != TokenType::EndOfFile) {
        Token t = peek();

        std::string access = "public";
        if      (t.type == TokenType::Private)   { access = "private";   advance(); t = peek(); }
        else if (t.type == TokenType::Protected)  { access = "protected"; advance(); t = peek(); }
        else if (t.type == TokenType::Global)     { access = "global";    advance(); t = peek(); }

        if (t.type == TokenType::Constructor) {
            advance();
            std::string ctorName = consume(TokenType::Identifier, "constructor name").value;
            consume(TokenType::LeftParen, "(");

            std::vector<std::pair<std::string, std::string>> params;
            if (peek().type != TokenType::RightParen) {
                do {
                    if (peek().type == TokenType::Comma) advance();
                    std::string ptype = parseTypeName();
                    std::string pname = consume(TokenType::Identifier, "param name").value;
                    if (peek().type == TokenType::LeftBracket) {
                        advance();
                        consume(TokenType::RightBracket, "]");
                        ptype += "[]";
                    }
                    params.push_back({ptype, pname});
                } while (peek().type == TokenType::Comma);
            }
            consume(TokenType::RightParen, ")");

            std::vector<std::string> bindings;
            std::string parentCtorClass;
            std::vector<std::string> parentCtorArgs;

            if (peek().type == TokenType::Arrow) {
                advance();
                if (peek().type == TokenType::LeftParen) {
                    advance();
                    while (peek().type != TokenType::RightParen) {
                        if (peek().type == TokenType::Comma) advance();
                        bindings.push_back(consume(TokenType::Identifier, "field name").value);
                    }
                    consume(TokenType::RightParen, ")");
                } else if (peek().type == TokenType::Identifier) {
                    parentCtorClass = advance().value;
                    consume(TokenType::LeftParen, "(");
                    while (peek().type != TokenType::RightParen) {
                        if (peek().type == TokenType::Comma) advance();
                        parentCtorArgs.push_back(consume(TokenType::Identifier, "arg").value);
                    }
                    consume(TokenType::RightParen, ")");
                }
            }

            auto body = parseBlock();
            MethodDef md;
            md.returnType             = "void";
            md.params                 = params;
            md.body                   = std::shared_ptr<BlockNode>(std::move(body));
            md.constructorBindings    = bindings;
            md.parentConstructorClass = parentCtorClass;
            md.parentConstructorArgs  = parentCtorArgs;
            md.ownerClass             = className;
            methods[ctorName] = std::move(md);
            continue;
        }

        bool isTypeKw = (t.type == TokenType::TypeInt   || t.type == TokenType::TypeString ||
                         t.type == TokenType::TypeFloat || t.type == TokenType::TypeBool   ||
                         t.type == TokenType::Void      || t.type == TokenType::Identifier);
        if (!isTypeKw) { advance(); continue; }

        std::string typeName   = parseTypeName();
        std::string memberName = consume(TokenType::Identifier, "member name").value;

        if (peek().type == TokenType::LeftParen) {
            advance();
            std::vector<std::pair<std::string, std::string>> params;
            if (peek().type != TokenType::RightParen) {
                do {
                    if (peek().type == TokenType::Comma) advance();
                    std::string ptype = parseTypeName();
                    std::string pname = consume(TokenType::Identifier, "param name").value;
                    if (peek().type == TokenType::LeftBracket) {
                        advance();
                        consume(TokenType::RightBracket, "]");
                        ptype += "[]";
                    }
                    params.push_back({ptype, pname});
                } while (peek().type == TokenType::Comma);
            }
            consume(TokenType::RightParen, ")");
            auto body = parseBlock();
            MethodDef md;
            md.returnType = typeName;
            md.params     = params;
            md.body       = std::shared_ptr<BlockNode>(std::move(body));
            md.ownerClass = className;
            methods[memberName] = std::move(md);
        } else {
            std::shared_ptr<ExprNode> initExpr = nullptr;
            if (peek().type == TokenType::Equals) {
                advance();
                initExpr = parseLogicalOr();
            }
            consume(TokenType::Semicolon, ";");
            FieldDef fd;
            fd.access   = access;
            fd.type     = typeName;
            fd.value    = initExpr ? initExpr->evaluate() : "0";
            fd.initExpr = initExpr;
            fields[memberName] = fd;
        }
    }

    consume(TokenType::RightBrace, "}");
    return std::make_unique<ClassDefNode>(className, parentName,
                                         std::move(fields), std::move(methods));
}

std::unique_ptr<ASTNode> Parser::parseStatement() {
    // dispatch to the right parser based on the next token
    Token t = peek();

    if (t.type == TokenType::Class)  return parseClassDef();
    if (t.type == TokenType::Struct) return parseStructDef();

    if (t.type == TokenType::Return) {
        advance(); auto e = parseLogicalOr();
        consume(TokenType::Semicolon, ";");
        return std::make_unique<ReturnNode>(std::move(e));
    }

    if (t.type == TokenType::Break) {
        advance();
        consume(TokenType::Semicolon, ";");
        return std::make_unique<BreakNode>();
    }

    if (t.type == TokenType::Continue) {
        advance();
        consume(TokenType::Semicolon, ";");
        return std::make_unique<ContinueNode>();
    }

    if (t.type == TokenType::Free) {
        advance();
        std::string name = consume(TokenType::Identifier, "Name").value;
        consume(TokenType::Semicolon, ";");
        return std::make_unique<FreeNode>(name);
    }

    if (t.type == TokenType::Identifier && t.value == "print") {
        advance();
        consume(TokenType::LeftParen, "(");
        auto e = parseLogicalOr();
        consume(TokenType::RightParen, ")");
        consume(TokenType::Semicolon, ";");
        return std::make_unique<PrintNode>(std::move(e));
    }

    if (t.type == TokenType::If) {
        advance();
        consume(TokenType::LeftParen, "(");
        auto cond = parseLogicalOr();
        consume(TokenType::RightParen, ")");
        auto thenBlock = parseBlock();
        auto ifNode = std::make_unique<IfNode>(std::move(cond), std::move(thenBlock));
        while (peek().type == TokenType::Else) {
            advance();
            if (peek().type == TokenType::If) {
                advance();
                consume(TokenType::LeftParen, "(");
                auto eiCond = parseLogicalOr();
                consume(TokenType::RightParen, ")");
                ifNode->addElseIf(std::move(eiCond), parseBlock());
            } else { ifNode->setElse(parseBlock()); break; }
        }
        return ifNode;
    }

    if (t.type == TokenType::While) {
        advance();
        consume(TokenType::LeftParen, "(");
        auto cond = parseLogicalOr();
        consume(TokenType::RightParen, ")");
        return std::make_unique<WhileNode>(std::move(cond), parseBlock());
    }

    if (t.type == TokenType::DoWhile) {
        advance();
        auto block = parseBlock();
        consume(TokenType::While, "Expected 'while'");
        consume(TokenType::LeftParen, "(");
        auto cond = parseLogicalOr();
        consume(TokenType::RightParen, ")");
        consume(TokenType::Semicolon, ";");
        return std::make_unique<DoWhileNode>(std::move(cond), std::move(block));
    }

    if (t.type == TokenType::For) {
        advance();
        consume(TokenType::LeftParen, "(");
        auto init = parseStatement();
        auto cond = parseLogicalOr();
        consume(TokenType::Semicolon, ";");
        std::unique_ptr<ExprNode> inc;
        if (peek().type == TokenType::Identifier) {
            std::string varName = advance().value;
            if (peek().type == TokenType::PlusPlus || peek().type == TokenType::MinusMinus) {
                inc = std::make_unique<PostfixOpNode>(varName, advance().value);
            } else if (isCompound(peek().type)) {
                std::string op = compoundOp(advance().type);
                inc = std::make_unique<AssignExprNode>(
                    varName,
                    std::make_unique<BinaryOpNode>(
                        op,
                        std::make_unique<VariableNode>(varName),
                        parseLogicalOr()
                    )
                );
            } else {
                consume(TokenType::Equals, "=");
                inc = std::make_unique<AssignExprNode>(varName, parseLogicalOr());
            }
        }
        consume(TokenType::RightParen, ")");
        return std::make_unique<ForNode>(std::move(init), std::move(cond),
                                        std::move(inc), parseBlock());
    }

    bool isConst = false;
    if (t.type == TokenType::Const) {
        isConst = true;
        advance();
        t = peek();
    }

    if (t.type == TokenType::TypeInt   || t.type == TokenType::TypeString ||
        t.type == TokenType::TypeFloat || t.type == TokenType::TypeBool   ||
        t.type == TokenType::Void) {
        std::string type = advance().value;
        bool ptr = false;
        if (peek().type == TokenType::Asterisk) { advance(); ptr = true; }
        std::string name = consume(TokenType::Identifier, "Name").value;

        if (peek().type == TokenType::LeftParen) {
            consume(TokenType::LeftParen, "(");
            std::vector<std::pair<std::string, std::string>> p;
            if (peek().type != TokenType::RightParen) {
                do {
                    if (peek().type == TokenType::Comma) advance();
                    std::string pt = parseTypeName();
                    std::string pname = consume(TokenType::Identifier, "Param").value;
                    if (peek().type == TokenType::LeftBracket) {
                        advance();
                        consume(TokenType::RightBracket, "]");
                        pt += "[]";
                    }
                    p.push_back({pt, pname});
                } while (peek().type == TokenType::Comma);
            }
            consume(TokenType::RightParen, ")");
            return std::make_unique<FunctionDefNode>(
                type + (ptr ? "*" : ""), name, p,
                std::shared_ptr<BlockNode>(parseBlock())
            );
        }
        if (peek().type == TokenType::LeftBracket) {
            advance();
            int sz = 0;
            if (peek().type == TokenType::Number) {
                sz = std::stoi(advance().value);
            }
            consume(TokenType::RightBracket, "]");
            std::vector<std::unique_ptr<ExprNode>> inits;
            if (peek().type == TokenType::Equals) {
                advance();
                consume(TokenType::LeftBracket, "[");
                if (peek().type != TokenType::RightBracket) {
                    do {
                        if (peek().type == TokenType::Comma) advance();
                        inits.push_back(parseLogicalOr());
                    } while (peek().type == TokenType::Comma);
                }
                consume(TokenType::RightBracket, "]");
            }
            consume(TokenType::Semicolon, ";");
            return std::make_unique<ArrayDeclarationNode>(type, name, sz, std::move(inits));
        }
        consume(TokenType::Equals, "=");
        auto init = parseLogicalOr();
        consume(TokenType::Semicolon, ";");
        return std::make_unique<VarDeclarationNode>(type, ptr, isConst, name, std::move(init));
    }

    if (t.type == TokenType::Identifier) {
        if (position + 1 < tokens.size() &&
            tokens[position + 1].type == TokenType::Dot &&
            position + 2 < tokens.size() &&
            tokens[position + 2].type == TokenType::Identifier &&
            position + 3 < tokens.size() &&
            isCompound(tokens[position + 3].type)) {
            std::string instName  = advance().value;
            advance(); // .
            std::string fieldName = advance().value;
            std::string op = compoundOp(advance().type);
            auto rhs = parseLogicalOr();
            consume(TokenType::Semicolon, ";");
            return std::make_unique<ExpressionStatement>(
                std::make_unique<MemberAssignNode>(
                    instName, fieldName,
                    std::make_unique<BinaryOpNode>(
                        op,
                        std::make_unique<MemberAccessNode>(instName, fieldName, false),
                        std::move(rhs)
                    )
                )
            );
        }

        if (position + 1 < tokens.size() &&
            tokens[position + 1].type == TokenType::Dot &&
            position + 2 < tokens.size() &&
            tokens[position + 2].type == TokenType::Identifier &&
            position + 3 < tokens.size() &&
            tokens[position + 3].type == TokenType::Equals) {
            std::string instName  = advance().value;
            advance(); // .
            std::string fieldName = advance().value;
            advance(); // =
            auto val = parseLogicalOr();
            consume(TokenType::Semicolon, ";");
            return std::make_unique<ExpressionStatement>(
                std::make_unique<MemberAssignNode>(instName, fieldName, std::move(val))
            );
        }

        if (position + 1 < tokens.size() &&
            tokens[position + 1].type == TokenType::LeftBracket) {
            std::string arrName = advance().value;
            advance(); // [
            auto idxExpr = parseLogicalOr();
            consume(TokenType::RightBracket, "]");
            if (peek().type == TokenType::Equals) {
                advance();
                auto valExpr = parseLogicalOr();
                consume(TokenType::Semicolon, ";");
                return std::make_unique<ExpressionStatement>(
                    std::make_unique<ArrayAssignNode>(arrName, std::move(idxExpr), std::move(valExpr))
                );
            }
            if (isCompound(peek().type)) {
                std::string op = compoundOp(advance().type);
                auto valExpr = parseLogicalOr();
                consume(TokenType::Semicolon, ";");
                return std::make_unique<ExpressionStatement>(
                    std::make_unique<ArrayCompoundAssignNode>(arrName, std::move(idxExpr), op, std::move(valExpr))
                );
            }
            consume(TokenType::Semicolon, ";");
            return std::make_unique<ExpressionStatement>(
                std::make_unique<ArrayAccessNode>(arrName, std::move(idxExpr))
            );
        }

        if (position + 1 < tokens.size() && isCompound(tokens[position + 1].type)) {
            std::string varName = advance().value;
            std::string op = compoundOp(advance().type);
            auto rhs = parseLogicalOr();
            consume(TokenType::Semicolon, ";");
            return std::make_unique<ExpressionStatement>(
                std::make_unique<AssignExprNode>(
                    varName,
                    std::make_unique<BinaryOpNode>(
                        op,
                        std::make_unique<VariableNode>(varName),
                        std::move(rhs)
                    )
                )
            );
        }

        if (position + 1 < tokens.size() &&
            tokens[position + 1].type == TokenType::Equals) {
            std::string varName = advance().value;
            advance();
            auto val = parseLogicalOr();
            consume(TokenType::Semicolon, ";");
            return std::make_unique<ExpressionStatement>(
                std::make_unique<AssignExprNode>(varName, std::move(val))
            );
        }

        if (position + 1 < tokens.size() &&
            tokens[position + 1].type == TokenType::Identifier &&
            position + 2 < tokens.size() &&
            tokens[position + 2].type == TokenType::LeftParen) {
            auto sit = SYMBOL_TABLE.find(t.value);
            if (sit != SYMBOL_TABLE.end() && sit->second.type == "class") {
                std::string cls  = advance().value;
                std::string inst = advance().value;
                consume(TokenType::LeftParen, "(");
                std::vector<std::unique_ptr<ExprNode>> iargs;
                if (peek().type != TokenType::RightParen) {
                    do {
                        if (peek().type == TokenType::Comma) advance();
                        iargs.push_back(parseLogicalOr());
                    } while (peek().type == TokenType::Comma);
                }
                consume(TokenType::RightParen, ")");
                consume(TokenType::Semicolon, ";");
                return std::make_unique<InstanceCreateNode>(cls, inst, std::move(iargs));
            }
        }

        auto e = parseLogicalOr();
        consume(TokenType::Semicolon, ";");
        return std::make_unique<ExpressionStatement>(std::move(e));
    }

    if (t.type == TokenType::Self) {
        if (position + 1 < tokens.size() &&
            tokens[position + 1].type == TokenType::Dot &&
            position + 2 < tokens.size() &&
            tokens[position + 2].type == TokenType::Identifier &&
            position + 3 < tokens.size() &&
            isCompound(tokens[position + 3].type)) {
            advance(); // self
            advance(); // .
            std::string fieldName = advance().value;
            std::string op = compoundOp(advance().type);
            auto rhs = parseLogicalOr();
            consume(TokenType::Semicolon, ";");
            return std::make_unique<ExpressionStatement>(
                std::make_unique<AssignExprNode>(
                    fieldName,
                    std::make_unique<BinaryOpNode>(
                        op,
                        std::make_unique<SelfAccessNode>(fieldName, false),
                        std::move(rhs)
                    )
                )
            );
        }

        if (position + 1 < tokens.size() &&
            tokens[position + 1].type == TokenType::Dot &&
            position + 2 < tokens.size() &&
            tokens[position + 2].type == TokenType::Identifier &&
            position + 3 < tokens.size() &&
            tokens[position + 3].type == TokenType::Equals) {
            advance(); // self
            advance(); // .
            std::string fieldName = advance().value;
            advance(); // =
            auto val = parseLogicalOr();
            consume(TokenType::Semicolon, ";");
            return std::make_unique<ExpressionStatement>(
                std::make_unique<AssignExprNode>(fieldName, std::move(val))
            );
        }

        auto e = parseLogicalOr();
        consume(TokenType::Semicolon, ";");
        return std::make_unique<ExpressionStatement>(std::move(e));
    }

    return nullptr;
}
```

### main.cpp
```cpp
#include "silo.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <stdexcept>
#include <set>
#include <cstdlib>

// flags for compiling
const std::string COMMANDS[] = {"-dump", "-d", "-v", "-version"};
bool dumpMode = false;
const std::string VERSION = "Silo Taurus-2.2.18~c++17";

void compileCommand(const std::string& cmd) {
    // check if cmd is a known flag
    auto cc = std::find(std::begin(COMMANDS), std::end(COMMANDS), cmd);
    if (cc != std::end(COMMANDS)) {
        if (*cc == "-dump" || *cc == "-d") dumpMode = true;
        if (*cc == "-v" || *cc == "-version") std::cout << VERSION << std::endl << std::endl;
    } else {
        std::cerr << "Unknown command: " << cmd << std::endl;
        return;
    }
}

// PREPROCESSOR

// Returns ~/.silo/lib/
std::string getSiloLibDir() {
    // grab ~/.silo/lib/ from $HOME
    const char* home = getenv("HOME");
    if (!home)
        throw std::runtime_error("Cannot find HOME directory. Is $HOME set?");
    return std::string(home) + "/.silo/lib/";
}

// Recursively preprocesses a .sl file, resolving all #include directives.
// <file>  -> looks in ~/.silo/lib/
// "file"  -> looks relative to the current source file
std::string preprocess(const std::string& filepath, std::set<std::string>& included) {
    // handles both <system> and "local" includes
    if (included.count(filepath)) return ""; // already included, skip
    included.insert(filepath);

    std::ifstream file(filepath);
    if (!file)
        throw std::runtime_error("Cannot open file: " + filepath);

    // Base directory of this file (for resolving local "" includes)
    std::string baseDir = "";
    size_t lastSlash = filepath.find_last_of("/\\");
    if (lastSlash != std::string::npos)
        baseDir = filepath.substr(0, lastSlash + 1);

    std::string result;
    std::string line;
    int lineNum = 0;

    while (std::getline(file, line)) {
        lineNum++;

        size_t start = line.find_first_not_of(" \t");
        if (start == std::string::npos) { result += "\n"; continue; }
        std::string trimmed = line.substr(start);

        if (trimmed.rfind("#include", 0) == 0) {
            size_t i = 8; // skip "#include"
            while (i < trimmed.size() && trimmed[i] == ' ') i++;

            if (i >= trimmed.size())
                throw std::runtime_error("Empty #include on line "
                                         + std::to_string(lineNum)
                                         + " of " + filepath);

            char openChar  = trimmed[i];
            char closeChar = (openChar == '<') ? '>' : '"';

            if (openChar != '<' && openChar != '"')
                throw std::runtime_error("Malformed #include on line "
                                         + std::to_string(lineNum)
                                         + " of " + filepath
                                         + " — expected < or \"");

            size_t nameStart = i + 1;
            size_t nameEnd   = trimmed.find(closeChar, nameStart);
            if (nameEnd == std::string::npos)
                throw std::runtime_error("Unclosed #include on line "
                                         + std::to_string(lineNum)
                                         + " of " + filepath);

            std::string name = trimmed.substr(nameStart, nameEnd - nameStart);

            // Add .sl if no extension provided
            if (name.find('.') == std::string::npos)
                name += ".sl";

            std::string includePath;
            if (openChar == '<') {
                // System library: ~/.silo/lib/
                includePath = getSiloLibDir() + name;
            } else {
                // Local file: relative to the current source file
                includePath = baseDir + name;
            }

            result += preprocess(includePath, included);
            result += "\n";
        } else {
            result += line + "\n";
        }
    }

    return result;
}

// MAIN

int main(int argc, char* argv[]) {
    // basic entry point: lex, parse, execute
    clear();
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <source_file> or <power_command>" << std::endl;
        return 1;
    }

    // Check for power commands first
    if (std::string(argv[1]) == "-v" || std::string(argv[1]) == "-version") {
        std::cout << VERSION << std::endl;
        return 0;
    }

    // make sure the source file is actually there
    {
        std::ifstream check(argv[1]);
        if (!check) {
            std::cerr << "Error: Could not open file " << argv[1] << std::endl;
            return 1;
        }
    }

    if (argc > 2) {
        for (int i = 2; i < argc; i++) {
            compileCommand(argv[i]);
        }
    }

    // pull in any #include'd files first
    std::string source;
    try {
        std::set<std::string> included;
        source = preprocess(std::string(argv[1]), included);
    } catch (const std::exception& e) {
        std::cerr << "[!] Preprocessor Error: " << e.what() << std::endl;
        return 1;
    }

    // turn source into a token stream
    Lexer lexer(source);
    std::vector<Token> tokens = lexer.tokenize();

    // run the program statement by statement
    Parser parser(tokens);
    try {
        while (true) {
            auto stmt = parser.parseStatement();
            if (!stmt) break;
            stmt->execute();
        }
    } catch (const std::exception& e) {
        std::cerr << "[!] Runtime/Parsing Error: " << e.what() << std::endl;
        return 1;
    }

    if (dumpMode) printSymbolTable();
    return 0;
}
```