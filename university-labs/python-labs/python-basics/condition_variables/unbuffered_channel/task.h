#pragma once

#include <mutex>
#include <condition_variable>
#include <optional>
#include <chrono>

using namespace std::chrono_literals;


class TimeOut : public std::exception {
    const char* what() const noexcept override {
            return "Timeout";
    }
};

template<typename T>
class UnbufferedChannel {
public:
    void Put(const T& data) {
        std::unique_lock<std::mutex> lock(mutex_);
        cvGet_.wait(lock, [this] { return !valueSet_; });
        value_ = data;
        valueSet_ = true;
        cvPut_.notify_one();
        cvPutDone_.wait(lock);
    }

    T Get(std::chrono::milliseconds timeout = std::chrono::milliseconds(0)) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (!valueSet_) {
            if (!cvPut_.wait_for(lock, timeout, [this] { return valueSet_; })) 
            {
                throw TimeOut();
            }
        }
        T data = value_;
        valueSet_ = false;
        cvPutDone_.notify_one();
        cvGet_.notify_one();
        return data;
    }

private:
    std::mutex mutex_;
    std::condition_variable cvPut_;
    std::condition_variable cvGet_;
    std::condition_variable cvPutDone_;
    T value_;
    bool valueSet_ = false;
};
