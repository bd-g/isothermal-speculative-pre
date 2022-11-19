#include <stdio.h>

/* if and else branch is taken 90% and 80% of time based on profiled date. 
So PRE must hoist z=x+y and assign to a reference. Output is :74
3
4
5
6
7
8
9
171
172
Better to look at CFG to test which instruction has been hoisted*/

int main(){
	int A[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	int B[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	int i, j;
	int x,y,z;
	int k = 37;
	j = 0;
	for(i = 0; i < 10; i++) {
  		B[i] = k * 2 + A[j] * 23 + i;
  		if (i % 7 == 0) {
			z = x + y;
  			if (i % 2 == 1) {
				j = i;
			} else {
				z = x + y;
  				k = i + 1;
            }
  		} 
		printf("%d\n", B[i]);
	}
	return 0;
}
