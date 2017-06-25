#ifndef _RECORD_H_
#define _RECORD_H_

#include <string>
#include <vector>
#include"Structure.h"
using namespace std;

int rmInsertRecord(const string &fileName, const vector<element> &entry, const table &datatable);

void rmDeleteWithoutIndex(const string fileName, const Fitter &fitter, const table &datatable);

vector <vector <element> > rmSelectWithoutIndex(const string fileName, const Fitter &fitter, const table &datatable);

void loadBlockStatus(const string &fileName);


void rmAddIndex(const string dbName, const string BTreeName, const table &datatable, int itemIndex);

void rmClear(const string fileName);

vector <vector <element> > rmSelectWithIndex(const string fileName, int offset, const Fitter &fitter, const table &datatable);

set<int> rmGetAllOffsets(const string &fileName);

void rmDeleteWithIndex(const string fileName, int offset, const Fitter &fitter, const table &datatable);
#endif
