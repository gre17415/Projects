#include "task.h"

void ReverseList(std::list<int>& l) {
    std::list<int> ans;
    while(!l.empty())
    {
        ans.push_back(l.front());
        l.pop_front();
    }
    while(!ans.empty())
    {
        l.push_back(ans.back());
        ans.pop_back();
    }
}
