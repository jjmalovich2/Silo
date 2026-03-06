#!/bin/bash

set -e

echo "Building Silo..."
g++ *.cpp -o silo
echo "✅ Compiled successfully"

echo "Installing system libraries to ~/.silo/lib/..."
mkdir -p ~/.silo/lib
cp lib/*.sl ~/.silo/lib/
echo "✅ Libraries installed"

echo "Installing silo binary to /usr/local/bin..."
sudo cp silo /usr/local/bin/silo
echo "✅ Done! Run: silo yourfile.sl"
```

---

### Project structure

Your repo should look like this:
```
Silo/
├── main.cpp
├── lexer.cpp
├── AST.cpp
├── silo.h
├── install.sh
├── lib/                  ← built-in .sl libraries (copied to ~/.silo/lib on install)
│   ├── stdlib.sl
│   ├── math.sl
│   └── strings.sl
└── test/
    └── classes.sl
```

---

### Starter `lib/math.sl`
```
// math.sl — Silo standard math library

int abs(int n) {
    if (n < 0) {
        return n - n - n;
    }
    return n;
}

int max(int a, int b) {
    if (a > b) { return a; }
    return b;
}

int min(int a, int b) {
    if (a < b) { return a; }
    return b;
}

int clamp(int val, int lo, int hi) {
    if (val < lo) { return lo; }
    if (val > hi) { return hi; }
    return val;
}

int pow(int base, int exp) {
    int result = 1;
    int i = 0;
    while (i < exp) {
        result = result * base;
        i++;
    }
    return result;
}
```

---

### Starter `lib/stdlib.sl`
```
// stdlib.sl — Silo standard library
```

---

### Usage
```
// my_program.sl

#include <math>          // from ~/.silo/lib/math.sl
#include "utils.sl"      // from same directory as my_program.sl
#include "lib/vectors.sl" // from a subdirectory relative to my_program.sl

print(max(10, 20));   // 20
print(pow(2, 8));     // 256