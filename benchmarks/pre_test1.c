#include <stdio.h>

/* This test case shows how traditional PRE will search for 
an optimal solution and make (x + y) fully redundant using a phi node. 
However, ISPRE will not optimize this case because x is redefined
in the else branch. */

int main() {
    int a = 0;
    int b = 0;
    int x = 0;
    int y = 1;
    int condition = 6;
    if (condition == 5) {
        y = 2;
    } else {
        x = 2;
        a = x + y;
    }
    b = x + y;

    printf("%d\n", b);

    return 0;
}