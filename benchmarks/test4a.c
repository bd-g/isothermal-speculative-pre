#include <stdio.h>

/* Test case that can be used to test LLVM PRE. 
Double diamond with code duplication on only left and right branch of first and second diamond.
May need code duplication for most optimum code (not sure how much LLVM PRE does).
If we take else and else path, then PRE must have no effect (set e=3 to test this path, output must be 0). 
PRE must affect only if and else path (output must be 14).
*/

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