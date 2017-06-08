#include<iostream>
#include<string>
#include<cstdio>
#include<sstream>
#include "api.h"
/*
void CreateIndex(Index& index)
{
    if (!cmExistTable(index.table_name)) {
		cout << "No such table" << endl;
		return;
	}

}



void DropIndex(const string indexName)
{
    if (!cmExistIndex(indexName)) {
		cout << "No such index" << endl;
		return;
	}
	if(cmDeleteIndex(indexName))
    {
        cout << "Index deleted." << endl;
    }
    else cout << "Deletion failed." << endl;
}



void CreateTable(table& table1, int pk)
{
	if (cmExistTable(table1.name)) {
	    cout << "Table already exists." << endl;
		return;
	}
	else {
        cmCreateTable(table1,pk);
        Index idx;
        idx.index_name = table1.name+".PrimaryKeyDefault";
        idx.table_name = table1.name;
        idx.attr_name = table1.items[pk].name;
        CreateIndex(idx);
        //newtable.items[pk].unique=true;
        //newtable.write();
        return;
	}
}



void DropTable(const string &tableName)
{
	if (!cmExistTable(tableName)) {
	    cout << "Table does not exist." << endl;
		return;
	}
	else {
        cmDropTable(tableName);
        cout << "Query OK. Table is dropped." << endl;
        return;
	}
}
*/
void CreateIndex(Index& index)
{
    cout << "CreateIndex Succeed!" << endl;
}
void DropIndex(const string indexName, const string tableName)
{
    cout << "DropIndex Succeed!" << endl;
}
void CreateTable(table& table1, int pk)
{
    cout << "CreateTable Succeed!" << endl;
}
void DropTable(const string &tableName)
{
    cout << "DropTable Succeed!" << endl;
}
void Select(vector <string> SelectItem,const string &tableName, const Fitter &fitter)
{
    cout << "Select Succeed!" << endl;
}
void Delete(const string &tableName, const Fitter &fitter)
{
    cout << "Delete Succeed!" << endl;
}
void Insert(const string tableName, const vector<element> entry)
{
    cout << "Insert Succeed!" << endl;
}

void DrawResult(table tbl)
{

}

double CalculateTime(long start, long stop)
{

}
