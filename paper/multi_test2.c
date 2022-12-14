#include <stdio.h>

// Optimized by ISPRE, but not
// by standard PRE

int main() {
    long long int a = 0;
    long long int b = 0;
    long long int c = 0;
    long long int d = 0;
    long long int sum = 0;
    long double sum2 = 0;
    long double sum3 = 0;
    long double sum4 = 0;
    for (long long int i = 0;
         i < 100000000; i++) {
        if (i < 90) {
            a = a + 2;
            b = i + a;
            c = a - b;
        } else {
            sum += a * 3;
            sum2 = sum / b;
            sum3 = sum2 * c;
            sum4 =
                (sum3 / sum) *
                b;
        }
    }

    return 0;
}