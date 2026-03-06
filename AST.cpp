#include "silo.h"
#include <iostream>
#include <sstream>
#include <cmath>
#include <stdexcept>
#include <set>

// =====================================================================
// GLOBALS
// =====================================================================

std::map<std::string, RuntimeValue> SYMBOL_TABLE;
std::string CURRENT_CLASS    = "";
std::string CURRENT_INSTANCE = "";

// =====================================================================
// HELPERS
// =====================================================================

struct ReturnException {
    std::string value;
};

void printSymbolTable() {
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

void clear() {
    std::cout << "\033[2J\033[1;1H";
    std::cout.flush();
}

static bool isNumeric(const std::string& s) {
    if (s.empty()) return false;
    bool hasDot = false;
    for (size_t i = (s[0] == '-' ? 1 : 0); i < s.size(); i++) {
        if (s[i] == '.') { if (hasDot) return false; hasDot = true; }
        else if (!isdigit((unsigned char)s[i])) return false;
    }
    return true;
}

static std::string formatNum(double v) {
    if (v == (long long)v)
        return std::to_string((long long)v);
    // Remove trailing zeros
    std::string s = std::to_string(v);
    size_t dot = s.find('.');
    if (dot != std::string::npos) {
        size_t last = s.find_last_not_of('0');
        if (last != std::string::npos && last > dot)
            s = s.substr(0, last + 1);
        else if (last == dot)
            s = s.substr(0, dot); // whole number, drop decimal entirely
    }
    return s;
}

// =====================================================================
// GLOBAL FIELD INJECT / CAPTURE / REAPPLY
// These three helpers let constructor and method bodies read/write
// global fields by plain name (e.g. numOfCars++) while keeping the
// authoritative values on the class definition in SYMBOL_TABLE.
// =====================================================================

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
                     type.compare(0, 9, "instance:") == 0 ||
                     type == "function" ||
                     globals.count(it->first) > 0);
        if (!keep) it = SYMBOL_TABLE.erase(it);
        else        ++it;
    }
}

// =====================================================================
// INSTANCE FIELD ACCESS
// =====================================================================

