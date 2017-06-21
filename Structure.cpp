#include "Structure.h"

#include <string>
#include <fstream>
#include <cassert>
#include <cmath>
#include <cstdio>

using namespace std;

/***********   table   ***********/
table::table (const string & name,const vector<item> &items) {
	entrySize=0;
	this->name=name;
	for (int i=0;i<items.size();i++) {
		switch (items[i].type) {
		case 0:entrySize+=sizeof(int);break;
		case 1:entrySize+=sizeof(float);break;
		case 2:entrySize+=items[i].length;break;
		default:assert(false);

		}
	}
	this->items=items;
}
table::table() {

}
void table::write() {
	ofstream fout((name).c_str());
	fout << name << endl;
	fout << entrySize << endl;
	fout << items.size() << endl;
	for (int i=0;i<items.size();i++) {
		fout << items[i].name << endl;
		fout << items[i].type <<endl;
		fout << items[i].length << endl;
		fout << items[i].unique << endl;
		fout << items[i].indices.size();
		for (set <string>:: iterator it=items[i].indices.begin();it!=items[i].indices.end();it++) {
			fout  << " "<< *it;
		}
		fout <<  endl;
	}
	fout.close();
}

/***********   item   ***********/
bool item::operator ==(const item &rhs) const {
	return type==rhs.type && length==rhs.length && unique==rhs.unique && indices==rhs.indices && name==rhs.name;
}

/***********   element   ***********/
bool element::operator < (const element &s) const {
	assert(this->type==s.type);
	switch (s.type) {
	case 0:return this->datai < s.datai;
	case 1:return this->dataf < s.dataf;
	case 2:return this->datas < s.datas;
	default: assert(false);
	}
}
bool element::operator == (const element &s) const {
	assert(this->type==s.type);
	switch (s.type) {
	case 0:return this->datai == s.datai;
	case 1:return fabs(this->dataf-s.dataf)<1e-7;
	case 2:return this->datas == s.datas;
	default: assert(false);
	}
}
bool element::operator > (const element &s) const {
	return !(*this<s || *this==s);
}
bool element::operator >= (const element &s) const {
	return (*this>s || *this==s);
}
bool element::operator <= (const element &s) const {
	return (*this<s || *this==s);
}
bool element::operator != (const element &s) const {
	return !(*this==s);
}
element::element():type(-1) {}
element::element(int d):datai(d),type(0) {}
element::element(float d):dataf(d),type(1) {}
element::element(double d):dataf(d),type(1) {}
element::element(string d):datas(d),type(2) {}
element::element(const char *d):datas(d),type(2) {}
void element::	 print() {
	switch (type) {
	case 0:printf("%d",datai); return;
	case 1:printf("%f",dataf); return;
	case 2:printf("%s",datas.c_str()); return;
	default: assert(false);
	}
}


/***********   Rule   ***********/
Rule::Rule(int type, element rhs):type(type),rhs(rhs){
    this->itemname = "";
}
Rule::Rule(string itemname,int type,element rhs): itemname(itemname),type(type),rhs(rhs) {

}


/***********   Fitter   ***********/
Fitter::Fitter() {

}
void Fitter::addRule(const Rule &rule) {
	rules.push_back(rule);
}

bool Fitter::test(const table &nowtable,const vector <element> &data) const{
	
	string temp;
	int index;

	for (int i=0;i<rules.size();i++) {

		//fitter有一个rules集合
		//int index=rules[i].index;

		temp = rules[i].itemname;//准备将名字翻译为下标
		for (int i = 0; i<nowtable.items.size(); i++) {
			if (nowtable.items[i].name == temp) {
				index = i;
				break;
			}
		}

		assert(index<data.size());

		switch (rules[i].type) {
		case 0:if (!(data[index]<rules[i].rhs)) return false;break;
		case 1:if (!(data[index]<=rules[i].rhs)) return false;break;
		case 2:if (!(data[index]==rules[i].rhs)) return false;break;
		case 3:if (!(data[index]>=rules[i].rhs)) return false;break;
		case 4:if (!(data[index]>rules[i].rhs)) return false;break;
		case 5:if (!(data[index]!=rules[i].rhs)) return false;break;
		default:assert(false);
		}
	}
	return true;
}
