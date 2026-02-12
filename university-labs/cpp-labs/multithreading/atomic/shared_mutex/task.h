#pragma once
#include <thread>
#include <atomic>

class Mutex {
public:
    void Lock() 
    {
        waiters.fetch_add(1);
        while (locked.exchange(true))
        {
            locked.wait(1);
        }
    }
    void Unlock() 
    {
        locked.store(false);
        if (waiters.fetch_sub(1) > 1)
        {
            locked.notify_one();
        }
    }

private:
    std::atomic<int> waiters{0};
    std::atomic<bool> locked{0};
};

class SharedMutex {
public:
    SharedMutex() = default;
    ~SharedMutex() = default;

    void lock() 
    {
        mtx.Lock();
        while (unique + shared) 
        {
            mtx.Unlock();
            std::this_thread::yield();
            mtx.Lock();
        }
        unique++;
        mtx.Unlock();
    }
    void unlock() 
    {
        mtx.Lock();
        if (unique)
            unique--;
        mtx.Unlock();
    }
    void lock_shared() 
    {
        mtx.Lock();
        while (unique) 
        {
            mtx.Unlock();
            std::this_thread::yield();
            mtx.Lock();
        }
        shared++;
        mtx.Unlock();
    }
    void unlock_shared() 
    {
        mtx.Lock();
        if (shared)
            shared--;
        mtx.Unlock();
    }
private:
    Mutex mtx;
    int unique = 0;
    int shared = 0;
    
};
