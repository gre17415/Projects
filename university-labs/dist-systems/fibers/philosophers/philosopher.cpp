#include "philosopher.h"

void Philosopher::AcquireForks() {
    Fork* first = &LeftFork_;
    Fork* second = &RightFork_;
    
    if (first > second) {
        std::swap(first, second);
    }
    
    first->mtx.lock();
    second->mtx.lock();
}

void Philosopher::ReleaseForks() {
    LeftFork_.mtx.unlock();
    RightFork_.mtx.unlock();
}