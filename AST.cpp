#include "silo.h"
#include <iostream>
#include <sstream>
#include <cmath> // Required for fmod (modulo)

// Global Symbol Table Definition
std::map<std::string, RuntimeValue> SYMBOL_TABLE;

// Helper Exception for Returns
struct ReturnException {
    std::string value;
};

// Helper function to debug memory
void printSymbolTable() {
    std::cout << "\n=== SYMBOL TABLE DUMP ===\n";
    if (SYMBOL_TABLE.empty()) {
        std::cout << "(Empty)\n";
    } else {
        for (const auto& pair : SYMBOL_TABLE) {
            const std::string& name = pair.first;
            const RuntimeValue& val = pair.second;

            std::cout << "Name: " << name << " | Type: " << val.type;

            if (!val.params.empty() || val.body) {
                std::cout << " | [Function]";
            } else if (!val.arrayElements.empty()) {
                std::cout << " | Values: [";
                for (size_t i = 0; i < val.arrayElements.size(); ++i) {
                    std::cout << val.arrayElements[i] << (i < val.arrayElements.size() - 1 ? ", " : "");
                }
                std::cout << "]";
            } else {
                std::cout << " | Value: " << val.value;
            }
            std::cout << "\n";
        }
    }
    std::cout << "=========================\n";
}

void clear() {
    std::cout << "\033[2J\033[1;1H"; // Clear console (Unix)
    std::cout.flush();
}

// Helper class for standalone expressions (like i++; or static_cast...;)
class ExpressionStatement : public ASTNode {
    std::unique_ptr<ExprNode> expr;
public:
    ExpressionStatement(std::unique_ptr<ExprNode> e) : expr(std::move(e)) {}
    void execute() override { expr->evaluate(); }
};

// --- Constructors ---
NumberLiteralNode::NumberLiteralNode(const std::string& val) : value(val) {}
StringLiteralNode::StringLiteralNode(const std::string& val) : value(val) {}
BooleanLiteralNode::BooleanLiteralNode(bool val) : value(val) {}
VariableNode::VariableNode(const std::string& n) : name(n) {}
CastOrRefNode::CastOrRefNode(const std::string& op, const std::string& var) : operation(op), targetVar(var) {}
ArrayAccessNode::ArrayAccessNode(const std::string& name, int idx) : arrayName(name), index(idx) {}
FunctionCallNode::FunctionCallNode(const std::string& name, std::vector<std::unique_ptr<ExprNode>> a) : funcName(name), args(std::move(a)) {}
VarDeclarationNode::VarDeclarationNode(const std::string& t, bool p, const std::string& id, std::unique_ptr<ExprNode> init) : baseType(t), isPointer(p), identifier(id), initializer(std::move(init)) {}
ArrayDeclarationNode::ArrayDeclarationNode(const std::string& t, const std::string& n, int s) : type(t), name(n), size(s) {}
RetypeNode::RetypeNode(const std::string& t, const std::string& v) : newType(t), targetVar(v) {}
PrintNode::PrintNode(std::unique_ptr<ExprNode> expr) : expression(std::move(expr)) {}
ReturnNode::ReturnNode(std::unique_ptr<ExprNode> v) : value(std::move(v)) {}
FunctionDefNode::FunctionDefNode(const std::string& rt, const std::string& n, const std::vector<std::pair<std::string, std::string>> p, std::unique_ptr<BlockNode> b) : returnType(rt), name(n), params(p), body(std::move(b)) {}
FreeNode::FreeNode(const std::string& id) : identifier(id) {}
IfNode::IfNode(std::unique_ptr<ExprNode> cond, std::unique_ptr<BlockNode> thenB) 
    : condition(std::move(cond)), thenBlock(std::move(thenB)), elseBlock(nullptr) {}
void IfNode::addElseIf(std::unique_ptr<ExprNode> cond, std::unique_ptr<BlockNode> block) {
    elseIfBlocks.push_back({std::move(cond), std::move(block)});
}
void IfNode::setElse(std::unique_ptr<BlockNode> block) {
    elseBlock = std::move(block);
}
BinaryOpNode::BinaryOpNode(std::string o, std::unique_ptr<ExprNode> l, std::unique_ptr<ExprNode> r) : op(o), left(std::move(l)), right(std::move(r)) {}

// --- Evaluators ---
std::string NumberLiteralNode::evaluate() const { return value; }
std::string StringLiteralNode::evaluate() const { return value; }
std::string BooleanLiteralNode::evaluate() const { return value ? "1" : "0"; }

std::string VariableNode::evaluate() const {
    if (SYMBOL_TABLE.find(name) != SYMBOL_TABLE.end())
        return SYMBOL_TABLE[name].value;
    return "0";
}

