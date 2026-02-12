#pragma once
#include <stdint.h>

inline void AtomicAdd(int64_t* atomic, int64_t value) {
    asm volatile(
            "lock addq %1, %0"
            : "+m" (*atomic)
            : "r" (value)
            : "cc"
            );
}

inline void AtomicSub(int64_t* atomic, int64_t value) {
    asm volatile(
            "lock subq %1, %0"
            : "+m" (*atomic)
            : "r" (value)
            : "cc"
            );
}

inline int64_t AtomicXchg(int64_t* atomic, int64_t value) {
    int64_t old;
    asm volatile(
            "lock xchgq %1, %0"
            : "=r" (old), "+m" (*atomic)
            : "r" (value)
            );
    return old;
}

inline int64_t AtomicCas(int64_t* atomic, int64_t* expected, int64_t value) {
    int64_t prev = *expected;
    int64_t ret;
    asm volatile(
            "lock cmpxchgq %3, %1\n\t"
            "sete %b0\n\t"
            "movzbl %b0, %k0"
            : "=r" (ret), "+m" (*atomic), "+a" (prev)
            : "r" (value)
            : "cc"
            );
    *expected = prev;
    return ret;
}