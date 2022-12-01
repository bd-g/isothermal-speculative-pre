#include <stdio.h>

/* Basic test case from Effective PRE paper that can be used to test LLVM PRE. No loops, just a
branch. The else branch is always taken so x+y is calculated twice when it doesnt change at all.
Output after PRE in CFG should be : x+y removed from outside the branches and into the if branch*/

int main() {
    int a=4;
    int c=5;
    int m=9;
    int b=2;
    if(m==9)
    {  
        c=a+b;
    }
    
    return 0;
}