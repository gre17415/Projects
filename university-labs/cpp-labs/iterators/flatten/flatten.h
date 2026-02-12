#pragma once
#include<algorithm>
#include<iterator>
#include<iostream>
#include<vector>
#include<cstdint>


template<class T>
class FlattenedVector {
private:
    std::vector<std::vector<T>>& matrix;
    std::vector<int64_t> pref;
public:
    class iterator: public std::iterator<std::random_access_iterator_tag, T>
    {
    private:
        std::vector<std::vector<T>>& data_;
        std::vector<int64_t>& pref_;
        int64_t ind_;
        
    public:

        iterator(std::vector<std::vector<T>>& data, std::vector<int64_t>& pref, int64_t ind)
        : data_(data)
        , pref_(pref)
        , ind_(ind) {}

        iterator(const iterator& other)
            : data_(other.data_)
            , pref_(other.pref_)
            , ind_(other.ind_) {}

        bool operator!=(const iterator& other) const {
            return ind_ != other.ind_;
        }

        iterator& operator+=(const int64_t n) {
            ind_ += n;
            return *this;
        }

        iterator& operator++() {
            return *this += 1;
        }

        iterator operator+(const int64_t n) const {
            iterator tmp(*this);
            tmp += n;
            return tmp;
        }

        int64_t operator-(const iterator& other) const {
            return ind_ - other.ind_;
        }
        iterator& operator=(const iterator& other){
            data_ = other.data_;
            pref_ = other.pref_;
            ind_ = other.ind_;
            return *this;
        }
        bool operator<=(const iterator& other) const {
            return ind_ <= other.ind_;
        }

        bool operator>(const iterator& other) const {
            return ind_ > other.ind_;
        }

        bool operator==(const iterator& other) const {
            return ind_ == other.ind_;
        }

        iterator& operator-=(const int64_t n) {
            return *this += -n;
        }

        bool operator<(const iterator& other) const {
            return ind_ < other.ind_;
        }

        iterator operator-(const int64_t n) const {
            iterator tmp(*this);
            tmp -= n;
            return tmp;
        }

        iterator& operator--() {
            return *this -= 1;
        }

        T& operator*() const {
            
            int64_t l = 0, r = pref_.size();
            while(r - l > 1)
            {
                int mid = (l + r) / 2;
                if(pref_[mid] > ind_)
                {
                    r = mid;
                }
                else
                l = mid;
            }
            int64_t ind_out = l;
            int64_t ind_in = ind_ - pref_[ind_out];
            return (data_[ind_out][ind_in]);
        }

        T& operator[](const int64_t n) const {
            iterator tmp(*this);
            tmp += n;
            return *tmp;
        }

    };

    FlattenedVector(std::vector<std::vector<T>>& mm)
        : matrix(mm)
    {
        pref.resize(matrix.size() + 1, 0);
        for(size_t i = 1; i <= matrix.size(); i++)
        {
            pref[i] = pref[i - 1] + matrix[i - 1].size();
        }
    }

    FlattenedVector(FlattenedVector& other) 
    : matrix(other.matrix)
    , pref(other.pref) {}

    iterator begin()
    {
        return iterator(matrix, pref, 0);
    }
    iterator end()
    {
        return iterator(matrix, pref, pref.back());
    }

};

template<class T>
T operator+(const int64_t n, T& it) {
    return it + n;
}
