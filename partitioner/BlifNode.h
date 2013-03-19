#pragma once
#include <set>
#include <list>
#include <string>
using namespace std;
class BlifNode
{
public:
    list<string> inputs;
    string output;
    string clock;
    string type;
    string contents;
    unsigned long id;
    BlifNode(void);
    ~BlifNode(void);
    static BlifNode* MakeNode(string type, list<string> params);
    bool AddContents(string line);
    unsigned cost;
    string GetText();
    BlifNode* Clone();
    bool operator==(const BlifNode &other) const;
    bool operator!=(const BlifNode &other) const;
private:
   static int counter;
};

