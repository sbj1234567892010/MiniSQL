#include <cstring>
#include <cassert>
#include "bplustree.h"

BPNode::BPNode(Block &b):indexName(b.fileName)
{
    unsigned int offset = 0;
    unsigned char* data = b.data;

    // memcpy(&(this->offset), data + offset, sizeof(this->offset));
    // offset += sizeof(this->offset) / sizeof(unsigned char);
    this->offset = b.offset;
    memcpy(&head, data + offset, sizeof(head));
    offset += sizeof(head) / sizeof(unsigned char);
    memcpy(&fanOut, data + offset, sizeof(fanOut));
    offset += sizeof(fanOut) / sizeof(unsigned char);

    NodeType eleType = head >> 2;
    size_t eleSize;
    if(eleType == INT) eleSize = sizeof(int);
    else if(eleType == FLOAT) eleSize = sizeof(float);
    else eleSize = (eleType >> 1);
    
    N = (BlockSize - offset*sizeof(unsigned char) - sizeof(*childOffset)) / (sizeof(*childOffset) + eleSize) + 1;
    childOffset = new unsigned int [N];
    for(unsigned int i = 0; i < fanOut; i++)
    {
        memcpy(childOffset + i, data + offset, sizeof(*childOffset));
        offset += sizeof(*childOffset) / sizeof(unsigned char);
    }
}
BPNode::BPNode(unsigned int offset, NodeType head, const string& indexName):offset(offset), head(head), indexName(indexName)
{
	unsigned int type = head >> 2;
	size_t eleSize;
	if (type == INT)
		eleSize = sizeof(int);
	else if (type == FLOAT)
		eleSize = sizeof(float);
	else
		eleSize = type >> 1;
	unsigned int bufferOffset = (sizeof(this->head) + sizeof(fanOut)) / sizeof(unsigned char);
	N = (BlockSize - bufferOffset * sizeof(unsigned char) - sizeof(*childOffset)) / (sizeof(*childOffset) + eleSize) + 1;

	this->childOffset = new unsigned int[N];
    if(head & LEAF)
    {
        this->fanOut = 1;
        this->childOffset[0] = 0;
    }
    else this->fanOut = 0;   
}

BPIntNode::BPIntNode(Block &b):BPNode(b)
{
	assert(this->head >> 2 == INT);
    unsigned char* data = b.data;
    unsigned int offset = (sizeof(this->offset) + sizeof(head) + sizeof(fanOut)\
                          + N * sizeof(*childOffset) ) / sizeof(unsigned char);
    keys = new int[N - 1];
    for(unsigned int i = 0; i < fanOut - 1; i++)
    {
        memcpy(keys + i, data + offset, sizeof(int));           //¾­³£±¨´í
        offset += sizeof(int) / sizeof(unsigned char);
    }
}
BPIntNode::BPIntNode(unsigned int nodeOffset, NodeType head, const string& indexName):BPNode(nodeOffset, head, indexName)
{
	assert(this->head >> 2 == INT);
    keys = new int[N - 1];  
}

BPFloatNode::BPFloatNode(Block &b):BPNode(b)
{
	assert(this->head >> 2 == FLOAT);
    unsigned char* data = b.data;
    unsigned int offset = (sizeof(this->offset) + sizeof(head) + sizeof(fanOut)\
                          + N * sizeof(*childOffset) ) / sizeof(unsigned char);
    keys = new float[N - 1];
    for(unsigned int i = 0; i < fanOut - 1; i++)
    {
        memcpy(keys + i, data + offset, sizeof(float));
        offset += sizeof(float) / sizeof(unsigned char);
    }
}
BPFloatNode::BPFloatNode(unsigned int nodeOffset, NodeType head, const string& indexName):BPNode(nodeOffset, head, indexName)
{
	assert(this->head >> 2 == FLOAT);
    keys = new float[N - 1];
}

BPStrNode::BPStrNode(Block &b):BPNode(b)
{
	assert(this->head >> 2 != INT && this->head >> 2 != FLOAT);
    unsigned char* data = b.data;
    unsigned int offset = (sizeof(this->offset) + sizeof(head) + sizeof(fanOut)\
                          + N * sizeof(*childOffset) ) / sizeof(unsigned char);
    unsigned int length = (head >> 3) / sizeof(char);

    keys = new char*[N - 1];
    for(unsigned int i = 0; i < N - 1; i++)
    {
        keys[i] = new char[length];
        if(i < fanOut - 1)
        {
            memcpy(keys[i], data + offset, sizeof(char) * length);
            offset += length * sizeof(char) / sizeof(unsigned char);
        }
    }
}
BPStrNode::BPStrNode(unsigned int nodeOffset, NodeType head, const string& indexName):BPNode(nodeOffset, head, indexName)
{
	assert(this->head >> 2 != INT && this->head >> 2 != FLOAT);
    unsigned int length = (head >> 3) / sizeof(char);
    keys = new char* [N - 1];
    for(unsigned int i = 0; i < N - 1; i++)
        keys[i] = new char[length];
}

void BPIntNode::WriteNode()
{
    Block b = bmReadBlock(indexName, this->offset);

    unsigned int offset = 0;
    unsigned char* data = b.data;

    // memcpy(data + offset, &(this->offset), sizeof(this->offset));
    // offset += sizeof(this->offset) / sizeof(unsigned char);
    memcpy(data + offset, &head, sizeof(head));
    offset += sizeof(head) / sizeof(unsigned char);
    memcpy(data + offset, &fanOut, sizeof(fanOut));
    offset += sizeof(fanOut) / sizeof(unsigned char);
    for(unsigned int i = 0; i < fanOut; i++)
    {
        memcpy(data + offset, childOffset + i, sizeof(*childOffset));
        offset += sizeof(*childOffset) / sizeof(unsigned char);
    }

    offset = (sizeof(this->offset) + sizeof(head) + sizeof(fanOut)\
             + N * sizeof(*childOffset) ) / sizeof(unsigned char);
    for(unsigned int i = 0; i < fanOut - 1; i++)
    {
        memcpy(data + offset, keys + i, sizeof(int));
        offset += sizeof(int) / sizeof(unsigned char);
    }

    bmWriteBlock(b);
}

