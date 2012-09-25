#pragma once
#include <list>
#include <string>
using namespace std;
class BlifNode
{
public:
    list<string> inputs;
    list<string> outputs;
    string type;
    string contents;
    BlifNode(void);
    ~BlifNode(void);
    static BlifNode* MakeNode(string type, list<string> params);
    bool AddContents(string line);

};

