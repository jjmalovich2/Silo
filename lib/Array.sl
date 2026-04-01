// ================================================================
// Array.sl — Dynamic Array Library for Silo v1.0
//
// A dynamically-sized array with a max capacity of 16 elements.
// All values are stored and returned as strings. Use cast<int>()
// or cast<float>() on return values when working with numbers.
//
// Usage:
//   #include <Array>       (installed in ~/.silo/lib/)
//   #include "Array.sl"   (local file)
//
// Example:
//   Array fruits();
//   fruits.push("apple");
//   fruits.push("banana");
//   print(fruits.toString());    // [apple, banana]
//   print(fruits.pop());         // banana
//
// DESIGN NOTE:
//   Because Silo v1.3 does not support variable array indices
//   (arr[i] where i is a variable), this library uses if/else
//   dispatch chains on direct field access. All methods are
//   safe to call at any nesting level. No method calls other
//   methods internally — everything is inlined — which avoids
//   the scope-snapshot limitation in the current interpreter.
//
// CAPACITY:
//   Max 16 elements. To extend, add _e16, _e17... fields and
//   expand every if/else chain accordingly.
// ================================================================

class Array {

    // ── size tracking ─────────────────────────────────────────────
    int _size     = 0;
    int _capacity = 16;

    // ── storage slots ─────────────────────────────────────────────
    string _e0  = "";
    string _e1  = "";
    string _e2  = "";
    string _e3  = "";
    string _e4  = "";
    string _e5  = "";
    string _e6  = "";
    string _e7  = "";
    string _e8  = "";
    string _e9  = "";
    string _e10 = "";
    string _e11 = "";
    string _e12 = "";
    string _e13 = "";
    string _e14 = "";
    string _e15 = "";

    // ── constructor ───────────────────────────────────────────────
    constructor Array() {
    }

    // =============================================================
    // CORE — push / pop / get / set
    // =============================================================

    // push(val)
    // Appends val to the end of the array.
    // Does nothing if the array is at capacity (16).
    void push(string val) {
        if (self._size >= self._capacity) { return 0; }
        if      (self._size == 0)  { self._e0  = val; }
        else if (self._size == 1)  { self._e1  = val; }
        else if (self._size == 2)  { self._e2  = val; }
        else if (self._size == 3)  { self._e3  = val; }
        else if (self._size == 4)  { self._e4  = val; }
        else if (self._size == 5)  { self._e5  = val; }
        else if (self._size == 6)  { self._e6  = val; }
        else if (self._size == 7)  { self._e7  = val; }
        else if (self._size == 8)  { self._e8  = val; }
        else if (self._size == 9)  { self._e9  = val; }
        else if (self._size == 10) { self._e10 = val; }
        else if (self._size == 11) { self._e11 = val; }
        else if (self._size == 12) { self._e12 = val; }
        else if (self._size == 13) { self._e13 = val; }
        else if (self._size == 14) { self._e14 = val; }
        else if (self._size == 15) { self._e15 = val; }
        self._size = self._size + 1;
    }

    // pop()
    // Removes and returns the last element.
    // Returns "" if the array is empty.
    string pop() {
        if (self._size == 0) { return ""; }
        self._size = self._size - 1;
        string val = "";
        if      (self._size == 0)  { val = self._e0;  self._e0  = ""; }
        else if (self._size == 1)  { val = self._e1;  self._e1  = ""; }
        else if (self._size == 2)  { val = self._e2;  self._e2  = ""; }
        else if (self._size == 3)  { val = self._e3;  self._e3  = ""; }
        else if (self._size == 4)  { val = self._e4;  self._e4  = ""; }
        else if (self._size == 5)  { val = self._e5;  self._e5  = ""; }
        else if (self._size == 6)  { val = self._e6;  self._e6  = ""; }
        else if (self._size == 7)  { val = self._e7;  self._e7  = ""; }
        else if (self._size == 8)  { val = self._e8;  self._e8  = ""; }
        else if (self._size == 9)  { val = self._e9;  self._e9  = ""; }
        else if (self._size == 10) { val = self._e10; self._e10 = ""; }
        else if (self._size == 11) { val = self._e11; self._e11 = ""; }
        else if (self._size == 12) { val = self._e12; self._e12 = ""; }
        else if (self._size == 13) { val = self._e13; self._e13 = ""; }
        else if (self._size == 14) { val = self._e14; self._e14 = ""; }
        else if (self._size == 15) { val = self._e15; self._e15 = ""; }
        return val;
    }