std::string CastOrRefNode::evaluate() const {
    // 1. Handle Reference (@var)
    if (operation == "@") {
        if (SYMBOL_TABLE.find(targetVar) != SYMBOL_TABLE.end()) {
            const void *address = static_cast<const void *>(&SYMBOL_TABLE[targetVar]);
            std::stringstream ss;
            ss << address;
            return ss.str();
        }
        return "NULL";
    }
    // 2. Get Current Value
    std::string currentValStr = "0";
    if (SYMBOL_TABLE.find(targetVar) != SYMBOL_TABLE.end())
        currentValStr = SYMBOL_TABLE[targetVar].value;
    // 3. Handle Casts (e.g., cast<int>)
    if (operation.find("int") != std::string::npos) {
        try {
            return std::to_string((int)std::stod(currentValStr));
        } catch (...) { return "0"; }
    }
    return currentValStr;
}

std::string ArrayAccessNode::evaluate() const {
    if (SYMBOL_TABLE.find(arrayName) == SYMBOL_TABLE.end()) return "ERR";
    RuntimeValue& arr = SYMBOL_TABLE[arrayName];
    if (index >= 0 && index < (int)arr.arrayElements.size())
        return arr.arrayElements[index];
    return "ERR";
}

std::string FunctionCallNode::evaluate() const {
    if (SYMBOL_TABLE.find(funcName) == SYMBOL_TABLE.end()) return "0";
    std::vector<std::string> argValues;
    std::vector<std::string> argTypes;
    
    // Evaluate arguments and determine their types
    for (auto& arg : args) {
        argValues.push_back(arg->evaluate());
        
        // Try to determine the type of the argument
        // Check if it's a variable reference
        VariableNode* varNode = dynamic_cast<VariableNode*>(arg.get());
        if (varNode) {
            // Get type from symbol table
            std::string varName = varNode->getName();
            if (SYMBOL_TABLE.find(varName) != SYMBOL_TABLE.end()) {
                argTypes.push_back(SYMBOL_TABLE[varName].type);
            } else {
                argTypes.push_back("unknown");
            }
        } else {
            // Infer type from value
            std::string val = argValues.back();
            std::string inferredType = "int"; // default
            if (val.find('.') != std::string::npos) {
                inferredType = "float";
            }
            argTypes.push_back(inferredType);
        }
    }
    
    RuntimeValue func = SYMBOL_TABLE[funcName];
    std::map<std::string, RuntimeValue> prevScope = SYMBOL_TABLE; // Snapshot

    for (size_t i = 0; i < func.params.size(); i++) {
        if (i < argValues.size()) {
            std::string paramType = func.params[i].first;
            std::string paramName = func.params[i].second;
            std::string argValue = argValues[i];
            std::string argType = argTypes[i];
            
            // Throw error if types don't match
            if (paramType != argType) {
                clear();
                throw std::runtime_error("Type mismatch: parameter '" + paramName + 
                    "' expects " + paramType + " but got " + argType);
            }
            
            SYMBOL_TABLE[paramName] = {paramType, argValue};
        }
    }

    std::string result = "0";
    try {
        if (func.body) func.body->execute();
    } catch (const ReturnException& ret) {
        result = ret.value;
    }
    SYMBOL_TABLE = prevScope; // Restore
    return result;
}

std::string BinaryOpNode::evaluate() const {
    std::string lStr = left->evaluate();
    std::string rStr = right->evaluate();
    double l = 0, r = 0;
    try {
        l = std::stod(lStr);
        r = std::stod(rStr);
    } catch (...) { return "0"; }

    double result = 0;
    if (op == "+") result = l + r;
    else if (op == "-") result = l - r;
    else if (op == "*") result = l * r;
    else if (op == "/") {
        if (r == 0) { std::cerr << "Runtime Error: Division by Zero\n"; return "0"; }
        result = l / r;
    }
    else if (op == "%") result = std::fmod(l, r);
    else if (op == "==") return (l == r) ? "1" : "0";
    else if (op == "!=") return (l != r) ? "1" : "0";
    else if (op == "<") return (l < r) ? "1" : "0";
    else if (op == ">") return (l > r) ? "1" : "0";
    else if (op == "<=") return (l <= r) ? "1" : "0";
    else if (op == ">=") return (l >= r) ? "1" : "0";
    else if (op == "&&") return ((l != 0) && (r != 0)) ? "1" : "0";
    else if (op == "||") return ((l != 0) || (r != 0)) ? "1" : "0";
    else return "0";
    
    // If result is a whole number, return as int
    if (result == (int)result) {
        return std::to_string((int)result);
    }
    return std::to_string(result);

}

