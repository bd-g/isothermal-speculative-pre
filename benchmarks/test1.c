#include <stdio.h>

/* Basic test case from Effective PRE paper that can be used to test LLVM PRE. No loops, just a
branch. The else branch is always taken so x+y is calculated twice when it doesnt change at all.
Output after PRE in CFG should be : x+y removed from outside the branches and into the if branch*/

int main() {
    int a = 0;
    int b = 0;
    int x = 0;
    int y = 1;
    int e = 700;
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