#pragma once

#include <iostream>
#include <iterator>
#include <algorithm>

template<typename T>
class Array {
public:
    class iterator {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type        = T;
        using difference_type   = std::ptrdiff_t;
        using pointer           = T*;
        using reference         = T&;

        iterator(T* ptr) : it(ptr) {}
        iterator(const iterator& other) : it(other.it) {}

        iterator& operator++() { ++it; return *this; }
        iterator  operator++(int) { iterator tmp = *this; ++it; return tmp; }
        iterator& operator--() { --it; return *this; }
        iterator  operator--(int) { iterator tmp = *this; --it; return tmp; }

        reference operator*() const { return *it; }
        pointer   operator->() const { return it; }

        iterator& operator+=(difference_type n) { it += n; return *this; }
        iterator& operator-=(difference_type n) { it -= n; return *this; }

        iterator operator+(difference_type n) const { return iterator(it + n); }
        iterator operator-(difference_type n) const { return iterator(it - n); }
        difference_type operator-(const iterator& other) const { return it - other.it; }

        reference operator[](difference_type n) const { return *(it + n); }

        bool operator==(const iterator& other) const { return it == other.it; }
        bool operator!=(const iterator& other) const { return it != other.it; }
        bool operator<(const iterator& other) const  { return it < other.it; }
        bool operator>(const iterator& other) const  { return other < *this; }
        bool operator<=(const iterator& other) const { return !(other < *this); }
        bool operator>=(const iterator& other) const { return !(*this < other); }

    private:
        T* it;
    };

    Array(std::ostream& ostream)
        : Ostream_(ostream), size_(0), capacity_(2), first(new T[2])
    {
        Ostream_ << "Constructed. ";
        Ostream_ << *this << "\n";
    }

    Array(const Array& other)
        : Ostream_(other.Ostream_), size_(other.size_), capacity_(other.capacity_), first(new T[capacity_])
    {
        for (size_t i = 0; i < size_; ++i)
            first[i] = other.first[i];
        Ostream_ << "Constructed from another Array. ";
        Ostream_ << *this << "\n";
    }

    Array(size_t size, std::ostream& ostream = std::cout, T defaultValue = T())
        : Ostream_(ostream), size_(size), capacity_(size * 2), first(new T[capacity_])
    {
        for (size_t i = 0; i < size_; ++i)
            first[i] = defaultValue;
        Ostream_ << "Constructed with default. ";
        Ostream_ << *this << "\n";
    }

    ~Array() {
        Ostream_ << "Destructed " << size_ << "\n";
        delete[] first;
    }

    size_t Size() const { return size_; }
    size_t Capacity() const { return capacity_; }

    void Reserve(size_t newCapacity) {
        if (newCapacity <= capacity_) return;
        capacity_ = newCapacity;
        T* newFirst = new T[capacity_];
        for (size_t i = 0; i < size_; ++i)
            newFirst[i] = first[i];
        delete[] first;
        first = newFirst;
    }

    void Resize(size_t newSize) {
        if (newSize > capacity_) {
            capacity_ = newSize;
            T* newFirst = new T[capacity_];
            for (size_t i = 0; i < size_; ++i)
                newFirst[i] = first[i];
            delete[] first;
            first = newFirst;
        }
        size_ = newSize;
    }

    void PushBack(T value = T{}) {
        if (size_ == capacity_) {
            capacity_ *= 2;
            T* newFirst = new T[capacity_];
            for (size_t i = 0; i < size_; ++i)
                newFirst[i] = first[i];
            delete[] first;
            first = newFirst;
        }
        first[size_++] = value;
    }

    void PopBack() {
        if (size_ > 0) --size_;
    }

    T& operator[](size_t i) { return first[i]; }
    const T& operator[](size_t i) const { return first[i]; }

    explicit operator bool() const { return size_ != 0; }

    bool operator<(const Array& other) const {
        size_t minSize = std::min(size_, other.size_);
        for (size_t i = 0; i < minSize; ++i) {
            if (first[i] < other.first[i]) return true;
            if (first[i] > other.first[i]) return false;
        }
        return size_ < other.size_;
    }

    bool operator>(const Array& other) const { return other < *this; }
    bool operator==(const Array& other) const {
        if (size_ != other.size_) return false;
        for (size_t i = 0; i < size_; ++i)
            if (first[i] != other.first[i]) return false;
        return true;
    }
    bool operator!=(const Array& other) const { return !(*this == other); }
    bool operator<=(const Array& other) const { return !(other < *this); }
    bool operator>=(const Array& other) const { return !(*this < other); }

    Array& operator<<(const T& value) {
        PushBack(value);
        return *this;
    }

    Array& operator<<(const Array& other) {
        for (size_t i = 0; i < other.size_; ++i)
            PushBack(other.first[i]);
        return *this;
    }

    bool Insert(size_t pos, const T& value) {
        if (pos > size_) return false;
        if (size_ == capacity_) {
            capacity_ *= 2;
            T* newFirst = new T[capacity_];
            for (size_t i = 0; i < pos; ++i)
                newFirst[i] = first[i];
            newFirst[pos] = value;
            for (size_t i = pos; i < size_; ++i)
                newFirst[i + 1] = first[i];
            delete[] first;
            first = newFirst;
        } else {
            for (size_t i = size_; i > pos; --i)
                first[i] = first[i - 1];
            first[pos] = value;
        }
        ++size_;
        return true;
    }

    bool Erase(size_t pos) {
        if (pos >= size_) return false;
        for (size_t i = pos + 1; i < size_; ++i)
            first[i - 1] = first[i];
        --size_;
        return true;
    }

    iterator begin() const { return iterator(first); }
    iterator end()   const { return iterator(first + size_); }

    template<class U>
    friend std::ostream& operator<<(std::ostream& os, const Array<U>& arr);

private:
    std::ostream& Ostream_;
    size_t size_;
    size_t capacity_;
    T* first;
};

template<class T>
std::ostream& operator<<(std::ostream& os, const Array<T>& arr) {
    os << "Result Array's capacity is " << arr.capacity_ << " and size is " << arr.size_;
    if (arr.size_ > 0) {
        os << ", elements are: ";
        for (size_t i = 0; i < arr.size_ - 1; ++i)
            os << arr.first[i] << ", ";
        os << arr.first[arr.size_ - 1];
    }
    return os;
}