// --- Executors ---
void VarDeclarationNode::execute() {
    std::string val = initializer ? initializer->evaluate() : "0";
    std::string fullType = baseType + (isPointer ? "*" : "");
    SYMBOL_TABLE[identifier] = {fullType, val};
}
void RetypeNode::execute() {
    if (SYMBOL_TABLE.find(targetVar) != SYMBOL_TABLE.end())
        SYMBOL_TABLE[targetVar].type = newType;
}
void ArrayDeclarationNode::execute() {
    RuntimeValue v; v.type = type + "[]";
    for (int i = 0; i < size; i++) v.arrayElements.push_back("0");
    SYMBOL_TABLE[name] = v;
}
void FunctionDefNode::execute() {
    SYMBOL_TABLE[name] = {returnType, "", {}, params, std::shared_ptr<BlockNode>(std::move(body))};
}
void BlockNode::execute() {
    for (const auto& s : statements) if (s) s->execute();
}
void FreeNode::execute() {
    if (SYMBOL_TABLE.find(identifier) != SYMBOL_TABLE.end()) SYMBOL_TABLE.erase(identifier);
    else std::cerr << "[!] Warning: Freeing non-existent '" << identifier << "'\n";
}
void IfNode::execute() {
    // Check if condition
    std::string condVal = condition->evaluate();
    if (condVal != "0") {
        thenBlock->execute();
        return;
    }
    
    // Check else-if conditions
    for (auto& elseIf : elseIfBlocks) {
        std::string elseIfVal = elseIf.first->evaluate();
        if (elseIfVal != "0") {
            elseIf.second->execute();
            return;
        }
    }
    
    // Execute else block if present
    if (elseBlock) {
        elseBlock->execute();
    }
}
void ReturnNode::execute() { throw ReturnException{value ? value->evaluate() : "0"}; }
void PrintNode::execute() { std::cout << expression->evaluate() << "\n"; }

// --- Parser ---
Parser::Parser(const std::vector<Token>& toks) : tokens(toks), position(0) {}
Token Parser::peek() { return (position < tokens.size()) ? tokens[position] : Token{TokenType::EndOfFile, ""}; }
Token Parser::advance() { return (position < tokens.size()) ? tokens[position++] : Token{TokenType::EndOfFile, ""}; }
Token Parser::consume(TokenType type, const std::string& err) {
    if (peek().type != type) throw std::runtime_error(err + " Got: " + peek().value);
    return advance();
}

// 1. Parse Primary (Numbers, Vars, Parens)
std::unique_ptr<ExprNode> Parser::parsePrimary() {
    Token t = peek();

    if (t.type == TokenType::True) { advance(); return std::make_unique<BooleanLiteralNode>(true); }
    if (t.type == TokenType::False) { advance(); return std::make_unique<BooleanLiteralNode>(false); }
    if (t.type == TokenType::StringLiteral) { advance(); return std::make_unique<StringLiteralNode>(t.value); }
    if (t.type == TokenType::Number) { advance(); return std::make_unique<NumberLiteralNode>(t.value); }
    
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
        if (peek().type == TokenType::Ampersand) advance();
        std::string var = consume(TokenType::Identifier, "Var").value;
        consume(TokenType::RightParen, ")");
        return std::make_unique<CastOrRefNode>(type, var);
    }
    if (t.type == TokenType::Identifier) {
        std::string name = advance().value;
        if (peek().type == TokenType::LeftParen) { // Function Call
            advance();
            std::vector<std::unique_ptr<ExprNode>> args;
            if (peek().type != TokenType::RightParen) do {
                if (peek().type == TokenType::Comma) advance();
                args.push_back(parseLogicalOr());
            } while (peek().type == TokenType::Comma);
            consume(TokenType::RightParen, ")");
            return std::make_unique<FunctionCallNode>(name, std::move(args));
        }
        if (peek().type == TokenType::LeftBracket) { // Array Access
            advance();
            int idx = std::stoi(consume(TokenType::Number, "Idx").value);
            consume(TokenType::RightBracket, "]");
            return std::make_unique<ArrayAccessNode>(name, idx);
        }
        return std::make_unique<VariableNode>(name);
    }
    if (t.type == TokenType::LeftParen) {
        advance();
        auto expr = parseLogicalOr();
        consume(TokenType::RightParen, ")");
        return expr;
    }
    return std::make_unique<NumberLiteralNode>("0");
}

// 2. Parse Term (*, /, %)
std::unique_ptr<ExprNode> Parser::parseTerm() {
    auto left = parsePrimary();
    while (peek().type == TokenType::Asterisk || peek().type == TokenType::Slash || peek().type == TokenType::Percent) {
        std::string op = advance().value;
        auto right = parsePrimary();
        left = std::make_unique<BinaryOpNode>(op, std::move(left), std::move(right));
    }
    return left;
}

