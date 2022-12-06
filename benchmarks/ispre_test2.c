#include <stdio.h>

// Optimized by ISPRE, but not by standard PRE

int main() {
    long long int a = 0;
    long long int sum = 0;
    for (int i = 0; i < 1000; i++) {
        if (i < 90) {
            a = a + 2;
        } else {
            sum += a * 3;
        }
    }

    printf("Result: %llu\n", sum);

    return 0;
}