void BPFloatNode::WriteNode()
{
    Block b = bmReadBlock(indexName, this->offset);

    unsigned int offset = 0;
    unsigned char* data = b.data;

    // memcpy(data + offset, &(this->offset), sizeof(this->offset));
    // offset += sizeof(this->offset) / sizeof(unsigned char);
    memcpy(data + offset, &head, sizeof(head));
    offset += sizeof(head) / sizeof(unsigned char);
    memcpy(data + offset, &fanOut, sizeof(fanOut));
    offset += sizeof(fanOut) / sizeof(unsigned char);
    for(unsigned int i = 0; i < fanOut; i++)
    {
        memcpy(data + offset, childOffset + i, sizeof(*childOffset));
        offset += sizeof(*childOffset) / sizeof(unsigned char);
    }
    offset = (sizeof(this->offset) + sizeof(head) + sizeof(fanOut)\
             + N * sizeof(*childOffset) ) / sizeof(unsigned char);

    for(unsigned int i = 0; i < fanOut - 1; i++)
    {
        memcpy(data + offset, keys + i, sizeof(float));
        offset += sizeof(float) / sizeof(unsigned char);
    }

    bmWriteBlock(b);
}

void BPStrNode::WriteNode()
{
    Block b = bmReadBlock(indexName, this->offset);

    unsigned int offset = 0;
    unsigned char* data = b.data;
    unsigned int length = (head >> 3) / sizeof(char);

    // memcpy(data + offset, &(this->offset), sizeof(this->offset));
    // offset += sizeof(this->offset) / sizeof(unsigned char);
    memcpy(data + offset, &head, sizeof(head));
    offset += sizeof(head) / sizeof(unsigned char);
    memcpy(data + offset, &fanOut, sizeof(fanOut));
    offset += sizeof(fanOut) / sizeof(unsigned char);
    for(unsigned int i = 0; i < fanOut; i++)
    {
        memcpy(data + offset, childOffset + i, sizeof(*childOffset));
        offset += sizeof(*childOffset) / sizeof(unsigned char);
    }
    offset = (sizeof(this->offset) + sizeof(head) + sizeof(fanOut)\
             + N * sizeof(*childOffset) ) / sizeof(unsigned char);

    for(unsigned int i = 0; i < fanOut - 1; i++)
    {
        memcpy(data + offset, keys[i], sizeof(char) * length);
        offset += length * sizeof(char) / sizeof(unsigned char);
    }

    bmWriteBlock(b);
}

int BPIntNode::FindKey(const int key)
{
    if(this->isLeaf())
    {
        for(unsigned int i = 0; i < this->fanOut - 1; i++){
            if(this->keys[i] == key)
            {
                return this->childOffset[i];
            }
            else if(key < this->keys[i])
            {
                return -1;
            }
        }
        return -1;
    }
    else
    {
		unsigned int i;
        for(i = 0; i < this->fanOut - 1; i++)
        {
            if(key < this->keys[i])
				break;
        }
        Block b = bmReadBlock(this->indexName, this->childOffset[i]);
        BPNode *node = new BPIntNode(b);
        int keyOffset = node->FindKey(key);
        delete node;
        return keyOffset;
    }
}

int BPFloatNode::FindKey(const float key)
{
    if(this->isLeaf())
    {
        for(unsigned int i = 0; i < this->fanOut - 1; i++){
            if(this->keys[i] == key)
            {
                return this->childOffset[i];
            }
            else if(key < this->keys[i])
            {
                return -1;
            }
        }
        return -1;
    }
    else
    {
		unsigned int i;
        for(i = 0; i < this->fanOut - 1; i++)
        {
			if (key < this->keys[i])
				break;
        }
        Block b = bmReadBlock(this->indexName, this->childOffset[i]);
        BPNode *node = new BPFloatNode(b);
        int keyOffset = node->FindKey(key);
        delete node;
        return keyOffset;
    }
}

int BPStrNode::FindKey(const char* key)
{
    unsigned int length = this->head >> 3;
    if(this->isLeaf())
    {
        for(unsigned int i = 0; i < this->fanOut - 1; i++){
            int res = strncmp(key, this->keys[i], length);
            if(res == 0)
            {
                return this->childOffset[i];
            }
            else if(res < 0)
            {
                return -1;
            }
        }
        return -1;
    }
    else
    {
		unsigned int i;
		for (i = 0; i < this->fanOut - 1; i++)
		{
			if (strncmp(key, this->keys[i], length) < 0)
				break;
		}
		Block b = bmReadBlock(this->indexName, this->childOffset[i]);
        BPNode *node = new BPStrNode(b);
        int keyOffset = node->FindKey(key);
        delete node;
        return keyOffset;
    }
}

