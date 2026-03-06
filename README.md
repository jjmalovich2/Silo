# Silo

> A compiled, statically-typed programming language with the memory safety of Rust and the manual memory management of C — built from scratch in C++.

---

## Table of Contents

- [Overview](#overview)
- [Getting Started](#getting-started)
- [Language Syntax](#language-syntax)
  - [Variables & Types](#variables--types)
  - [Operators](#operators)
  - [Control Flow](#control-flow)
  - [Loops](#loops)
  - [Functions](#functions)
  - [Arrays](#arrays)
  - [Memory Management](#memory-management)
  - [Type Casting](#type-casting)
  - [References](#references)
  - [Strings & F-Strings](#strings--f-strings)
  - [Classes](#classes)
- [Access Modifiers](#access-modifiers)
- [Inheritance](#inheritance)
- [Full Example Programs](#full-example-programs)
- [Architecture](#architecture)
- [Building Silo](#building-silo)

---

## Overview

Silo is a from-scratch programming language with a hand-written lexer, recursive descent parser, and tree-walking interpreter — all implemented in C++. Silo files use the `.sl` extension.

Silo is designed with a few core goals:

- **Explicit memory management** — you allocate and free memory yourself, like C
- **Type safety** — types are checked at runtime and mismatches throw errors
- **Familiar syntax** — if you know C++, Python, or Java, Silo will feel immediately readable
- **Object-oriented** — full class support with inheritance, access modifiers, constructors, and `self`

---

## Getting Started

### Building

```bash
g++ *.cpp -o silo
```

### Running a Silo file

```bash
./silo yourfile.sl
```

### Your first program

```
// hello.sl
print("Hello, World!");
```

```bash
./silo hello.sl
# Hello, World!
```

---

## Language Syntax

### Variables & Types

Silo is statically typed. Every variable must be declared with an explicit type.

| Type     | Description              | Example                    |
|----------|--------------------------|----------------------------|
| `int`    | Integer number           | `int x = 10;`              |
| `float`  | Floating point number    | `float pi = 3.14;`         |
| `double` | Alias for `float`        | `double d = 2.718;`        |
| `string` | Text value               | `string name = "Silo";`    |
| `bool`   | Boolean (`true`/`false`) | `bool flag = true;`        |

```
int age = 25;
float temperature = 98.6;
string language = "Silo";
bool isAwesome = true;
```

Pointer types are also supported using `*`:

```
int* ptr = 0;
```

---

### Operators

#### Arithmetic

| Operator | Description    | Example         |
|----------|----------------|-----------------|
| `+`      | Addition       | `x + y`         |
| `-`      | Subtraction    | `x - y`         |
| `*`      | Multiplication | `x * y`         |
| `/`      | Division       | `x / y`         |
| `%`      | Modulo         | `x % y`         |
| `++`     | Post-increment | `x++`           |
| `--`     | Post-decrement | `x--`           |

#### Comparison

| Operator | Description           |
|----------|-----------------------|
| `==`     | Equal to              |
| `!=`     | Not equal to          |
| `<`      | Less than             |
| `>`      | Greater than          |
| `<=`     | Less than or equal    |
| `>=`     | Greater than or equal |

#### Logical

| Operator | Description |
|----------|-------------|
| `&&`     | Logical AND |
| `\|\|`   | Logical OR  |
| `!`      | Logical NOT |

#### String Concatenation

The `+` operator works on strings too:

```
string first = "Hello";
string second = " World";
string result = first + second;
print(result); // Hello World
```

---

### Control Flow

#### If / Else If / Else

```
int score = 85;

if (score >= 90) {
    print("A");
} else if (score >= 80) {
    print("B");
} else if (score >= 70) {
    print("C");
} else {
    print("F");
}
```

---

### Loops

#### While Loop

Executes as long as the condition is true.

```
int i = 0;
while (i < 5) {
    print(i);
    i++;
}
// 0 1 2 3 4
```

#### Do-While Loop

Executes the body at least once, then checks the condition.

```
int i = 0;
do {
    print(i);
    i++;
} while (i < 3);
// 0 1 2
```

#### For Loop

Classic C-style for loop with initializer, condition, and increment.

```
for (int i = 0; i < 5; i++) {
    print(i);
}
// 0 1 2 3 4
```

The increment supports both `++`/`--` and full assignment expressions:

```
for (int i = 10; i > 0; i = i - 3) {
    print(i);
}
// 10 7 4 1
```

---

### Functions

Functions are declared with a return type, name, parameters, and a block body. Use `void` for functions that don't return a value.

```
int add(int a, int b) {
    return a + b;
}

void greet(string name) {
    print(f"Hello, {name}!");
}

int result = add(3, 7);
print(result); // 10

greet("Silo"); // Hello, Silo!
```

> **Note:** Silo enforces type checking on function arguments. Passing the wrong type will throw a runtime error.

---

### Arrays

Arrays are declared with a fixed size. All elements are initialized to `0`.

```
int[5] scores;

// Array access uses bracket notation
scores[0] = 95;
scores[1] = 87;

print(scores[0]); // 95
print(scores[1]); // 87
```

---

### Memory Management

Silo gives you direct control over memory. Variables can be explicitly freed from memory using the `free` keyword. This removes them from the runtime symbol table entirely.

```
int x = 42;
print(x); // 42

free x;

// Accessing x after free is undefined — x no longer exists
```

> **Tip:** Freeing memory you no longer need is good practice in Silo. Trying to free a variable that doesn't exist will print a warning but won't crash the program.

---

### Type Casting

Silo supports explicit type casting using `cast<type>()` or `static_cast<type>()`.

```
float price = 9.99;
int rounded = cast<int>(price);
print(rounded); // 9

int count = 42;
string display = cast<string>(count);
print("Count: " + display); // Count: 42
```

You can also cast instance fields directly:

```
print(cast<string>(honda.numOfCars));
```

---

### References

Use the `@` operator to get the memory address of a variable.

```
int x = 100;
print(@x); // prints memory address of x
```

---

### Strings & F-Strings

#### Regular Strings

```
string greeting = "Hello, World!";
print(greeting);
```

#### F-Strings (Interpolated Strings)

Silo supports Python-style f-strings for embedding expressions directly inside string literals. Wrap any expression in `{}` inside an `f"..."` string.

```
string name = "Silo";
int version = 1;

print(f"Language: {name}, Version: {version}");
// Language: Silo, Version: 1
```

F-strings also work with `self` inside class methods:

```
print(f"Model: {self.model}");
print(f"VIN: {self.getVin()}");
```

---

### Classes

Silo has full object-oriented class support. Classes can have fields, methods, constructors, access modifiers, and inheritance.

#### Defining a Class

```
class Animal {
    string name;
    int age;

    constructor Animal(string n, int a) -> (name, age) {
        print(f"Animal {name} created");
    }

    string getName() {
        return name;
    }

    void birthday() {
        age++;
        print(f"{name} is now {age}");
    }
}
```

#### Creating Instances

Instances are created with the class name followed by the instance name and constructor arguments — similar to C++.

```
Animal cat("Whiskers", 3);
// Animal Whiskers created

cat.birthday();
// Whiskers is now 4

print(cat.getName());
// Whiskers
```

#### Constructors

Constructors use the `constructor` keyword. The `-> (field, field, ...)` syntax automatically assigns constructor arguments to class fields in order — no manual assignment needed.

```
class Point {
    float x;
    float y;

    constructor Point(float px, float py) -> (x, y) {
        print(f"Point created at {x}, {y}");
    }
}

Point p(3.5, 7.2);
// Point created at 3.5, 7.2
```

#### Methods

Methods are defined inside the class body just like functions. Use `self.fieldName` or `self.method()` to access the current instance.

```
class Counter {
    int count;

    constructor Counter(int start) -> (count) {}

    void increment() {
        count++;
    }

    void reset() {
        count = 0;
    }

    int getValue() {
        return count;
    }
}

Counter c(0);
c.increment();
c.increment();
c.increment();
print(cast<string>(c.getValue())); // 3
c.reset();
print(cast<string>(c.getValue())); // 0
```

---

## Access Modifiers

Silo provides four levels of field visibility:

| Modifier    | Description                                                                 |
|-------------|-----------------------------------------------------------------------------|
| *(none)*    | **Public** — accessible from anywhere                                       |
| `private`   | Accessible only within the class that defines it                            |
| `protected` | Accessible within the defining class and all subclasses                     |
| `global`    | A single shared value across all instances of the class (like Java `static`)|

```
class BankAccount {
    private int pin;           // only accessible inside BankAccount
    protected float balance;   // accessible in BankAccount and subclasses
    string owner;              // public — accessible anywhere
    global int totalAccounts;  // shared across all BankAccount instances

    constructor BankAccount(int p, float b, string o) -> (pin, balance, owner) {
        totalAccounts++;
    }
}
```

> **Private enforcement:** Accessing a `private` field from outside its class will throw a runtime error.

> **Global fields:** `global` fields belong to the class itself, not individual instances. All instances read from and write to the same value. Access them via any instance or by casting: `cast<string>(myObj.totalAccounts)`.

---

## Inheritance

Classes inherit from a parent using the `~` operator. Child classes inherit all non-private fields and all methods from the parent.

```
class Vehicle {
    protected float fuel;
    string model;
    global int numOfCars;

    constructor Vehicle(float f, string m) -> (fuel, model) {
        numOfCars++;
    }

    void refuel(float amount) {
        fuel = fuel + amount;
    }
}

class Truck~Vehicle {
    global int numTrucks;

    constructor Truck(float fuel, string model) -> Vehicle(fuel, model) {
        numTrucks++;
    }

    void status() {
        print(f"Model: {self.model}");
        print(f"Fuel: {self.fuel}");
    }
}
```

#### Parent Constructor Delegation

When a child class constructor uses `-> ParentClass(args)`, it automatically calls the parent constructor first, running its field bindings and body before continuing with the child's own constructor body.

```
Truck myTruck(100.0, "Ford F-150");
// Parent Vehicle constructor runs first
// Then Truck constructor body runs

myTruck.status();
// Model: Ford F-150
// Fuel: 100
```

#### Method Inheritance

Child classes automatically have access to all parent methods:

```
myTruck.refuel(50.0); // inherited from Vehicle
```

#### Protected Field Access

Child classes can access `protected` fields from the parent:

```
class SportsTruck~Truck {
    void boost() {
        fuel = fuel + 10.0; // works — fuel is protected in Vehicle
    }
}
```

---

## Full Example Programs

### Fibonacci Sequence

```
int fibonacci(int n) {
    if (n <= 1) {
        return n;
    }
    return fibonacci(n - 1) + fibonacci(n - 2);
}

for (int i = 0; i < 10; i++) {
    print(fibonacci(i));
}
// 0 1 1 2 3 5 8 13 21 34
```

### FizzBuzz

```
for (int i = 1; i <= 20; i++) {
    if (i % 15 == 0) {
        print("FizzBuzz");
    } else if (i % 3 == 0) {
        print("Fizz");
    } else if (i % 5 == 0) {
        print("Buzz");
    } else {
        print(i);
    }
}
```

### Class Hierarchy Example

```
class Vehicle {
    private int vin;
    protected float fuel;
    string model;
    global int numOfCars;

    constructor Vehicle(string v, float f, string n) -> (vin, fuel, model) {
        numOfCars++;
        print(f"Vehicle {model} created");
    }

    string getVin() {
        return vin;
    }
}

class Truck~Vehicle {
    global int numTrucks;

    constructor Truck(string vin, float fuel, string name) -> Vehicle(vin, fuel, name) {
        numTrucks++;
    }

    void status(bool showVin) {
        print(f"Model: {self.model}");
        print(f"Fuel: {self.fuel}");
        if (showVin) {
            print(f"VIN: {self.getVin()}");
        }
    }
}

Vehicle honda("12345678", 94.5, "Honda");
Truck toyota("87654321", 55.7, "Toyota");

toyota.status(true);
print(cast<string>(honda.numOfCars));  // 2 — both instances counted
print(cast<string>(toyota.numTrucks)); // 1
```

**Output:**
```
Vehicle Honda created
Vehicle Toyota created
Model: Toyota
Fuel: 55.7
VIN: 87654321
2
1
```

---

## Architecture

Silo is implemented as a classic three-stage pipeline:

```
Source Code (.sl)
      │
      ▼
  ┌────────┐
  │ Lexer  │  lexer.cpp
  └────────┘
      │  Token stream
      ▼
  ┌────────┐
  │ Parser │  AST.cpp (Parser section)
  └────────┘
      │  Abstract Syntax Tree
      ▼
  ┌─────────────┐
  │ Interpreter │  AST.cpp (execute/evaluate)
  └─────────────┘
      │
      ▼
   Output
```

### Lexer (`lexer.cpp`)
Converts raw source text into a flat list of typed tokens. Handles keywords, identifiers, literals, operators, f-strings, and comments.

### Parser (`AST.cpp` — Parser section)
A hand-written recursive descent parser. Consumes the token stream and produces an Abstract Syntax Tree (AST) made of `ASTNode` objects. Implements full operator precedence through layered parsing functions:

```
parseLogicalOr
  └─ parseLogicalAnd
       └─ parseComparison
            └─ parseExpression  (+, -)
                 └─ parseTerm   (*, /, %)
                      └─ parsePostfix  (++, --)
                           └─ parsePrimary
```

### Interpreter (`AST.cpp` — execute/evaluate)
A tree-walking interpreter. Each AST node implements `execute()` (for statements) or `evaluate()` (for expressions). All runtime state lives in a global `SYMBOL_TABLE` — a `std::map<std::string, RuntimeValue>`.

### Header (`silo.h`)
Defines all token types, AST node classes, runtime structures (`RuntimeValue`, `FieldDef`, `MethodDef`), and the `Lexer` / `Parser` class interfaces.

---

## Building Silo

### Requirements

- `g++` with C++14 or later
- macOS or Linux

### Compile

```bash
g++ *.cpp -o silo
```

### Run

```bash
./silo path/to/yourfile.sl
```

### Debug (symbol table dump)

At the end of every program run, Silo prints a full dump of the symbol table showing all variables, instances, and classes that were alive at program exit. This is useful for inspecting runtime state.

---

*Silo is a work in progress. New features are actively being developed.*