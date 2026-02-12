#include "task.h"

bool is_palindrome(const std::string& s) {
    std::string q;
    int nn = s.size();
    for(int i = 0; i < nn; i++)
    {
        char c = s[i];
        if('a' <= c && c <= 'z')
            q += c;
        else if('A' <= c && c <= 'Z')
        {
            q += char(int('a') + c - 'A');
        }
    }
    std::string q2 = q;
    int n = q.size();
    for(int i = 0; i < n; i++)
    {
        if(q[i] != q[n - i - 1])
            return false;
    }
    return true;
}
