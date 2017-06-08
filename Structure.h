#ifndef STRUCTURE_H
#define STRUCTURE_H

#include <set>
#include <string>
#include <vector>
using namespace std;


struct item { //item means attribute
	string name;
	int type;
	int length;
	bool unique;
	set<string> indices;
	bool operator ==(const item &rhs) const;
};

struct table {
	int entrySize;
	string name;
	vector <item> items;
	table (const string & name,const vector<item> &items);
	table ();
	void write();
};

class element {
public :
	int datai;
	float dataf;
	string datas;
	int type; // 0: int 1: float 2:string -1:invalid;
	bool operator < (const element &s) const ;
	bool operator == (const element &s) const ;
	bool operator > (const element &s) const ;
	bool operator >= (const element &s) const ;
	bool operator <= (const element &s) const ;
	bool operator != (const element &s) const ;
	element();
	element(int d);
	element(float d);
	element(double d);
	element(string d);
	element(const char *d);
	void print();
};

class Rule {
public:
	int index;
	int type; // 0: < 1: <= 2: = 3: >= 4: > 5: <>
	element rhs;
	Rule(int type, element rhs);
	Rule(int index,int type,element rhs);
};

class Fitter {
public:
	vector <Rule> rules;
	Fitter();
	void addRule(const Rule &rule);
	bool test(const vector <element> &data) const;
};

struct Index
{
    string index_name;
    string table_name;
    string attr_name;
    Index(string s1,string s2,string s3):index_name(s1),table_name(s2),attr_name(s3){};
};

#endif // STRUCTURE_H
