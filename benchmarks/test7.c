#include <stdio.h>

/* 
Example from the ISPRE paper(Fig 4)
We will take the if branch 990 times among the total 1000 trials. Therefore, we need to add a+b to the ingress edge from b1 to b2
Details are illustrated in fig 4
*/

int main() {
    int a = 1;
    int b = 0;
    int count = 0;
    int c;
    while (count < 1000) {
        if (count < 990) {
            c = a + b;
        } else {
            b = 2;
        }
        count++;
    }
}

