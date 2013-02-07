#include "Model.h"
#include <boost/foreach.hpp>
#include <algorithm>

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


set<string> cycles;
void Model::MakeSignalList(bool cutLoops){
    pair<string, Signal*> signalPair;
    BOOST_FOREACH(signalPair, signals){
        delete signalPair.second;
    }
    signals.clear();
    if(cutLoops){
        BOOST_FOREACH(BlifNode* node, nodes){
        #pragma warning(suppress : 6246) // Keep Visual Studio from complaining about duplicate declaration as part of the nested FOREACH macro
            BOOST_FOREACH(string sig, cycles){
                replace(node->inputs.begin(), node->inputs.end(), sig, "qqrinput"+sig);
                replace(node->outputs.begin(), node->outputs.end(), sig, "qqroutput"+sig);
                /*
                list<string>::iter = node->inputs.find(sig);
                if(node->inputs.find(sig) != node->inputs.end()){
                    node->inputs.remove(sig);
                    node->inputs.insert("qqrinput"+sig);
                }
                if(node->outputs.find(sig) != node->outputs.end()){
                    node->outputs.remove(sig);
                    node->outputs.insert("qqroutput"+sig);
                }*/
            }
        }
    }
    
    BOOST_FOREACH(BlifNode* node, nodes){
    #pragma warning(suppress : 6246) // Keep Visual Studio from complaining about duplicate declaration as part of the nested FOREACH macro
        BOOST_FOREACH(string s, node->inputs){
            string sigName = s;
            if(signals.count(s) == 0){
                signals[sigName] = new Signal(sigName);
            }
            signals[sigName]->sinks.push_back(node);
        }
    #pragma warning(suppress : 6246) // Keep Visual Studio from complaining about duplicate declaration as part of the nested FOREACH macro
        BOOST_FOREACH(string s, node->outputs){
            string sigName = s;
            if(signals.count(s) == 0){
                signals[sigName] = new Signal(sigName);
            }
            signals[sigName]->sources.push_back(node);
        }
    }
    
    MakeIOList();
}

unordered_map<unsigned long, unsigned> maxLatencies;
unordered_map<unsigned long, bool> visited;


void Model::AddNode(BlifNode* node){
    AddNode(node, true);
}

void Model::Cut(BlifNode* node){
    BOOST_FOREACH(string s, node->inputs){ //Remove all inputs from this node
        signals.erase(s);
    }
}

void Model::AddNode(BlifNode* node, bool shouldDetectLoops){
    nodes.insert(node);
    unsigned maxInCost = 0;
    BOOST_FOREACH(string s, node->inputs){ //Need to not add a signal if it causes a cycle
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
    if(shouldDetectLoops == false) //Adding a no-cost node can't increase the max path, so skip calculating the changes.
        return; //We do our loop detection in here, since we're already traversing and explicitly testing for cycles. Therefore need to always traverse.
    visited.clear();
    updateCosts(node, maxInCost);
}
unsigned maxCost = 0;
void Model::updateCosts(BlifNode* node, unsigned costToReach){
    if(visited[node->id] == true){ //Already been here, we need to cut the loop.
        cycles.insert(*node->inputs.begin()); //Can't modify nodes or signals while looping, so save them to cut afterwards.
        return;
    }
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
    //All feedback cycles must include a latch, therefore a node which can be reached through a loop must have a greater cost to reach than the first time we reached it.
            //Therefore we can skip nodes of the same cost.
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
    return _latency;
}

double Model::CalculateArea(){
    return nodes.size();
}



Signal* Model::GetBaseSignal(string name){
    if(name[0] == 'q' && name[1] == 'q'){ //Special meaning signal. Translate into what it is in the untouched original
        if(name[3] == 'i'){ //qqrinput
            string realName = name.substr(8, string::npos);
            return this->signals[realName];
        } else if (name[3] == 'o'){ //qqroutput
            string realName = name.substr(9, string::npos);
            return this->signals[realName];
        }
    } else {
        return this->signals[name];
    }

}