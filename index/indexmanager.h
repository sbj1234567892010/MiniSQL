#ifndef _INDEXMANAGER_H
#define _INDEXMANAGER_H

/**
    The Format of an Index File:    (Assume int type contains 4 bytes)
    Block 0 (The header): Eletype(4 bytes) + rootOffset(4 bytes).
    
    Block N (A Node):NodeType(4 byte) + fanOut(4 bytes) + N*offset(4N bytes)(including child or next leaf) + (N-1)*keys
*/

#include <string>
#include <set>
#include <cassert>
#include "bplustree.h"
#include "element.h"

//typedef unsigned int EleType;
//const EleType INT = 0;
//const EleType FLOAT = 1;
//CHAR(N) = N << 1;

struct BPHeader
{
public:
    std::string indexName;
    EleType type;
    unsigned int rootOffset;  
    
    BPHeader(const std::string& indexName);
    ~BPHeader();
    void WriteFile(void);

    bool InsertKey(unsigned int offset, const int key);
    bool InsertKey(unsigned int offset, const float key);
    bool InsertKey(unsigned int offset, const char* key);
    int FindKey(const int key);
    int FindKey(const float key);
    int FindKey(const char* key);
    int DeleteKey(const int key);
    int DeleteKey(const float key);
    int DeleteKey(const char* key);
	std::set<unsigned int> FindLessThan(const int key);
	std::set<unsigned int> FindLessThan(const float key);
	std::set<unsigned int> FindLessThan(const char* key);
	std::set<unsigned int> FindMoreThan(const int key);
	std::set<unsigned int> FindMoreThan(const float key);
	std::set<unsigned int> FindMoreThan(const char* key);

    friend BPHeader CreateIndex(const std::string& indexName, EleType type);
    inline bool IsInt(void){return this->type == INT;};
    inline bool IsFloat(void){return this->type == FLOAT;};
    inline bool IsStr(void){return this->type != INT && this->type != FLOAT;};

private:
    //Only when creating index
    BPHeader(const std::string& indexName, EleType type):indexName(indexName), type(type), rootOffset(0){}  
};


// My interfaces
BPHeader CreateIndex(const std::string& indexName, EleType type);
BPHeader OpenIndex(const std::string& indexName);

// His interfaces

void btCreate(const std::string &fileName,int type, int length=-1);

//returns -1 when fails
int btFind(const std::string &fileName,const element &key);

void btInsert(const string &fileName, const element &key, unsigned int offset);

//returns the offset of the deleted record, and -1 if fails
int btDelete(const string &fileName, const element &key);

std::set <unsigned int> btFindLess(const string &fileName, const element &key);

std::set <unsigned int> btFindMore(const string &fileName, const element &key);

#endif