bool BPIntNode::InsertKey(unsigned int offset, const int key)
{
    if(this->isLeaf())
    {
        for(unsigned int i = 0; i < this->fanOut - 1; i++)
        {
            if(this->keys[i] == key) return false;
            else if(key < this->keys[i])
            {
                for(unsigned int j = this->fanOut; j > i; j--)
                {
                    this->childOffset[j] = this->childOffset[j - 1];
                    if(j < this->fanOut) this->keys[j] = this->keys[j - 1];
                }
                this->keys[i] = key;
                this->childOffset[i] = offset;
                this->fanOut++;
                this->WriteNode();
                return true;
            }
        }
        this->keys[this->fanOut-1] = key;
        this->childOffset[this->fanOut] = this->childOffset[this->fanOut-1];
        this->childOffset[this->fanOut-1] = offset;
        this->fanOut++;
        this->WriteNode();
        return true;
    }
    else
    {
        bool res;
        BPIntNode *node = nullptr;
        unsigned int i;

        for(i = 0; i < this->fanOut - 1; i++)
        {
			if (key < this->keys[i])  //this->keys[i] > key
				break;
        }
		Block b = bmReadBlock(this->indexName, this->childOffset[i]);
		node = new BPIntNode(b);
        res = node->InsertKey(offset, key);
        if(node->fanOut == N)
        {
            if(node->isLeaf())
            {
                Block newBlock = bmNewBlock(node->indexName);
                BPIntNode *newNode = new BPIntNode(newBlock.offset, node->head, node->indexName);
                newNode->fanOut = node->N - node->N / 2;
                for(unsigned int j = node->N / 2; j < node->fanOut; j++)
                {
                    newNode->childOffset[j - node->N/2] = node->childOffset[j];
                    if(j < node->fanOut - 1)newNode->keys[j - node->N/2] = node->keys[j];
                }
                node->fanOut = node->N / 2 + 1;
                node->childOffset[node->fanOut - 1] = newNode->offset;
                for(unsigned int j = this->fanOut - 1; j > i; j--)
                {
                    this->childOffset[j + 1] = this->childOffset[j];
                    this->keys[j] = this->keys[j - 1];
                }
				this->childOffset[i + 1] = newNode->offset;
                this->keys[i] = newNode->keys[0];
                this->fanOut++;

                node->WriteNode();
                newNode->WriteNode();              
                this->WriteNode();

                delete newNode;
            }
            else
            {
                Block newBlock = bmNewBlock(node->indexName);
                BPIntNode* newNode = new BPIntNode(newBlock.offset, node->head, node->indexName);
                newNode->fanOut = node->N - node->N / 2;
                for(unsigned int j = 0; j < newNode->fanOut; j++)
                {
                    newNode->childOffset[j] = node->childOffset[j + node->N / 2];
                    if(j < newNode->fanOut - 1)newNode->keys[j] = node->keys[ node->N/2 + j];
                }
                for(unsigned int j = this->fanOut-1; j > i; j--)
                {
                    this->childOffset[j+1] = this->childOffset[j];
                    this->keys[j] = this->keys[j - 1];
                }
				this->childOffset[i + 1] = newNode->offset;
                this->keys[i] = node->keys[ node->N / 2 - 1];
                node->fanOut = node->N / 2;
                this->fanOut++;

                node->WriteNode();
                newNode->WriteNode();
                this->WriteNode();
                delete newNode;
            }

            
        }
        delete node;
        return res;
    }
}

bool BPFloatNode::InsertKey(unsigned int offset, const float key)
{
	if (this->isLeaf())
	{
		for (unsigned int i = 0; i < this->fanOut - 1; i++)
		{
			if (this->keys[i] == key) return false;
			else if (key < this->keys[i])
			{
				for (unsigned int j = this->fanOut; j > i; j--)
				{
					this->childOffset[j] = this->childOffset[j - 1];
					if (j < this->fanOut) this->keys[j] = this->keys[j - 1];
				}
				this->keys[i] = key;
				this->childOffset[i] = offset;
				this->fanOut++;
				this->WriteNode();
				return true;
			}
		}
		this->keys[this->fanOut - 1] = key;
		this->childOffset[this->fanOut] = this->childOffset[this->fanOut - 1];
		this->childOffset[this->fanOut - 1] = offset;
		this->fanOut++;
		this->WriteNode();
		return true;
	}
	else
	{
		bool res;
		BPFloatNode *node = nullptr;
		unsigned int i;

		for (i = 0; i < this->fanOut - 1; i++)
		{
			if (key < this->keys[i])  //this->keys[i] > key
				break;
		}
		Block b = bmReadBlock(this->indexName, this->childOffset[i]);
		res = node->InsertKey(offset, key);
		if (node->fanOut == N)
		{
			if (node->isLeaf())
			{
				Block newBlock = bmNewBlock(node->indexName);
				BPFloatNode *newNode = new BPFloatNode(newBlock.offset, node->head, node->indexName);
				newNode->fanOut = node->N - node->N / 2;
				for (unsigned int j = node->N / 2; j < node->fanOut; j++)
				{
					newNode->childOffset[j - node->N / 2] = node->childOffset[j];
					if (j < node->fanOut - 1)newNode->keys[j - node->N / 2] = node->keys[j];
				}
				node->fanOut = node->N / 2 + 1;
				node->childOffset[node->fanOut - 1] = newNode->offset;
				for (unsigned int j = this->fanOut - 1; j > i; j--)
				{
					this->childOffset[j + 1] = this->childOffset[j];
					this->keys[j] = this->keys[j - 1];
				}
				this->childOffset[i + 1] = newNode->offset;
				this->keys[i] = newNode->keys[0];
				this->fanOut++;

				node->WriteNode();
				newNode->WriteNode();
				this->WriteNode();

				delete newNode;
			}
			else
			{
				Block newBlock = bmNewBlock(node->indexName);
				BPFloatNode* newNode = new BPFloatNode(newBlock.offset, node->head, node->indexName);
				newNode->fanOut = node->N - node->N / 2;
				for (unsigned int j = 0; j < newNode->fanOut; j++)
				{
					newNode->childOffset[j] = node->childOffset[j + node->N / 2];
					if (j < newNode->fanOut - 1)newNode->keys[j] = node->keys[node->N / 2 + j];
				}
				for (unsigned int j = this->fanOut - 1; j > i; j--)
				{
					this->childOffset[j + 1] = this->childOffset[j];
					this->keys[j] = this->keys[j - 1];
				}
				this->childOffset[i + 1] = newNode->offset;
				this->keys[i] = node->keys[node->N / 2 - 1];
				node->fanOut = node->N / 2;
				this->fanOut++;

				node->WriteNode();
				newNode->WriteNode();
				this->WriteNode();
				delete newNode;
			}
		}
		delete node;
		return res;
	}
}

