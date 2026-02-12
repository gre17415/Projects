#pragma once

#include <stdint.h>

struct SeqLock {
    volatile int64_t counter;
};

inline void SeqLock_Init(struct SeqLock* lock) {
    lock->counter = 0;
}

inline int64_t SeqLock_ReadLock(struct SeqLock* lock) {
    int64_t val;
    __asm__ __volatile__ (
            "movq %1, %0\n\t"
            : "=r" (val)
            : "m" (lock->counter)
            : "memory"
            );
    return val;
}

inline int SeqLock_ReadUnlock(struct SeqLock* lock, int64_t value) {
    int64_t current;
    __asm__ __volatile__ (
            "movq %1, %0\n\t"
            : "=r" (current)
            : "m" (lock->counter)
            : "memory"
            );
    return (current == value) && ((value & 1) == 0);
}

inline void SeqLock_WriteLock(struct SeqLock* lock) {
    while (1) {
        int64_t expected = lock->counter;
        if (expected & 1) continue;

        int64_t desired = expected + 1;
        int64_t prev;
        __asm__ __volatile__ (
                "lock cmpxchgq %2, %1\n\t"
                : "=a" (prev), "+m" (lock->counter)
                : "r" (desired), "a" (expected)
                : "memory"
                );

        if (prev == expected) break;
    }
}

inline void SeqLock_WriteUnlock(struct SeqLock* lock) {
    __asm__ __volatile__ (
            "lock addq $1, %0"
            : "+m" (lock->counter)
            :
            : "memory"
            );
}

static inline void AtomicAdd(int64_t* ptr, int64_t value) {
    __asm__ __volatile__ (
            "lock addq %1, %0"
            : "+m" (*ptr)
            : "r" (value)
            : "memory"
            );
}