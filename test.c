#include "mem_alloc.h"
#include <stdio.h>

#define SIZE 500

struct small {
    unsigned long test;
    unsigned long test2;
};

struct large {
    unsigned long values[10];
};

int main()
{
    if (init(SIZE))
    {
        int i = 0;
        struct small *small_array[SIZE / 2];
        struct large *large_array[SIZE / 2];
        for (i; i < SIZE / 2; i++)
        {

            // printf("sending small req %d: \n", i);
            small_array[i] = mem_allocate(sizeof(struct small));
            if(small_array[i] != NULL)
            {
                small_array[i]->test = i;
                small_array[i]->test2 = 2 * i;
            }
            // printf("-----------\n");
        }
        i = 0;
        for (i; i < SIZE / 2; i++)
        {
            if (i == 126)
            {
                int f = 123;
            }
            // printf("small free %d\n", i);
            mem_free(small_array[i]);
            // printf("----=\n");
        }
        i = 0;
        for (i; i < SIZE / 2; i++)
        {
            // printf("large alloc req #%d\n", i);
            large_array[i] = mem_allocate(sizeof(struct large));
            if (large_array[i] != NULL)
            {
                for(int j = 0; j < 10; j++)
                {
                    large_array[i]->values[j] = i * j;
                }
            }
        }
        i = 0;
        for (i; i < SIZE / 2; i++)
        {
            mem_free(large_array[i]);
        }
    }
    else
    {
        perror("failed\n");
        return -1;
    }
    return 0;
}
