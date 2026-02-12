#include "chrono"
#include "iostream"

class Timer {
private:
    std::chrono::time_point<std::chrono::steady_clock> end;
public:
    Timer(std::chrono::steady_clock::duration time)
    : end(std::chrono::steady_clock::now() + time)
    {}

    bool Expired() const
    {
        if(end <= std::chrono::steady_clock::now())
            return true;
        return false;
    }
};
class TimeMeasurer {
private:
    std::ostream & str;
    std::chrono::time_point<std::chrono::steady_clock> start;


public:
    TimeMeasurer(std::ostream & ist)
    :str(ist)
    ,start(std::chrono::steady_clock::now())
    {}

    ~TimeMeasurer()
    {
        str <<"Elapsed time: " << (std::chrono::duration_cast <std::chrono::milliseconds> (std::chrono::steady_clock::now() - start)).count() << "ms";
    }

};
