#include <stdio.h>

// Optimized by ISPRE, but not by standard PRE

int main() {
    long long int a = 0;
    long long int b = 0;
    long long int sum = 0;
    long long int sum2 = 0;
    long double sum3 = 0;
    for (int i = 0; i < 100000000; i++) {
        if (i % 200000 == 0) {
            a = i;
            b = a + 3;
        } else {
            sum += a * a;
            sum2 = sum * b %2;
            sum3 = sum2 / (a -b);
        }
    }

    // printf("Result: %llu\n", sum3);

    return 0;
}