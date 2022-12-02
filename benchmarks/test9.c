// FROM ISPRE PAPER Fig. 2

#include <stdio.h>

int main() {
    int array[10] = {-1, 2, 4, 1, 1, 1, 7, 1, 1, 1};
    int a = 0, sum = 0;
    for (int i = 0; i < 10; i++) {
        if (array[i] < 0) {
            a = a + 1;
        } else {
            sum += a * a;
        }
    }

    return 0;
}