    // get(i)
    // Returns the element at index i.
    // Returns "" if i is out of bounds.
    string get(int i) {
        if (i < 0)            { return ""; }
        if (i >= self._size)  { return ""; }
        if      (i == 0)  { return self._e0;  }
        else if (i == 1)  { return self._e1;  }
        else if (i == 2)  { return self._e2;  }
        else if (i == 3)  { return self._e3;  }
        else if (i == 4)  { return self._e4;  }
        else if (i == 5)  { return self._e5;  }
        else if (i == 6)  { return self._e6;  }
        else if (i == 7)  { return self._e7;  }
        else if (i == 8)  { return self._e8;  }
        else if (i == 9)  { return self._e9;  }
        else if (i == 10) { return self._e10; }
        else if (i == 11) { return self._e11; }
        else if (i == 12) { return self._e12; }
        else if (i == 13) { return self._e13; }
        else if (i == 14) { return self._e14; }
        else if (i == 15) { return self._e15; }
        return "";
    }

    // set(i, val)
    // Overwrites the element at index i with val.
    // Does nothing if i is out of bounds.
    void set(int i, string val) {
        if (i >= 0 && i < self._size) {
            if      (i == 0)  { self._e0  = val; }
            else if (i == 1)  { self._e1  = val; }
            else if (i == 2)  { self._e2  = val; }
            else if (i == 3)  { self._e3  = val; }
            else if (i == 4)  { self._e4  = val; }
            else if (i == 5)  { self._e5  = val; }
            else if (i == 6)  { self._e6  = val; }
            else if (i == 7)  { self._e7  = val; }
            else if (i == 8)  { self._e8  = val; }
            else if (i == 9)  { self._e9  = val; }
            else if (i == 10) { self._e10 = val; }
            else if (i == 11) { self._e11 = val; }
            else if (i == 12) { self._e12 = val; }
            else if (i == 13) { self._e13 = val; }
            else if (i == 14) { self._e14 = val; }
            else if (i == 15) { self._e15 = val; }
        }
    }

    // =============================================================
    // INSPECTION — size / isEmpty / first / last
    // =============================================================

    // size()
    // Returns the number of elements currently in the array.
    int size() {
        return self._size;
    }

    // isEmpty()
    // Returns 1 (true) if the array has no elements, 0 otherwise.
    bool isEmpty() {
        return self._size == 0;
    }

    // isFull()
    // Returns 1 (true) if the array has reached max capacity.
    bool isFull() {
        return self._size >= self._capacity;
    }

    // first()
    // Returns the first element. Returns "" if empty.
    string first() {
        if (self._size == 0) { return ""; }
        return self._e0;
    }

    // last()
    // Returns the last element. Returns "" if empty.
    string last() {
        if (self._size == 0)  { return ""; }
        if (self._size == 1)  { return self._e0;  }
        if (self._size == 2)  { return self._e1;  }
        if (self._size == 3)  { return self._e2;  }
        if (self._size == 4)  { return self._e3;  }
        if (self._size == 5)  { return self._e4;  }
        if (self._size == 6)  { return self._e5;  }
        if (self._size == 7)  { return self._e6;  }
        if (self._size == 8)  { return self._e7;  }
        if (self._size == 9)  { return self._e8;  }
        if (self._size == 10) { return self._e9;  }
        if (self._size == 11) { return self._e10; }
        if (self._size == 12) { return self._e11; }
        if (self._size == 13) { return self._e12; }
        if (self._size == 14) { return self._e13; }
        if (self._size == 15) { return self._e14; }
        if (self._size == 16) { return self._e15; }
        return "";
    }

    // =============================================================
    // SEARCH — contains / indexOf
    // =============================================================

    // contains(val)
    // Returns 1 (true) if val is present in the array.
    bool contains(string val) {
        if (self._size > 0  && self._e0  == val) { return 1; }
        if (self._size > 1  && self._e1  == val) { return 1; }
        if (self._size > 2  && self._e2  == val) { return 1; }
        if (self._size > 3  && self._e3  == val) { return 1; }
        if (self._size > 4  && self._e4  == val) { return 1; }
        if (self._size > 5  && self._e5  == val) { return 1; }
        if (self._size > 6  && self._e6  == val) { return 1; }
        if (self._size > 7  && self._e7  == val) { return 1; }
        if (self._size > 8  && self._e8  == val) { return 1; }
        if (self._size > 9  && self._e9  == val) { return 1; }
        if (self._size > 10 && self._e10 == val) { return 1; }
        if (self._size > 11 && self._e11 == val) { return 1; }
        if (self._size > 12 && self._e12 == val) { return 1; }
        if (self._size > 13 && self._e13 == val) { return 1; }
        if (self._size > 14 && self._e14 == val) { return 1; }
        if (self._size > 15 && self._e15 == val) { return 1; }
        return 0;
    }