static std::string getInstanceField(const std::string& instName,
                                    const std::string& fieldName,
                                    const std::string& callerClass = "") {
    auto iit = SYMBOL_TABLE.find(instName);
    if (iit == SYMBOL_TABLE.end())
        throw std::runtime_error("Unknown instance: " + instName);

    RuntimeValue& inst = iit->second;
    std::string className = inst.instanceOf;

    // Global fields live on the class definition
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

// =====================================================================
// METHOD CALL
// =====================================================================

static std::string callMethod(const std::string& instName,
                              const std::string& methodName,
                              const std::vector<std::string>& argVals,
                              const std::vector<std::string>& argTypes) {
    auto iit = SYMBOL_TABLE.find(instName);
    if (iit == SYMBOL_TABLE.end())
        throw std::runtime_error("Unknown instance: " + instName);

    std::string className = iit->second.instanceOf;

    // Find method — walk class hierarchy
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

    // COPY by value — the pointer becomes dangling after SYMBOL_TABLE = prevSymbols
    MethodDef method = *methodPtr;

    std::string outerClass    = CURRENT_CLASS;
    std::string outerInstance = CURRENT_INSTANCE;

    // 1. Inject globals
    injectGlobals(className);
    // 2. Clear leaked outer locals so inner methods get a clean scope
    clearOuterLocals(className);
    // 3. Snapshot clean state
    auto prevSymbols = SYMBOL_TABLE;

    // 4. Set context
    CURRENT_CLASS    = className;
    CURRENT_INSTANCE = instName;

    // 5. Bind parameters
    for (size_t i = 0; i < method.params.size() && i < argVals.size(); i++) {
        const std::string& pname = method.params[i].second;
        const std::string& ptype = method.params[i].first;
        if (ptype != argTypes[i] && argTypes[i] != "unknown") {
            SYMBOL_TABLE     = prevSymbols;
            CURRENT_CLASS    = outerClass;
            CURRENT_INSTANCE = outerInstance;
            throw std::runtime_error("Type mismatch: param '" + pname +
                                     "' expects " + ptype + " but got " + argTypes[i]);
        }
        SYMBOL_TABLE[pname] = {ptype, argVals[i]};
    }

    // 6. Execute
    std::string result = "0";
    try {
        if (method.body) method.body->execute();
    } catch (const ReturnException& ret) {
        result = ret.value;
    }

    // 7. Capture globals, restore, reapply
    auto globalUpdates = captureGlobals(className);
    RuntimeValue updatedInst = SYMBOL_TABLE[instName];
    SYMBOL_TABLE = prevSymbols;
    SYMBOL_TABLE[instName] = updatedInst;
    reapplyGlobals(globalUpdates);

    CURRENT_CLASS    = outerClass;
    CURRENT_INSTANCE = outerInstance;

    return result;
}

// =====================================================================
// EXPRESSION STATEMENT WRAPPER
// =====================================================================

class ExpressionStatement : public ASTNode {
    std::unique_ptr<ExprNode> expr;
public:
    ExpressionStatement(std::unique_ptr<ExprNode> e) : expr(std::move(e)) {}
    void execute() override { expr->evaluate(); }
};

// =====================================================================
// CONSTRUCTORS
// =====================================================================

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
FunctionDefNode::FunctionDefNode(const std::string& rt, const std::string& n, const std::vector<std::pair<std::string,std::string>> p, std::unique_ptr<BlockNode> b) : returnType(rt), name(n), params(p), body(std::move(b)) {}
FreeNode::FreeNode(const std::string& id) : identifier(id) {}
WhileNode::WhileNode(std::unique_ptr<ExprNode> cond, std::unique_ptr<BlockNode> b) : condition(std::move(cond)), body(std::move(b)) {}
DoWhileNode::DoWhileNode(std::unique_ptr<ExprNode> cond, std::unique_ptr<BlockNode> b) : condition(std::move(cond)), body(std::move(b)) {}
ForNode::ForNode(std::unique_ptr<ASTNode> init, std::unique_ptr<ExprNode> cond, std::unique_ptr<ExprNode> inc, std::unique_ptr<BlockNode> b) : init(std::move(init)), condition(std::move(cond)), increment(std::move(inc)), body(std::move(b)) {}
PostfixOpNode::PostfixOpNode(const std::string& name, const std::string& o) : varName(name), op(o) {}
AssignExprNode::AssignExprNode(const std::string& name, std::unique_ptr<ExprNode> val) : varName(name), value(std::move(val)) {}
BinaryOpNode::BinaryOpNode(std::string o, std::unique_ptr<ExprNode> l, std::unique_ptr<ExprNode> r) : op(std::move(o)), left(std::move(l)), right(std::move(r)) {}

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

CastExprNode::CastExprNode(const std::string& t, std::unique_ptr<ExprNode> e)
    : targetType(t), expr(std::move(e)) {}

std::string CastExprNode::evaluate() const {
    std::string val = expr->evaluate();
    if (targetType.find("int") != std::string::npos) {
        try { return std::to_string((int)std::stod(val)); } catch (...) { return "0"; }
    }
    if (targetType.find("string") != std::string::npos) {
        return val;
    }
    if (targetType.find("float") != std::string::npos ||
        targetType.find("double") != std::string::npos) {
        try { return formatNum(std::stod(val)); } catch (...) { return "0.0"; }
    }
    return val;
}

// =====================================================================
// EXPRESSION EVALUATORS
// =====================================================================

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

            if (expr.size() >= 5 && expr.substr(0, 5) == "self.") {
                std::string member = expr.substr(5);
                if (!CURRENT_INSTANCE.empty()) {
                    size_t parenPos = member.find('(');
                    if (parenPos != std::string::npos) {
                        result += callMethod(CURRENT_INSTANCE, member.substr(0, parenPos), {}, {});
                    } else {
                        result += getInstanceField(CURRENT_INSTANCE, member, CURRENT_CLASS);
                    }
                }
            } else if (expr.find('.') != std::string::npos) {
                size_t dot = expr.find('.');
                std::string inst   = expr.substr(0, dot);
                std::string member = expr.substr(dot + 1);
                auto it = SYMBOL_TABLE.find(inst);
                if (it != SYMBOL_TABLE.end())
                    result += getInstanceField(inst, member, CURRENT_CLASS);
            } else {
                auto it = SYMBOL_TABLE.find(expr);
                if (it != SYMBOL_TABLE.end()) {
                    result += it->second.value;
                } else if (!CURRENT_INSTANCE.empty()) {
                    // Check instance fields
                    auto iit = SYMBOL_TABLE.find(CURRENT_INSTANCE);
                    if (iit != SYMBOL_TABLE.end()) {
                        auto fit = iit->second.fields.find(expr);
                        if (fit != iit->second.fields.end()) {
                            result += fit->second.value;
                        } else {
                            std::string cls = iit->second.instanceOf;
                            auto cit = SYMBOL_TABLE.find(cls);
                            if (cit != SYMBOL_TABLE.end()) {
                                auto gfit = cit->second.fields.find(expr);
                                if (gfit != cit->second.fields.end())
                                    result += gfit->second.value;
                                else result += expr;
                            } else result += expr;
                        }
                    } else result += expr;
                } else {
                    result += expr;
                }
            }
        }
    }
    return result;
}

