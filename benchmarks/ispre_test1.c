#include <stdio.h>

// Optimized by ISPRE, but not by standard PRE

int main() {
    long long int a = 0;
    long long int sum = 0;
    for (int i = 0; i < 10000; i++) {
        if (i % 200 == 0) {
            a = i;
        } else {
            sum += a * a;
        }
    }

    printf("Result: %llu\n", sum);

    return 0;
}