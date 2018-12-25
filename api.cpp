#include<iostream>
#include<string>
#include<cstdio>
#include<sstream>
#include "api.h"
#include "catalogmanager.h"
#include "RecordManager.h"
#include "buffermanager.h"
#include "indexmanager.h"
#include "bplustree.h"

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
    //cout << "CreateIndex Succeed!" << endl;
	if (!cmExistTable(index.table_name + ".table")) {
		cout << "No such table" << endl;
		return;
	}


	table nowtable = cmReadTable(index.table_name + ".table");
	int itemIndex = -1;
	for (int i = 0; i<nowtable.items.size(); i++) {
		if (nowtable.items[i].name == index.attr_name) {
			itemIndex = i;
		}
	}


	if (itemIndex == -1) {
		cout << "No such item" << endl;
		return;
	}

	if (!nowtable.items[itemIndex].unique) {
		cout << "The item must be unique" << endl;
		return;
	}

	if (!cmRegisterIndex(index.table_name, index.index_name, itemIndex)) {
		cout << "The index already exists" << endl;
		return;
	}

	nowtable.items[itemIndex].indices.insert(index.index_name);
	nowtable.write();

	if (nowtable.items[itemIndex].indices.size() == 1) {
		rmAddIndex(index.table_name + ".db", index.table_name + "." + nowtable.items[itemIndex].name + ".index", nowtable, itemIndex);
	}
	cout << "Create Succeed!" << endl;
	return ;
}



void DropIndex(const string indexName, const string tableName)
{
   // cout << "DropIndex Succeed!" << endl;
	if (!cmExistIndex(indexName, tableName)) {
		cout << "No such index" << endl;
		return ;
	}

	pair<string, int> psi = cmAskIndex(indexName, tableName);

	table nowtable = cmReadTable(psi.first + ".table");
	assert(nowtable.items[psi.second].indices.count(indexName));

	nowtable.items[psi.second].indices.erase(indexName);
	nowtable.write();
	//此处加表名
	cmDeleteIndex(indexName, tableName);
	if (nowtable.items[psi.second].indices.size() == 0)
		bmClear(psi.first + "." + nowtable.items[psi.second].name + ".index");
	cout << "Drop Succeed!" << endl;
	return ;
}



void CreateTable(table& table1, int pk)   //第pk个是primary key 从0开始
{
    //cout << "CreateTable Succeed!" << endl;
	if (cmExistTable(table1.name + ".table")) {
		cout << "Table already exists" << endl;
		return ;
	}

	table newtable = cmCreateTable(table1.name + ".table", table1.items);
	newtable.items[pk].unique = true;
	newtable.write();

	struct Index x(table1.name + ".PrimaryKeyDefault", table1.name, newtable.items[pk].name);

	return CreateIndex(x);
}



void DropTable(const string &tableName)
{
    //cout << "DropTable Succeed!" << endl;
	if (!cmExistTable(tableName + ".table")) {
		cout << "No such table" << endl;
		return ;
	}
	cmDropTable(tableName + ".table");
	cout << "Drop Success!" << endl;
	return ;
}



int Select(vector <string> SelectItem,const string &tableName, const Fitter &fitter)
{
    //cout << "Select Succeed!" << endl;
	if (!cmExistTable(tableName + ".table")) {
		cout << "No such table!" << endl;
		return 0;
	}

	//读表
	const table nowtable = cmReadTable(tableName + ".table");

	string dbname = tableName + ".db";
	string temp;
	int ind;

	//检测fitter是否与相应属性类型匹配
	for (int i = 0; i<fitter.rules.size(); i++) {
		element rhs = fitter.rules[i].rhs;

		//int index = fitter.rules[i].index;
		temp = fitter.rules[i].itemname;//准备将名字翻译为下标
		for (int i = 0; i<nowtable.items.size(); i++) {
			if (nowtable.items[i].name == temp) {
				ind = i;
				break;
			}
		}

		if (nowtable.items[ind].type != rhs.type) {
			cout << "Type mismatch!" << endl;
			return 0;
		}
	}

	vector <vector <element> > ret;

	// 等于情况的查询
	for (int i = 0; i<fitter.rules.size(); i++) {
		if (fitter.rules[i].type == 2) {  // 等于情况的查询
			element rhs = fitter.rules[i].rhs;

			temp = fitter.rules[i].itemname;
			for (int i = 0; i<nowtable.items.size(); i++) {
				if (nowtable.items[i].name == temp) {
					ind = i;
					break;
				}
			}

			int index = ind;
			//有index用offset查
			if (nowtable.items[index].indices.size()) { // has index on it
				string btreename = tableName + "." + nowtable.items[index].name + ".index";
				int offset = btFind(btreename, rhs);
				//return rmSelectWithIndex(dbname, offset, fitter, nowtable);
				vector <vector <element> > SelectItem = rmSelectWithIndex(dbname, offset, fitter, nowtable);
				DrawResult( nowtable, SelectItem);
				return 1;
			}
		}
	}

	//全部查询
	if (fitter.rules.size() == 0) {
		//return rmSelectWithoutIndex(dbname, fitter, nowtable);
		vector <vector <element> > SelectItem = rmSelectWithoutIndex(dbname, fitter, nowtable);
		DrawResult(nowtable, SelectItem);
		return 1;
	}

	set<int> offset = rmGetAllOffsets(tableName + ".db");

	for (int i = 0; i<fitter.rules.size(); i++) {
		Rule rule = fitter.rules[i];

		temp = fitter.rules[i].itemname;
		for (int i = 0; i<nowtable.items.size(); i++) {
			if (nowtable.items[i].name == temp) {
				ind = i;
				break;
			}
		}

		int elementIndex = ind;
		if (!nowtable.items[elementIndex].indices.size())
			continue;
		element rhs = rule.rhs;
		string btreename = tableName + "." + nowtable.items[elementIndex].name + ".index";
		set <unsigned int> newset;
		switch (rule.type) {
		case 0:newset = btFindLess(btreename, rhs); break;
		case 1:newset = btFindLess(btreename, rhs); newset.insert(btFind(btreename, rhs)); break;
		case 2:assert(false); break;
		case 3:newset = btFindMore(btreename, rhs); newset.insert(btFind(btreename, rhs)); break;
		case 4:newset = btFindMore(btreename, rhs);
		case 5:break;
		default:assert(false);
		}
		vector <int> tmp(offset.size());
		vector <int>::iterator end = set_intersection(offset.begin(), offset.end(), newset.begin(), newset.end(), tmp.begin());
		offset.clear();
		for (vector<int>::iterator it = tmp.begin(); it != end; it++) {
			offset.insert(*it);
		}
	}

	vector <vector <element> > res;
	for (set <int>::iterator it = offset.begin(); it != offset.end(); it++) {
		vector <vector <element> > tmp = rmSelectWithIndex(tableName + ".db", *it, fitter, nowtable);
		for (int i = 0; i<tmp.size(); i++) {
			res.push_back(tmp[i]);
		}
	}

	//return Response(res);
	DrawResult(nowtable, res);
	return 1;
}