std::string VariableNode::evaluate() const {
    auto it = SYMBOL_TABLE.find(name);
    if (it != SYMBOL_TABLE.end()) return it->second.value;

    // Fallback: check instance fields when inside a class method/constructor
    if (!CURRENT_INSTANCE.empty()) {
        auto iit = SYMBOL_TABLE.find(CURRENT_INSTANCE);
        if (iit != SYMBOL_TABLE.end()) {
            // Check instance fields
            auto fit = iit->second.fields.find(name);
            if (fit != iit->second.fields.end())
                return fit->second.value;
            // Check global fields on the class definition
            std::string cls = iit->second.instanceOf;
            auto cit = SYMBOL_TABLE.find(cls);
            if (cit != SYMBOL_TABLE.end()) {
                auto gfit = cit->second.fields.find(name);
                if (gfit != cit->second.fields.end() && gfit->second.access == "global")
                    return gfit->second.value;
            }
        }
    }
    return "0";
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
    std::string result = value->evaluate();
    auto it = SYMBOL_TABLE.find(varName);
    if (it != SYMBOL_TABLE.end()) it->second.value = result;
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
    if (operation.find("float")  != std::string::npos) {
        try { return formatNum(std::stod(currentValStr)); } catch (...) { return "0.0"; }
    }
    return currentValStr;
}

std::string ArrayAccessNode::evaluate() const {
    auto it = SYMBOL_TABLE.find(arrayName);
    if (it == SYMBOL_TABLE.end()) return "ERR";
    if (index >= 0 && index < (int)it->second.arrayElements.size())
        return it->second.arrayElements[index];
    return "ERR";
}

std::string FunctionCallNode::evaluate() const {
    auto it = SYMBOL_TABLE.find(funcName);
    if (it == SYMBOL_TABLE.end()) return "0";

    std::vector<std::string> argValues, argTypes;
    for (auto& arg : args) {
        argValues.push_back(arg->evaluate());
        const VariableNode* vn = dynamic_cast<const VariableNode*>(arg.get());
        if (vn) {
            auto vit = SYMBOL_TABLE.find(vn->getName());
            if (vit != SYMBOL_TABLE.end()) { argTypes.push_back(vit->second.type); continue; }
        }
        argTypes.push_back("unknown");
    }

    RuntimeValue func = it->second;
    auto prevScope = SYMBOL_TABLE;

    for (size_t i = 0; i < func.params.size() && i < argValues.size(); i++) {
        const std::string& ptype = func.params[i].first;
        const std::string& pname = func.params[i].second;
        if (ptype != argTypes[i] && argTypes[i] != "unknown")
            throw std::runtime_error("Type mismatch: param '" + pname +
                                     "' expects " + ptype + " but got " + argTypes[i]);
        SYMBOL_TABLE[pname] = {ptype, argValues[i]};
    }

    std::string result = "0";
    try {
        if (func.body) func.body->execute();
    } catch (const ReturnException& ret) { result = ret.value; }
    SYMBOL_TABLE = prevScope;
    return result;
}

