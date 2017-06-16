#include <cstdio>
#include <cstring>
#include <cassert>
#include "indexmanager.h"
#include "buffermanager.h"

BPHeader::BPHeader(const std::string& indexName):indexName(indexName)
{
    Block b = bmReadBlock(indexName, 0);
    unsigned char* blockData = b.data;

    memcpy(&(this->type), blockData, sizeof(EleType));
    memcpy(&(this->rootOffset), blockData + sizeof(EleType)/sizeof(unsigned char), sizeof(int));
}

BPHeader::~BPHeader()
{
    this->WriteFile();
}

void BPHeader::WriteFile(void)
{
    Block b = bmReadBlock(this->indexName, 0);
    unsigned char* blockData = b.data;

    memcpy(blockData, &(this->type), sizeof(EleType));
    memcpy(blockData + sizeof(EleType)/sizeof(unsigned char), &(this->rootOffset), sizeof(int));

    bmWriteBlock(b);
}

int BPHeader::FindKey(const int key)
{
    int offset;
    if(!this->rootOffset || !(this->IsInt())) return -1;

    Block b = bmReadBlock(this->indexName, this->rootOffset);
    BPIntNode *node = new BPIntNode(b);
    offset = node->FindKey(key);
    delete node;
    return offset;
}

int BPHeader::FindKey(const float key)
{
    int offset;
    if(!this->rootOffset || !(this->IsFloat())) return -1;

    Block b = bmReadBlock(this->indexName, this->rootOffset);
    BPFloatNode *node = new BPFloatNode(b);
    offset = node->FindKey(key);
    delete node;
    return offset;
}

int BPHeader::FindKey(const char* key)
{
    int offset;
    if(!this->rootOffset || !(this->IsStr())) return -1;

    Block b = bmReadBlock(this->indexName, this->rootOffset);
    BPStrNode *node = new BPStrNode(b);
    offset = node->FindKey(key);
    delete node;
    return offset;
}

bool BPHeader::InsertKey(unsigned int offset, const int key)
{
    if(!this->IsInt())return false;
    if(this->rootOffset)
    {
        Block b = bmReadBlock(indexName, rootOffset);
        BPIntNode *root = new BPIntNode(b);
        bool res = root->InsertKey(offset, key);
        if(root->fanOut == root->N)
        {
            Block b = bmNewBlock(indexName);
            BPIntNode* newRoot = new BPIntNode(b.offset, root->head & (~LEAF), indexName);
			newRoot->fanOut++;
			newRoot->childOffset[0] = root->offset;
			newRoot->WriteNode();
            root->head &= (~ROOT);
			root->WriteNode();
            
            newRoot->InsertKey(offset, key);
			newRoot->WriteNode();
            this->rootOffset = newRoot->offset;
            delete newRoot;
        }
		this->WriteFile();
        delete root;
        return res;
    }
    else
    {
        Block b = bmNewBlock(indexName);
        BPIntNode* newRoot = new BPIntNode(b.offset, type << 2 | ROOT | LEAF, indexName);
        bool res = newRoot->InsertKey(offset, key);
        this->rootOffset = newRoot->offset;
        newRoot->WriteNode();
        this->WriteFile();
        delete newRoot;
        return res;
    }
}

bool BPHeader::InsertKey(unsigned int offset, const float key)
{
	if (!this->IsFloat())return false;
	if (this->rootOffset)
	{
		Block b = bmReadBlock(indexName, rootOffset);
		BPFloatNode *root = new BPFloatNode(b);
		bool res = root->InsertKey(offset, key);
		if (root->fanOut == root->N)
		{
			Block b = bmNewBlock(indexName);
			BPFloatNode* newRoot = new BPFloatNode(b.offset, root->head & (~LEAF), indexName);
			newRoot->fanOut++;
			newRoot->childOffset[0] = root->offset;
			newRoot->WriteNode();
			root->head &= (~ROOT);
			root->WriteNode();

			newRoot->InsertKey(offset, key);
			newRoot->WriteNode();
			this->rootOffset = newRoot->offset;
			delete newRoot;
		}
		this->WriteFile();
		delete root;
		return res;
	}
	else
	{
		Block b = bmNewBlock(indexName);
		BPFloatNode* newRoot = new BPFloatNode(b.offset, type << 2 | ROOT | LEAF, indexName);
		bool res = newRoot->InsertKey(offset, key);
		this->rootOffset = newRoot->offset;
		newRoot->WriteNode();
		this->WriteFile();
		delete newRoot;
		return res;
	}
}

