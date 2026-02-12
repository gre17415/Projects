#pragma once

#include "fork.h"

#include <userver/engine/async.hpp>
#include <userver/engine/sleep.hpp>

#include <userver/utest/utest.hpp>

#include <atomic>
#include <vector>

using namespace USERVER_NAMESPACE;

class Plate {
public:
    void Access() {
        ASSERT_FALSE(Accessed_.exchange(true, std::memory_order_relaxed));
        engine::Yield();
        ++AccessCount_;
        ASSERT_TRUE(Accessed_.exchange(false, std::memory_order_relaxed));
    }

    int AccessCount() const {
        return AccessCount_;
    }

private:
    std::atomic<bool> Accessed_ = false;
    int AccessCount_ = 0;
};

class Table {
public:
    explicit Table(int seatCount)
        : SeatCount_(seatCount)
        , Plates_(seatCount)
        , Forks_(seatCount)
    { }

    Fork& LeftFork(int seat) {
        return Forks_[seat];
    }

    Fork& RightFork(int seat) {
        return Forks_[RightSeat(seat)];
    }

    int RightSeat(int seat) const {
        return (seat + 1) % SeatCount_;
    }

    void AccessPlate(int seat) {
        Plates_[seat].Access();
    }

private:
    const int SeatCount_;
    std::vector<Plate> Plates_;
    std::vector<Fork> Forks_;
};

class Philosopher {
public:
    Philosopher(Table& table, int seat)
        : Table_(table)
        , Seat_(seat)
        , LeftFork_(Table_.LeftFork(seat))
        , RightFork_(Table_.RightFork(seat))
    { }

    void EatAndThink() {
        AcquireForks();
        Eat();
        ReleaseForks();
        Think();
    }

    int Meals() const {
        return Meals_;
    }

private:
    void Eat() {
        Table_.AccessPlate(Seat_);
        Table_.AccessPlate(Table_.RightSeat(Seat_));
        ++Meals_;
    }

    void Think() {
        engine::Yield();
    }

    void AcquireForks();
    void ReleaseForks();

private:
    Table& Table_;
    int Seat_;

    Fork& LeftFork_;
    Fork& RightFork_;

    int Meals_ = 0;
};