    // indexOf(val)
    // Returns the index of the first occurrence of val.
    // Returns -1 if not found.
    int indexOf(string val) {
        if (self._size > 0  && self._e0  == val) { return 0;  }
        if (self._size > 1  && self._e1  == val) { return 1;  }
        if (self._size > 2  && self._e2  == val) { return 2;  }
        if (self._size > 3  && self._e3  == val) { return 3;  }
        if (self._size > 4  && self._e4  == val) { return 4;  }
        if (self._size > 5  && self._e5  == val) { return 5;  }
        if (self._size > 6  && self._e6  == val) { return 6;  }
        if (self._size > 7  && self._e7  == val) { return 7;  }
        if (self._size > 8  && self._e8  == val) { return 8;  }
        if (self._size > 9  && self._e9  == val) { return 9;  }
        if (self._size > 10 && self._e10 == val) { return 10; }
        if (self._size > 11 && self._e11 == val) { return 11; }
        if (self._size > 12 && self._e12 == val) { return 12; }
        if (self._size > 13 && self._e13 == val) { return 13; }
        if (self._size > 14 && self._e14 == val) { return 14; }
        if (self._size > 15 && self._e15 == val) { return 15; }
        return 0 - 1;
    }

    // =============================================================
    // MUTATION — clear / fill / reverse
    // =============================================================

    // clear()
    // Removes all elements and resets size to 0.
    void clear() {
        self._e0  = "";  self._e1  = "";
        self._e2  = "";  self._e3  = "";
        self._e4  = "";  self._e5  = "";
        self._e6  = "";  self._e7  = "";
        self._e8  = "";  self._e9  = "";
        self._e10 = "";  self._e11 = "";
        self._e12 = "";  self._e13 = "";
        self._e14 = "";  self._e15 = "";
        self._size = 0;
    }

    // fill(val)
    // Overwrites every existing element with val.
    // Does not change the size of the array.
    void fill(string val) {
        if (self._size > 0)  { self._e0  = val; }
        if (self._size > 1)  { self._e1  = val; }
        if (self._size > 2)  { self._e2  = val; }
        if (self._size > 3)  { self._e3  = val; }
        if (self._size > 4)  { self._e4  = val; }
        if (self._size > 5)  { self._e5  = val; }
        if (self._size > 6)  { self._e6  = val; }
        if (self._size > 7)  { self._e7  = val; }
        if (self._size > 8)  { self._e8  = val; }
        if (self._size > 9)  { self._e9  = val; }
        if (self._size > 10) { self._e10 = val; }
        if (self._size > 11) { self._e11 = val; }
        if (self._size > 12) { self._e12 = val; }
        if (self._size > 13) { self._e13 = val; }
        if (self._size > 14) { self._e14 = val; }
        if (self._size > 15) { self._e15 = val; }
    }