vector <vector <element> >  Select_Test(const string &tableName, const Fitter &fitter)
{
	vector <vector <element> > ret;
	//读表
	const table nowtable = cmReadTable(tableName + ".table");

	string dbname = tableName + ".db";
	string temp;
	int ind;

	//检测fitter是否与相应属性类型匹配
	for (int i = 0; i<fitter.rules.size(); i++) {
		element rhs = fitter.rules[i].rhs;

		//int index = fitter.rules[i].index;
		temp = fitter.rules[i].itemname;//准备将名字翻译为下标
		for (int i = 0; i<nowtable.items.size(); i++) {
			if (nowtable.items[i].name == temp) {
				ind = i;
				break;
			}
		}

		if (nowtable.items[ind].type != rhs.type) {
			cout << "Type mismatch!" << endl;
			return ret;
		}
	}

	// 等于情况的查询
	for (int i = 0; i<fitter.rules.size(); i++) {
		if (fitter.rules[i].type == 2) {  // 等于情况的查询
			element rhs = fitter.rules[i].rhs;

			temp = fitter.rules[i].itemname;
			for (int i = 0; i<nowtable.items.size(); i++) {
				if (nowtable.items[i].name == temp) {
					ind = i;
					break;
				}
			}

			int index = ind;
			//有index用offset查
			if (nowtable.items[index].indices.size()) { // has index on it
				string btreename = tableName + "." + nowtable.items[index].name + ".index";
				int offset = btFind(btreename, rhs);
				return rmSelectWithIndex(dbname, offset, fitter, nowtable);

			}

		}
	}

	return rmSelectWithoutIndex(dbname, fitter, nowtable);

}


