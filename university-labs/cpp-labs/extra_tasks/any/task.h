#pragma once

#include <typeinfo>
#include <utility>

class Any {
public:
    Any() : ptr(nullptr) {}

    template<typename T>
    Any(const T& value) : ptr(new Holder<T>(value)) {}

    Any(const Any& other) : ptr(other.ptr ? other.ptr->clone() : nullptr) {}

    Any(Any&& other) noexcept : ptr(nullptr) {
        std::swap(ptr, other.ptr);
    }

    ~Any() {
        delete ptr;
    }

    void Reset() {
        delete ptr;
        ptr = nullptr;
    }

    template<typename T>
    Any& operator=(const T& value) {
        Any(value).Swap(*this);
        return *this;
    }

    Any& operator=(const Any& other) {
        Any(other).Swap(*this);
        return *this;
    }

    Any& operator=(Any&& other) noexcept {
        std::swap(ptr, other.ptr);
        other.Reset();
        return *this;
    }

    bool Empty() const {
        return ptr == nullptr;
    }

    template<typename T>
    T& Value() {
        if (ptr && typeid(Holder<T>) == typeid(*ptr)) {
            return static_cast<Holder<T>*>(ptr)->value;
        } else {
            throw std::bad_cast();
        }
    }

    void Swap(Any& other) {
        std::swap(ptr, other.ptr);
    }

private:
    struct BaseHolder {
        virtual ~BaseHolder() = default;
        virtual BaseHolder* clone() const = 0;
    };

    template<typename T>
    struct Holder : public BaseHolder {
        T value;

        Holder(const T& val) : value(val) {}

        BaseHolder* clone() const override {
            return new Holder<T>(value);
        }
    };

    BaseHolder* ptr;
};
