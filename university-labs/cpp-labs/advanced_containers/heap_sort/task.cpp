#include "task.h"

using namespace std;

vector<int> HeapSort(const vector<int>& v) {
    priority_queue<int> pq;
    for(auto x: v)
        pq.push(x);
    vector<int> ans;
    while(!pq.empty())
    {
        ans.push_back(pq.top());
        pq.pop();
    }
    int anssz = ans.size();
    for(int i = 0; i < anssz / 2; i++)
    swap(ans[i], ans[anssz - i - 1]);
    return ans;
}
