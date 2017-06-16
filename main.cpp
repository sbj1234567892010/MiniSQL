#include<iostream>
#include<cstdio>
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
        gets(line);
        LinetoQuery(line);
    }
    return 0;
}
