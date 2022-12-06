#include <stdio.h>

/* This is a classic PRE example that both PRE and ISPRE will optimize. */

int main() {
    int a = 1;
    int b = 14;
    int c = 0;
    int d = 0;
    int e = 2;
    if (e == 2) {
        a = b + c;
    } else {
        e = 3;
    }
    e = e + 1;  // e = 3
    if (e > 3) {
        e = e - 1;
    } else {
        d = b + c;
    }
    
    printf("%d\n", d);

    return 0;
}