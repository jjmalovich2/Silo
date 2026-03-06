#ifndef SILO_H
#define SILO_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <iostream>
#include <functional>

// --- Token Definitions ---
enum TokenType {
    // Keywords
    TypeInt, TypeString, TypeFloat, TypeBool,
    Return, Print, If, Else, While, For, DoWhile,
    True, False,
    Cast, StaticCast, Free,
    Class, Constructor, Private, Protected, Global, Void,
    Self,

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
class BlockNode;

// --- Field definition for class members ---
struct FieldDef {
    std::string access;  // "private", "protected", "global", "public"
    std::string type;
    std::string value;
};

// --- Method definition for class members ---
struct MethodDef {
    std::string returnType;
    std::vector<std::pair<std::string, std::string>> params;
    std::shared_ptr<BlockNode> body;
    // For constructor: -> (field1, field2, ...) style bindings
    std::vector<std::string> constructorBindings;
    // For constructor: -> ParentClass(arg1, arg2, ...) style
    std::string parentConstructorClass;
    std::vector<std::string> parentConstructorArgs;
    // Which class this method belongs to (for access checking)
    std::string ownerClass;
};

// --- Runtime Value ---
struct RuntimeValue {
    std::string type;   // "int", "string", "float", "bool", "class", "instance", "void", etc.
    std::string value;
    std::vector<std::string> arrayElements;

    // For plain functions
    std::vector<std::pair<std::string, std::string>> params;
    std::shared_ptr<ASTNode> body;

    // For class definitions & instances
    std::string parentClass;                    // parent class name (inheritance)
    std::string instanceOf;                     // for instances: which class
    std::map<std::string, FieldDef> fields;     // field name -> FieldDef
    std::map<std::string, MethodDef> methods;   // method name -> MethodDef
};

// Global Symbol Table
extern std::map<std::string, RuntimeValue> SYMBOL_TABLE;

// Current execution context (class name we're inside, instance name)
extern std::string CURRENT_CLASS;    // class being executed in (for access checks)
extern std::string CURRENT_INSTANCE; // instance name currently executing

void printSymbolTable();
void clear();

// --- AST Node Base Classes ---
struct ASTNode {
    virtual ~ASTNode() = default;
    virtual void execute() = 0;
};

struct ExprNode : public ASTNode {
    virtual std::string evaluate() const = 0;
    void execute() override { evaluate(); }
};

// =====================================================================
// EXPRESSION NODES
// =====================================================================

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

// f"Hello {name}" — stores parts: alternating literal and expr strings
class FStringNode : public ExprNode {
public:
    // parts: vector of (isExpr, content)
    // isExpr=false => literal string
    // isExpr=true  => variable/expression name to evaluate
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

// i++ or i-- (postfix)
class PostfixOpNode : public ExprNode {
    std::string varName;
    std::string op; // "++" or "--"
public:
    PostfixOpNode(const std::string& name, const std::string& op);
    std::string evaluate() const override;
};

// Assignment as expression: used in for-loop increment (i = i + 1)
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

// self.fieldName or self.method(args)
class SelfAccessNode : public ExprNode {
    std::string memberName;
    bool isCall;
    std::vector<std::unique_ptr<ExprNode>> callArgs;
public:
    SelfAccessNode(const std::string& member, bool isCall,
                   std::vector<std::unique_ptr<ExprNode>> args = {});
    std::string evaluate() const override;
};

// instance.field or instance.method(args)
class MemberAccessNode : public ExprNode {
    std::string instanceName;
    std::string memberName;
    bool isCall;
    std::vector<std::unique_ptr<ExprNode>> callArgs;
public:
    MemberAccessNode(const std::string& inst, const std::string& member,
                     bool isCall, std::vector<std::unique_ptr<ExprNode>> args = {});
    std::string evaluate() const override;
};

// =====================================================================
// STATEMENT NODES
// =====================================================================

class BlockNode : public ASTNode {
public:
    std::vector<std::unique_ptr<ASTNode>> statements;
    void execute() override;
};

class VarDeclarationNode : public ASTNode {
    std::string baseType;
    bool isPointer;
    std::string identifier;
    std::unique_ptr<ExprNode> initializer;
public:
    VarDeclarationNode(const std::string& t, bool p, const std::string& id,
                       std::unique_ptr<ExprNode> init);
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
    std::unique_ptr<BlockNode> body;
public:
    FunctionDefNode(const std::string& rt, const std::string& n,
                    const std::vector<std::pair<std::string, std::string>> p,
                    std::unique_ptr<BlockNode> b);
    void execute() override;
};

// Defines a class (fields + methods) into SYMBOL_TABLE
class ClassDefNode : public ASTNode {
    std::string className;
    std::string parentName; // "" if no parent
    std::map<std::string, FieldDef> fields;
    std::map<std::string, MethodDef> methods;
public:
    ClassDefNode(const std::string& name, const std::string& parent,
                 std::map<std::string, FieldDef> f,
                 std::map<std::string, MethodDef> m);
    void execute() override;
};

// Creates an instance: Vehicle honda("12345", 94.5, "Honda");
class InstanceCreateNode : public ASTNode {
    std::string className;
    std::string instanceName;
    std::vector<std::unique_ptr<ExprNode>> args;
public:
    InstanceCreateNode(const std::string& cls, const std::string& inst,
                       std::vector<std::unique_ptr<ExprNode>> a);
    void execute() override;
};

// Member access as a statement (e.g. toyota.status(true);)
class MemberAccessStatement : public ASTNode {
    std::unique_ptr<MemberAccessNode> node;
public:
    MemberAccessStatement(std::unique_ptr<MemberAccessNode> n);
    void execute() override;
};

// =====================================================================
// LEXER & PARSER
// =====================================================================

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

    // Returns the type string from a type keyword token
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
};

#endif