bool BPStrNode::InsertKey(unsigned int offset, const char * key)
{
	unsigned int length = (this->head >> 3) / sizeof(char);
	if (this->isLeaf())
	{
		for (unsigned int i = 0; i < this->fanOut - 1; i++)
		{
			int cmpRes = strncmp(this->keys[i], key, length);
			if (cmpRes == 0) return false; //this->keys[i] == key
			else if (cmpRes > 0)			//key < this->keys[i]
			{
				for (unsigned int j = this->fanOut; j > i; j--)
				{
					this->childOffset[j] = this->childOffset[j - 1];
					if (j < this->fanOut) memcpy(this->keys[j], this->keys[j-1], length * sizeof(char));
						//this->keys[j] = this->keys[j - 1];
				}
				memcpy(this->keys[i], key, length * sizeof(char));//this->keys[i] = key;
				this->childOffset[i] = offset;
				this->fanOut++;
				this->WriteNode();
				return true;
			}
		}
		memcpy(this->keys[this->fanOut - 1], key, length * sizeof(char)); //this->keys[this->fanOut - 1] = key;
		this->childOffset[this->fanOut] = this->childOffset[this->fanOut - 1];
		this->childOffset[this->fanOut - 1] = offset;
		this->fanOut++;
		this->WriteNode();
		return true;
	}
	else
	{
		bool res;
		BPStrNode *node = nullptr;
		unsigned int i;

		for (i = 0; i < this->fanOut; i++)
		{
			if (strncmp(this->keys[i], key, length) > 0)  //this->keys[i] > key
				break;
		}
		Block b = bmReadBlock(this->indexName, this->childOffset[i]);
		node = new BPStrNode(b);
		res = node->InsertKey(offset, key);
		if (node->fanOut == N)
		{
			if (node->isLeaf())
			{
				Block newBlock = bmNewBlock(node->indexName);
				BPStrNode *newNode = new BPStrNode(newBlock.offset, node->head, node->indexName);
				newNode->fanOut = node->N - node->N / 2;
				for (unsigned int j = node->N / 2; j < node->fanOut; j++)
				{
					newNode->childOffset[j - node->N / 2] = node->childOffset[j];
					if (j < node->fanOut - 1) memcpy(newNode->keys[j - node->N / 2], node->keys[j], length * sizeof(char));
						//newNode->keys[j - node->N / 2] = node->keys[j];
				}
				node->fanOut = node->N / 2 + 1;
				node->childOffset[node->fanOut - 1] = newNode->offset;
				for (unsigned int j = this->fanOut - 1; j > i; j--)
				{
					this->childOffset[j + 1] = this->childOffset[j];
					memcpy(this->keys[j], this->keys[j - 1], length * sizeof(char));//this->keys[j] = this->keys[j - 1];
				}
				this->childOffset[i + 1] = newNode->offset;
				memcpy(this->keys[i], newNode->keys[0], length * sizeof(char));//this->keys[i] = newNode->keys[0];
				this->fanOut++;

				node->WriteNode();
				newNode->WriteNode();
				this->WriteNode();

				delete newNode;
			}
			else
			{
				Block newBlock = bmNewBlock(node->indexName);
				BPStrNode* newNode = new BPStrNode(newBlock.offset, node->head, node->indexName);
				newNode->fanOut = node->N - node->N / 2;
				for (unsigned int j = 0; j < newNode->fanOut; j++)
				{
					newNode->childOffset[j] = node->childOffset[j + node->N / 2];
					if (j < newNode->fanOut - 1) memcpy(newNode->keys[j], node->keys[node->N/2 + j], length * sizeof(char));
						//newNode->keys[j] = node->keys[node->N / 2 + j];
				}
				for (unsigned int j = this->fanOut - 1; j > i; j--)
				{
					this->childOffset[j + 1] = this->childOffset[j];
					memcpy(this->keys[j], this->keys[j - 1], length * sizeof(char)); //this->keys[j] = this->keys[j - 1];
				}
				this->childOffset[i + 1] = newNode->offset;
				memcpy(this->keys[i], node->keys[node->N / 2 - 1], length * sizeof(char)); //this->keys[i] = node->keys[node->N / 2 - 1];
				node->fanOut = node->N / 2;
				this->fanOut++;

				node->WriteNode();
				newNode->WriteNode();
				this->WriteNode();
				delete newNode;
			}
		}
		delete node;
		return res;
	}
}