bool BPHeader::InsertKey(unsigned int offset, const char* key)
{
	if (!this->IsStr())return false;
	if (this->rootOffset)
	{
		Block b = bmReadBlock(indexName, rootOffset);
		BPStrNode *root = new BPStrNode(b);
		bool res = root->InsertKey(offset, key);
		if (root->fanOut == root->N)
		{
			Block b = bmNewBlock(indexName);
			BPStrNode* newRoot = new BPStrNode(b.offset, root->head & (~LEAF), indexName);
			newRoot->fanOut++;
			newRoot->childOffset[0] = root->offset;
			newRoot->WriteNode();
			root->head &= (~ROOT);
			root->WriteNode();

			newRoot->InsertKey(offset, key);
			newRoot->WriteNode();
			this->rootOffset = newRoot->offset;
			delete newRoot;
		}
		this->WriteFile();
		delete root;
		return res;
	}
	else
	{
		Block b = bmNewBlock(indexName);
		BPStrNode* newRoot = new BPStrNode(b.offset, type << 2 | ROOT | LEAF, indexName);
		bool res = newRoot->InsertKey(offset, key);
		this->rootOffset = newRoot->offset;
		newRoot->WriteNode();
		this->WriteFile();
		delete newRoot;
		return res;
	}
}

int BPHeader::DeleteKey(const int key)
{
	if(!this->rootOffset || !this->IsInt() )
		return -1;
	else
	{
		Block b = bmReadBlock(this->indexName, this->rootOffset);
		BPIntNode *root = new BPIntNode(b);
		
		int retOffset = root->DeleteKey(key);
		if (root->fanOut == 1)	
		{
			if (root->isLeaf())	//No value is stored.
			{
				bmReleaseBlock(this->indexName, root->offset);
				this->rootOffset = 0;
			}
			else				//Root has too few children
			{
				Block newBlock = bmReadBlock(this->indexName, root->childOffset[0]);
				BPIntNode *newRoot = new BPIntNode(newBlock);
				newRoot->head |= ROOT;
				this->rootOffset = newRoot->offset;
				bmReleaseBlock(this->indexName, root->offset);
				delete newRoot;
			}
		}
		delete root;
		return retOffset;
	}
}

int BPHeader::DeleteKey(const float key)
{
	if (!this->rootOffset || !this->IsInt())
		return -1;
	else
	{
		Block b = bmReadBlock(this->indexName, this->rootOffset);
		BPFloatNode *root = new BPFloatNode(b);

		int retOffset = root->DeleteKey(key);
		if (root->fanOut == 1)
		{
			if (root->isLeaf())	//No value is stored.
			{
				bmReleaseBlock(this->indexName, root->offset);
				this->rootOffset = 0;
			}
			else				//Root has too few children
			{
				Block newBlock = bmReadBlock(this->indexName, root->childOffset[0]);
				BPFloatNode *newRoot = new BPFloatNode(newBlock);
				newRoot->head |= ROOT;
				this->rootOffset = newRoot->offset;
				bmReleaseBlock(this->indexName, root->offset);
				delete newRoot;
			}
		}
		delete root;
		return retOffset;
	}
}

int BPHeader::DeleteKey(const char* key)
{
	if (!this->rootOffset || !this->IsInt())
		return -1;
	else
	{
		Block b = bmReadBlock(this->indexName, this->rootOffset);
		BPStrNode *root = new BPStrNode(b);

		int retOffset = root->DeleteKey(key);
		if (root->fanOut == 1)
		{
			if (root->isLeaf())	//No value is stored.
			{
				bmReleaseBlock(this->indexName, root->offset);
				this->rootOffset = 0;
			}
			else				//Root has too few children
			{
				Block newBlock = bmReadBlock(this->indexName, root->childOffset[0]);
				BPStrNode *newRoot = new BPStrNode(newBlock);
				newRoot->head |= ROOT;
				this->rootOffset = newRoot->offset;
				bmReleaseBlock(this->indexName, root->offset);
				delete newRoot;
			}
		}
		delete root;
		return retOffset;
	}
}