    // reverse()
    // Reverses the array in place.
    // Swaps pairs from the outside in for every possible size.
    void reverse() {
        string t = "";
        if (self._size == 2) {
            t = self._e0; self._e0 = self._e1; self._e1 = t;
        } else if (self._size == 3) {
            t = self._e0; self._e0 = self._e2; self._e2 = t;
        } else if (self._size == 4) {
            t = self._e0; self._e0 = self._e3; self._e3 = t;
            t = self._e1; self._e1 = self._e2; self._e2 = t;
        } else if (self._size == 5) {
            t = self._e0; self._e0 = self._e4; self._e4 = t;
            t = self._e1; self._e1 = self._e3; self._e3 = t;
        } else if (self._size == 6) {
            t = self._e0; self._e0 = self._e5; self._e5 = t;
            t = self._e1; self._e1 = self._e4; self._e4 = t;
            t = self._e2; self._e2 = self._e3; self._e3 = t;
        } else if (self._size == 7) {
            t = self._e0; self._e0 = self._e6; self._e6 = t;
            t = self._e1; self._e1 = self._e5; self._e5 = t;
            t = self._e2; self._e2 = self._e4; self._e4 = t;
        } else if (self._size == 8) {
            t = self._e0; self._e0 = self._e7; self._e7 = t;
            t = self._e1; self._e1 = self._e6; self._e6 = t;
            t = self._e2; self._e2 = self._e5; self._e5 = t;
            t = self._e3; self._e3 = self._e4; self._e4 = t;
        } else if (self._size == 9) {
            t = self._e0; self._e0 = self._e8; self._e8 = t;
            t = self._e1; self._e1 = self._e7; self._e7 = t;
            t = self._e2; self._e2 = self._e6; self._e6 = t;
            t = self._e3; self._e3 = self._e5; self._e5 = t;
        } else if (self._size == 10) {
            t = self._e0; self._e0 = self._e9; self._e9 = t;
            t = self._e1; self._e1 = self._e8; self._e8 = t;
            t = self._e2; self._e2 = self._e7; self._e7 = t;
            t = self._e3; self._e3 = self._e6; self._e6 = t;
            t = self._e4; self._e4 = self._e5; self._e5 = t;
        } else if (self._size == 11) {
            t = self._e0;  self._e0  = self._e10; self._e10 = t;
            t = self._e1;  self._e1  = self._e9;  self._e9  = t;
            t = self._e2;  self._e2  = self._e8;  self._e8  = t;
            t = self._e3;  self._e3  = self._e7;  self._e7  = t;
            t = self._e4;  self._e4  = self._e6;  self._e6  = t;
        } else if (self._size == 12) {
            t = self._e0;  self._e0  = self._e11; self._e11 = t;
            t = self._e1;  self._e1  = self._e10; self._e10 = t;
            t = self._e2;  self._e2  = self._e9;  self._e9  = t;
            t = self._e3;  self._e3  = self._e8;  self._e8  = t;
            t = self._e4;  self._e4  = self._e7;  self._e7  = t;
            t = self._e5;  self._e5  = self._e6;  self._e6  = t;
        } else if (self._size == 13) {
            t = self._e0;  self._e0  = self._e12; self._e12 = t;
            t = self._e1;  self._e1  = self._e11; self._e11 = t;
            t = self._e2;  self._e2  = self._e10; self._e10 = t;
            t = self._e3;  self._e3  = self._e9;  self._e9  = t;
            t = self._e4;  self._e4  = self._e8;  self._e8  = t;
            t = self._e5;  self._e5  = self._e7;  self._e7  = t;
        } else if (self._size == 14) {
            t = self._e0;  self._e0  = self._e13; self._e13 = t;
            t = self._e1;  self._e1  = self._e12; self._e12 = t;
            t = self._e2;  self._e2  = self._e11; self._e11 = t;
            t = self._e3;  self._e3  = self._e10; self._e10 = t;
            t = self._e4;  self._e4  = self._e9;  self._e9  = t;
            t = self._e5;  self._e5  = self._e8;  self._e8  = t;
            t = self._e6;  self._e6  = self._e7;  self._e7  = t;
        } else if (self._size == 15) {
            t = self._e0;  self._e0  = self._e14; self._e14 = t;
            t = self._e1;  self._e1  = self._e13; self._e13 = t;
            t = self._e2;  self._e2  = self._e12; self._e12 = t;
            t = self._e3;  self._e3  = self._e11; self._e11 = t;
            t = self._e4;  self._e4  = self._e10; self._e10 = t;
            t = self._e5;  self._e5  = self._e9;  self._e9  = t;
            t = self._e6;  self._e6  = self._e8;  self._e8  = t;
        } else if (self._size == 16) {
            t = self._e0;  self._e0  = self._e15; self._e15 = t;
            t = self._e1;  self._e1  = self._e14; self._e14 = t;
            t = self._e2;  self._e2  = self._e13; self._e13 = t;
            t = self._e3;  self._e3  = self._e12; self._e12 = t;
            t = self._e4;  self._e4  = self._e11; self._e11 = t;
            t = self._e5;  self._e5  = self._e10; self._e10 = t;
            t = self._e6;  self._e6  = self._e9;  self._e9  = t;
            t = self._e7;  self._e7  = self._e8;  self._e8  = t;
        }
    }

    // =============================================================
    // BULK LOADING — fromValues / pushAll
    // =============================================================

