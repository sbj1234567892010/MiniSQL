#include "RecordManager.h"
#include "buffermanager.h"

#include <map>
#include<assert.h>
#include "indexmanager.h"
using namespace std;

//文件状态 （大小）
struct FileStatus {
	FILE *fp;
	int fileSize;
};

//块状态 
map <string, map <int, int> >  blockStatus;


//类型检测
bool fitInTable(const vector<element> &entry, const table &datatable) {
	//元素个数是否符合表的要求
	if (entry.size() != datatable.items.size())
		return false;

	//依次检测属性类型是否匹配
	for (int i = 0; i<entry.size(); i++) {
		if (entry[i].type != datatable.items[i].type)
			return false;
	}
	return true;
}

//属性转为2进制
void entryToBinary(const vector<element> &entry, unsigned char * c, const table &datatable) {
	
	int entrySize = datatable.entrySize;//表属性大小（字）

	memset(c, 0, entrySize); //c的entrySize位初始化都为0

	for (int i = 0; i<entry.size(); i++) {
		switch (entry[i].type) {
			//0:int
		case 0: memcpy(c, &entry[i].datai, sizeof(entry[i].datai)); c += sizeof(entry[i].datai); break;
			//1:float
		case 1: memcpy(c, &entry[i].dataf, sizeof(entry[i].dataf)); c += sizeof(entry[i].dataf); break;
			//2:string
		case 2: memcpy(c, entry[i].datas.c_str(), entry[i].datas.length()); c += datatable.items[i].length; break;
		default:assert(false);
		}
	}
}


static int fetchint(const unsigned char *loc) {
	int ret;
	memcpy(&ret, loc, sizeof(int));//string头调用
	return ret;
}
static float fetchfloat(const unsigned char *loc) {
	float ret;
	memcpy(&ret, loc, sizeof(float));
	return ret;
}
static string fetchstring(const unsigned char *loc, int len) {
	char ret[BlockSize];														//这里用了buffer
	memcpy(ret, loc, len);
	ret[len] = 0;
	return ret;
}

//2进制转回属性
vector <element> binaryToEntry(unsigned char *c, const table &datatable) {
	vector <element> res;

	//遍历表的每个属性（主要查类型）
	for (int i = 0; i<datatable.items.size(); i++) {
		switch (datatable.items[i].type) {
			//0:int
		case 0:res.push_back(fetchint(c)); c += sizeof(int); break;
			//1:float
		case 1:res.push_back(fetchfloat(c)); c += sizeof(float); break;
			//2:string
		case 2:res.push_back(fetchstring(c, datatable.items[i].length)); c += datatable.items[i].length; break;
		
		default:assert(false);
		}
	}
	return res;
}

//读块状态
void loadBlockStatus(const string &fileName) {
	//块还没满，返回
	if (blockStatus.find(fileName) != blockStatus.end())
		return;

	//块满了，开新块
	FILE *fplist = fopen((fileName + ".blockinfo").c_str(), "r");//////////////////
	if (fplist) {
		int offset, occ;
		while (fscanf(fplist, "%d%d", &offset, &occ) != EOF) {
			blockStatus[fileName][offset] = occ;
		}
		fclose(fplist);
	}
}

//数据插入  return the offset of data inserted
int rmInsertRecord(const string &fileName, const vector<element> &entry, const table &datatable) { 
	if (!fitInTable(entry, datatable)) {
		return -1;
	}

	//看看块满了没，没有新开
	loadBlockStatus(fileName);

	int capacity = BlockSize / (datatable.entrySize + 1);
	int offset = -1;
	//找offset
	for (map<int, int>::iterator it = blockStatus[fileName].begin(); it != blockStatus[fileName].end(); it++) {
		if (it->second<capacity) {
			offset = it->first;
			break;
		}
	}

	Block block;
	if (offset == -1) {

		block = bmNewBlock(fileName);
		offset = block.offset;
		blockStatus[fileName][offset] = 0;
		memset(block.data, 0, sizeof(block.data));
	}
	else {
		block = bmReadBlock(fileName, offset);
	}
	bool findd = false;
	for (int i = 0; i<capacity; i++) {
		if (!block.data[i]) {
			//数据写入块
			entryToBinary(entry, block.data + capacity + i*datatable.entrySize, datatable);
			findd = true;
			block.data[i] = true;
			break;
		}
	}

	assert(findd);
	bmWriteBlock(block);
	blockStatus[fileName][offset]++;
	return offset;
}


