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
    
    //As per the two parameter version, with updateCost defaulting to true
    void AddNode(BlifNode* node);
    //node is the node the add, updateCost is whether to update the latencies associated with each node.
    //If the graph contains any cycles after the node is added updateCost _MUST_ be false, as max latency is always infinite.
    void AddNode(BlifNode* node, bool shouldUpdateCosts);

    unsigned CalculateCriticalPath();

    double CalculateLatency();

    double CalculateArea();
private:
    unsigned CalculateCriticalPath(BlifNode* node, unordered_map<int, unsigned> &visited);
    void Model::updateCosts(BlifNode* node, unsigned costToReach);
};