int BPIntNode::DeleteKey(const int key)
{
	if (this->isLeaf())
	{
		unsigned int i;
		for (i = 0; i < this->fanOut - 1; i++)
		{
			if (this->keys[i] == key) break;
		}
		if (i == this->fanOut - 1) return -1;
		else
		{
			int retOffset = this->childOffset[i];
			for (unsigned int j = i; j < this->fanOut - 1; j++)
			{
				if(j < this->fanOut - 2)this->keys[j] = this->keys[j + 1];
				this->childOffset[j] = this->childOffset[j + 1];
			}
			this->fanOut--;

			this->WriteNode();
			return retOffset;
		}
	}
	else
	{
		unsigned int i;
		for (i = 0; i < this->fanOut - 1; i++)
		{
			if (key < this->keys[i]) break;
		}
		Block b = bmReadBlock(this->indexName, this->childOffset[i]);
		BPIntNode *child = new BPIntNode(b);
		int retOffset = child->DeleteKey(key);
		if (child->fanOut < N / 2)			
		{
			BPIntNode *rChild = nullptr;	// Find an adjacent child node to merge or rearrange the values.
			if (i < this->fanOut - 1) 
				rChild = new BPIntNode(bmReadBlock(this->indexName, this->childOffset[i + 1]));
			else
			{
				rChild = child;
				child = new BPIntNode(bmReadBlock(this->indexName, this->childOffset[this->fanOut - 2]));
				i --;	//i = this->fanOut - 2
			}		//rChild is always the right brother of 'child', and i always points at 'child'.

			if (child->isLeaf())
			{
				if (child->fanOut - 1 + rChild->fanOut - 1 < N - 1)	//merge condition. Note there is a 'next' offset in the leaf node.
				{
					child->childOffset[child->fanOut - 1] = rChild->childOffset[0];
					for (unsigned int j = 1; j < rChild->fanOut; j++)
					{
						child->childOffset[child->fanOut - 1 + j] = rChild->childOffset[j];
						child->keys[child->fanOut - 1 + j - 1] = rChild->keys[j - 1];
					}
					child->fanOut += rChild->fanOut - 1;

					for (unsigned int j = i + 1; j < this->fanOut - 1; j++)
					{
						this->childOffset[j] = this->childOffset[j + 1];
						this->keys[j - 1] = this->keys[j];
					}
					this->fanOut--;

					child->WriteNode();
					this->WriteNode();
					bmReleaseBlock(this->indexName, rChild->offset);
				}
				else	//Rearrange the offsets and keys of two children nodes.
				{
					if (child->fanOut < N / 2)	//rChild->fanOut > N/2
					{
						child->childOffset[N / 2 - 1] = child->childOffset[child->fanOut - 1];	//'Next' offset of 'child'
						for (unsigned int j = child->fanOut - 1; j < N / 2 - 1; j++)
						{
							child->childOffset[j] = rChild->childOffset[j - child->fanOut + 1];
							child->keys[j] = rChild->keys[j - child->fanOut + 1];
						}
						for (unsigned int j = N / 2 - child->fanOut; j < rChild->fanOut - 1; j++)
						{
							rChild->childOffset[j - N / 2 + child->fanOut] = rChild->childOffset[j];
							rChild->keys[j - N / 2 + child->fanOut] = rChild->keys[j];
						}					
						rChild->childOffset[rChild->fanOut + child->fanOut - N / 2 - 1] = rChild->childOffset[rChild->fanOut - 1];
					}
					else //child->fanOut > N / 2. SINCE child->fanOut + rChild->fanOur > N, it cannot happen that child->fanOut == N/2 and rChild->fanOut < N/2
					{
						rChild->childOffset[rChild->fanOut + child->fanOut - N / 2 - 1] = rChild->childOffset[rChild->fanOut - 1];
						for (unsigned int j = rChild->fanOut + child->fanOut - N / 2 - 2; j >= child->fanOut - N / 2; j--)
						{
							rChild->childOffset[j] = rChild->childOffset[j - child->fanOut + N / 2];
							rChild->keys[j] = rChild->keys[j - child->fanOut + N / 2];
						}
						for (unsigned int j = 0; j < child->fanOut - N / 2; j++)
						{
							rChild->childOffset[j] = child->childOffset[N / 2 + j];
							rChild->keys[j] = child->keys[N / 2 + j];
						}
						child->childOffset[N / 2 - 1] = child->childOffset[child->fanOut - 1];
					}
					rChild->fanOut += (child->fanOut - N / 2);
					child->fanOut = N / 2;
					this->keys[i] = rChild->keys[0];

					rChild->WriteNode();
					child->WriteNode();
					this->WriteNode();
				}
			}
			else //child is not a leaf node
			{
				if (child->fanOut + rChild->fanOut < N)	//merge condition.
				{
					child->keys[child->fanOut] = this->keys[i];
					for (unsigned int j = 0; j < rChild->fanOut; j++)
					{
						child->childOffset[child->fanOut + j] = rChild->childOffset[j];
						if (j < rChild->fanOut - 1) child->keys[child->fanOut + j] = rChild->keys[j];
					}
					child->fanOut += rChild->fanOut;
					
					for (unsigned int j = i + 1; j < this->fanOut - 1; j++)
					{
						this->childOffset[j] = this->childOffset[j + 1];
						this->keys[j - 1] = this->keys[j];
					}
					this->fanOut--;
					
					child->WriteNode();
					this->WriteNode();
					bmReleaseBlock(this->indexName, rChild->offset);
				}
				else //rearrange condition
				{
					if (child->fanOut < N / 2)
					{
						child->keys[child->fanOut - 1] = this->keys[i];
						for (unsigned int j = child->fanOut; j < N / 2; j++)
						{
							child->childOffset[j] = rChild->childOffset[j - child->fanOut];
							if (j < N / 2 - 1) child->keys[j] = rChild->keys[j - child->fanOut];
						}
						this->keys[i] = rChild->keys[N / 2 - 1 - child->fanOut];
						for (unsigned int j = 0; j < rChild->fanOut + child->fanOut - N / 2; j++)
						{
							rChild->childOffset[j] = rChild->childOffset[j + N / 2 - child->fanOut];
							if(j < rChild->fanOut + child->fanOut - N / 2 - 1)
								rChild->keys[j] = rChild->keys[j + N / 2 - child->fanOut];
						}
					}
					else
					{
						for (unsigned int j = rChild->fanOut - 1; j >= 0; j--)
						{
							rChild->childOffset[j + child->fanOut - N / 2] = rChild->childOffset[j];
							if (j < rChild->fanOut - 1)
								rChild->keys[j + child->fanOut - N / 2] = rChild->keys[j];
						}
						rChild->keys[child->fanOut - N / 2 - 1] = this->keys[i];
						for (unsigned int j = 0; j < child->fanOut - N / 2; j++)
						{
							rChild->childOffset[j] = child->childOffset[j + N / 2];
							if (j < child->fanOut - N / 2 - 1)
								rChild->keys[j] = child->keys[j + N / 2];
						}
						this->keys[i] = child->keys[N / 2];
					}
					rChild->fanOut += (child->fanOut - N / 2);
					child->fanOut = N / 2;

					rChild->WriteNode();
					child->WriteNode();
					this->WriteNode();
				}
			}

			delete rChild;
		}
		delete child;
		return retOffset;
	}
}

