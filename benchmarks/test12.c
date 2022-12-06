// FROM ISPRE PAPER Fig. 2

#include <stdio.h>

int main()
{
    int count = 1000;
    int sum = 0;
    int value = 10;
    while (count > 0)
    {
        if (count > 980)
        {
            value += 1;
        }
        else
        {
            sum += value * 2;
        }
    }
    return 0;
}