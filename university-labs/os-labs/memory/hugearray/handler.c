#include <stdlib.h>
#include <sys/mman.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

extern size_t PAGE_SIZE;
extern double* SQRTS;
extern void CalculateSqrts(double* sqrt_pos, int start, int n);

#define MAX_SQRTS_COUNT (1 << 27)
#define MEM_LIMIT (1 << 25)
#define MAX_PAGES (MEM_LIMIT / 4096)

static void* page_pool[MAX_PAGES];
static int pool_index = 0;

static void shuffle_array(void** arr, size_t n)
{
    srand(time(NULL));
    for (int i = n - 1; i > 0; i--)
    {
        int j = rand() % (i + 1);
        void* tmp = arr[i];
        arr[i] = arr[j];
        arr[j] = tmp;
    }
}

static void evict_random_page()
{
    if (pool_index == 0) return;
    shuffle_array(page_pool, pool_index);
    void* victim = page_pool[0];
    if (munmap(victim, PAGE_SIZE) == -1)
    {
        perror("munmap failed");
        _exit(EXIT_FAILURE);
    }
    memmove(page_pool, page_pool + 1, (pool_index - 1) * sizeof(void*));
    pool_index--;
}

void HandleSigsegv(int sig, siginfo_t* siginfo, void* ctx)
{
    if (sig != SIGSEGV)
    {
        _exit(EXIT_FAILURE);
    }
    void* fault_addr = siginfo->si_addr;
    double* array_end = SQRTS + MAX_SQRTS_COUNT;
    if ((double*)fault_addr < SQRTS || (double*)fault_addr >= array_end)
    {
        _exit(EXIT_FAILURE);
    }

    uintptr_t page_start = (uintptr_t)fault_addr & ~(PAGE_SIZE - 1);
    void* target_page = (void*)page_start;
    for (int i = 0; i < pool_index; i++)
    {
        if (page_pool[i] == target_page)
        {
            mprotect(target_page, PAGE_SIZE, PROT_READ | PROT_WRITE);
            return;
        }
    }

    void* new_page = mmap(target_page, PAGE_SIZE, PROT_READ | PROT_WRITE,
                          MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (new_page == MAP_FAILED)
    {
        if (errno != ENOMEM)
        {
            _exit(EXIT_FAILURE);
        }

        evict_random_page();
        new_page = mmap(target_page, PAGE_SIZE, PROT_READ | PROT_WRITE,
                        MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

        if (new_page == MAP_FAILED) {
            _exit(EXIT_FAILURE);
        }
    }

    int start_idx = (page_start - (uintptr_t)SQRTS) / sizeof(double);
    CalculateSqrts((double*)new_page, start_idx, PAGE_SIZE / sizeof(double));
    if (pool_index >= MAX_PAGES)
    {
        evict_random_page();
    }
    page_pool[pool_index++] = new_page;
}