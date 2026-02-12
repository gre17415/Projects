#pragma once

#include <cstdint>
#include <mutex>
#include <set>
#include <atomic>

/*
 * Класс PrimeNumbersSet -- множество простых чисел в каком-то диапазоне
 */
class PrimeNumbersSet {
public:
    PrimeNumbersSet();

    bool IsPrime(uint64_t number) const;

    uint64_t GetNextPrime(uint64_t number) const;

    void AddPrimesInRange(uint64_t from, uint64_t to);

    size_t GetPrimesCountInRange(uint64_t from, uint64_t to) const;

    uint64_t GetMaxPrimeNumber() const;

    std::chrono::nanoseconds GetTotalTimeWaitingForMutex() const;

    std::chrono::nanoseconds GetTotalTimeUnderMutex() const;
private:
    std::set<uint64_t> primes_;
    mutable std::mutex set_mutex_;
    std::atomic<uint64_t> nanoseconds_under_mutex_, nanoseconds_waiting_mutex_;
};
