#ifndef _INTERPRETER_H_
#define _INTERPRETER_H_

#include <sstream>
#include <string>
#include "Structure.h"
using namespace std;

template <class Type>
Type stringToNum(const string& str)
{
    istringstream iss(str);
    Type num;
    iss >> num;
    return num;
}

void InterpretQuery(string query);

void LinetoQuery(string line);

void ProcessCreateTable(stringstream& querystream);

void ProcessCreateIndex(stringstream& querystream);

void ProcessDropTable(stringstream& querystream);

void ProcessDropIndex(stringstream& querystream);

void ProcessInsert(stringstream& querystream);

void ProcessDelete(stringstream& querystream);

void ProcessSelect(stringstream& querystream);

void ProcessExecfile(stringstream& querystream);

void ExecfileLinetoQuery(string line);

element ProcessElement(string Element);

#endif