// 3. Parse Addition/Subtraction (+, -)
std::unique_ptr<ExprNode> Parser::parseExpression() {
    auto left = parseTerm();
    while (peek().type == TokenType::Plus || peek().type == TokenType::Minus) {
        std::string op = advance().value;
        auto right = parseTerm();
        left = std::make_unique<BinaryOpNode>(op, std::move(left), std::move(right));
    }
    return left;
}

// 4. Parse Comparison (==, !=, <, >, <=, >=)
std::unique_ptr<ExprNode> Parser::parseComparison() {
    auto left = parseExpression();
    while (peek().type == TokenType::EqualEqual || peek().type == TokenType::NotEqual ||
           peek().type == TokenType::LessThan || peek().type == TokenType::GreaterThan ||
           peek().type == TokenType::LessEqual || peek().type == TokenType::GreaterEqual) {
        std::string op = advance().value;
        auto right = parseExpression();
        left = std::make_unique<BinaryOpNode>(op, std::move(left), std::move(right));
    }
    return left;
}

// 5. Parse Logical AND (&&)
std::unique_ptr<ExprNode> Parser::parseLogicalAnd() {
    auto left = parseComparison();
    while (peek().type == TokenType::AndAnd) {
        advance();
        auto right = parseComparison();
        left = std::make_unique<BinaryOpNode>("&&", std::move(left), std::move(right));
    }
    return left;
}

// 6. Parse Logical OR (||)
std::unique_ptr<ExprNode> Parser::parseLogicalOr() {
    auto left = parseLogicalAnd();
    while (peek().type == TokenType::OrOr) {
        advance();
        auto right = parseLogicalAnd();
        left = std::make_unique<BinaryOpNode>("||", std::move(left), std::move(right));
    }
    return left;
}

std::unique_ptr<BlockNode> Parser::parseBlock() {
    consume(TokenType::LeftBrace, "{");
    auto b = std::make_unique<BlockNode>();
    while (peek().type != TokenType::RightBrace && peek().type != TokenType::EndOfFile)
        if (auto s = parseStatement()) b->statements.push_back(std::move(s));
    consume(TokenType::RightBrace, "}");
    return b;
}

std::unique_ptr<ASTNode> Parser::parseStatement() {
    Token t = peek();

    if (t.type == TokenType::Return) {
        advance();
        auto e = parseLogicalOr();
        consume(TokenType::Semicolon, ";");
        return std::make_unique<ReturnNode>(std::move(e));
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
        
        // Handle else if and else
        while (peek().type == TokenType::Else) {
            advance();
            if (peek().type == TokenType::If) {
                advance();
                consume(TokenType::LeftParen, "(");
                auto elseIfCond = parseLogicalOr();
                consume(TokenType::RightParen, ")");
                auto elseIfBlock = parseBlock();
                ifNode->addElseIf(std::move(elseIfCond), std::move(elseIfBlock));
            } else {
                auto elseBlock = parseBlock();
                ifNode->setElse(std::move(elseBlock));
                break;
            }
        }
        return ifNode;
    }
    // Type Declarations (int, string, float, bool)
    if (t.type == TokenType::TypeInt || t.type == TokenType::TypeString || t.type == TokenType::TypeFloat || t.type == TokenType::TypeBool) {
        std::string type = t.value;
        advance();
        bool ptr = false;
        if (peek().type == TokenType::Asterisk) { advance(); ptr = true; }
        
        std::string name = consume(TokenType::Identifier, "Name").value;
        
        if (peek().type == TokenType::LeftParen) { // Function Def
            consume(TokenType::LeftParen, "(");
            std::vector<std::pair<std::string, std::string>> p;
            if (peek().type != TokenType::RightParen) do {
                if (peek().type == TokenType::Comma) advance();
                std::string pt = advance().value;
                p.push_back({pt, consume(TokenType::Identifier, "Param").value});
            } while (peek().type == TokenType::Comma);
            consume(TokenType::RightParen, ")");
            return std::make_unique<FunctionDefNode>(type + (ptr ? "*" : ""), name, p, parseBlock());
        }
        if (peek().type == TokenType::LeftBracket) { // Array Decl
            advance();
            int sz = std::stoi(consume(TokenType::Number, "Sz").value);
            consume(TokenType::RightBracket, "]");
            consume(TokenType::Semicolon, ";");
            return std::make_unique<ArrayDeclarationNode>(type, name, sz);
        }
        // Variable Decl
        consume(TokenType::Equals, "=");
        auto init = parseLogicalOr();
        consume(TokenType::Semicolon, ";");
        return std::make_unique<VarDeclarationNode>(type, ptr, name, std::move(init));
    }
    
    // Standalone expressions (e.g., function calls, math without assignment)
    if (t.type == TokenType::Identifier || t.type == TokenType::StaticCast) {
        auto e = parseLogicalOr();
        consume(TokenType::Semicolon, ";");
        return std::make_unique<ExpressionStatement>(std::move(e)); 
    }
    return nullptr;
}