#include "malloc.h"
#include "syscall.h"

#define PAGE_SIZE 4096

void* malloc(size_t size)
{
    if (size == 0)
    {
        return NULL;
    }
    size_t required = sizeof(size_t) + size;
    size_t allocated_size = (required + PAGE_SIZE - 1) / PAGE_SIZE * PAGE_SIZE;
    void* addr = mmap(NULL, allocated_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (addr == MAP_FAILED)
    {
        return NULL;
    }
    *(size_t*)addr = allocated_size;
    return (void*)((size_t*) addr + 1);
}

void free(void* ptr)
{
    if (ptr == NULL)
    {
        return;
    }
    size_t* header = (size_t*) ptr - 1;
    size_t allocated_size = *header;
    munmap(header, allocated_size);
}
void* realloc(void* ptr, size_t size)
{
    if (ptr == NULL)
    {
        return malloc(size);
    }
    if (size == 0)
    {
        free(ptr);
        return NULL;
    }
    size_t* header = (size_t*)ptr - 1;
    size_t old_allocated_size = *header;
    size_t required = sizeof(size_t) + size;
    size_t new_allocated_size = (required + PAGE_SIZE - 1) / PAGE_SIZE * PAGE_SIZE;

    if (new_allocated_size <= old_allocated_size)
    {
        return ptr;
    }

    void* new_addr = mremap(
            header,
            old_allocated_size,
            new_allocated_size,
            MREMAP_MAYMOVE,
            NULL
    );

    if (new_addr == MAP_FAILED)
    {
        return NULL;
    }
    *(size_t*) new_addr = new_allocated_size;
    return (size_t*)new_addr + 1;
}