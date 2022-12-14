#include <stdio.h>

// Optimized by ISPRE, but not by standard PRE

int main() {
    long long int count = 100000000;
    long long int sum = 0;
    long double sum2 = 0;
    long double sum3 = 0;
    long double b = 1;
    int value = 10;
    while (count > 0) {
        if (count > 98000000) {
            value += 1;
            b = b * 1.1;
        } else {
            sum += value * 2;
            sum2 += ((sum)*2) / 3;
            sum3 = sum2 / sum * sum2 - sum * sum / sum2;
        }
        count--;
    }

    printf("Result: %llu\n", sum);

    return 0;
}