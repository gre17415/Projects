#pragma once

struct SpinLock {
    int locked;
};

inline void SpinLock_Init(struct SpinLock* lock) {
    lock->locked = 0;
}

inline void SpinLock_Lock(struct SpinLock* lock) {
    while (__sync_lock_test_and_set(&lock->locked, 1))
    {}
    __sync_synchronize();
}

inline void SpinLock_Unlock(struct SpinLock* lock) {
    __sync_synchronize();
    __sync_lock_release(&lock->locked);
}