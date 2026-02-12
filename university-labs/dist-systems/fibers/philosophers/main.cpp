#include "philosopher.h"

#include <userver/engine/async.hpp>
#include <userver/engine/run_standalone.hpp>
#include <userver/engine/sleep.hpp>

#include <userver/engine/task/task_base.hpp>
#include <userver/engine/task/task_with_result.hpp>

#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <thread>

using namespace std::chrono_literals;

void Dining(int seats, std::chrono::seconds sleepTime) {
    Table table(seats);
    std::vector<Philosopher> philosophers;

    for (int seat = 0; seat < seats; ++seat) {
        philosophers.emplace_back(table, seat);
    }

    std::atomic<bool> stopToken = false;
    std::atomic<bool> exit = false;

    std::thread background([&, sleepTime] {
        std::this_thread::sleep_for(sleepTime);
        stopToken.store(true);
        std::this_thread::sleep_for(200ms);
        ASSERT_TRUE(exit.load());
    });

    std::vector<engine::TaskWithResult<void>> futures;
    for (int seat = 0; seat < static_cast<int>(philosophers.size()); ++seat) {
        futures.push_back(engine::AsyncNoSpan([&, seat] {
            auto& plato = philosophers[seat];
            do {
                plato.EatAndThink();
            } while (!stopToken.load());
            exit.store(true);
        }));
    }

    for (const auto& future : futures) {
        future.Wait();
    }

    std::vector<int> meals;
    for (const auto& ph : philosophers) {
        meals.push_back(ph.Meals());
    }

    ASSERT_GT(*std::min_element(meals.begin(), meals.end()), 0);

    background.join();
}

UTEST_MT(Philosophers, Dining2_1, 1) {
    Dining(2, 3s);
}

UTEST_MT(Philosophers, Dining2_2, 2) {
    Dining(2, 3s);
}

UTEST_MT(Philosophers, Dining5_1, 1) {
    Dining(5, 5s);
}

UTEST_MT(Philosophers, Dining5_5, 5) {
    Dining(5, 5s);
}

UTEST_MT(Philosophers, Dining10_1, 1) {
    Dining(10, 7s);
}

UTEST_MT(Philosophers, Dining10_2, 2) {
    Dining(10, 7s);
}

UTEST_MT(Philosophers, Dining10_10, 10) {
    Dining(10, 7s);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
