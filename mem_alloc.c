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
#include "mem_alloc.h"

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
            head_pointer = (memory_list**) temp->next;
    }
    /*
     * print statements to show request information.
    printf("request to alloc %ld bytes\n", true_size);
    printf("head has %ld space\n", head->size);
    if (head->next != NULL)
    {
        printf("       head->next had %ld size\n", head->next->size);
    }
    */
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
            while (next_location == (void *) runner + sizeof(memory_list) + runner->size) {
                runner->size += sizeof(memory_list) + next_location->size;
                runner->next = runner->next->next;
                next_location = runner->next;
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
            // TESTING LIST
            /*
            printf("free space:\n");
            memory_list *tester = head;
            while(tester != NULL)
            {
                printf("      block of %ld space @ addr %p\n", tester->size, tester);
                printf("lower: %p | upper: %p\n", lower_bound, upper_bound);
                tester = tester->next;
            }
             */
            // END TESTING LIST
        }
        else
            perror("pointer in memory space, but not allocated.\n");
    }
}

