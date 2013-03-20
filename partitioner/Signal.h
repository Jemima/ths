#pragma once
#include <string>
#include <list>
#include "BlifNode.h"
#include <unordered_map>

using namespace std;

class Signal
{
public:
    Signal(string signalName);
    ~Signal(void);
    string name;
    BlifNode* source;
    list<BlifNode*> sinks;
};

