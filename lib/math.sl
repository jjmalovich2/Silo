double PI = 3.14159265358979;
double E  = 2.71828182845904;

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
    if (ex == 0) {
        return 1;
    }

    double result = 1.0;

    if (ex > 0) {
        for (int i = 0; i < ex; i++) {
            result = result * bs;
        }
        return result;
    } else {
        for (int i = 0; i < ex; i++) {
            result = result * bs;
        }
        temp = cast<double>(result);
        return 1 / temp;
    }

    return 0;
}

double sqrt(double val) {
    if (val < 0) {
        print("Cannot sqrt negative number");
        return -1;
    }
    if (val == 0) { return 0; }

    double guess = val / 2.0; // initial guess
    double prev_guess = 0.0;

    while (abs(guess - prev_guess) >= 0.00001) {
        prev_guess = guess;
        guess = 0.5 * (guess + val / guess);
    }
    return guess;
}

double max(double n1, double n2) {
    if (n1 >= n2) {
        return n1;
    } else {
        return n2;
    }
}

double min(double n1, double n2) {
    if (n1 <= n2) {
        return n1;
    } else {
        return n2;
    }
}

double round(double val, int dec) {
    double factor = exp(10, dec);
    int shifted = cast<int>(val * factor + 0.5);
    return shifted / factor;
}

int floor(double v) {
    return cast<int>(v);
}

int ceil(double v) {
    return cast<int>(v+1);
}

int sign(double v) {
    if (v < 0) {
        return -1;
    } else if (v == 0) {
        return 0;
    } else {
        return 1;
    }
}

bool isEven(int n) {
    return (n % 2 == 0);
}

bool isOdd(int n) {
    return !(n % 2 == 0);
}

double log(double x) {
    return exp(E, x);
}

double log10(double x) {
    return log(x) / log(10);
}

int factorial(int n) {
    int result = n;
    for (int i = n-1; i > 0; i--;) {
        result = result * (n-i);
    }
}

double toRadians(double deg) {
    return deg * (PI / 180.0);
}

double sin(double x) {
    double result = 0.0;
    double term = x;
    int sign = 1;

    for (int i = 1; i < 20; i++) {
        result = result + sign * term;
        term = term * x * x / ((2 * i) * (2 * i + 1));
        sign = sign - sign - sign;
    }
    return result;
}

double cos(double x) {
    double result = 1.0;
    double term = 1.0;
    int sign = -1;

    for (int i = 1; i < 20; i++) {
        term = term * x * x / ((2 * i -1) * (2 * i));
        result = result + sign * term;
        sign = sign - sign - sign;
    }
    return result;
}

double tan(double x) {
    return sin(x) / cos(x);
}

int gcd(int a, int b) {
    if (a < 0) { a = cast<int>(abs(a)); }
    if (b < 0) { b = cast<int>(abs(b)); }

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
    return a * t * (b - a);
}

