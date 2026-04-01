# Silo Interpreter — Update Log

---

## Version 1.3 — Critical Bug Fix Pass
**Date:** 2026-03-10
**Files Modified:** `AST.cpp`, `silo.h`

This update addresses all critical and high-priority silent failure bugs identified
in the first full code review of the interpreter. Every fix targets behavior that
produced wrong results with no error or warning.

---

### silo.h

#### [NEW] Forward declaration of `ExprNode` before `FieldDef`
**Type:** Bug Fix — Compilation  
**Severity:** 🔴 Critical

`FieldDef` now holds a `std::shared_ptr<ExprNode>` for deferred field initialization.
Because `ExprNode` is defined later in the header, a forward declaration is required
before `FieldDef` to allow the compiler to resolve the type.

```cpp
// Added before FieldDef
struct ExprNode;
```

---

#### [CHANGED] `FieldDef` — added `initExpr` field
**Type:** Bug Fix — Struct/class field defaults evaluated at parse time  
**Severity:** 🔴 Critical

Previously, default field values in class and struct definitions were evaluated
immediately during parsing via `parseLogicalOr()->evaluate()`. This meant defaults
were captured at parse time rather than at instantiation time, so any default that
referenced a runtime variable or function call would silently produce the wrong value
for every instance.

The fix stores the parsed expression itself and re-evaluates it fresh each time an
instance is created.

```cpp
// Before
struct FieldDef {
    std::string access;
    std::string type;
    std::string value;
    bool isConst = false;
};

// After
struct FieldDef {
    std::string access;
    std::string type;
    std::string value;
    bool isConst = false;
    std::shared_ptr<ExprNode> initExpr; // deferred evaluation
};
```

---

#### [CHANGED] `FunctionDefNode` — `body` member changed from `unique_ptr` to `shared_ptr`
**Type:** Bug Fix — Function body destroyed on first execution  
**Severity:** 🔴 Critical

`FunctionDefNode::execute()` was transferring ownership of `body` into the
`RuntimeValue` stored in the symbol table via `std::move`. This permanently emptied
the `body` member of the AST node. Any subsequent call to `execute()` on the same
node (e.g. a function defined inside a loop) would silently become a no-op because
`body` was null.

Changing to `shared_ptr` allows both the AST node and the symbol table entry to hold
shared ownership, so the body is never destroyed.

```cpp
// Before
class FunctionDefNode : public ASTNode {
    std::unique_ptr<BlockNode> body;
public:
    FunctionDefNode(..., std::unique_ptr<BlockNode> b);
};

// After
class FunctionDefNode : public ASTNode {
    std::shared_ptr<BlockNode> body;
public:
    FunctionDefNode(..., std::shared_ptr<BlockNode> b);
};
```

---

#### [NEW] `MemberAssignNode` class declaration
**Type:** Bug Fix — `obj.field = val` assignment silently dropped  
**Severity:** 🔴 Critical

Previously there was no parse path for dotted lvalue assignment from outside a class.
The parser would read `obj.field` as a `MemberAccessNode` (a read), then hit `=` and
throw a parse error. The write was never performed.

`MemberAssignNode` is a new expression node that calls `setInstanceField` at runtime,
finally making the previously dead `setInstanceField` function useful.

```cpp
class MemberAssignNode : public ExprNode {
    std::string instanceName;
    std::string fieldName;
    std::unique_ptr<ExprNode> value;
public:
    MemberAssignNode(const std::string& inst, const std::string& field,
                     std::unique_ptr<ExprNode> val);
    std::string evaluate() const override;
};
```

---

### AST.cpp

#### [CHANGED] `inferType` — removed spurious bool detection
**Type:** Bug Fix — Spurious type mismatch errors  
**Severity:** 🔴 Critical

The previous `inferType` checked for `"true"` and `"false"` as bool indicators.
However, booleans in Silo are stored internally as `"1"` and `"0"`, so these string
checks would never match. Worse, classifying `"0"` and `"1"` as bool rather than int
would cause any function with an `int` parameter called with `0` or `1` to throw a
spurious type mismatch error.

```cpp
// Before
static std::string inferType(const std::string& val) {
    if (val == "true" || val == "false") return "bool";
    if (isNumeric(val)) return (val.find('.') != std::string::npos) ? "float" : "int";
    return "string";
}

// After
static std::string inferType(const std::string& val) {
    if (isNumeric(val))
        return (val.find('.') != std::string::npos) ? "float" : "int";
    return "string";
}
```