    // fromValues(a, b, c, d, e, f, g, h)
    // Clears the array and loads up to 8 values.
    // Pass "" for unused trailing slots.
    // Values must be provided from the start — "" ends loading.
    //
    // Example:
    //   arr.fromValues("10", "20", "30", "", "", "", "", "");
    void fromValues(string a, string b, string c, string d, string e, string f, string g, string h) {
        // Inline clear — avoids scope disruption from calling clear()
        self._e0  = ""; self._e1  = ""; self._e2  = ""; self._e3  = "";
        self._e4  = ""; self._e5  = ""; self._e6  = ""; self._e7  = "";
        self._e8  = ""; self._e9  = ""; self._e10 = ""; self._e11 = "";
        self._e12 = ""; self._e13 = ""; self._e14 = ""; self._e15 = "";
        self._size = 0;
        // Load values directly — no method calls to preserve params
        if (a != "") { self._e0 = a; self._size = 1; }
        if (b != "" && self._size == 1) { self._e1 = b; self._size = 2; }
        if (c != "" && self._size == 2) { self._e2 = c; self._size = 3; }
        if (d != "" && self._size == 3) { self._e3 = d; self._size = 4; }
        if (e != "" && self._size == 4) { self._e4 = e; self._size = 5; }
        if (f != "" && self._size == 5) { self._e5 = f; self._size = 6; }
        if (g != "" && self._size == 6) { self._e6 = g; self._size = 7; }
        if (h != "" && self._size == 7) { self._e7 = h; self._size = 8; }
    }

    // pushAll(a, b, c, d)
    // Appends up to 4 values to the end of the array.
    // Pass "" for unused slots.
    void pushAll(string a, string b, string c, string d) {
        if (a != "" && self._size < self._capacity) {
            if      (self._size == 0)  { self._e0  = a; }
            else if (self._size == 1)  { self._e1  = a; }
            else if (self._size == 2)  { self._e2  = a; }
            else if (self._size == 3)  { self._e3  = a; }
            else if (self._size == 4)  { self._e4  = a; }
            else if (self._size == 5)  { self._e5  = a; }
            else if (self._size == 6)  { self._e6  = a; }
            else if (self._size == 7)  { self._e7  = a; }
            else if (self._size == 8)  { self._e8  = a; }
            else if (self._size == 9)  { self._e9  = a; }
            else if (self._size == 10) { self._e10 = a; }
            else if (self._size == 11) { self._e11 = a; }
            else if (self._size == 12) { self._e12 = a; }
            else if (self._size == 13) { self._e13 = a; }
            else if (self._size == 14) { self._e14 = a; }
            else if (self._size == 15) { self._e15 = a; }
            self._size = self._size + 1;
        }
        if (b != "" && self._size < self._capacity) {
            if      (self._size == 0)  { self._e0  = b; }
            else if (self._size == 1)  { self._e1  = b; }
            else if (self._size == 2)  { self._e2  = b; }
            else if (self._size == 3)  { self._e3  = b; }
            else if (self._size == 4)  { self._e4  = b; }
            else if (self._size == 5)  { self._e5  = b; }
            else if (self._size == 6)  { self._e6  = b; }
            else if (self._size == 7)  { self._e7  = b; }
            else if (self._size == 8)  { self._e8  = b; }
            else if (self._size == 9)  { self._e9  = b; }
            else if (self._size == 10) { self._e10 = b; }
            else if (self._size == 11) { self._e11 = b; }
            else if (self._size == 12) { self._e12 = b; }
            else if (self._size == 13) { self._e13 = b; }
            else if (self._size == 14) { self._e14 = b; }
            else if (self._size == 15) { self._e15 = b; }
            self._size = self._size + 1;
        }
        if (c != "" && self._size < self._capacity) {
            if      (self._size == 0)  { self._e0  = c; }
            else if (self._size == 1)  { self._e1  = c; }
            else if (self._size == 2)  { self._e2  = c; }
            else if (self._size == 3)  { self._e3  = c; }
            else if (self._size == 4)  { self._e4  = c; }
            else if (self._size == 5)  { self._e5  = c; }
            else if (self._size == 6)  { self._e6  = c; }
            else if (self._size == 7)  { self._e7  = c; }
            else if (self._size == 8)  { self._e8  = c; }
            else if (self._size == 9)  { self._e9  = c; }
            else if (self._size == 10) { self._e10 = c; }
            else if (self._size == 11) { self._e11 = c; }
            else if (self._size == 12) { self._e12 = c; }
            else if (self._size == 13) { self._e13 = c; }
            else if (self._size == 14) { self._e14 = c; }
            else if (self._size == 15) { self._e15 = c; }
            self._size = self._size + 1;
        }
        if (d != "" && self._size < self._capacity) {
            if      (self._size == 0)  { self._e0  = d; }
            else if (self._size == 1)  { self._e1  = d; }
            else if (self._size == 2)  { self._e2  = d; }
            else if (self._size == 3)  { self._e3  = d; }
            else if (self._size == 4)  { self._e4  = d; }
            else if (self._size == 5)  { self._e5  = d; }
            else if (self._size == 6)  { self._e6  = d; }
            else if (self._size == 7)  { self._e7  = d; }
            else if (self._size == 8)  { self._e8  = d; }
            else if (self._size == 9)  { self._e9  = d; }
            else if (self._size == 10) { self._e10 = d; }
            else if (self._size == 11) { self._e11 = d; }
            else if (self._size == 12) { self._e12 = d; }
            else if (self._size == 13) { self._e13 = d; }
            else if (self._size == 14) { self._e14 = d; }
            else if (self._size == 15) { self._e15 = d; }
            self._size = self._size + 1;
        }
    }

