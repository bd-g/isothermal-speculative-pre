#include <stdio.h>

/* Test case from ISPRE paper that can be used to test LLVM PRE. With loop. Must print 251.
a+b is loop invariant if sum%10 is not 0. After PRE, CFG should be 
t1=a+b;
while(t1>sum)
same if but added t1=a+b. */

int main() {
    int a = 200;
    int b = 46;
    int sum = 205;
    while (a + b > sum) {
        if(sum % 10 == 0) {
            a = a + 1;
        }
        sum = sum + b;
    }
    
    printf("%d\n", sum);

    return 0;
}