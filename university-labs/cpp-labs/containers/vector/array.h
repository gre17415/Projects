#include <iostream>

class Array {
public:
    Array(std::ostream& ostream)
        : Ostream_(ostream)
        , size_(0)
        , capacity_(2)
        , data_(new int[2])
    {
        Ostream_ << "Constructed. ";
        Ostream_ << *this << "\n";
    }

    Array(const Array& other)
        : Ostream_(other.Ostream_)
        , size_(other.size_)
        , capacity_(other.capacity_)
        , data_(new int[capacity_])
    {
        for (size_t i = 0; i < size_; ++i)
            data_[i] = other.data_[i];
        Ostream_ << "Constructed from another Array. ";
        Ostream_ << *this << "\n";
    }

    Array(size_t size, std::ostream& ostream = std::cout, int defaultValue = 0)
        : Ostream_(ostream)
        , size_(size)
        , capacity_(size * 2)
        , data_(new int[capacity_])
    {
        for (size_t i = 0; i < size_; ++i)
            data_[i] = defaultValue;
        Ostream_ << "Constructed with default. ";
        Ostream_ << *this << "\n";
    }

    ~Array() {
        Ostream_ << "Destructed " << size_ << "\n";
        delete[] data_;
    }

    size_t Size() const { return size_; }
    size_t Capacity() const { return capacity_; }

    void Reserve(size_t newCapacity) {
        if (newCapacity <= capacity_)
            return;
        capacity_ = newCapacity;
        int* newData = new int[capacity_];
        for (size_t i = 0; i < size_; ++i)
            newData[i] = data_[i];
        delete[] data_;
        data_ = newData;
    }

    void Resize(size_t newSize) {
        if (newSize > capacity_) {
            capacity_ = newSize;  
            int* newData = new int[capacity_];
            for (size_t i = 0; i < size_; ++i)
                newData[i] = data_[i];
            delete[] data_;
            data_ = newData;
        }
        size_ = newSize;
    }

    void PushBack(int value = 0) {
        if (size_ == capacity_) {
            capacity_ *= 2;
            int* newData = new int[capacity_];
            for (size_t i = 0; i < size_; ++i)
                newData[i] = data_[i];
            delete[] data_;
            data_ = newData;
        }
        data_[size_++] = value;
    }

    void PopBack() {
        if (size_ > 0)
            --size_;
    }

    int& operator[](size_t index) {
        return data_[index];
    }
    const int& operator[](size_t index) const {
        return data_[index];
    }

    explicit operator bool() const {
        return size_ != 0;
    }

    Array& operator<<(int value) {
        PushBack(value);
        return *this;
    }

    Array& operator<<(const Array& other) {
        for (size_t i = 0; i < other.size_; ++i)
            PushBack(other.data_[i]);
        return *this;
    }

    bool operator<(const Array& other) const {
        size_t minSize = size_ < other.size_ ? size_ : other.size_;
        for (size_t i = 0; i < minSize; ++i) {
            if (data_[i] < other.data_[i])
                return true;
            if (data_[i] > other.data_[i])
                return false;
        }
        return size_ < other.size_;
    }

    bool operator>(const Array& other) const {
        return other < *this;
    }

    bool operator==(const Array& other) const {
        if (size_ != other.size_)
            return false;
        for (size_t i = 0; i < size_; ++i)
            if (data_[i] != other.data_[i])
                return false;
        return true;
    }

    bool operator!=(const Array& other) const {
        return !(*this == other);
    }

    bool operator<=(const Array& other) const {
        return !(other < *this);
    }

    bool operator>=(const Array& other) const {
        return !(*this < other);
    }

    bool Insert(size_t pos, int value) {
        if (pos > size_)
            return false;
        if (size_ == capacity_) {
            capacity_ *= 2;
            int* newData = new int[capacity_];
            for (size_t i = 0; i < pos; ++i)
                newData[i] = data_[i];
            newData[pos] = value;
            for (size_t i = pos; i < size_; ++i)
                newData[i + 1] = data_[i];
            delete[] data_;
            data_ = newData;
        } else {
            for (size_t i = size_; i > pos; --i)
                data_[i] = data_[i - 1];
            data_[pos] = value;
        }
        ++size_;
        return true;
    }

    bool Erase(size_t pos) {
        if (pos >= size_)
            return false;
        for (size_t i = pos + 1; i < size_; ++i)
            data_[i - 1] = data_[i];
        --size_;
        return true;
    }

    friend std::ostream& operator<<(std::ostream& os, const Array& arr);

private:
    std::ostream& Ostream_;
    size_t size_;
    size_t capacity_;
    int* data_;
};

std::ostream& operator<<(std::ostream& os, const Array& arr) {
    os << "Result Array's capacity is " << arr.capacity_
       << " and size is " << arr.size_;
    if (arr.size_ > 0) {
        os << ", elements are: ";
        for (size_t i = 0; i < arr.size_ - 1; ++i)
            os << arr.data_[i] << ", ";
        os << arr.data_[arr.size_ - 1];
    }
    return os;
}