#include "fiber.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#define FIBER_STACK_SIZE 512

struct ExecutionState {
    uint64_t rbx;
    uint64_t rbp;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    uint64_t rsp;
    uint64_t rip;
};

struct Fiber {
    bool finished;
    void (*func)(void*);
    void* data;
    struct ExecutionState state;
    int64_t stack[FIBER_STACK_SIZE];
    struct Fiber* next;
};
static struct Fiber mainFiber = {
        .finished = false,
        .func = NULL,
        .data = NULL,
        .next = NULL
};
static struct Fiber* currentFiber = NULL;

extern void StateSave(struct ExecutionState* state);
extern void StateLoad(struct ExecutionState* state);

void AddFiber(struct Fiber* fiber) {
    if (currentFiber == NULL) {
        fiber->next = fiber;
        currentFiber = fiber;
    } else {
        struct Fiber* tail = currentFiber;
        while (tail->next != currentFiber) {
            tail = tail->next;
        }
        fiber->next = currentFiber;
        tail->next = fiber;
    }
}

void InitFiber(struct Fiber* fiber) {
    fiber->next = NULL;
    fiber->finished = false;
    fiber->func = NULL;
    fiber->data = NULL;
    fiber->state.rsp = (uint64_t)(fiber->stack + FIBER_STACK_SIZE);
    uintptr_t stack_top = (uintptr_t)(fiber->stack + FIBER_STACK_SIZE);
    stack_top = stack_top & ~0xF;
    fiber->state.rsp = stack_top;
}

void FiberStart() {
    currentFiber->func(currentFiber->data);
    currentFiber->finished = true;
    FiberYield();
}
void FiberSpawn(void (*func)(void*), void* data) {
    if (currentFiber == NULL) {
        currentFiber = &mainFiber;
        InitFiber(currentFiber);
        currentFiber->next = currentFiber;
    }

    struct Fiber* newFiber = malloc(sizeof(struct Fiber));
    InitFiber(newFiber);
    newFiber->func = func;
    newFiber->data = data;

    uint64_t* stack_top = (uint64_t*)(newFiber->stack + FIBER_STACK_SIZE);
    *--stack_top = (uint64_t)FiberStart;
    newFiber->state.rsp = (uint64_t)stack_top;

    AddFiber(newFiber);
}

void FiberYield() {
    if (currentFiber == NULL) {
        fprintf(stderr, "FiberYield: no active fibers\n");
        return;
    }
    struct Fiber* prev = currentFiber;
    currentFiber = currentFiber->next;

    if (prev->finished) {
        if (prev != &mainFiber) {
            struct Fiber* p = prev;
            int max_checks = 1024;
            while (p->next != prev && max_checks-- > 0)
                p = p->next;
            if (max_checks <= 0 || currentFiber == NULL || p->next != prev) {
                fprintf(stderr,
                        "FiberYield: corrupted list\n"
                        "prev: %p\n"
                        "current: %p\n"
                        "iterations: %d\n",
                        prev, currentFiber, 1024 - max_checks);
                abort();
            }
            p->next = currentFiber;
            free(prev);
        }
    }
    else {
        StateSave(&prev->state);
    }

    if (currentFiber->state.rsp == 0 || currentFiber->state.rsp >= (uint64_t)(currentFiber->stack + FIBER_STACK_SIZE)) {
        fprintf(stderr, "FiberYield: corrupted stack pointer\n");
    }
    if (currentFiber != &mainFiber) {
        uint64_t stack_start = (uint64_t)currentFiber->stack;
        uint64_t stack_end = (uint64_t)(currentFiber->stack + FIBER_STACK_SIZE);

        if (currentFiber->state.rsp < stack_start || currentFiber->state.rsp >= stack_end) {
            fprintf(stderr, "FiberYield: rsp over stack\n");
            abort();
        }
    }
    StateLoad(&currentFiber->state);
}

int FiberTryJoin() {
    if (currentFiber == NULL) return 1;
    return (currentFiber == &mainFiber && currentFiber->next == currentFiber) ? 1 : 0;
}