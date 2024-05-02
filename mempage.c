// #include "mempage.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "mempage.h"

typedef struct memory_page_t {
    int size;
    int magic;
    void *memory;
} mem_t;


mem_t *head;

int init(int heap_size)
{
    if(head == NULL)
    {
        // void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
        head = mmap(NULL, heap_size, PROT_EXEC, 0, 0, 0);
        if (head == MAP_FAILED)
            return -1;
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
