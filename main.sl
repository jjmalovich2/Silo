bool is_safe = true;
bool* is_safe_ptr = @is_safe;

float myNum = 1.0;
int myNewInt = cast<int>(myNum); // copies myNum to myNewInt as an int rather than a float
static_cast<double>(myNum); // recasts myNum as a double
int myInt = cast<int*>(&myNum); // copies myNum to myInt as an int then frees myNum memory

print("HELLO");

string val = "Hello";
string* val_ptr = @val;
static_cast<string>(val_ptr);
print(val_ptr);

