#include <cstdlib>
#include <iterator>
#include<cstdint>
#include <vector>
/*
 * Нужно написать функцию, которая принимает на вход диапазон, применяет к каждому элементу данную операцию и затем складывает результат применения операции в новый диапазон
 * Входной диапазон задан итераторами [firstIn; lastIn)
 * Выходной диапазон начинается с firstOut и имеет такую же длину как входной диапазон
 * Операция является функцией с одним аргументом (унарная функция), возвращающая результат такого типа, который можно положить в OutputIt
 */

template<class InputIt, class OutputIt, class UnaryOperation>
void Transform(InputIt firstIn, InputIt lastIn, OutputIt firstOut, UnaryOperation op) {
    auto ret = firstOut;
    for(auto it = firstIn; it != lastIn; it++)
    {
        *ret = op(*it);
        ret++;
    }
}

/*
 * Нужно написать функцию, которая принимает на вход диапазон и переставляет элементы в нем таким образом, чтобы элементы,
 * которые удовлетворяют условию p, находились ближе к началу диапазона, чем остальные элементы.
 * Входной диапазон задан итераторами [first; last)
 * p является функцией с одним аргументом (унарная функция), возвращающая результат типа bool
 */

template<class BidirIt, class UnaryPredicate>
void Partition(BidirIt first, BidirIt last, UnaryPredicate p) {
    for(auto it = first; it !=last; it++)
    {
        auto ithelp = it;
        ithelp++;
        for(auto it2 = ithelp; it2 != last; it2++)
        {
            if(!p(*it) && p(*it2)){
                std::swap(*it, *it2);
                break;
            }
        }
    }

}


/*
 * Нужно написать функцию, которая принимает на вход два отстотированных диапазона и объединяет их в новый отсортированный диапазон, содержащий все элементы обоих входных диапазонов.
 */
template<class InputIt1, class InputIt2, class OutputIt>
void Merge(InputIt1 first1, InputIt1 last1, InputIt2 first2, InputIt2 last2, OutputIt firstOut) {
    auto it = first1;
    auto ithelp = firstOut;
    for(; it != last1; it++)
    {
        *ithelp = *it;
        ithelp++;

    }
    auto it2 = first2;
    for(;it2 != last2; it2++)
    {
        *ithelp = *it2;
        ithelp++;
    }
    for(auto it1 = firstOut; it1 != ithelp; it1++)
    {
        auto i = it1;
        i++;
        for(; i != ithelp; i++)
        {
            if(*it1 > *i)
            std::swap(*it1, *i);
        }
    }
}


/*
 * Напишите класс "диапазон чисел Фибоначчи"
 * Экземпляр класса представляет из себя диапазон от первого до N-го числа Фибоначчи (1, 2, 3, 5, 8, 13 и т.д.)
 * С помощью функций begin и end можно получить итераторы и пробежать по диапазону или передать их в STL-алгоритмы
 */
class FibonacciRange {
private:
    std::vector<uint64_t> fr;
    size_t size_;
public:

    class Iterator {
    private:
        uint64_t now, next;
        friend class FibonacciRange;
    public:
        using value_type = uint64_t;
        using difference_type = ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::input_iterator_tag;

        Iterator(uint64_t now_, uint64_t next_)
        : now(now_)
        , next(next_)
        {}
        Iterator(Iterator & other)
        : now(other.now)
        , next(other.next)
        {}
        value_type operator *() const {
            return now;
        }

        Iterator& operator ++() {
            auto help = now + next;
            now = next;
            next = help;
            return *this;
        }

        Iterator operator ++(int) {
            auto it2 = *this;
            auto help = now + next;
            now = next;
            next = help;
            return it2;
        }

        bool operator ==(const Iterator& rhs) const {
            return now == rhs.now;
        }
        

        bool operator <(const Iterator& rhs) const {
            return now < rhs.now;
        }

    };

    FibonacciRange(size_t amount) {
        fr.resize(amount);
        size_ = amount;
        if(amount == 1)
        {
            fr[0] = 1;
        }
        else if(amount == 2)
        {
            fr[0] = 1;
            fr[1] = 2;
        }
        else{
            fr[0] = 1;
            fr[1] = 2;
            for(size_t i = 2; i < amount; i++)
            {
                fr[i] = fr[i - 1] + fr[i - 2];
            }
        }


    }

    Iterator begin() const {
        return {1, 2};
    }

    Iterator end() const {
        auto now = fr[size_ - 2];
        auto next = fr[size_ - 1];
        auto help = now + next;
            now = next;
            next = help;
            help = now + next;
            now = next;
            next = help;

        return {now, next};
    }

    size_t size() const {
        return size_;
    }
};
