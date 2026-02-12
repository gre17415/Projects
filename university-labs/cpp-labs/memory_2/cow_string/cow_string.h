#include <cstdint>
#include <cstddef>

struct State {
    char* first;
    size_t size_, capacity_, ref_count;
};
class CowString {
private:
    State* st;
public:

    CowString()
    {
        st = new State({new char[2], 0, 2, 1});
    }
    CowString(const CowString & other)
    {
        other.st->ref_count++;

        st = other.st;
    }
   
    ~CowString()
    {
        if(st->ref_count == 1)
        {
            delete [] st->first;
            delete st;
        }
        else
        {
            st->ref_count--;
        }
    }
    CowString& operator=(const CowString & other)
    {
        if(&other == this)
            return *this;  
        st->ref_count--;
        if(st->ref_count==0)
        {
            delete [] st->first;
            delete st;
            st = nullptr;
        }
        
        st = other.st;
        st->ref_count++;
        return *this;
    }
    const char& At(size_t index) const
    {
        return const_cast <const char&> (st->first[index]);
    }

    char& operator[](size_t index)
    {
        if(st->ref_count == 1)
            return st->first[index];
        else
        {
            st->ref_count--;
            char* newfirst = new char[st->capacity_];
            for(size_t i = 0; i < st->size_; i++)
            {
                newfirst[i] = st->first[i];
            }

            st = new State({newfirst, st->size_, st->capacity_, 1});
            return st->first[index];
        }
    }

    const char& Back() const
    {
        return st->first[st->size_ - 1];
    }

    void PushBack(char c)
    {
        if(st->ref_count == 1)
        {
            if(st->size_ == st->capacity_)
            {
                st->capacity_ *= 2;
                char* newfirst = new char[st->capacity_];
                for(size_t i = 0; i < st->size_; i++)
                    newfirst[i] = st->first[i];
                delete [] st->first;
                st->first = newfirst;
            }
            st->first[st->size_] = c;
            st->size_++;
        }
        else
        {
            st->ref_count--;
            size_t newcapacity = st->capacity_;
            if(st->size_ == newcapacity)
            {
                newcapacity *= 2;
            }
            char* newfirst = new char[newcapacity];
            for(size_t i = 0; i < st->size_; i++)
                newfirst[i] = st->first[i];
            st = new State({newfirst, st->size_ + 1, newcapacity, 1});
            st->first[st->size_] = c;
        }
    }

    size_t Size() const
    {
        return st->size_;
    }
    size_t Capacity() const
    {
        return st->capacity_;
    }

    void Reserve(size_t capacity)
    {
        if(capacity <= st->capacity_)
            return;
        st->capacity_ = capacity;
        char* newfirst = new char[capacity];
        for(size_t i = 0; i < st->size_; i++)
            newfirst[i] = st->first[i];
        delete [] st->first;
        st->first = newfirst;
    }
    void Resize(size_t size)
    {
        if(size > st->capacity_)
        {
            st->capacity_ = size;
            char* newfirst = new char[st->capacity_];
            for(size_t i = 0; i < st->size_; i++)
                newfirst[i] = st->first[i];
            delete [] st->first;
            st->first = newfirst;
        }
        st->size_ = size;
    }
};
