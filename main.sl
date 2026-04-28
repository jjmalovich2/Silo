#include <math>

int scores[] = [84, 86, 75, 96, 84, 92, 71, 64, 68, 77];

int countPassing(int threshold) {
    int count = 0;
    
    for (int i = 0; i < scores.size(); i++) {
        if (scores[i] >= threshold) {
            count = count + 1;
        }
    }
    return count;
}

string name = input("What is your name? ");
int minPassing = input<int>("What is the minimum passing score (int)? ");
int* minPassing_ptr = @minPassing;

int totalPassing = countPassing(minPassing);

print(f"Name: {name}");
print(f"Passing Grade: {minPassing}");
print(f"MemAddr of minPassing: {minPassing_ptr}");
print(f"Total Passing Grades: {totalPassing}");
print(f"MemAddr of totalPassing: {@totalPassing}");
print(f"Average Grade: {avg_int(scores)} (imported from 'math' library)");
print("Freed Memory by Deleting 'scores' array from Values Table");
free scores;