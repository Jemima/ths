#include "Model.h"
#include <boost/foreach.hpp>
#include <bitset>

Model::Model()
{
    _latency = 0;
}
Model::Model(double latency)
{
    _latency = latency;
}

Model::Model(Model* model)
{
    name = model->name;
    _latency = model->_latency;
}


Model::~Model(void)
{
    BOOST_FOREACH(BlifNode* node, nodes){
        delete node;
    }
    pair<string, Signal*> s;
    BOOST_FOREACH(s, signals){
        delete s.second;
    }
}


void Model::MakeSignalList(){
    //list<Signal*> oldSignals;
    signals.clear();
    pair<string, Signal*> signalPair;
 /*   BOOST_FOREACH(signalPair, signals){
        oldSignals.push_back(signalPair.second);
    }*/
    BOOST_FOREACH(BlifNode* node, nodes){
    #pragma warning(suppress : 6246) // Keep Visual Studio from complaining about duplicate declaration as part of the nested FOREACH macro
        BOOST_FOREACH(string s, node->inputs){
            if(signals.count(s) == 0){
                signals[s] = new Signal(s);
            }
            signals[s]->sinks.push_back(node);
        }
    #pragma warning(suppress : 6246) // Keep Visual Studio from complaining about duplicate declaration as part of the nested FOREACH macro
        BOOST_FOREACH(string s, node->outputs){
            if(signals.count(s) == 0){
                signals[s] = new Signal(s);
            }
            signals[s]->sources.push_back(node);
        }
    }
    
    //If a signal appears in the input list but is never used remove it.
    BOOST_FOREACH(Signal* s, inputs){
        if(signals.count(s->name) == 0)
            inputs.remove(s);
    }
    
    //If a signal appears in the output list but is never used remove it
    BOOST_FOREACH(Signal* s, outputs){
        if(signals.count(s->name) == 0)
            outputs.remove(s);
    }

    //Delete any newly unused signals. Because of our previous two loops, they won't be in inputs or outputs either.
  /*  BOOST_FOREACH(Signal* s, oldSignals){
        if(signals.count(s->name) == 0)
            delete s;
    }*/
}

unordered_map<unsigned long, unsigned> maxLatencies;
unordered_map<unsigned long, bool> visited;


void Model::AddNode(BlifNode* node){
    AddNode(node, true);
}

void Model::AddNode(BlifNode* node, bool shouldUpdateCosts){
    nodes.push_back(node);
    unsigned maxInCost = 0;
    BOOST_FOREACH(string s, node->inputs){
        if(signals.count(s) == 0){
            signals[s] = new Signal(s);
        }
        signals[s]->sinks.push_back(node);
    #pragma warning(suppress : 6246) // Keep Visual Studio from complaining about duplicate declaration as part of the nested FOREACH macro
        BOOST_FOREACH(BlifNode* node, signals[s]->sources){
            if(maxLatencies[node->id] > maxInCost)
                maxInCost = maxLatencies[node->id];
        }
    }
    maxLatencies[node->id] = maxInCost+node->cost;
    BOOST_FOREACH(string s, node->outputs){
        if(signals.count(s) == 0){
            signals[s] = new Signal(s);
        }
        signals[s]->sources.push_back(node);
    }
    if(node->cost == 0 || shouldUpdateCosts == false) //Adding a no-cost node can't increase the max path, so skip calculating the changes.
        return;
    visited.clear();
    updateCosts(node, maxInCost);
}
unsigned maxCost = 0;
void Model::updateCosts(BlifNode* node, unsigned costToReach){
    if(visited[node->id] == true)
        return;
    visited[node->id] = true;
    costToReach += node->cost;
    maxLatencies[node->id] = costToReach;
    if(costToReach > maxCost)
        maxCost = costToReach;
    BOOST_FOREACH(string s, node->outputs){
    #pragma warning(suppress : 6246) // Keep Visual Studio from complaining about duplicate declaration as part of the nested FOREACH macro
        BOOST_FOREACH(BlifNode* newNode, this->signals[s]->sinks){
            if(newNode->id == node->id)
                continue;
            if(maxLatencies[newNode->id] >= costToReach+newNode->cost)
                continue; //If it won't increase the cost skip it
            updateCosts(newNode, costToReach);
        }
    }
}

void Model::MakeIOList(){
    inputs.clear();
    outputs.clear();
    pair<string, Signal*> s;
    BOOST_FOREACH(s, signals){
        if(s.second->sources.size() == 0)
            inputs.push_back(s.second);
        else
            outputs.push_back(s.second); //If it's not an input always export it as an output. We can trim unused signals later, and currently we can't detect if a signal is used in another partition or not.
    }                            //Excess outputs will get stripped anyway when the circuit is flattened.
}

unsigned Model::CalculateCriticalPath(){
    return maxCost;
}
double Model::CalculateLatency(){
    return CalculateCriticalPath();
}

double Model::CalculateArea(){
    return nodes.size();
}
