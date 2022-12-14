#include <stdio.h>

// Optimized by ISPRE, but not by standard PRE

int main() {
    long long int a = 0, sum = 0, b = 0, c = 0, sum2 = 0, sum3 = 0;
    for (long long int i = 0; i < 10000000; i++) {
        if (i < 90) {
            a = a + 1;
            b = b + 2;
            c = c + 3;
        } else {
            sum += a * 3;
            sum2 += b * 2;
            sum3 += c * 4;
        }
    }

    printf("Result1: %llu\n", sum);
    printf("Result2: %llu\n", sum2);
    printf("Result3: %llu\n", sum3);

    return 0;
}