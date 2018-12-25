#include <cassert>
#include <string>
#include <vector>
#include <fstream>
#include <map>
#include <algorithm>
#include "Structure.h"
#include "catalogmanager.h"
#include "buffermanager.h"
#include "recordmanager.h"
using namespace std;

map <string, pair<string, int> > IndexToTable;

class IndexManager {
public:
	IndexManager() {
		ifstream Iin("index.catalog");

		if (!Iin)          //如果不存在该文本，初始化完毕 
			return;

		string IndexName, TableName;
		int ItemIndex;

		while (Iin >> IndexName >> TableName >> ItemIndex) {
			IndexToTable[IndexName + "_" + TableName] = make_pair(TableName, ItemIndex);

		}

		Iin.close();
	}
	~IndexManager() {

		ofstream Iout("index.catalog");

		for (map <string, pair<string, int> >::iterator it = IndexToTable.begin(); it != IndexToTable.end(); it++) {

			Iout << it->first << " " << it->second.first << " " << it->second.second << endl;

		}

		Iout.close();
	}

}Instance;


bool cmRegisterIndex( const string &TableName, const string &IndexName, int ItemIndex) {
	if (IndexToTable.find(IndexName + "_" + TableName) != IndexToTable.end())
		return false;
	IndexToTable[IndexName + "_" + TableName] = make_pair(TableName, ItemIndex);
	return true;
}

pair<string, int> cmAskIndex(const string &IndexName, const string &TableName) {
	assert(cmExistIndex(IndexName, TableName));
	return IndexToTable[IndexName + "_" + TableName];
}

bool cmExistIndex(const string &IndexName, const string &TableName) {
	return IndexToTable.find(IndexName + "_" + TableName) != IndexToTable.end();
}

bool cmDeleteIndex(const string IndexName, const string &TableName) {

	if (IndexToTable.find(IndexName + "_" + TableName) == IndexToTable.end())
		return false;

	IndexToTable.erase(IndexToTable.find(IndexName + "_" + TableName));
	return true;

}
bool cmExistTable(const string &TableName) {
	ifstream Tin((TableName).c_str());
	if (!Tin)
		return false;
	string tableName;
	Tin >> tableName;
	Tin.close();
	return tableName == TableName;

}
table cmCreateTable(const string &TableName, const vector <item> &ItemData) {
	assert(!cmExistTable(TableName));
	table NewTable(TableName, ItemData);
	NewTable.write();
	return NewTable;
}


table cmReadTable(const string &TableName) {
	assert(cmExistTable(TableName));
	table temp;
	ifstream TIin((TableName).c_str());
	TIin >> temp.name;
	TIin >> temp.entrySize;
	int ItemSize;
	TIin >> ItemSize;
	for (int i = 0; i<ItemSize; i++) {
		item NewItem;
		TIin >> NewItem.name;
		TIin >> NewItem.type;
		TIin >> NewItem.length;
		TIin >> NewItem.unique;
		int IndexSize;
		TIin >> IndexSize;
		for (int j = 0; j<IndexSize; j++) {
			string Index;
			TIin >> Index;
			NewItem.indices.insert(Index);
		}
		temp.items.push_back(NewItem);
	}
	return temp;
}

void cmDropTable(const string &TableName) {
	assert(cmExistTable(TableName));
	string tableName = TableName;
	while (*(tableName.end() - 1) != '.')
		tableName.erase(tableName.end() - 1);
	tableName.erase(tableName.end() - 1);

	table DropTable = cmReadTable(tableName);
	for (int i = 0; i<DropTable.items.size(); i++) {
		bool flag = false;
		for (set<string>::iterator it = DropTable.items[i].indices.begin(); it != DropTable.items[i].indices.end(); it++) {
			cmDeleteIndex(*it, tableName);
			flag = true;
		}
		if (flag) bmClear(tableName + "." + DropTable.items[i].name + ".index");
	}
	rmClear(tableName + ".db");
	remove((TableName).c_str());
}