//删除
void rmDeleteWithoutIndex(const string fileName, const Fitter &fitter, const table &datatable) {

	//看看块满了没，没有新开
	loadBlockStatus(fileName);

	//遍历指定文件名字的块
	for (map <int, int> ::iterator it = blockStatus[fileName].begin(); it != blockStatus[fileName].end(); it++) {
		int offset = it->first;
		if (it->second == 0)
			continue;
		Block block = bmReadBlock(fileName, offset);
		int capacity = BlockSize / (datatable.entrySize + 1);

		unsigned char *c = block.data + capacity;

		//遍历block中的data
		for (int i = 0; i<capacity; i++) {
			if (block.data[i]) {
				vector <element> entry = binaryToEntry(c, datatable);

				//符合删除条件
				if (fitter.test(datatable,entry)) {

					block.data[i] = false;
					blockStatus[fileName][offset]--;
				}
			}
			c += datatable.entrySize;
		}


		assert(blockStatus[fileName][offset] >= 0);

		//更改后的块写回
		bmWriteBlock(block);
	}
}

//查询
vector <vector <element> > rmSelectWithoutIndex(const string fileName, const Fitter &fitter, const table &datatable) {
	vector <vector <element> > ret;
	loadBlockStatus(fileName);

	//遍历指定文件名字的块
	for (map <int, int> ::iterator it = blockStatus[fileName].begin(); it != blockStatus[fileName].end(); it++) {
		int offset = it->first;
		if (it->second == 0)
			continue;
		Block block = bmReadBlock(fileName, offset);
		int capacity = BlockSize / (datatable.entrySize + 1);

		unsigned char *c = block.data + capacity;
		for (int i = 0; i<capacity; i++) {
			if (block.data[i]) {
				vector <element> entry = binaryToEntry(c, datatable);
				if (fitter.test(datatable,entry)) {
					ret.push_back(entry);
				}
			}
			c += datatable.entrySize;
		}
	}


	return ret;
}




void rmAddIndex(const string dbName, const string BTreeName, const table &datatable, int itemIndex) {
	btCreate(BTreeName, datatable.items[itemIndex].type, datatable.items[itemIndex].length);
	loadBlockStatus(dbName);

	for (map <int, int> ::iterator it = blockStatus[dbName].begin(); it != blockStatus[dbName].end(); it++) {
		int offset = it->first;
		if (it->second == 0)
			continue;
		Block block = bmReadBlock(dbName, offset);
		int capacity = BlockSize / (datatable.entrySize + 1);

		unsigned char *c = block.data + capacity;
		for (int i = 0; i<capacity; i++) {
			if (block.data[i]) {
				vector <element> entry = binaryToEntry(c, datatable);
				assert(itemIndex<entry.size());
				assert(btFind(BTreeName, entry[itemIndex]) !=-1);
				btInsert(BTreeName, entry[itemIndex], offset);
			}
			c += datatable.entrySize;
		}
	}
}

void rmClear(const string fileName) {
	if (blockStatus.find(fileName) != blockStatus.end())
		blockStatus.erase(blockStatus.find(fileName));
	bmClear(fileName);
	remove((fileName + ".blockinfo").c_str());
}

vector <vector <element> > rmSelectWithIndex(const string fileName, int offset, const Fitter &fitter, const table &datatable) {
	vector <vector <element> > ret;
	loadBlockStatus(fileName);

	Block block = bmReadBlock(fileName, offset);
	int capacity = BlockSize / (datatable.entrySize + 1);

	unsigned char *c = block.data + capacity;
	for (int i = 0; i<capacity; i++) {
		if (block.data[i]) {
			vector <element> entry = binaryToEntry(c, datatable);
			if (fitter.test(datatable,entry)) {
				ret.push_back(entry);
			}
		}
		c += datatable.entrySize;
	}
	return ret;
}


set<int> rmGetAllOffsets(const string &fileName) {
	loadBlockStatus(fileName);
	set <int> ret;
	for (map<int, int>::iterator it = blockStatus[fileName].begin(); it != blockStatus[fileName].end(); it++) {
		ret.insert(it->first);
	}
	return ret;

}

void rmDeleteWithIndex(const string fileName, int offset, const Fitter &fitter, const table &datatable) {
	loadBlockStatus(fileName);

	Block block = bmReadBlock(fileName, offset);
	int capacity = BlockSize / (datatable.entrySize + 1);

	unsigned char *c = block.data + capacity;
	for (int i = 0; i<capacity; i++) {
		if (block.data[i]) {
			vector <element> entry = binaryToEntry(c, datatable);
			if (fitter.test(datatable,entry)) {
				for (int j = 0; j<datatable.items.size(); j++) {
					if (datatable.items[j].indices.size()) {
						string tablename = datatable.name;
						while (*(tablename.end() - 1) != '.')
							tablename.erase(tablename.end() - 1);
						tablename.erase(tablename.end() - 1);
						string btreename = tablename + "." + datatable.items[j].name + ".index";
						assert(btFind(btreename, entry[j])!=-1);
						btDelete(btreename, entry[j]);
					}
				}
				block.data[i] = false;
				blockStatus[fileName][offset]--;
			}
		}
		c += datatable.entrySize;
	}
	assert(blockStatus[fileName][offset] >= 0);

	bmWriteBlock(block);
}


