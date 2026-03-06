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
    int temp = 10 * dec;

    return (val * temp) / temp;
}