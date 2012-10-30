#pragma once
#include <string>
#include <iostream>
#include "BlifNode.h"
#include "Signal.h"
#include <unordered_map>

using namespace std;
class Model
{
public:
    Model(void);
    Model(Model* model);
    ~Model(void);
    list<Signal*> inputs;
    list<Signal*> outputs;
    list<BlifNode*> nodes;
    unordered_map<string, Signal*> signals;
    string name;

    void MakeSignalList();

    void MakeIOList();

    void AddNode(BlifNode* node);
};

