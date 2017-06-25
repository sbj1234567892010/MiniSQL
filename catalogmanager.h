#ifndef _CATALOG_MANAGER_H_
#define _CATALOG_MANAGER_H_

#include <string>
#include <algorithm>
#include "Structure.h"
using namespace std;

table cmCreateTable(const string &TableName, const vector <item> &ItemData);

bool cmRegisterIndex( const string &TableName, const string &IndexName, int ItemIndex);

table cmReadTable(const string &TableName);

void cmDropTable(const string &TableName);

bool cmExistTable(const string &TableName);

bool cmExistIndex(const string &IndexName, const string &TableName);

pair<string, int> cmAskIndex(const string &IndexName, const string &TableName);
bool cmDeleteIndex(const string IndexName, const string &TableName);

#endif
