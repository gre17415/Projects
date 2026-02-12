#include "task.h"
#include <vector>
#include <thread>
inline bool IsBePrime(uint64_t x)
{
    if(x < 2)   return false;
    if(x == 2)  return true;
    for(uint64_t i = 2; i * i <= x; i++) if(x % i == 0)  return false;
    return true;
}

using namespace std::chrono_literals;

PrimeNumbersSet::PrimeNumbersSet()
{
    nanoseconds_under_mutex_ = 0;
    nanoseconds_waiting_mutex_= 0;
}
bool PrimeNumbersSet::IsPrime(uint64_t number) const
{   
    std::lock_guard<std::mutex> lg (set_mutex_);
    return primes_.find(number) != primes_.end();
}
uint64_t PrimeNumbersSet::GetNextPrime(uint64_t number) const
{
    
    std::lock_guard<std::mutex> lg (set_mutex_);

        // const std::chrono::time_point time2 = std::chrono::high_resolution_clock::now();                
        // nanoseconds_waiting_mutex_ += std::chrono::duration_cast<std::chrono::nanoseconds>(time2 - time1).count();

    auto it = primes_.lower_bound(number);
    if(it == primes_.end())
        throw std::invalid_argument("Don't know next prime after limit\n");
    it++;
    if(it == primes_.end())
        throw std::invalid_argument("Don't know next prime after limit\n");
    // set_mutex_.unlock();

    return *it;
        // const std::chrono::time_point time3 = std::chrono::high_resolution_clock::now();                
        // nanoseconds_under_mutex_ += std::chrono::duration_cast<std::chrono::nanoseconds>(time3 - time2).count();
}
void PrimeNumbersSet::AddPrimesInRange(uint64_t from, uint64_t to)
{
    std::vector<uint64_t> foradd;
    for(uint64_t x = from; x < to; x++)
    {
        if(IsBePrime(x))
        {
            foradd.push_back(x);
            // const std::chrono::time_point time1 = std::chrono::high_resolution_clock::now();                            
            // set_mutex_.lock();
            // const std::chrono::time_point time2 = std::chrono::high_resolution_clock::now();                
            // nanoseconds_waiting_mutex_ += std::chrono::duration_cast<std::chrono::nanoseconds>(time2 - time1).count();
            // for(const auto & x: foradd)
            //     primes_.insert(x);
            // const std::chrono::time_point time3 = std::chrono::high_resolution_clock::now();                
            // nanoseconds_under_mutex_ += std::chrono::duration_cast<std::chrono::nanoseconds>(time3 - time2).count();
            // set_mutex_.unlock();
        }
    }
    if(foradd.size() != 0)
    {
        const std::chrono::time_point time1 = std::chrono::high_resolution_clock::now();                            
        // set_mutex_.lock();
        std::this_thread::sleep_for(500ms);
        std::lock_guard<std::mutex> lg (set_mutex_);
        const std::chrono::time_point time2 = std::chrono::high_resolution_clock::now();                
        nanoseconds_waiting_mutex_ += std::chrono::duration_cast<std::chrono::nanoseconds>(time2 - time1).count();
        for(size_t i = 0; i < foradd.size(); i++)
            primes_.insert(foradd[i]);
        const std::chrono::time_point time3 = std::chrono::high_resolution_clock::now();                
        nanoseconds_under_mutex_ += std::chrono::duration_cast<std::chrono::nanoseconds>(time3 - time2).count();
        // set_mutex_.unlock();
    }
}

size_t PrimeNumbersSet::GetPrimesCountInRange(uint64_t from, uint64_t to) const
{
    std::lock_guard<std::mutex> lg (set_mutex_);
    // set_mutex_.lock();
    size_t cnt = 0;
    for(auto it = primes_.lower_bound(from); it != primes_.upper_bound(to); it++)
    {
        cnt++;
    }
    // set_mutex_.unlock();
    return cnt;


}

uint64_t PrimeNumbersSet::GetMaxPrimeNumber() const
{
    std::lock_guard<std::mutex> lg(set_mutex_);
    if(primes_.empty())
    return 0;
    return *--primes_.end();
}

std::chrono::nanoseconds PrimeNumbersSet::GetTotalTimeWaitingForMutex() const
{
    return std::chrono::nanoseconds(nanoseconds_waiting_mutex_);
}

std::chrono::nanoseconds PrimeNumbersSet::GetTotalTimeUnderMutex() const
{
    return std::chrono::nanoseconds(nanoseconds_under_mutex_);
}
