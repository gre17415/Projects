#include "tasks.h"
#include "cmath"
#include "climits"

void NPointers::Increment(int *a)
{
    ++*a;
}
int NPointers::Multiply(int a, int b, bool *f)
{
    int q = INT_MAX;
    q -= q % b;
    int c = q / b;
    if(c >= a)
        *f = false;
    else
        *f = true;
    return a * b;
}
int NPointers::ScalarProduct(const int* a, const int* b, int cnt)
{
    int q = 0;
    for(int i = 0; i < cnt; i++)
        q += *(a + i) * *(b + i);
    return q;
}
int NPointers::SizeOfMaximumSquareOfCrosses(const char* v, int n, int m)
{
    for(int ans = std::min(n, m); ans >= 0; ans--)
    {
        for(int sti = 0; sti <= n - ans; sti++)
        {
            for(int stj = 0; stj <= m - ans; stj++)
            {
                bool f = true;
                for(int i = 0; i < ans && f; i++)
                {
                    for(int j = 0; j < ans && f; j++)
                    {
                        if(*(v + (i + sti) * m + (stj + j)) != '+')
                            f = false;
                    }
                }
                if(f)
                    return ans;
            }
        }
    }
    return 0;
}

long long* NPointers::MultiplyToLongLong  (int a, int b)
{
    return new long long((long long) a * b);
}

void NReferences::MultiplyInplace (int&a, int b)
{
    a = a * b;
}

int NReferences::CompareArraysByAverage(const int* a, int na, const int* b)
{
    int s1 = 0, s2 = 0;
    for(int i = 0; i < na; i++)
        s1 += *(a + i);
    for(int i = 0; i < 5; i++)
        s2 += *(b + i);
    if(s1 * 5 > s2 * na)
        return 1;
    else if(s1 * 5 == s2 * na)
        return 0;
    else
        return -1;
}
