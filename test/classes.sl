class Vehicle {
    private int vin; // private, can only be used in this class definiton
    protected float fuel; // can be used by children classed
    string model; // can be seen globally
    global int numOfCars; // one variable that all children see, like `final` in Java

    constructor Vehicle(string v, float f, string n) -> (vin, fuel, model) {  // constructor functions are preceded by the constructor keyword, the `-> ()` assigns the args to the variables
        numOfCars++; // create plus-plus BinaryOp if needed, not sure if i did this already
        print(f"Vehicle {model} created"); // also create Python-like fstrings
    }

    string getVin() { // while we are here, lets create void token for functions
        return vin;
    }
}

class Truck~Vehicle {
    global int numTrucks;
    constructor Truck(string vin, float fuel, string name) -> Vehicle(vin, fuel, name) {
        // this child class also runs the parent class constructor
        numTrucks++;
    }

    void status(bool vin) {
        print(f"Model: {self.model}");
        print(f"Fuel: {self.fuel}");

        if (vin) {
            print(f"VIN: {self.getVin()}");
        }
    }
}

Vehicle honda("12345678", 94.5, "Honda");
Truck toyota(87654321, 55.7, "Toyota");

toyota.status(true);
print(cast<string>(honda.numOfCars));
print(cast<string>(toyota.numTrucks));