#include<iostream>
#include<string>
#include<cstdlib>
#include<cstring>
#include<cstdio>
#include<sstream>
#include "interpreter.h"
#include "api.h"
//#include "table.h"
//#include "interface.h"
//#include "catalogmanager.h"

using namespace std;

void LinetoQuery(string line)
{
    static string query = "";
    for(int i=0;i < line.length();i++)
    {
        if(line[i]==';')
        {
            InterpretQuery(query);
            query = "";
        }
        else query += line[i];
    }
}

void InterpretQuery(string query)
{
    if(query == "quit")
    {
        cout << "Bye~" << endl;
        exit(0);
    }
    for(int i=0;i < query.length();i++)
    {
        if(query[i]=='(' || query[i]==')' || query[i]==',')
        {
            query.insert(i," ");
            query.insert(i+2," ");
            i+=2;
        }
    }
    stringstream querystream;
    querystream << query;
    string operation;
    querystream >> operation;
    if(operation == "create")
    {
        string object;
        querystream >> object;
        if(object == "table")
        {
            ProcessCreateTable(querystream);
        }
        else if(object == "index")
        {
            ProcessCreateIndex(querystream);
        }
        else{
            cout << "Syntax Error!" << endl;
        }
    }
    else if(operation == "drop")
    {
        string object;
        querystream >> object;
        if(object == "table")
        {
            ProcessDropTable(querystream);
        }
        else if(object == "index")
        {
            ProcessDropIndex(querystream);
        }
        else{
            cout << "Syntax Error!" << endl;
        }
    }
    else if(operation == "insert")
    {
        ProcessInsert(querystream);
    }
    else if(operation == "delete")
    {
        ProcessDelete(querystream);
    }
    else if(operation == "select")
    {
        ProcessSelect(querystream);
    }
    else if(operation == "execfile")
    {
        ProcessExecfile(querystream);
    }
    else{
        cout << "Syntax Error!" << endl;
    }
}

void ProcessCreateTable(stringstream& querystream)
{
    string tablename;
    string temp;
    querystream >> tablename;
    querystream >> temp;
    //cout << tablename << endl;

    vector <item> data;
    int pk=0x3f3f3f3f;
    while (1) {
        string itemname;
        querystream>>itemname;
        if (itemname=="primary") {
            querystream>>temp;
            querystream>>temp;
            string pkname;
            querystream>> pkname;
            for (int i=0;i<data.size();i++) {
                if (data[i].name==pkname)
                    pk=i;
            }
            break;
        }

        string datatype;
        int length=0;
        querystream>>datatype;
        if (datatype=="char") {
            querystream>>temp;
            querystream>>length;
            querystream>>temp;
        }
        string unique;
        querystream>>unique;
        item newitem;
        if (unique=="unique") {
            newitem.unique=true;
            querystream>>temp;
        } else {
            newitem.unique=false;
        }
        newitem.name=itemname;
        if (datatype=="char") {
            newitem.type=2;
            newitem.length=length;
        } else if (datatype=="int"){
            newitem.type=0;
            newitem.length=0;
        } else if (datatype=="float"){
            newitem.type=1;
            newitem.length=0;
        } else {
            printf("unrecognized type %s\n",datatype.c_str()) ;
            return ;
        }
        data.push_back(newitem);
    }
    if (pk==0x3f3f3f3f) {
        cout << "primary key wrong" << endl;
    }
    else {
        table table1(tablename,data);
        CreateTable(table1, pk);
    }
}

//primary key is necessary



void ProcessCreateIndex(stringstream& querystream)
{
    string indexname;
    string tablename;
    string itemname;
    string on;
    string temp;
    querystream >> indexname >> on >> tablename >> temp >> itemname;
    if(on != "on")
    {
        cout << "Syntax Error!" << endl;
        return;
    }
    Index idx(indexname, tablename, itemname);
    CreateIndex(idx);
}

void ProcessDropTable(stringstream& querystream)
{
    string tablename;
    querystream >> tablename;
    DropTable(tablename);
}

