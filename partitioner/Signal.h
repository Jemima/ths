#pragma once
#include <string>
#include <list>
#include "BlifNode.h"

using namespace std;

class Signal
{
public:
    Signal(string signalName);
    ~Signal(void);
    string name;
    list<BlifNode*> sources;
    list<BlifNode*> sinks;
};

