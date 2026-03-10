struct Math {
    const double PI = 3.14159265358979;
    const double E  = 2.71828182845904;
}

double abs(double val) {
    if (val < 0) { return val - val - val; }
    return val;
}

double clamp(double val, double lo, double hi) {
    if (val < lo) { return lo; }
    if (val > hi) { return hi; }
    return val;
}

double exp(double bs, int ex) {
    if (ex == 0) { return 1; }
    double result = 1.0;
    if (ex > 0) {
        for (int i = 0; i < ex; i++) {
            result = result * bs;
        }
        return result;
    } else {
        int posEx = 0 - ex;
        for (int i = 0; i < posEx; i++) {
            result = result * bs;
        }
        return 1 / result;
    }
    return 0;
}

double sqrt(double val) {
    if (val < 0) {
        print("Cannot sqrt negative number");
        return -1;
    }
    if (val == 0) { return 0; }
    double guess = val / 2.0;
    double prev_guess = 0.0;
    while (abs(guess - prev_guess) >= 0.00001) {
        prev_guess = guess;
        guess = 0.5 * (guess + val / guess);
    }
    return guess;
}

double max(double n1, double n2) {
    if (n1 >= n2) { return n1; }
    return n2;
}

double min(double n1, double n2) {
    if (n1 <= n2) { return n1; }
    return n2;
}

double round(double val, int dec) {
    double factor = exp(10, dec);
    double shifted = val * factor + 0.5;
    return shifted / factor;
}

int floor(double v) {
    int f = v;
    return f;
}

int ceil(double v) {
    int f = v;
    if (v - f > 0) { return f + 1; }
    return f;
}

int sign(double v) {
    if (v < 0) { return -1; }
    if (v == 0) { return 0; }
    return 1;
}

bool isEven(int n) {
    return (n % 2 == 0);
}

bool isOdd(int n) {
    return !isEven(n);
}

double log(double x) {
    if (x <= 0) { return 0; }
    double guess = 1.0;
    double prev = 0.0;
    for (int i = 0; i < 100; i++) {
        prev = guess;
        guess = guess - 1.0 + x / exp(Math.E, cast<int>(guess));
        if (abs(guess - prev) < 0.00001) { return guess; }
    }
    return guess;
}

double log10(double x) {
    return log(x) / log(10);
}

int factorial(int n) {
    int result = 1;
    for (int i = 1; i <= n; i++) {
        result = result * i;
    }
    return result;
}

double toRadians(double deg) {
    return deg * (Math.PI / 180.0);
}

double sin(double x) {
    double result = 0.0;
    double term = x;
    int s = 1;
    for (int i = 1; i < 20; i++) {
        result = result + s * term;
        term = term * x * x / ((2 * i) * (2 * i + 1));
        s = s - s - s;
    }
    return result;
}

double cos(double x) {
    double result = 1.0;
    double term = 1.0;
    int s = 0 - 1;
    for (int i = 1; i < 20; i++) {
        term = term * x * x / ((2 * i - 1) * (2 * i));
        result = result + s * term;
        s = s - s - s;
    }
    return result;
}

double tan(double x) {
    return sin(x) / cos(x);
}

int gcd(int a, int b) {
    if (a < 0) { a = abs(a); }
    if (b < 0) { b = abs(b); }
    while (b != 0) {
        int temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

int lcm(int a, int b) {
    return (a / gcd(a, b)) * b;
}

double lerp(double a, double b, double t) {
    return a + t * (b - a);
}