#pragma once
#include <string>
#include "BlifNode.h"
#include "Signal.h"
#include <unordered_map>


using namespace std;
class Model
{
public:
    Model();
    Model(double latency);
    Model(Model* model);
    ~Model(void);
    list<Signal*> inputs;
    list<Signal*> outputs;
    list<BlifNode*> nodes;
    unordered_map<string, Signal*> signals;
    typedef unordered_map<string, Model*> ModelMap;
    string name;
    double _latency;

    void MakeSignalList();

    void MakeIOList();

    void AddNode(BlifNode* node);

    unsigned CalculateCriticalPath();

    double CalculateLatency();

    double CalculateArea();
private:
    unsigned CalculateCriticalPath(BlifNode* node);
};

