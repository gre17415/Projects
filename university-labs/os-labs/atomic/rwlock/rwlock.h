#pragma once

#include <stdatomic.h>
#include <stdint.h>

static inline void AtomicAdd(int64_t* ptr, int64_t value)
{
    __atomic_add_fetch(ptr, value, __ATOMIC_SEQ_CST);
}

struct RwLock {
    int reader_count;
    atomic_flag readers_lock;
    atomic_flag writer_lock;
    atomic_flag help_lock;
};

void RwLock_Init(struct RwLock* lock) {
    lock->reader_count = 0;
    atomic_flag_clear(&lock->readers_lock);
    atomic_flag_clear(&lock->writer_lock);
    atomic_flag_clear(&lock->help_lock);
}

inline void RwLock_ReadLock(struct RwLock* lock) {
    while (atomic_flag_test_and_set(&lock->help_lock)) {}

    while (atomic_flag_test_and_set(&lock->readers_lock)) {}
    lock->reader_count++;
    if (lock->reader_count == 1)
    {
        while (atomic_flag_test_and_set(&lock->writer_lock)) {}
    }
    atomic_flag_clear(&lock->readers_lock);
    atomic_flag_clear(&lock->help_lock);
}

inline void RwLock_ReadUnlock(struct RwLock* lock) {
    while (atomic_flag_test_and_set(&lock->readers_lock)) {}
    lock->reader_count--;
    if (lock->reader_count == 0)
    {
        atomic_flag_clear(&lock->writer_lock);
    }
    atomic_flag_clear(&lock->readers_lock);
}

inline void RwLock_WriteLock(struct RwLock* lock) {
    while (atomic_flag_test_and_set(&lock->help_lock)) {}
    while (atomic_flag_test_and_set(&lock->writer_lock)) {}
}

inline void RwLock_WriteUnlock(struct RwLock* lock) {
    atomic_flag_clear(&lock->writer_lock);
    atomic_flag_clear(&lock->help_lock);
}