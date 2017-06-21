#ifndef API_H
#define API_H
#include "Structure.h"
#include <ctime>
#include<assert.h>

void CreateIndex(Index& index);

void DropIndex(const string& indexName, const string& tableName);

void CreateTable(table& table1, int pk);//pk: number of primary key

void DropTable(const string &tableName);

int Select(vector <string> SelectItem,const string &tableName, const Fitter &fitter);//select ** from ** where

vector <vector <element> >  Select_Test(const string &tableName, const Fitter &fitter);

void Delete(const string& tableName, const Fitter &fitter);
void Insert(const string& tableName, vector<element> entry);

void DrawResult(const table &nowtable, vector <vector <element> > &result);

double CalculateTime(long start, long stop);//calculate time

#endif // API_H

//bool cmExistTable(string tableName);
//void cmDropTable(string tableName);
//void cmCreateTable(table& table1,int pk);