std::string BinaryOpNode::evaluate() const {
    std::string lStr = left->evaluate();
    std::string rStr = right->evaluate();

    if ((!isNumeric(lStr) || !isNumeric(rStr)) && op == "+")
        return lStr + rStr;

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
    if (CURRENT_INSTANCE.empty())
        throw std::runtime_error("'self' used outside of a class method");
    if (isCall) {
        std::vector<std::string> argVals, argTypes;
        for (auto& a : callArgs) { argVals.push_back(a->evaluate()); argTypes.push_back("unknown"); }
        return callMethod(CURRENT_INSTANCE, memberName, argVals, argTypes);
    }
    return getInstanceField(CURRENT_INSTANCE, memberName, CURRENT_CLASS);
}

std::string MemberAccessNode::evaluate() const {
    auto iit = SYMBOL_TABLE.find(instanceName);
    if (iit == SYMBOL_TABLE.end())
        throw std::runtime_error("Unknown identifier: " + instanceName);

    RuntimeValue& inst = iit->second;

    if (inst.type == "class") {
        auto fit = inst.fields.find(memberName);
        if (fit != inst.fields.end()) return fit->second.value;
        throw std::runtime_error("Field '" + memberName + "' not found on class " + instanceName);
    }

    if (isCall) {
        std::vector<std::string> argVals, argTypes;
        for (auto& a : callArgs) {
            std::string val = a->evaluate();
            argVals.push_back(val);
            const VariableNode* vn = dynamic_cast<const VariableNode*>(a.get());
            if (vn) {
                auto vit = SYMBOL_TABLE.find(vn->getName());
                if (vit != SYMBOL_TABLE.end()) { argTypes.push_back(vit->second.type); continue; }
            }
            if      (val == "0" || val == "1")                        argTypes.push_back("bool");
            else if (isNumeric(val) && val.find('.') != std::string::npos) argTypes.push_back("float");
            else if (isNumeric(val))                                   argTypes.push_back("int");
            else                                                       argTypes.push_back("string");
        }
        return callMethod(instanceName, memberName, argVals, argTypes);
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

// =====================================================================
// STATEMENT EXECUTORS
// =====================================================================

void VarDeclarationNode::execute() {
    std::string val = initializer ? initializer->evaluate() : "0";
    SYMBOL_TABLE[identifier] = {baseType + (isPointer ? "*" : ""), val};
}
void RetypeNode::execute() {
    auto it = SYMBOL_TABLE.find(targetVar);
    if (it != SYMBOL_TABLE.end()) it->second.type = newType;
}
void ArrayDeclarationNode::execute() {
    RuntimeValue v; v.type = type + "[]";
    for (int i = 0; i < size; i++) v.arrayElements.push_back("0");
    SYMBOL_TABLE[name] = v;
}
void FunctionDefNode::execute() {
    SYMBOL_TABLE[name] = {"function", "", {}, params,
                          std::shared_ptr<BlockNode>(std::move(body))};
}
void BlockNode::execute() {
    for (const auto& s : statements) if (s) s->execute();
}
void FreeNode::execute() {
    auto it = SYMBOL_TABLE.find(identifier);
    if (it != SYMBOL_TABLE.end()) SYMBOL_TABLE.erase(it);
    else std::cerr << "[!] Warning: Freeing non-existent '" << identifier << "'\n";
}
void IfNode::execute() {
    if (condition->evaluate() != "0") { thenBlock->execute(); return; }
    for (auto& ei : elseIfBlocks)
        if (ei.first->evaluate() != "0") { ei.second->execute(); return; }
    if (elseBlock) elseBlock->execute();
}
void WhileNode::execute() {
    while (condition->evaluate() != "0") body->execute();
}
void DoWhileNode::execute() {
    do { body->execute(); } while (condition->evaluate() != "0");
}
void ForNode::execute() {
    if (init) init->execute();
    while (condition->evaluate() != "0") {
        body->execute();
        if (increment) increment->evaluate();
    }
}
void ReturnNode::execute() { throw ReturnException{value ? value->evaluate() : "0"}; }
void PrintNode::execute()  { std::cout << expression->evaluate() << "\n"; }
void MemberAccessStatement::execute() { node->evaluate(); }

// =====================================================================
// CLASS DEFINITION
// =====================================================================

void ClassDefNode::execute() {
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

// =====================================================================
// INSTANCE CREATION
// =====================================================================

void InstanceCreateNode::execute() {
    auto cit = SYMBOL_TABLE.find(className);
    if (cit == SYMBOL_TABLE.end())
        throw std::runtime_error("Unknown class: " + className);

    RuntimeValue inst;
    inst.type       = "instance:" + className;
    inst.instanceOf = className;
    for (const auto& kv : cit->second.fields)
        if (kv.second.access != "global")
            inst.fields[kv.first] = kv.second;

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

    // ---- Parent constructor delegation ----
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

                // 1. Inject parent globals BEFORE snapshot
                injectGlobals(parentCls);
                // 2. Snapshot
                auto prevSymbols = SYMBOL_TABLE;
                // 3. Bind parent ctor params
                for (size_t i = 0; i < parentCtor.params.size() && i < parentArgVals.size(); i++)
                    SYMBOL_TABLE[parentCtor.params[i].second] =
                        {parentCtor.params[i].first, parentArgVals[i]};
                // 4. Run parent ctor body
                try { if (parentCtor.body) parentCtor.body->execute(); }
                catch (const ReturnException&) {}
                // 5. Capture, restore, reapply
                auto globalUpdates = captureGlobals(parentCls);
                RuntimeValue updatedInst = SYMBOL_TABLE[instanceName];
                SYMBOL_TABLE = prevSymbols;
                SYMBOL_TABLE[instanceName] = updatedInst;
                reapplyGlobals(globalUpdates);
            }
        }
    }

    // ---- This constructor's -> (field, ...) bindings ----
    for (size_t i = 0; i < ctor.constructorBindings.size() && i < argVals.size(); i++) {
        const std::string& fn = ctor.constructorBindings[i];
        auto fit = SYMBOL_TABLE[instanceName].fields.find(fn);
        if (fit != SYMBOL_TABLE[instanceName].fields.end())
            fit->second.value = argVals[i];
    }

    // ---- Run this constructor's body ----

    // 1. Inject globals BEFORE snapshot
    injectGlobals(className);

    // 2. Snapshot (globals now included — survive restore)
    auto prevSymbols = SYMBOL_TABLE;

    // 3. Bind ctor params
    for (size_t i = 0; i < ctor.params.size() && i < argVals.size(); i++)
        SYMBOL_TABLE[ctor.params[i].second] = {ctor.params[i].first, argVals[i]};

    // 4. Execute
    try { if (ctor.body) ctor.body->execute(); }
    catch (const ReturnException&) {}

    // 5. Capture globals
    auto globalUpdates = captureGlobals(className);

    // 6. Restore scope, keep instance updates
    RuntimeValue updatedInst = SYMBOL_TABLE[instanceName];
    SYMBOL_TABLE = prevSymbols;
    SYMBOL_TABLE[instanceName] = updatedInst;

    // 7. Reapply globals
    reapplyGlobals(globalUpdates);

    CURRENT_CLASS    = outerClass;
    CURRENT_INSTANCE = outerInstance;
}

