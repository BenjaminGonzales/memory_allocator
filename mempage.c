// #include "mempage.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "mempage.h"

typedef struct memory_page_t {
    int size;
    int magic;
    struct memory_page_t *next;
    //void *memory;
} mem_t;

mem_t *head;
void *heap_space;


int roundup(int num, int cieling)
{
    return num + cieling - (num % cieling);
}

int init(int heap_size)
{
    if(head == NULL)
    {
        // 4096 on wsl
        int page_size = getpagesize();
        // round up heap_size to a multiple of page_size
        heap_size = roundup(heap_size, page_size);

        // void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
        heap_space = mmap(NULL, heap_size, PROT_EXEC, 0, 0, 0);

        if (head == MAP_FAILED)
        {
            perror("mmap");
            return -1;
        }

        return 1;
    }
    else
    {
        return 0;
    }
}

void *mem_allocate(int size)
{
    void* memory_to_give = NULL;

    return memory_to_give;
}

void mem_free (void *pointer_to_free)
{
    if(pointer_to_free != NULL)
    {

    }
}

int main()
{
    int test = 55;
    test = roundup(test, 2);

    printf("roundup: %d\n", test);
}
