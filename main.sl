#include "lib/math.sl"

string name = input("What is your name: ");
int age = input<int>("How old are you: ");

print(name);
print(@name);
print(cast<string>(age));
print(@age);