int BPFloatNode::DeleteKey(const float key)
{
	if (this->isLeaf())
	{
		unsigned int i;
		for (i = 0; i < this->fanOut - 1; i++)
		{
			if (this->keys[i] == key) break;
		}
		if (i == this->fanOut - 1) return -1;
		else
		{
			int retOffset = this->childOffset[i];
			for (unsigned int j = i; j < this->fanOut - 1; j++)
			{
				if (j < this->fanOut - 2)this->keys[j] = this->keys[j + 1];
				this->childOffset[j] = this->childOffset[j + 1];
			}
			this->fanOut--;
			this->WriteNode();
			return retOffset;
		}
	}
	else
	{
		unsigned int i;
		for (i = 0; i < this->fanOut - 1; i++)
		{
			if (key < this->keys[i]) break;
		}
		Block b = bmReadBlock(this->indexName, this->childOffset[i]);
		BPFloatNode *child = new BPFloatNode(b);
		int retOffset = child->DeleteKey(key);
		if (child->fanOut < N / 2)
		{
			BPFloatNode *rChild = nullptr;	// Find an adjacent child node to merge or rearrange the values.
			if (i < this->fanOut - 1)
				rChild = new BPFloatNode(bmReadBlock(this->indexName, this->childOffset[i + 1]));
			else
			{
				rChild = child;
				child = new BPFloatNode(bmReadBlock(this->indexName, this->childOffset[this->fanOut - 2]));
				i--;	//i = this->fanOut - 2
			}		//rChild is always the right brother of 'child', and i always points at 'child'.

			if (child->isLeaf())
			{
				if (child->fanOut - 1 + rChild->fanOut - 1 < N - 1)	//merge condition. Note there is a 'next' offset in the leaf node.
				{
					child->childOffset[child->fanOut - 1] = rChild->childOffset[0];
					for (unsigned int j = 1; j < rChild->fanOut; j++)
					{
						child->childOffset[child->fanOut - 1 + j] = rChild->childOffset[j];
						child->keys[child->fanOut - 1 + j - 1] = rChild->keys[j - 1];
					}
					child->fanOut += rChild->fanOut - 1;

					for (unsigned int j = i + 1; j < this->fanOut - 1; j++)
					{
						this->childOffset[j] = this->childOffset[j + 1];
						this->keys[j - 1] = this->keys[j];
					}
					this->fanOut--;

					child->WriteNode();
					this->WriteNode();
					bmReleaseBlock(this->indexName, rChild->offset);
				}
				else	//Rearrange the offsets and keys of two children nodes.
				{
					if (child->fanOut < N / 2)	//rChild->fanOut > N/2
					{
						child->childOffset[N / 2 - 1] = child->childOffset[child->fanOut - 1];	//'Next' offset of 'child'
						for (unsigned int j = child->fanOut - 1; j < N / 2 - 1; j++)
						{
							child->childOffset[j] = rChild->childOffset[j - child->fanOut + 1];
							child->keys[j] = rChild->keys[j - child->fanOut + 1];
						}
						for (unsigned int j = N / 2 - child->fanOut; j < rChild->fanOut - 1; j++)
						{
							rChild->childOffset[j - N / 2 + child->fanOut] = rChild->childOffset[j];
							rChild->keys[j - N / 2 + child->fanOut] = rChild->keys[j];
						}
						rChild->childOffset[rChild->fanOut + child->fanOut - N / 2 - 1] = rChild->childOffset[rChild->fanOut - 1];
					}
					else //child->fanOut > N / 2. SINCE child->fanOut + rChild->fanOur > N, it cannot happen that child->fanOut == N/2 and rChild->fanOut < N/2
					{
						rChild->childOffset[rChild->fanOut + child->fanOut - N / 2 - 1] = rChild->childOffset[rChild->fanOut - 1];
						for (unsigned int j = rChild->fanOut + child->fanOut - N / 2 - 2; j >= child->fanOut - N / 2; j--)
						{
							rChild->childOffset[j] = rChild->childOffset[j - child->fanOut + N / 2];
							rChild->keys[j] = rChild->keys[j - child->fanOut + N / 2];
						}
						for (unsigned int j = 0; j < child->fanOut - N / 2; j++)
						{
							rChild->childOffset[j] = child->childOffset[N / 2 + j];
							rChild->keys[j] = child->keys[N / 2 + j];
						}
						child->childOffset[N / 2 - 1] = child->childOffset[child->fanOut - 1];
					}
					rChild->fanOut += (child->fanOut - N / 2);
					child->fanOut = N / 2;
					this->keys[i] = rChild->keys[0];

					rChild->WriteNode();
					child->WriteNode();
					this->WriteNode();
				}
			}
			else //child is not a leaf node
			{
				if (child->fanOut + rChild->fanOut < N)	//merge condition.
				{
					child->keys[child->fanOut] = this->keys[i];
					for (unsigned int j = 0; j < rChild->fanOut; j++)
					{
						child->childOffset[child->fanOut + j] = rChild->childOffset[j];
						if (j < rChild->fanOut - 1) child->keys[child->fanOut + j] = rChild->keys[j];
					}
					child->fanOut += rChild->fanOut;

					for (unsigned int j = i + 1; j < this->fanOut - 1; j++)
					{
						this->childOffset[j] = this->childOffset[j + 1];
						this->keys[j - 1] = this->keys[j];
					}
					this->fanOut--;

					child->WriteNode();
					this->WriteNode();
					bmReleaseBlock(this->indexName, rChild->offset);
				}
				else //rearrange condition
				{
					if (child->fanOut < N / 2)
					{
						child->keys[child->fanOut - 1] = this->keys[i];
						for (unsigned int j = child->fanOut; j < N / 2; j++)
						{
							child->childOffset[j] = rChild->childOffset[j - child->fanOut];
							if (j < N / 2 - 1) child->keys[j] = rChild->keys[j - child->fanOut];
						}
						this->keys[i] = rChild->keys[N / 2 - 1 - child->fanOut];
						for (unsigned int j = 0; j < rChild->fanOut + child->fanOut - N / 2; j++)
						{
							rChild->childOffset[j] = rChild->childOffset[j + N / 2 - child->fanOut];
							if (j < rChild->fanOut + child->fanOut - N / 2 - 1)
								rChild->keys[j] = rChild->keys[j + N / 2 - child->fanOut];
						}
					}
					else
					{
						for (unsigned int j = rChild->fanOut - 1; j >= 0; j--)
						{
							rChild->childOffset[j + child->fanOut - N / 2] = rChild->childOffset[j];
							if (j < rChild->fanOut - 1)
								rChild->keys[j + child->fanOut - N / 2] = rChild->keys[j];
						}
						rChild->keys[child->fanOut - N / 2 - 1] = this->keys[i];
						for (unsigned int j = 0; j < child->fanOut - N / 2; j++)
						{
							rChild->childOffset[j] = child->childOffset[j + N / 2];
							if (j < child->fanOut - N / 2 - 1)
								rChild->keys[j] = child->keys[j + N / 2];
						}
						this->keys[i] = child->keys[N / 2];
					}
					rChild->fanOut += (child->fanOut - N / 2);
					child->fanOut = N / 2;

					rChild->WriteNode();
					child->WriteNode();
					this->WriteNode();
				}
			}

			delete rChild;
		}
		delete child;
		return retOffset;
	}
}

