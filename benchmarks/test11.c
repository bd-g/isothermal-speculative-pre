// FROM ISPRE PAPER Fig. 2

#include <stdio.h>

int main()
{
    int arr[1000] = {0};
    int a = 0, sum = 0, b = 0, c = 0, sum2 = 0, sum3 = 0;
    for (int i = 0; i < 1000; i++)
    {
        if (i < 90)
        {
            a = a + 1;
            b = b + 2;
            c = c + 3;
        }
        else
        {
            sum += a * 3;
            sum2 += b * 2;
            sum3 += c * 4;
        }
    }

    return 0;
}