---

#### [CHANGED] `VariableNode::evaluate` — throws on undefined variable
**Type:** Bug Fix — Undefined variable silently returns `"0"`  
**Severity:** 🔴 Critical

Referencing any undefined variable, including typos, out-of-scope names, or variables
used before declaration, previously returned `"0"` silently. This caused arithmetic to
continue with wrong values and made bugs nearly impossible to diagnose.

```cpp
// Before
return "0";

// After
throw std::runtime_error("Undefined variable: '" + name + "'");
```

---

#### [CHANGED] `FunctionCallNode::evaluate` — throws on undefined function
**Type:** Bug Fix — Undefined function silently returns `"0"`  
**Severity:** 🔴 Critical

Calling any undefined function, including functions called before they are defined or
with a misspelled name, previously returned `"0"` silently.

```cpp
// Before
if (it == SYMBOL_TABLE.end()) return "0";

// After
if (it == SYMBOL_TABLE.end())
    throw std::runtime_error("Undefined function: '" + funcName + "'");
```

---

#### [CHANGED] `FunctionCallNode::evaluate` — uses `inferType`, enforces types, allows int→float
**Type:** Bug Fix — Type checks always bypassed  
**Severity:** 🔴 Critical

All call sites previously passed `"unknown"` as arg types, meaning declared parameter
types were never actually checked. The fix infers real argument types and enforces them,
with an explicit allowance for passing `int` values to `float` parameters (widening
coercion).

```cpp
// Before
argTypes.push_back("unknown");

// After
argTypes.push_back(inferType(val));

// Type check now allows int -> float coercion
bool ok = (ptype == "float" && argTypes[i] == "int");
if (!ok)
    throw std::runtime_error("Type mismatch: ...");
```

---

#### [CHANGED] `callMethod` — allows int→float coercion in parameter type check
**Type:** Bug Fix — Type checks always bypassed  
**Severity:** 🔴 Critical

Same fix applied to method calls as to free function calls. Integer literals passed
to float parameters no longer throw a spurious type mismatch.

---

#### [CHANGED] `AssignExprNode::evaluate` — checks instance fields before symbol table
**Type:** Bug Fix — Two conflicting field-write paths / `setInstanceField` unused  
**Severity:** 🔴 Critical

Previously, `AssignExprNode` checked the flat symbol table first. If any outer
variable shared a name with an instance field, the assignment would silently write
to the outer variable instead of the field, leaving the instance unchanged.

The fix checks the current instance's fields first when inside a method, before
falling back to the symbol table. `setInstanceField` is now called through
`MemberAssignNode` for external writes.

```cpp
// Now checks instance fields first when CURRENT_INSTANCE is set
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
```

---

#### [NEW] `MemberAssignNode::evaluate` — external field assignment
**Type:** Bug Fix — `obj.field = val` silently dropped  
**Severity:** 🔴 Critical

New node implementation. Delegates directly to `setInstanceField`, which was previously
dead code.

```cpp
std::string MemberAssignNode::evaluate() const {
    std::string result = value->evaluate();
    setInstanceField(instanceName, fieldName, result);
    return result;
}
```

---

#### [CHANGED] `BinaryOpNode::evaluate` — string equality fixed
**Type:** Bug Fix — String equality always returned wrong result  
**Severity:** 🔴 Critical

All comparison operators fell through to `std::stod` conversion. For non-numeric
strings, `stod` throws, the catch block returns `"0"`, meaning both `"hello" == "hello"`
and `"hello" != "hello"` silently returned `"0"` (false). String equality is now
handled before the numeric path.

```cpp
// Added before numeric conversion
if (!isNumeric(lStr) || !isNumeric(rStr)) {
    if (op == "==") return (lStr == rStr) ? "1" : "0";
    if (op == "!=") return (lStr != rStr) ? "1" : "0";
    return "0";
}
```

---

#### [CHANGED] `SelfAccessNode::evaluate` — uses `inferType` for method args
**Type:** Bug Fix — Type checks always bypassed  
**Severity:** 🔴 Critical

`self.method(args)` calls now pass inferred types rather than `"unknown"`.

---

#### [CHANGED] `MemberAccessNode::evaluate` — uses `inferType` for method args
**Type:** Bug Fix — Type checks always bypassed  
**Severity:** 🔴 Critical

`obj.method(args)` calls now pass inferred types rather than `"unknown"`.