int BPStrNode::DeleteKey(const char* key)
{
	unsigned int length = this->head >> 3;
	if (this->isLeaf())
	{
		unsigned int i;
		for (i = 0; i < this->fanOut - 1; i++)
		{
			if (strncmp( this->keys[i], key, length / sizeof(char) ) == 0 )
				break;	//this->keys[i] == key
		}
		if (i == this->fanOut - 1) return -1;
		else
		{
			int retOffset = this->childOffset[i];
			for (unsigned int j = i; j < this->fanOut - 1; j++)
			{
				if (j < this->fanOut - 2)
					memcpy(this->keys[j], this->keys[j + 1], length);//this->keys[j] = this->keys[j + 1]
				this->childOffset[j] = this->childOffset[j + 1];
			}
			this->fanOut--;
			this->WriteNode();
			return retOffset;
		}
	}
	else
	{
		unsigned int i;
		for (i = 0; i < this->fanOut - 1; i++)
		{
			if (strncmp(key, this->keys[i], length / sizeof(char) ) < 0) break;		//key < this->keys[i]
		}
		Block b = bmReadBlock(this->indexName, this->childOffset[i]);
		BPStrNode *child = new BPStrNode(b);
		int retOffset = child->DeleteKey(key);
		if (child->fanOut < N / 2)
		{
			BPStrNode *rChild = nullptr;	// Find an adjacent child node to merge or rearrange the values.
			if (i < this->fanOut - 1)
				rChild = new BPStrNode(bmReadBlock(this->indexName, this->childOffset[i + 1]));
			else
			{
				rChild = child;
				child = new BPStrNode(bmReadBlock(this->indexName, this->childOffset[this->fanOut - 2]));
				i--;	//i = this->fanOut - 2
			}		//rChild is always the right brother of 'child', and i always points at 'child'.

			if (child->isLeaf())
			{
				if (child->fanOut - 1 + rChild->fanOut - 1 < N - 1)	//merge condition. Note there is a 'next' offset in the leaf node.
				{
					child->childOffset[child->fanOut - 1] = rChild->childOffset[0];
					for (unsigned int j = 1; j < rChild->fanOut; j++)
					{
						child->childOffset[child->fanOut - 1 + j] = rChild->childOffset[j];
						//child->keys[child->fanOut - 1 + j - 1] = rChild->keys[j - 1]
						memcpy(child->keys[child->fanOut - 1 + j - 1], rChild->keys[j - 1], length);
					}
					child->fanOut += rChild->fanOut - 1;

					for (unsigned int j = i + 1; j < this->fanOut - 1; j++)
					{
						this->childOffset[j] = this->childOffset[j + 1];
						//this->keys[j - 1] = this->keys[j]
						memcpy(this->keys[j - 1], this->keys[j], length);
					}
					this->fanOut--;

					child->WriteNode();
					this->WriteNode();
					bmReleaseBlock(this->indexName, rChild->offset);
				}
				else	//Rearrange the offsets and keys of two children nodes.
				{
					if (child->fanOut < N / 2)	//rChild->fanOut > N/2
					{
						child->childOffset[N / 2 - 1] = child->childOffset[child->fanOut - 1];	//'Next' offset of 'child'
						for (unsigned int j = child->fanOut - 1; j < N / 2 - 1; j++)
						{
							child->childOffset[j] = rChild->childOffset[j - child->fanOut + 1];
							//child->keys[j] = rChild->keys[j - child->fanOut + 1]
							memcpy(child->keys[j], rChild->keys[j - child->fanOut + 1], length);
						}
						for (unsigned int j = N / 2 - child->fanOut; j < rChild->fanOut - 1; j++)
						{
							rChild->childOffset[j - N / 2 + child->fanOut] = rChild->childOffset[j];
							//rChild->keys[j - N / 2 + child->fanOut] = rChild->keys[j]
							memcpy(rChild->keys[j - N / 2 + child->fanOut], rChild->keys[j], length);
						}
						rChild->childOffset[rChild->fanOut + child->fanOut - N / 2 - 1] = rChild->childOffset[rChild->fanOut - 1];
					}
					else //child->fanOut > N / 2. SINCE child->fanOut + rChild->fanOur > N, it cannot happen that child->fanOut == N/2 and rChild->fanOut < N/2
					{
						rChild->childOffset[rChild->fanOut + child->fanOut - N / 2 - 1] = rChild->childOffset[rChild->fanOut - 1];
						for (unsigned int j = rChild->fanOut + child->fanOut - N / 2 - 2; j >= child->fanOut - N / 2; j--)
						{
							rChild->childOffset[j] = rChild->childOffset[j - child->fanOut + N / 2];
							//rChild->keys[j] = rChild->keys[j - child->fanOut + N / 2]
							memcpy(rChild->keys[j], rChild->keys[j - child->fanOut + N / 2], length);
						}
						for (unsigned int j = 0; j < child->fanOut - N / 2; j++)
						{
							rChild->childOffset[j] = child->childOffset[N / 2 + j];
							//rChild->keys[j] = child->keys[N / 2 + j]
							memcpy(rChild->keys[j], child->keys[N / 2 + j], length);
						}
						child->childOffset[N / 2 - 1] = child->childOffset[child->fanOut - 1];
					}
					rChild->fanOut += (child->fanOut - N / 2);
					child->fanOut = N / 2;
					//this->keys[i] = rChild->keys[0]
					memcpy(this->keys[i], rChild->keys[0], length);

					rChild->WriteNode();
					child->WriteNode();
					this->WriteNode();
				}
			}
			else //child is not a leaf node
			{
				if (child->fanOut + rChild->fanOut < N)	//merge condition.
				{
					//child->keys[child->fanOut] = this->keys[i]
					memcpy(child->keys[child->fanOut], this->keys[i], length);
					for (unsigned int j = 0; j < rChild->fanOut; j++)
					{
						child->childOffset[child->fanOut + j] = rChild->childOffset[j];
						if (j < rChild->fanOut - 1) //child->keys[child->fanOut + j] = rChild->keys[j]
							memcpy(child->keys[child->fanOut + j], rChild->keys[j], length);
					}
					child->fanOut += rChild->fanOut;

					for (unsigned int j = i + 1; j < this->fanOut - 1; j++)
					{
						this->childOffset[j] = this->childOffset[j + 1];
						//this->keys[j - 1] = this->keys[j]
						memcpy(this->keys[j - 1], this->keys[j], length);
					}
					this->fanOut--;

					child->WriteNode();
					this->WriteNode();
					bmReleaseBlock(this->indexName, rChild->offset);
				}
				else //rearrange condition
				{
					if (child->fanOut < N / 2)
					{
						//child->keys[child->fanOut - 1] = this->keys[i]
						memcpy(child->keys[child->fanOut - 1], this->keys[i], length);
						for (unsigned int j = child->fanOut; j < N / 2; j++)
						{
							child->childOffset[j] = rChild->childOffset[j - child->fanOut];
							if (j < N / 2 - 1) //child->keys[j] = rChild->keys[j - child->fanOut]
								memcpy(child->keys[j], rChild->keys[j - child->fanOut], length);
						}
						//this->keys[i] = rChild->keys[N / 2 - 1 - child->fanOut]
						memcpy(this->keys[i], rChild->keys[N / 2 - 1 - child->fanOut], length);
						for (unsigned int j = 0; j < rChild->fanOut + child->fanOut - N / 2; j++)
						{
							rChild->childOffset[j] = rChild->childOffset[j + N / 2 - child->fanOut];
							if (j < rChild->fanOut + child->fanOut - N / 2 - 1)
								//rChild->keys[j] = rChild->keys[j + N / 2 - child->fanOut]
								memcpy(rChild->keys[j], rChild->keys[j + N / 2 - child->fanOut], length);
						}
					}
					else
					{
						for (unsigned int j = rChild->fanOut - 1; j >= 0; j--)
						{
							rChild->childOffset[j + child->fanOut - N / 2] = rChild->childOffset[j];
							if (j < rChild->fanOut - 1)
								//rChild->keys[j + child->fanOut - N / 2] = rChild->keys[j]
								memcpy(rChild->keys[j + child->fanOut - N / 2], rChild->keys[j], length);
						}
						//rChild->keys[child->fanOut - N / 2 - 1] = this->keys[i]
						memcpy(rChild->keys[child->fanOut - N / 2 - 1], this->keys[i], length);
						for (unsigned int j = 0; j < child->fanOut - N / 2; j++)
						{
							rChild->childOffset[j] = child->childOffset[j + N / 2];
							if (j < child->fanOut - N / 2 - 1)
								//rChild->keys[j] = child->keys[j + N / 2]
								memcpy(rChild->keys[j], child->keys[j + N / 2], length);
						}
						//this->keys[i] = child->keys[N / 2]
						memcpy(this->keys[i], child->keys[N / 2], length);
					}
					rChild->fanOut += (child->fanOut - N / 2);
					child->fanOut = N / 2;

					rChild->WriteNode();
					child->WriteNode();
					this->WriteNode();
				}
			}

			delete rChild;
		}
		delete child;
		return retOffset;
	}
}