// FROM ISPRE PAPER Fig. 2

#include <stdio.h>

int main()
{
    int arr[1000] = {0};
    int a = 0, sum = 0;
    for (int i = 0; i < 1000; i++)
    {
        if (i < 90)
        {
            a = a + 2;
        }
        else
        {
            sum += a * 3;
        }
    }

    return 0;
}