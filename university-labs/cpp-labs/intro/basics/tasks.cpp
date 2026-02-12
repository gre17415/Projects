#include "tasks.h"

#include <cmath>

int NOuter::NInner::DoSomething(int lhs, int rhs) {
    return lhs + rhs;
}

int NOuter::DoSomething(int lhs, int rhs) {
    return lhs - rhs;
}

int NOverload::ProcessTwoArgs(int lhs, int rhs) {
    return lhs + rhs;
}

char NOverload::ProcessTwoArgs(char lhs, char rhs) {
    return std::max(rhs, lhs);
}

int NOverload::ProcessTwoArgs(int lhs, char rhs) {
    return lhs - rhs;
}

unsigned int NOverflow::WithOverflow(int lhs, int rhs) {
    return lhs + rhs;
}

uint64_t NOverflow::WithOverflow(int64_t lhs, int64_t rhs) {
    return lhs - rhs;
}

int NLoop::SumInRange(const int lhs, const int rhs) {
    int s = 0;
    for(int i = lhs; i < rhs; i++)
        s += i;
    return s;
}

int NLoop::CountFixedBitsInRange(const int from, const int to, const int bitsCnt) {
    int k = 0;
    for(int i = from + 1; i < to; i++)
    {
        int cnt = 0;
        for(int j = 0; j < 32; j++)
        {
            cnt += ((i >> j) & 1);
        }
        if(cnt == bitsCnt)
            k++;
    }
    return k;
}

double NMath::ComputeMathFormula(const double arg) {
    return fabs((sin(arg) / 2 + cos(arg)) * (sin(arg) / 2 + cos(arg)) + tan(arg) * atan(arg));
}

bool NMath::IsPositive(int arg) {
    return arg > 0;
}

int NRecursion::CalculateFibonacci(const int arg) {
    if(arg == 0)
        return 0;
    if(arg == 1)
        return 1;
    return CalculateFibonacci(arg - 1) + CalculateFibonacci(arg - 2);
}