std::set<unsigned int> BPHeader::FindLessThan(const int key)
{
	std::set<unsigned int> resSet = std::set<unsigned int>();
	Block b = bmReadBlock(this->indexName, this->rootOffset);
	BPIntNode *node = new BPIntNode(b);
	while (!node->isLeaf())
	{
		b = bmReadBlock(this->indexName, node->childOffset[0]);
		delete node;
		node = new BPIntNode(b);
	}

	int ri = 0, k = node->keys[ri];
	while (k < key)
	{		
		resSet.insert(node->childOffset[ri]);
		ri++;
		if (ri == node->fanOut - 1)
		{
			unsigned int nextOffset = node->childOffset[ri];
			delete node;
			if (!nextOffset) break;	//search ends;

			node = new BPIntNode(bmReadBlock(this->indexName, nextOffset));
			ri = 0;
		}
		k = node->keys[ri];
	}

	return resSet;
}

std::set<unsigned int> BPHeader::FindLessThan(const float key)
{
	std::set<unsigned int> resSet = std::set<unsigned int>();
	Block b = bmReadBlock(this->indexName, this->rootOffset);
	BPFloatNode *node = new BPFloatNode(b);
	while (!node->isLeaf())
	{
		b = bmReadBlock(this->indexName, node->childOffset[0]);
		delete node;
		node = new BPFloatNode(b);
	}

	int ri = 0;
	float k = node->keys[ri];
	while (k < key)
	{
		resSet.insert(node->childOffset[ri]);
		ri++;
		if (ri == node->fanOut - 1)
		{
			unsigned int nextOffset = node->childOffset[ri];
			delete node;
			if (!nextOffset) break;	//search ends;

			node = new BPFloatNode(bmReadBlock(this->indexName, nextOffset));
			ri = 0;
		}
		k = node->keys[ri];
	}

	return resSet;
}

std::set<unsigned int> BPHeader::FindLessThan(const char* key)
{
	unsigned int length = this->type >> 1;
	std::set<unsigned int> resSet = std::set<unsigned int>();
	Block b = bmReadBlock(this->indexName, this->rootOffset);
	BPStrNode *node = new BPStrNode(b);
	while (!node->isLeaf())
	{
		b = bmReadBlock(this->indexName, node->childOffset[0]);
		delete node;
		node = new BPStrNode(b);
	}

	int ri = 0;
	//char *k = new char[length];
	//memcpy(k, node->keys[ri], length);
	while (strncmp(node->keys[ri], key, length) < 0)		//k < key
	{
		resSet.insert(node->childOffset[ri]);
		ri++;
		if (ri == node->fanOut - 1)
		{
			unsigned int nextOffset = node->childOffset[ri];
			delete node;
			if (!nextOffset) break;	//search ends;

			node = new BPStrNode(bmReadBlock(this->indexName, nextOffset));
			ri = 0;
		}
		//memcpy(k, node->keys[ri], length);
	}
	return resSet;
}

std::set<unsigned int> BPHeader::FindMoreThan(const int key)
{
	std::set<unsigned int> resSet = std::set<unsigned int>();
	Block b = bmReadBlock(this->indexName, this->rootOffset);
	BPIntNode *node = new BPIntNode(b);
	while (!node->isLeaf())
	{
		int begin = 0, end = node->fanOut - 2;
		while (begin < end)
		{
			int mid = (begin + end + 1) >> 1;
			if (node->keys[mid] <= key) begin = mid;
			else end = mid - 1;
		}
		if (node->keys[begin] <= key) b = bmReadBlock(this->indexName, node->childOffset[begin + 1]);
		else b = bmReadBlock(this->indexName, node->childOffset[begin]);
		delete node;
		node = new BPIntNode(b);
	}

	int begin = 0, end = node->fanOut - 2;
	while (begin < end)
	{
		int mid = (begin + end) >> 1;
		if (node->keys[mid] < key)begin = mid + 1;
		else end = mid;
	}
	
	int ri = begin;
	while (1)
	{
		resSet.insert(node->childOffset[ri++]);
		if (ri == node->fanOut - 1)
		{
			unsigned int nextOffset = node->childOffset[ri];
			delete node;

			if (!nextOffset) break;	//search ends
			else
			{
				b = bmReadBlock(this->indexName, nextOffset);
				node = new BPIntNode(b);
				ri = 0;
			}
		}
	}
	return resSet;
}

