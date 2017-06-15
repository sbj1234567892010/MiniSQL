#ifndef _BPLUSTREE_H
#define _BPLUSTREE_H

#include <string>
#include "buffermanager.h"

const int MAX_N (4);

typedef unsigned int NodeType;
const NodeType ROOT = 2; 
// const NodeType ITN = 0; //internal node
const NodeType LEAF = 1;

typedef unsigned int EleType;
const EleType INT = 0;
const EleType FLOAT = 1;
//CHAR(N) = N << 1;


struct BPNode
{
    unsigned int offset;
    NodeType head; //head&2 -> isRoot;  head&1 -> isLeaf; head>>2 -> EleType
    unsigned int fanOut;
    unsigned int *childOffset;
    const string indexName;

    unsigned int N;
    
    BPNode(Block &b);
    virtual ~BPNode() { delete[] childOffset; }
    virtual void WriteNode() = 0;
    virtual int FindKey(const int key){return -1;}
    virtual int FindKey(const float key){return -1;}
    virtual int FindKey(const char* key){return -1;}
    virtual bool InsertKey(unsigned int offset, const int key){ return false;}
    virtual bool InsertKey(unsigned int offset, const float key){ return false;}
    virtual bool InsertKey(unsigned int offset, const char* key){ return false;}
	virtual int DeleteKey(const int key) { return -1; }
	virtual int DeleteKey(const float key) { return -1; }
	virtual int DeleteKey(const char* key) { return -1; }
    inline bool isLeaf(void) {return this->head & 1;}
    inline bool isRoot(void) {return this->head & 2;}
    inline unsigned int EleType(void){return this->head >> 2;}

    BPNode(unsigned int offset, NodeType head, const string& indexName);
};

struct BPIntNode : public BPNode
{
    int *keys;

    BPIntNode(Block&);
    virtual ~BPIntNode() {delete[] keys;}
    virtual void WriteNode();
    virtual int FindKey(const int key);
    virtual bool InsertKey(unsigned int offset, const int key);
	virtual int DeleteKey(const int key);

    BPIntNode(unsigned int offset, NodeType head, const string& indexName);
};

struct BPFloatNode : public BPNode
{
    float *keys;

    BPFloatNode(Block&);
    virtual ~BPFloatNode() {delete[] keys;}
    virtual void WriteNode();
    virtual int FindKey(const float key);
    virtual bool InsertKey(unsigned int offset, const float key);
	virtual int DeleteKey(const float key);

    BPFloatNode(unsigned int offset, NodeType head, const string& indexName);
};

struct BPStrNode : public BPNode
{
    char **keys;

    BPStrNode(Block&);
    virtual ~BPStrNode() {
        for(unsigned int i = 0; i < fanOut; i++) 
            delete[] keys[i];
        delete[] keys;
    }
    virtual void WriteNode();
    virtual int FindKey(const char* key);
    virtual bool InsertKey(unsigned int offset, const char* key);
	virtual int DeleteKey(const char* key);

    BPStrNode(unsigned int offset, NodeType head, const string& indexName);
};

#endif