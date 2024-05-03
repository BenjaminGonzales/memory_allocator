// #include "mempage.h"
/*
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "mempage.h"

struct free_header {
    int size;
    void *next;
};

typedef struct memory_page_t {
    struct free_header header;
    void *memory;
} memory_list;

struct alloc_header {
    int size;
    int magic;
};

typedef struct allocated_memory {
    struct alloc_header header;
    void* memory;
} alloc_t;

memory_list *head;
void* lower_bound;
void* upper_bound;



int init(int heap_size)
{
    if(head == NULL)
    {
        // 4096 on wsl
        int page_size = getpagesize();
        // round up heap_size to a multiple of page_size
        int real_size = heap_size + page_size - (heap_size % page_size);

        // void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
        //int file_descriptor = open("/dev/zero", O_RDWR);
        head = mmap(NULL, real_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        lower_bound = head;
        upper_bound = (void*) head + real_size;

        if (head == MAP_FAILED)
        {
            perror("mmap");
            return -1;
        }

        head->header.size = real_size - sizeof(memory_list);
        head->header.next = NULL;
        head->memory = (void*) head + sizeof(struct free_header);

        return 1;
    }
    else
    {
        return 0;
    }
}

void *mem_allocate(int size)
{
    if(head == NULL)
        return NULL;

    void* memory_to_give = NULL;
    memory_list* runner = head;
    // true size is requested size + size of header
    int true_size = size + sizeof(alloc_t);

    while(runner != NULL)
    {
        if (runner->header.size >= true_size)
        {
            alloc_t *alloc = (void*) runner;
            // rearrange free list
            runner = (void*) runner + true_size;
            runner->header.size -= true_size;
            runner->memory = (void*) runner + sizeof(struct allocated_memory);

            // allocate available memory from top of available free section
            alloc->header.size = true_size;
            alloc->header.magic = 12345;
            alloc->memory = (void*) alloc + sizeof(struct alloc_header);
            memory_to_give = alloc->memory;

            // set runner to NULL to break loop
            runner = NULL;
        }
        if (runner != NULL)
            runner = runner->header.next;
    }

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
    if (init(1))
    {
        printf("head pointer: %p\n", head);
        printf("head->size =  %p\n", &head->header.size);
        printf("head->next =  %p\n", &head->header.next);
        printf("head->mem:    %p\n", head->memory);
        printf("sizeof fh:    %lu\n", sizeof(struct free_header));
        printf("sizeof fl:    %lu\n\n", sizeof(memory_list));

        // other stuff
        void* test = mem_allocate(5);
        printf("loc of test: %p\n", test);
    }
    else
    {
        printf("failed\n");

    }
    
}
