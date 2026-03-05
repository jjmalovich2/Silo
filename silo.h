#ifndef SILO_H
#define SILO_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <iostream>

// --- Token Definitions ---
enum TokenType {
    // Keywords
    TypeInt, TypeString, TypeFloat, TypeBool, True, False, // types and bools
    Return, Print, If, Else, // logic and functions
    While, For, DoWhile, // loops
    Cast, StaticCast, Free, // memory management

    // Identifiers & Literals
    Identifier, Number, StringLiteral,

    // Operators & Symbols
    Equals,         // =
    Plus,           // +
    Minus,          // -
    Asterisk,       // *
    Slash,          // /
    Percent,        // %
    LeftParen,      // (
    RightParen,     // )
    LeftBrace,      // {
    RightBrace,     // }
    LeftBracket,    // [
    RightBracket,   // ]
    Semicolon,      // ;
    Comma,          // ,
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

    EndOfFile,
    Unknown
};

struct Token {
    TokenType type;
    std::string value;
};

// --- Runtime Structures ---
struct ASTNode; // Forward declaration

struct RuntimeValue {
    std::string type;  // "int", "string", "int[]", etc.
    std::string value; // "10", "hello"
    std::vector<std::string> arrayElements; // For arrays
    // Function storage
    std::vector<std::pair<std::string, std::string>> params; 
    std::shared_ptr<ASTNode> body; 
};

// Global Symbol Table
extern std::map<std::string, RuntimeValue> SYMBOL_TABLE;

// Helper to print debug info
void printSymbolTable();
void clear();

// --- AST Node Classes ---
struct ASTNode {
    virtual ~ASTNode() = default;
    virtual void execute() = 0;
};

struct ExprNode : public ASTNode {
    virtual std::string evaluate() const = 0;
    void execute() override { evaluate(); }
};

// Expressions
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

class VariableNode : public ExprNode {
    std::string name;
public:
    VariableNode(const std::string& n);
    std::string evaluate() const override;
    std::string getName() const { return name; }
};

// math node
class BinaryOpNode : public ExprNode {
    std::string op;
    std::unique_ptr<ExprNode> left;
    std::unique_ptr<ExprNode> right;
public:
    BinaryOpNode(std::string op, std::unique_ptr<ExprNode> l, std::unique_ptr<ExprNode> r);
    std::string evaluate() const override;
};

class CastOrRefNode : public ExprNode {
    std::string operation;
    std::string targetVar;
public:
    CastOrRefNode(const std::string& op, const std::string& var);
    std::string evaluate() const override;
};

class ArrayAccessNode : public ExprNode {
    std::string arrayName;
    int index;
public:
    ArrayAccessNode(const std::string& name, int idx);
    std::string evaluate() const override;
};

class FunctionCallNode : public ExprNode {
    std::string funcName;
    std::vector<std::unique_ptr<ExprNode>> args;
public:
    FunctionCallNode(const std::string& name, std::vector<std::unique_ptr<ExprNode>> a);
    std::string evaluate() const override;
};

// Statements
class VarDeclarationNode : public ASTNode {
    std::string baseType;
    bool isPointer;
    std::string identifier;
    std::unique_ptr<ExprNode> initializer;
public:
    VarDeclarationNode(const std::string& t, bool p, const std::string& id, std::unique_ptr<ExprNode> init);
    void execute() override;
};

class ArrayDeclarationNode : public ASTNode {
    std::string type;
    std::string name;
    int size;
public:
    ArrayDeclarationNode(const std::string& t, const std::string& n, int s);
    void execute() override;
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

class BlockNode : public ASTNode {
public:
    std::vector<std::unique_ptr<ASTNode>> statements;
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
    std::unique_ptr<ASTNode> increment;
    std::unique_ptr<BlockNode> body;
public:
    ForNode(std::unique_ptr<ASTNode> i, std::unique_ptr<ExprNode> cond, std::unique_ptr<ASTNode> inc, std::unique_ptr<BlockNode> b);
    void execute() override;
};

// for loop helper node
class AssignExprNode : public ExprNode {
    std::string varName;
    std::unique_ptr<ExprNode> value;
public:
    AssignExprNode(const std::string& name, std::unique_ptr<ExprNode> val);
    std::string evaluate() const override;
};

class FunctionDefNode : public ASTNode {
    std::string returnType;
    std::string name;
    std::vector<std::pair<std::string, std::string>> params;
    std::unique_ptr<BlockNode> body;
public:
    FunctionDefNode(const std::string& rt, const std::string& n, const std::vector<std::pair<std::string, std::string>> p, std::unique_ptr<BlockNode> b);
    void execute() override;
};

// --- Lexer & Parser ---
class Lexer {
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
    std::vector<Token> tokens;
    size_t position;

    Token peek();
    Token advance();
    Token consume(TokenType type, const std::string& err);

public:
    Parser(const std::vector<Token>& toks);
    
    // Updated Parsing Logic for Math Order of Operations
    std::unique_ptr<ExprNode> parsePrimary();        // Numbers, Vars, Parens
    std::unique_ptr<ExprNode> parseTerm();           // *, /, %
    std::unique_ptr<ExprNode> parseExpression();     // +, -
    std::unique_ptr<ExprNode> parseComparison();     // ==, !=, <, >, <=, >=
    std::unique_ptr<ExprNode> parseLogicalAnd();     // &&
    std::unique_ptr<ExprNode> parseLogicalOr();      // || (Entry Point)

    std::unique_ptr<BlockNode> parseBlock();
    std::unique_ptr<ASTNode> parseStatement();
};

#endif