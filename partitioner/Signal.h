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
    list<BlifNode*> sources;
    list<BlifNode*> sinks;
    typedef unordered_map<string, Signal*> SignalMap;
};