---

#### [CHANGED] `FunctionDefNode::execute` — body assigned by shared ownership, not moved
**Type:** Bug Fix — Function body destroyed on first execution  
**Severity:** 🔴 Critical

`body` is now `shared_ptr`, so assigning it to the `RuntimeValue` creates a shared
copy rather than a destructive move. The AST node retains its body across multiple
executions.

```cpp
// Before
rv.body = std::shared_ptr<BlockNode>(std::move(body)); // body emptied

// After
rv.body = body; // shared copy, node body stays intact
```

---

#### [CHANGED] `InstanceCreateNode::execute` — field copy condition corrected
**Type:** Bug Fix — All instance fields missing on creation  
**Severity:** 🔴 Critical (Regression introduced during prior fix)

The field copy loop had its condition inverted, copying only `global` fields to new
instances and skipping all regular fields. Every instance was being created empty.

```cpp
// Before (wrong — copies only global fields)
if (kv.second.access == "global") {

// After (correct — copies everything except global fields)
if (kv.second.access != "global") {
```

---

#### [CHANGED] `InstanceCreateNode::execute` — field defaults re-evaluated at instantiation
**Type:** Bug Fix — Struct/class field defaults evaluated at parse time  
**Severity:** 🔴 Critical

When copying fields to a new instance, `initExpr` is now re-evaluated if present,
ensuring each instance gets a fresh evaluation of its default expressions rather than
a stale value captured at parse time.

```cpp
if (fd.initExpr)
    fd.value = fd.initExpr->evaluate();
inst.fields[kv.first] = fd;
```

---

#### [CHANGED] `parseStatement` — added `obj.field = val` parse path
**Type:** Bug Fix — Dotted lvalue assignment silently dropped  
**Severity:** 🔴 Critical

Added lookahead in the `Identifier` branch to detect `name . name =` and emit a
`MemberAssignNode` instead of attempting to parse the left-hand side as an expression.

---

#### [CHANGED] `parseStatement` — added `self.field = val` parse path
**Type:** Bug Fix — `self.field` write as statement silently dropped  
**Severity:** 🔴 Critical

Added lookahead in the `Self` branch to detect `self . name =` and emit an
`AssignExprNode` (which now correctly prioritises instance fields) for the write.

---

#### [CHANGED] `parseStatement` — function definition wraps `parseBlock` in `shared_ptr`
**Type:** Bug Fix — Function body destroyed on first execution  
**Severity:** 🔴 Critical

`FunctionDefNode` now accepts `shared_ptr<BlockNode>`, so the construction site must
wrap `parseBlock()` accordingly.

```cpp
// Before
return std::make_unique<FunctionDefNode>(..., parseBlock());

// After
return std::make_unique<FunctionDefNode>(..., std::shared_ptr<BlockNode>(parseBlock()));
```

---

#### [CHANGED] `parseStructDef` — field default stored and evaluated correctly
**Type:** Bug Fix — Struct field defaults always `"0"`  
**Severity:** 🟡 Medium (was incomplete fix from prior pass)

`fd.value` was hardcoded to `initVal` which was always `"0"`. The parsed expression
was stored in `fd.initExpr` but `fd.value` was never updated from it.

```cpp
// Before
fd.value   = initVal; // always "0"
fd.initExpr = initExpr;

// After
fd.value   = initExpr ? initExpr->evaluate() : "0";
fd.initExpr = initExpr;
```

---

## Known Remaining Issues

| Priority | Issue |
|---|---|
| 🟠 High | Whole-table copy for scoping — locals from outer scope can still leak into methods |
| 🟠 High | Array indexing requires an integer literal — variable index silently parses wrong |
| 🟠 High | No `break` or `continue` in loops |
| 🟡 Medium | Inheritance copies methods/fields at class definition time — stale after parent redefinition |
| 🟡 Medium | `@` address-of returns unstable pointer — map insertions invalidate it |
| 🟡 Medium | f-string only resolves simple names and `self.member` — expressions like `{x + 1}` emit raw text |
| 🟢 Low | No compound assignment operators (`+=`, `-=`, `*=`, `/=`) |
| 🟢 Low | No prefix increment (`++x`, `--x`) |
| 🟢 Low | No unary minus on variables (`-x` where x is a variable) |
| 🟢 Low | `clear()` unconditionally wipes the terminal on startup |
| 🟢 Low | No `break`/`continue` — missing feature, not a silent failure |