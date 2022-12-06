#include <stdio.h>

// Optimized by ISPRE, but not by standard PRE

int main() {
    int count = 1000;
    long long int sum = 0;
    int value = 10;
    while (count > 0) {
        if (count > 980) {
            value += 1;
        } else {
            sum += value * 2;
        }
        count--;
    }

    printf("Result: %llu\n", sum);

    return 0;
}