// =====================================================================
// PARSER
// =====================================================================

Parser::Parser(const std::vector<Token>& toks) : tokens(toks), position(0) {}

Token Parser::peek() {
    return (position < tokens.size()) ? tokens[position] : Token{TokenType::EndOfFile, ""};
}
Token Parser::advance() {
    return (position < tokens.size()) ? tokens[position++] : Token{TokenType::EndOfFile, ""};
}
Token Parser::consume(TokenType type, const std::string& err) {
    if (peek().type != type)
        throw std::runtime_error(err + " Got: '" + peek().value + "'");
    return advance();
}

std::string Parser::parseTypeName() {
    Token t = peek();
    if (t.type == TokenType::TypeInt    || t.type == TokenType::TypeString ||
        t.type == TokenType::TypeFloat  || t.type == TokenType::TypeBool   ||
        t.type == TokenType::Void       || t.type == TokenType::Identifier) {
        advance(); return t.value;
    }
    throw std::runtime_error("Expected type name, got: " + t.value);
}

std::unique_ptr<ExprNode> Parser::parsePrimary() {
    Token t = peek();

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
        // Parse a full expression instead of just an identifier
        auto inner = parseLogicalOr();
        consume(TokenType::RightParen, ")");
        // Wrap in a helper node that casts the result of any expression
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
            return std::make_unique<FunctionCallNode>(name, std::move(args));
        }

        if (peek().type == TokenType::LeftBracket) {
            advance();
            int idx = std::stoi(consume(TokenType::Number, "Array index").value);
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

std::unique_ptr<ExprNode> Parser::parsePostfix() {
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
    auto left = parseTerm();
    while (peek().type == TokenType::Plus || peek().type == TokenType::Minus) {
        std::string op = advance().value;
        left = std::make_unique<BinaryOpNode>(op, std::move(left), parseTerm());
    }
    return left;
}

std::unique_ptr<ExprNode> Parser::parseComparison() {
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
    auto left = parseComparison();
    while (peek().type == TokenType::AndAnd) {
        advance();
        left = std::make_unique<BinaryOpNode>("&&", std::move(left), parseComparison());
    }
    return left;
}

std::unique_ptr<ExprNode> Parser::parseLogicalOr() {
    auto left = parseLogicalAnd();
    while (peek().type == TokenType::OrOr) {
        advance();
        left = std::make_unique<BinaryOpNode>("||", std::move(left), parseLogicalAnd());
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

std::unique_ptr<ASTNode> Parser::parseClassDef() {
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
            std::string initVal = "0";
            if (peek().type == TokenType::Equals) {
                advance();
                initVal = parseLogicalOr()->evaluate();
            }
            consume(TokenType::Semicolon, ";");
            fields[memberName] = {access, typeName, initVal};
        }
    }

    consume(TokenType::RightBrace, "}");
    return std::make_unique<ClassDefNode>(className, parentName,
                                         std::move(fields), std::move(methods));
}

std::unique_ptr<ASTNode> Parser::parseStatement() {
    Token t = peek();

    if (t.type == TokenType::Class) return parseClassDef();

    if (t.type == TokenType::Return) {
        advance(); auto e = parseLogicalOr();
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
            if (peek().type == TokenType::PlusPlus || peek().type == TokenType::MinusMinus)
                inc = std::make_unique<PostfixOpNode>(varName, advance().value);
            else {
                consume(TokenType::Equals, "=");
                inc = std::make_unique<AssignExprNode>(varName, parseLogicalOr());
            }
        }
        consume(TokenType::RightParen, ")");
        return std::make_unique<ForNode>(std::move(init), std::move(cond),
                                        std::move(inc), parseBlock());
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
                    p.push_back({pt, consume(TokenType::Identifier, "Param").value});
                } while (peek().type == TokenType::Comma);
            }
            consume(TokenType::RightParen, ")");
            return std::make_unique<FunctionDefNode>(type + (ptr ? "*" : ""), name, p, parseBlock());
        }
        if (peek().type == TokenType::LeftBracket) {
            advance();
            int sz = std::stoi(consume(TokenType::Number, "Sz").value);
            consume(TokenType::RightBracket, "]");
            consume(TokenType::Semicolon, ";");
            return std::make_unique<ArrayDeclarationNode>(type, name, sz);
        }
        consume(TokenType::Equals, "=");
        auto init = parseLogicalOr();
        consume(TokenType::Semicolon, ";");
        return std::make_unique<VarDeclarationNode>(type, ptr, name, std::move(init));
    }

    if (t.type == TokenType::Identifier) {
        // Detect class instantiation: ClassName varName(args);
        if (position + 1 < tokens.size() &&
            tokens[position + 1].type == TokenType::Equals) {
            std::string varName = advance().value; // consume name
            advance();                             // consume =
            auto val = parseLogicalOr();
            consume(TokenType::Semicolon, ";");
            return std::make_unique<ExpressionStatement>(
                std::make_unique<AssignExprNode>(varName, std::move(val))
            );
        }
        auto e = parseLogicalOr();
        consume(TokenType::Semicolon, ";");
        return std::make_unique<ExpressionStatement>(std::move(e));
    }

    if (t.type == TokenType::Self) {
        auto e = parseLogicalOr();
        consume(TokenType::Semicolon, ";");
        return std::make_unique<ExpressionStatement>(std::move(e));
    }

    return nullptr;
}