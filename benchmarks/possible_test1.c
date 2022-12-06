#include <stdio.h>

/* Test case that can be used to test LLVM PRE. To check for divide by 0 exceptions. 
Even after PRE, exceptions must be thrown from same spot in code. 
While testing, change value of b to -2 and check => there should be no exception from else part 
but an exception from d=b/c only. Current output is an exception. */

int main() {
    int a = 1;
    int b = 14;
    int c = 0;
    int d = 0;
    if (b > c) {
        printf("next line throws exception in if");
        a = b / c;
    } else {
        a = 0;
    }
    d = b / c;
    
    printf("%d\n", d);

    return 0;
}