void ProcessDropIndex(stringstream& querystream)
{
    string indexname;
    string tablename;
    string on;
    querystream >> indexname >> on >> tablename;
    if(on != "on")
    {
        cout << "Syntax Error!" << endl;
        return;
    }
    DropIndex(indexname,tablename);
}


void ProcessInsert(stringstream& querystream)
{
    string tablename;
    string into;
    string values;
    string temp;
    querystream >> into >> tablename >> values >> temp;
    if (into != "into" || values != values )
    {
        cout << "Syntax Error!" << endl;
        return;
    }
    vector <element> entry;
    while (1) {
        string Element;
        querystream >> Element;
        entry.push_back(ProcessElement(Element));
        querystream >>temp;
        if (temp==")")
            break;
    }
    Insert(tablename,entry);
}

void ProcessDelete(stringstream& querystream)
{
    string tablename;
    string from;
    string where;
    querystream >> from >> tablename >> where;
    if (from != "from" || where != "where" )
    {
        cout << "Syntax Error!" << endl;
        return;
    }
    Fitter fitter;
    string And;
    do{
        string itemname;
        string oper;
        string data;
        querystream >> itemname >> oper >> data;
        int op;
        if (oper=="<") {
            op=0;
        } else if (oper=="<=") {
            op=1;
        } else if (oper=="=") {
            op=2;
        } else if (oper==">=") {
            op=3;
        } else if (oper==">") {
            op=4;
        } else if (oper=="<>") {
            op=5;
        } else {
            printf("Operation Error\n");
            return ;
        }
        fitter.addRule(Rule(op,ProcessElement(data)));
    }
    while(querystream >> And);
    Delete(tablename,fitter);
}

void ProcessSelect(stringstream& querystream)
{
    vector <string> SelectItem;
    string selectname;
    querystream >> selectname;
    SelectItem.push_back(selectname);
    string temp;
    if(selectname != "*")
    {
        querystream >> temp;
        while(temp == ",")
        {
            querystream >> selectname;
            SelectItem.push_back(selectname);
            querystream >> temp;
        }
    }
    else {
        querystream >> temp;
    }
    if(temp != "from")
    {
        cout << "Syntax Error." << endl;
        return;
    }

    string tablename;
    string where;
    Fitter fitter;
    querystream >> tablename ;
    if ( !(querystream >> where))// no where clause
    {
        Select(SelectItem,tablename,fitter);
        return;
    }
    if (where != "where" )
    {
        cout << "Syntax Error!" << endl;
        return;
    }
    string And;
    do{
        string itemname;
        string oper;
        string data;
        querystream >> itemname >> oper >> data;
        int op;
        if (oper=="<") {
            op=0;
        } else if (oper=="<=") {
            op=1;
        } else if (oper=="=") {
            op=2;
        } else if (oper==">=") {
            op=3;
        } else if (oper==">") {
            op=4;
        } else if (oper=="<>") {
            op=5;
        } else {
            printf("Operation Error\n");
            return ;
        }
        fitter.addRule(Rule(op,ProcessElement(data)));
    }
    while(querystream >> And);
    Select(SelectItem,tablename,fitter);
}

element ProcessElement(string Element)
{
    if(Element[0]=='\'')//char
    {
        string str = Element.substr(1,Element.length()-2);
        //cout << str << endl;
        return str;
    }
    else if(Element.find(".") != string::npos)//float
    {
        return stringToNum<float>(Element);
    }
    else{//int
        return stringToNum<int>(Element);
    }
}

void ProcessExecfile(stringstream& querystream)
{
    string filename;
    querystream >> filename;
    cout << filename << endl;
    FILE *fp=fopen(filename.c_str(),"r");
	if (!fp)
		printf("no such file\n");
	else {
		char line[1024];
		while (fgets(line,1024,fp)) {
			int len=strlen(line);
			if (line[len-1]=='\n') {
				line[len-1]=0;
			}
			ExecfileLinetoQuery(line);
		}
	}
}
void ExecfileLinetoQuery(string line)
{
    static string query = "";
    for(int i=0;i < line.length();i++)
    {
        if(line[i]==';')
        {
            InterpretQuery(query);
            query = "";
        }
        else query += line[i];
    }
}