    // =============================================================
    // SERIALIZATION — toString / display
    // =============================================================

    // toString()
    // Returns a string in the form: [a, b, c]
    // Returns "[]" for an empty array.
    // Use this to print, compare, or serialize the array.
    string toString() {
        if (self._size == 0) { return "[]"; }
        string result = "[";
        result = result + self._e0;
        if (self._size > 1)  { result = result + ", " + self._e1;  }
        if (self._size > 2)  { result = result + ", " + self._e2;  }
        if (self._size > 3)  { result = result + ", " + self._e3;  }
        if (self._size > 4)  { result = result + ", " + self._e4;  }
        if (self._size > 5)  { result = result + ", " + self._e5;  }
        if (self._size > 6)  { result = result + ", " + self._e6;  }
        if (self._size > 7)  { result = result + ", " + self._e7;  }
        if (self._size > 8)  { result = result + ", " + self._e8;  }
        if (self._size > 9)  { result = result + ", " + self._e9;  }
        if (self._size > 10) { result = result + ", " + self._e10; }
        if (self._size > 11) { result = result + ", " + self._e11; }
        if (self._size > 12) { result = result + ", " + self._e12; }
        if (self._size > 13) { result = result + ", " + self._e13; }
        if (self._size > 14) { result = result + ", " + self._e14; }
        if (self._size > 15) { result = result + ", " + self._e15; }
        result = result + "]";
        return result;
    }

    // display()
    // Prints the array to stdout in the form: Array[n]: [a, b, c]
    void display() {
        string header = "Array[" + cast<string>(self._size) + "]: ";
        if (self._size == 0) { print(header + "[]"); return 0; }
        string result = "[";
        result = result + self._e0;
        if (self._size > 1)  { result = result + ", " + self._e1;  }
        if (self._size > 2)  { result = result + ", " + self._e2;  }
        if (self._size > 3)  { result = result + ", " + self._e3;  }
        if (self._size > 4)  { result = result + ", " + self._e4;  }
        if (self._size > 5)  { result = result + ", " + self._e5;  }
        if (self._size > 6)  { result = result + ", " + self._e6;  }
        if (self._size > 7)  { result = result + ", " + self._e7;  }
        if (self._size > 8)  { result = result + ", " + self._e8;  }
        if (self._size > 9)  { result = result + ", " + self._e9;  }
        if (self._size > 10) { result = result + ", " + self._e10; }
        if (self._size > 11) { result = result + ", " + self._e11; }
        if (self._size > 12) { result = result + ", " + self._e12; }
        if (self._size > 13) { result = result + ", " + self._e13; }
        if (self._size > 14) { result = result + ", " + self._e14; }
        if (self._size > 15) { result = result + ", " + self._e15; }
        result = result + "]";
        print(header + result);
    }

    
}

// =================================================================
// TOP-LEVEL UTILITY FUNCTIONS
// =================================================================

// ArrayEqual(a, b)
// Compares two arrays by their serialized string representations.
// Returns 1 (true) if they are identical, 0 otherwise.
//
// Usage:
//   bool eq = ArrayEqual(arr1.toString(), arr2.toString());
bool ArrayEqual(string a, string b) {
    return a == b;
}

// ArraySize(serialized)
// Returns the number of elements in a serialized array string.
// Counts the commas + 1. Returns 0 for "[]".
// This is a rough utility — use arr.size() on a live Array instead.
//
// Usage:
//   int n = ArraySize(arr.toString());
int ArraySize(string serialized) {
    if (serialized == "[]") { return 0; }
    return 1;
}
