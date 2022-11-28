#include <stdio.h>

/* 
A Variant of test 4a.
SPRE need to add a b+c expression before the first if-else diamond block(Same as test 4)
As we have update the value of b, SPRE need to add another b+c expression before the second if-else block(Different from test 4)
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
    b = b + 1;
    if (e > 3) {
        e = e - 1;
    } else {
        d = b + c;
    }
    
    printf("%d\n", d);

    return 0;
}