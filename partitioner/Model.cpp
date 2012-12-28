#include "Model.h"

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
    for(BlifNode* node : nodes){
        delete node;
    }
    for(pair<string, Signal*> s : signals){
        delete s.second;
    }
}


void Model::MakeSignalList(){
    list<Signal*> oldSignals;
    for(pair<string, Signal*> s : signals){
        oldSignals.push_back(s.second);
    }
    for(BlifNode* node : nodes){
        for(string s : node->inputs){
            if(signals.count(s) == 0){
                signals[s] = new Signal(s);
            }
            signals[s]->sinks.push_back(node);
        }
        for(string s : node->outputs){
            if(signals.count(s) == 0){
                signals[s] = new Signal(s);
            }
            signals[s]->sources.push_back(node);
        }
    }
    
    //If a signal appears in the input list but is never used remove it.
    for(Signal* s : inputs){
        if(signals.count(s->name) == 0)
            inputs.remove(s);
    }
    
    //If a signal appears in the output list but is never used remove it
    for(Signal* s : outputs){
        if(signals.count(s->name) == 0)
            outputs.remove(s);
    }

    //Delete any newly unused signals. Because of our previous two loops, they won't be in inputs or outputs either.
    for(Signal* s : oldSignals){
        if(signals.count(s->name) == 0)
            delete s;
    }
}


void Model::AddNode(BlifNode* node){
    nodes.push_back(node);
    for(string s : node->inputs){
        if(signals.count(s) == 0){
            signals[s] = new Signal(s);
        }
        signals[s]->sinks.push_back(node);
    }
    for(string s : node->outputs){
        if(signals.count(s) == 0){
            signals[s] = new Signal(s);
        }
        signals[s]->sources.push_back(node);
    }
}

void Model::MakeIOList(){
    inputs.clear();
    outputs.clear();
    for(pair<string, Signal*> s : signals){
        if(s.second->sinks.size() == 0)
            outputs.push_back(s.second);
        if(s.second->sources.size() == 0)
            inputs.push_back(s.second);
    }
}


unsigned Model::CalculateCriticalPath(){
    return 1;
}
double Model::CalculateLatency(){
    return 0;
}

double Model::CalculateArea(){
    return nodes.size();
}
