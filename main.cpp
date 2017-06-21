#include<iostream>
#include<cstdio>
#include <string>
#include "interpreter.h"
using namespace std;

#define MAX 1000
int main()
{
    int i;
    char line[MAX];
    while(1)
    {
        cout << "Minisql>>";
        gets_s(line);
        LinetoQuery(line);
    }
    return 0;
}