void Delete(const string &tableName, const Fitter &fitter)
{
    //cout << "Delete Succeed!" << endl;
	if (!cmExistTable(tableName + ".table")) {
		cout << "No such table!" << endl;
		return;
	}

	const table nowtable = cmReadTable(tableName + ".table");
	string dbname = tableName + ".db";

	string temp;
	int ind;

	//检测fitter是否与相应属性类型匹配
	for (int i = 0; i<fitter.rules.size(); i++) {
		element rhs = fitter.rules[i].rhs;

		//int index = fitter.rules[i].index;
		temp = fitter.rules[i].itemname;//准备将名字翻译为下标
		for (int i = 0; i<nowtable.items.size(); i++) {
			if (nowtable.items[i].name == temp) {
				ind = i;
				break;
			}
		}

		if (nowtable.items[ind].type != rhs.type) {
			cout << "Type mismatch!" << endl;
			return;
		}
	}

	//等于情况删除
	for (int i = 0; i<fitter.rules.size(); i++) {
		if (fitter.rules[i].type == 2) {
			element rhs = fitter.rules[i].rhs;
			temp = fitter.rules[i].itemname;
			for (int i = 0; i<nowtable.items.size(); i++) {
				if (nowtable.items[i].name == temp) {
					ind = i;
					break;
				}
			}

			int index = ind;
			if (nowtable.items[index].indices.size()) { // has index on it
				string btreename = tableName + "." + nowtable.items[index].name + ".index";
				int offset = btFind(btreename, rhs);
				rmDeleteWithIndex(dbname, offset, fitter, nowtable);
				return ;
			}
		}
	}


	if (fitter.rules.size() == 0) {
		rmDeleteWithoutIndex(dbname, fitter, nowtable);
		return ;
	}

	set<int> offset = rmGetAllOffsets(tableName + ".db");
	for (int i = 0; i<fitter.rules.size(); i++) {
		Rule rule = fitter.rules[i];

		temp = fitter.rules[i].itemname;
		for (int i = 0; i<nowtable.items.size(); i++) {
			if (nowtable.items[i].name == temp) {
				ind = i;
				break;
			}
		}

		int elementIndex = ind;
		if (!nowtable.items[elementIndex].indices.size())
			continue;
		element rhs = rule.rhs;
		string btreename = tableName + "." + nowtable.items[elementIndex].name + ".index";
		set <unsigned int> newset;
		switch (rule.type) {
		case 0:newset = btFindLess(btreename, rhs); break;
		case 1:newset = btFindLess(btreename, rhs); newset.insert(btFind(btreename, rhs)); break;
		case 2:assert(false); break;
		case 3:newset = btFindMore(btreename, rhs); newset.insert(btFind(btreename, rhs)); break;
		case 4:newset = btFindMore(btreename, rhs);
		case 5:break;
		default:assert(false);
		}
		vector <int> tmp(offset.size());
		vector <int>::iterator end = set_intersection(offset.begin(), offset.end(), newset.begin(), newset.end(), tmp.begin());
		offset.clear();
		for (vector<int>::iterator it = tmp.begin(); it != end; it++) {
			offset.insert(*it);
		}
	}
	for (set <int>::iterator it = offset.begin(); it != offset.end(); it++) {
		rmDeleteWithIndex(tableName + ".db", *it, fitter, nowtable);
	}
	return ;

}



void Insert(const string tableName, const vector<element> entry)
{
	if (!cmExistTable(tableName + ".table")) {
		cout << "No such table" << endl;
		return;
	}

	const table nowtable = cmReadTable(tableName + ".table");
	string dbname = tableName + ".db";
	if (nowtable.items.size() != entry.size()) {
		cout << "Type mismatch" << endl;
		return;
	}

	for (int i = 0; i<nowtable.items.size(); i++) {
		if (nowtable.items[i].type != entry[i].type) {
			cout << "Type mismatch" << endl;
			return;
		}
	}

	for (int i = 0; i<nowtable.items.size(); i++) {

		//插入unique项目报错
		if (nowtable.items[i].unique) {
			Fitter fitter;
			fitter.addRule(Rule(nowtable.items[i].name, 2, entry[i]));//条件是等于
			// Select(tableName, fitter);
			if (Select_Test(tableName, fitter).size()) {
				cout << "unique integrity violation" << endl;
				return;
			}
		}
	}
	rmInsertRecord(tableName + ".db", entry, nowtable);
	cout << "Insert Succeed!" << endl;
	return;
}

void DrawResult(const table &nowtable, vector <vector <element> > &result)//void DrawResult(table& tbl)
{
	vector <int> space(nowtable.items.size());
	for (int i = 0; i<nowtable.items.size(); i++) {
		space[i] = nowtable.items[i].name.length() + 1;
	}
	for (int i = 0; i<result.size(); i++) {
		for (int j = 0; j<result[i].size(); j++) {
			int len;
			stringstream str;
			switch (result[i][j].type) {
			case 0: {str << result[i][j].datai; string s; str >> s; len = s.length(); break; }
			case 1: {str << result[i][j].dataf; string s; str >> s; len = s.length(); break; }
			case 2: {len = result[i][j].datas.length(); break; }
			}
			if (len>space[j])
				space[j] = len + 1;
		}
	}
	int sum = 0;
	for (int i = 0; i<nowtable.items.size(); i++)
		sum += space[i] + 1;
	sum++;
	for (int i = 0; i<sum; i++) printf("-");
	printf("\n");
	printf("|");
	for (int i = 0; i<nowtable.items.size(); i++) {
		printf("%s", nowtable.items[i].name.c_str());
		for (int j = 0; j<space[i] - nowtable.items[i].name.length(); j++)
			printf(" ");
		printf("|");
	}
	printf("\n");
	for (int i = 0; i<result.size(); i++) {
		printf("|");
		for (int j = 0; j<result[i].size(); j++) {
			int len;
			stringstream str;
			switch (result[i][j].type) {
			case 0: {str << result[i][j].datai; string s; str >> s; len = s.length(); break; }
			case 1: {str << result[i][j].dataf; string s; str >> s; len = s.length(); break; }
			case 2: {len = result[i][j].datas.length(); break; }
			}
			result[i][j].print();
			for (int s = 0; s<space[j] - len; s++)
				printf(" ");
			printf("|");
		}
		printf("\n");
	}
	for (int i = 0; i<sum; i++) printf("-");
	printf("\n");
}

double CalculateTime(long start, long end)
{
	return (double)(end - start) / CLK_TCK;
}
