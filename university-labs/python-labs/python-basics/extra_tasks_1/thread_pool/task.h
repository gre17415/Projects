#include <thread>
#include <mutex>
#include <vector>
#include <queue>
#include <functional>
#include <condition_variable>
#include <atomic>
#include <stdexcept>
#include <cassert>
#include <iostream>
#include <chrono>
#include <iostream>
/*
 * Требуется написать класс ThreadPool, реализующий пул потоков, которые выполняют задачи из общей очереди.
 * С помощью метода PushTask можно положить новую задачу в очередь
 * С помощью метода Terminate можно завершить работу пула потоков.
 * Если в метод Terminate передать флаг wait = true,
 *  то пул подождет, пока потоки разберут все оставшиеся задачи в очереди, и только после этого завершит работу потоков.
 * Если передать wait = false, то все невыполненные на момент вызова Terminate задачи, которые остались в очереди,
 *  никогда не будут выполнены.
 * После вызова Terminate в поток нельзя добавить новые задачи.
 * Метод IsActive позволяет узнать, работает ли пул потоков. Т.е. можно ли подать ему на выполнение новые задачи.
 * Метод GetQueueSize позволяет узнать, сколько задач на данный момент ожидают своей очереди на выполнение.
 * При создании нового объекта ThreadPool в аргументах конструктора указывается количество потоков в пуле. Эти потоки
 *  сразу создаются конструктором.
 * Задачей может являться любой callable-объект, обернутый в std::function<void()>.
 */

class ThreadPool {
public:
    ThreadPool(size_t threadCount) : stop(false) 
    {
        for (size_t i = 0; i < threadCount; ++i) {


            threads.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(queueMutex);
                        condition.wait(lock, [this] { return stop || !tasks.empty(); });
                        if(stop)
                        {
                            if((need && tasks.empty()) || !need)
                            {
                                return;
                            }
                        }
                        task = std::move(tasks.front());
                        tasks.pop();
                    }   
                    task();
                }
            });
            
        }
    }

    void PushTask(const std::function<void()>& task) {
        if(stop)
            throw std::invalid_argument("logic_error");

        {
            std::unique_lock<std::mutex> lock(queueMutex);
            tasks.push(task);
        }
        condition.notify_one();
    }

    void Terminate(bool wait) {
        need = wait;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            stop = true;
        }
        condition.notify_all();
        
        for (std::thread & thread : threads) 
        {
            thread.join();
        }
        
    }

    bool IsActive() const {
        return !stop;
    }

    size_t QueueSize() const {
        std::unique_lock<std::mutex> lock(queueMutex);
        return tasks.size();
    }

    ~ThreadPool() {
        if(!stop)
            Terminate(true);
    }
private:
    std::vector<std::thread> threads;
    std::queue<std::function<void()>> tasks;
    mutable std::mutex queueMutex;
    std::condition_variable condition;
    bool stop;
    bool need;
};






// // Код, помогающий в отладке

// /*
//  * Складывает числа на полуинтервале [from, to)
//  */
// uint64_t SumNumbers(uint64_t from, uint64_t to) {
//     uint64_t sum = 0;
//     for (uint64_t number = from; number < to; ++number) {
//         sum += number;
//     }
//     return sum;
// }

// int main() {
//     using namespace std::literals::chrono_literals;

//     {
//         ThreadPool pool(10);

//         std::mutex mutex;
//         uint64_t sum = 0;

//         constexpr int step = 1000;
//         constexpr uint64_t maxNumber = 500000000;
//         for (uint64_t l = 0, r = l + step; l <= maxNumber; l = r, r = l + step) {
//             if (r > maxNumber + 1) {
//                 r = maxNumber + 1;
//             }
//             pool.PushTask([&sum, &mutex, l, r]() {
//                 std::this_thread::sleep_for(25us);
//                 const uint64_t subsum = SumNumbers(l, r);
//                 std::lock_guard<std::mutex> lockGuard(mutex);
//                 sum += subsum;
//             });
//         }

//         std::cout << "QueueSize before terminate is " << pool.QueueSize() << std::endl;
//         assert(pool.QueueSize() > 100000);
//         pool.Terminate(true);
//         std::cout << "Terminated. Queue size is " << pool.QueueSize() << ". IsActive: " << pool.IsActive() << std::endl;

//         const uint64_t expectedSum = SumNumbers(1, maxNumber + 1);
//         assert(expectedSum == sum);

//         try {
//             pool.PushTask([](){
//                 std::cout << "I am a new task!" << std::endl;
//             });
//             assert(false);
//         } catch (const std::exception& e) {
//             std::cout << "Cannot push tasks after termination" << std::endl;
//         }
//     }

//     {
//         // std::cout << "A" << std::endl;

//         ThreadPool pool(10);
//         // std::cout << "B" << std::endl;

//         std::mutex mutex;
//                 // std::cout << "CA" << std::endl;

//         uint64_t sum = 0;

//         constexpr int step = 1000;
//         constexpr uint64_t maxNumber = 500000000;
//         for (uint64_t l = 0, r = l + step; l <= maxNumber; l = r, r = l + step) {
//             if (r > maxNumber + 1) {
//                 r = maxNumber + 1;
//             }
//             pool.PushTask([&sum, &mutex, l, r]() {
//                 std::this_thread::sleep_for(25us);
//                 const uint64_t subsum = SumNumbers(l, r);
//                 std::lock_guard<std::mutex> lockGuard(mutex);
//                 sum += subsum;
//             });
//         }
//         // std::cout << "BBB" << std::endl;

//         std::cout << "QueueSize before terminate is " << pool.QueueSize() << std::endl;
//         assert(pool.QueueSize() > 100000);
//             // std::cout << "QWE1" << std::endl;

//         pool.Terminate(false);
//             // std::cout << "QWE2" << std::endl;

//         std::cout << "Terminated. Queue size is " << pool.QueueSize() << ". IsActive: " << pool.IsActive() << std::endl;

//         const uint64_t expectedSum = SumNumbers(1, maxNumber + 1);
//         assert(expectedSum > sum);

//         try {
//             pool.PushTask([](){
//                 std::cout << "I am a new task!" << std::endl;
//             });
//             assert(false);
//         } catch (const std::exception& e) {
//             std::cout << "Cannot push tasks after termination" << std::endl;

//         }
//             // std::cout << "QqqWE" << std::endl;

//     }
//     // std::cout << "QWE" << std::endl;
//     return 0;
// }
