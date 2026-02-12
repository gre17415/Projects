#pragma once

namespace NPointers {

    void Increment(int*a);
    int Multiply(int a, int b, bool * f);
    int ScalarProduct(const int* a,  const int* b, int cnt);

    int SizeOfMaximumSquareOfCrosses(const char* v, int n, int m);
    long long* MultiplyToLongLong (int a, int b);

}

namespace NReferences {
    void MultiplyInplace(int&a, int b);
    int CompareArraysByAverage(const int* a, int na, const int* b);
}
