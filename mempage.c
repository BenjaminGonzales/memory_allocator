// #include "mempage.h"
/*
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
*/

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include "mempage.h"

#define MAGIC_NUMBER 1234567

typedef struct memory_page_t {
    unsigned long size;
    struct memory_page_t *next;
} memory_list;

typedef struct allocated_memory {
    unsigned long size;
    unsigned long magic;
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
        unsigned long real_size = heap_size + page_size - (heap_size % page_size);

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

        head->size = real_size - sizeof(memory_list);
        head->next = NULL;

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
    memory_list** head_pointer = &head;
    // true size is requested size + size of header
    unsigned long true_size = size + sizeof(alloc_t);

    while(head_pointer != NULL)
    {
        memory_list* temp = *head_pointer;
        if (temp->size >= true_size)
        {
            // store current info before shifting head up and clear data
            void *addr_holder = temp;
            unsigned long size_holder = temp->size;
            temp->size = 0;
            memory_list *next_holder = temp->next;
            temp->next = NULL;

            // move free_list node to new spot and repopulate data
            *head_pointer = (void*) temp + true_size;
            temp = *head_pointer;
            temp->size = size_holder - true_size;
            temp->next = next_holder;

            // allocate available memory from new area between old node spot and new node spot
            alloc_t *alloc = addr_holder;
            alloc->size = size;
            alloc->magic = MAGIC_NUMBER;
            memory_to_give = (void*) alloc + sizeof(alloc_t);

            // set head_pointer to NULL to break loop
            head_pointer = NULL;
        }
        if (head_pointer != NULL)
            head_pointer = (memory_list**) &temp->next;
    }
    return memory_to_give;
}

void coalesce()
{
    memory_list *runner = head;
    while(runner != NULL)
    {
        if(runner->next != NULL)
        {
            memory_list *next_location = runner->next;
            if (next_location == (void *) runner + sizeof(memory_list) + runner->size) {
                runner->size += sizeof(memory_list) + next_location->size;
                runner->next = runner->next->next;
            }
        }
        runner = runner->next;
    }
}

void mem_free (void *pointer_to_free)
{
    if(pointer_to_free != NULL)
    {
        unsigned long* magic_finder = pointer_to_free - sizeof(unsigned long);
        if(pointer_to_free < lower_bound || pointer_to_free > upper_bound || (void*) magic_finder < lower_bound)
        {
            perror("mem_free: invalid pointer\n");
        }
        else if (*magic_finder == MAGIC_NUMBER)
        {
            // we have a valid pointer, so we change it from an allocated header to a free header.
            alloc_t *to_remove = (void*) pointer_to_free - sizeof(alloc_t);
            // this might be unnecessary ?
            unsigned int size = to_remove->size;
            /*
             * my compiler tells me the structs are always the same size, and they are on my machine, but
             * I'm not 100% sure it is on every machine. I'm sure it's fine to not worry about this for now.
            unsigned int difference = 0;
            if(sizeof(alloc_t) > sizeof(memory_list))
                difference = sizeof(alloc_t) - sizeof(memory_list);
            else
                difference = sizeof(memory_list) - sizeof(alloc_t);
            */
            memory_list *new_free = (void*) to_remove;
            new_free->size = size;
            new_free->next = NULL;

            // now add newly made free list to master list at lowest addr location
            if (new_free < head)
            {
                new_free->next = head;
                head = new_free;
            }
            else
            {
                memory_list *runner = head->next;
                memory_list *prev = head;
                while (runner != NULL)
                {
                    if (new_free < runner)
                    {
                        new_free->next = runner;
                        prev->next = new_free;
                        runner = NULL;
                    }
                    else
                    {
                        if(runner->next == NULL)
                            runner->next = new_free;
                        prev = runner;
                        runner = runner->next;
                    }

                } // end while (insert traversal)
            }
            coalesce();
        }
        else
            perror("pointer in memory space, but not allocated.\n");
    }
}

int main()
{
    if (init(1))
    {
        printf("head pointer: %p\n", head);
        printf("head->size =  %ld\n", head->size);
        printf("head->next =  %p\n", head->next);
        printf("mem:          %p\n", (void*) head + sizeof(memory_list));
        printf("sizeof fl:    %lu\n\n", sizeof(alloc_t));

        // other stuff
        char* test = mem_allocate(16);
        printf("loc of test: %p\n", test);
        printf("loc of head: %p\n", head);

        char* test2 = mem_allocate(16);
        printf("loc of test2: %p\n", test2);
        printf("loc of head: %p\n", head);

        strcpy(test, "asdfqwer");
        strcpy(test2, "123456789");
        printf("test1: %s, test2: %s\n", test, test2);

        printf("\n");
        mem_free(test2);
        printf("loc of head: %p\n", head);
        printf("     head->size = %ld\n", head->size);
        if (head->next != NULL)
            printf("     next->size = %ld\n", head->next->size);
        printf("loc of test: %p\n", test);
        printf("loc of test2: %p\n", test2);

    }
    else
    {
        printf("failed\n");

    }
    
}
