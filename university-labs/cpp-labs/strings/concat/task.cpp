#include "task.h"
char* concat(const char* lhs, const char* rhs) {
    int n1 = 0;
    while(lhs[n1] != '\0')
        n1++;
    int n2 = 0;
    while(rhs[n2] != '\0')
        n2++;
    char* f = new char[n1 + n2 + 1];
    for(int i = 0; i < n1; i++)
    {
        f[i] = lhs[i];
    }
    for(int i = 0; i < n2; i++)
    {
        f[i + n1] = rhs[i];
    }
    f[n1 + n2] = '\0';
    return f;
}
