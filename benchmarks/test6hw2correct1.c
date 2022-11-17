#include <stdio.h>
/* Used to test LLVM PRE in a loop. 90% of time it goes in the if part based on profile data.
 So second z=x+y is executed rarely. First z=x+y is always executed. Look at cfg to test. Output :
 0
1
13
25
37
49
61
73
85
86
*/
int main(){
	int A[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	int B[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	int i, j;
	j = 0;
	int x, y, z;
	for(i = 0; i < 10; i++) {
  		B[i] = A[j] * 11 + i;
		z = x + y;
  		if (i < 8) 
  			j = i;
		z = x + y;
		printf("%d\n", B[i]);
	}
	return 0;
}