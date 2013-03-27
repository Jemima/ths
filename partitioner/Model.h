#pragma once
#include <string>
#include "BlifNode.h"
#include "Signal.h"
#include <unordered_map>
#include <set>


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
    set<BlifNode*> nodes;
    unordered_map<string, Signal*> signals;
    string name;
    double _latency;

    void MakeSignalList(bool cutLoops = true);

    void MakeIOList();

    //Cuts the input from this node, indicating the input should be external to this submodel.
    void Cut(BlifNode* node);
    
    void AddNode(BlifNode* node);

    unsigned CalculateCriticalPath();

    double CalculateLatency();

    double CalculateArea();

    Signal* GetBaseSignal(string name);

    void CutLoops();

    double RecoveryTime(double voterArea);

    double CalculateReconfigurationTime(double voterArea);

    void SetDotFile(string path);
private:
    unsigned CalculateCriticalPath(BlifNode* node, unordered_map<int, unsigned> &visited);
    void updateCosts(BlifNode* node, unsigned costToReach);
    void CutLoopsRecurse(BlifNode* parent, Signal* signal);
    unsigned maxCost;
};

