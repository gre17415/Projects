#pragma once
#include <cstdint>
#include <iterator>
#include <iostream>
#include <algorithm>

class Range {
public:
    class iterator
    : public std::iterator<std::input_iterator_tag, int64_t>
    {
        public:
           iterator(const int64_t uk, const int64_t st, const int64_t see)
            : it(uk)
            , step(st)
            , se(see)
            {
            }
            iterator(const iterator&itt)
            : it(itt.it)
            , step(itt.step)
            , se(itt.se)
            {
            }
            iterator & operator++() 
            {
                if(step > 0)
                {
                    if(it + step > se)
                        it = se;
                    else
                        it = it + step;
                }
                else
                {
                    if(it + step > se)
                        it = it + step;
                    else
                        it = se;
                }
                return *this;
            }
            iterator operator++(int) 
            {
                iterator cop = iterator(*this);
                if(step > 0)
                {
                    if(it + step > se)
                        it = se;
                    else
                        it = it + step;
                }
                else
                {
                    if(it + step > se)
                        it = it + step;
                    else
                        it = se;
                }
                return cop;
            }  
            bool operator !=(iterator other) const
            {
                return it != other.it;
            }
            bool operator ==(const iterator other) const
            {
                return other.it - it == 0;
            }
            int64_t operator*() const
            {
                return it;
            }
        private:
            int64_t it;
            int64_t step;
            int64_t se;
    };
    Range(const int64_t n)
        : fir(0)
        , sec(n)
        , step(1)
    {}
    Range(const int64_t a, const int64_t b)
        : fir(a)
        , sec(b)
        , step(1)
    {}
    Range(const int64_t  a, const int64_t b, const int64_t step)
        : fir(a)
        , sec(b)
        , step(step)
    {}
    iterator begin() const
    {
        return iterator(fir,step, sec);
    }

    iterator end() const
    {
        return iterator(sec, step, sec);
    }
    int64_t Size() const
    {
        if(fir!=sec)
            return 1 + (std::abs(sec - fir) - 1) / std::abs(step);
        return 0;
    }
private:
    int64_t fir;
    int64_t sec;
    int64_t step;
};