std::set<unsigned int> BPHeader::FindMoreThan(const float key)
{
	std::set<unsigned int> resSet = std::set<unsigned int>();
	Block b = bmReadBlock(this->indexName, this->rootOffset);
	BPFloatNode *node = new BPFloatNode(b);
	while (!node->isLeaf())
	{
		int begin = 0, end = node->fanOut - 2;
		while (begin < end)
		{
			int mid = (begin + end + 1) >> 1;
			if (node->keys[mid] <= key) begin = mid;
			else end = mid - 1;
		}
		if (node->keys[begin] <= key) b = bmReadBlock(this->indexName, node->childOffset[begin + 1]);
		else b = bmReadBlock(this->indexName, node->childOffset[begin]);
		delete node;
		node = new BPFloatNode(b);
	}

	int begin = 0, end = node->fanOut - 2;
	while (begin < end)
	{
		int mid = (begin + end) >> 1;
		if (node->keys[mid] < key)begin = mid + 1;
		else end = mid;
	}

	int ri = begin;
	while (1)
	{
		resSet.insert(node->childOffset[ri++]);
		if (ri == node->fanOut - 1)
		{
			unsigned int nextOffset = node->childOffset[ri];
			delete node;

			if (!nextOffset) break;	//search ends
			else
			{
				b = bmReadBlock(this->indexName, nextOffset);
				node = new BPFloatNode(b);
				ri = 0;
			}
		}
	}
	return resSet;
}

std::set<unsigned int> BPHeader::FindMoreThan(const char * key)
{
	unsigned int length = this->type >> 1;
	std::set<unsigned int> resSet = std::set<unsigned int>();
	Block b = bmReadBlock(this->indexName, this->rootOffset);
	BPStrNode *node = new BPStrNode(b);
	while (!node->isLeaf())
	{
		int begin = 0, end = node->fanOut - 2;
		while (begin < end)
		{
			int mid = (begin + end + 1) >> 1;
			if (strncmp(node->keys[mid], key, length) <= 0) begin = mid;		//node->keys[mid] <= key
			else end = mid - 1;
		}
		//node->keys[begin] <= key
		if (strncmp(node->keys[begin], key, length) <= 0) b = bmReadBlock(this->indexName, node->childOffset[begin + 1]);
		else b = bmReadBlock(this->indexName, node->childOffset[begin]);
		delete node;
		node = new BPStrNode(b);
	}

	int begin = 0, end = node->fanOut - 2;
	while (begin < end)
	{
		int mid = (begin + end) >> 1;
		if (strncmp(node->keys[mid], key, length) < 0)begin = mid + 1;		//node->keys[mid] < key
		else end = mid;
	}

	int ri = begin;
	while (1)
	{
		resSet.insert(node->childOffset[ri++]);
		if (ri == node->fanOut - 1)
		{
			unsigned int nextOffset = node->childOffset[ri];
			delete node;

			if (!nextOffset) break;	//search ends
			else
			{
				b = bmReadBlock(this->indexName, nextOffset);
				node = new BPStrNode(b);
				ri = 0;
			}
		}
	}
	return resSet;
}

BPHeader OpenIndex(const std::string& indexName)
{
#ifndef _MY_RELEASE
    FILE *fp = fopen(indexName.c_str(), "r");
    assert(fp);
    fclose(fp);
#endif
    return BPHeader(indexName);
}

BPHeader CreateIndex(const std::string& indexName, EleType type)
{
    BPHeader header(indexName, type);
    header.WriteFile();
    return header;
}

void btCreate(const std::string &fileName,int type, int length) {
    EleType t = (type != INT && type != FLOAT) ? length << 1 : type;
    CreateIndex(fileName, t);
}

int btFind(const std::string &fileName,const element &key){
    BPHeader h = OpenIndex(fileName);
    int offset(0);
    if(key.type == INT)
        offset = h.FindKey(key.datai);
    else if(key.type == FLOAT)
        offset = h.FindKey(key.dataf);
    else {
        offset = h.FindKey(key.datas.c_str());
    }
    return offset;
}

void btInsert(const string &fileName, const element &key, unsigned int offset){
    BPHeader h = OpenIndex(fileName);
    if(key.type == INT)
        h.InsertKey(offset, key.datai);
    else if(key.type == FLOAT)
        h.InsertKey(offset, key.dataf);
    else{
        h.InsertKey(offset, key.datas.c_str());
    }
}

int btDelete(const string &fileName, const element &key){
     BPHeader h = OpenIndex(fileName);
     if(key.type == INT)
         return h.DeleteKey(key.datai);
     else if(key.type == FLOAT)
         return h.DeleteKey(key.dataf);
     else{
         return h.DeleteKey(key.datas.c_str());
     }
    return false;
}
