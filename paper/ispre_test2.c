#include <stdio.h>

// Optimized by ISPRE, but not
// by standard PRE

int main() {
    long long int a = 0;
    long long int sum = 0;
    for (long long int i = 0;
         i < 100000000; i++) {
        if (i < 90) {
            a = a + 2;
        } else {
            sum += a * 3;
        }
    }

    printf("Result: %llu\n",
           sum);

    return 0;
}