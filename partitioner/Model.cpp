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
    for each(BlifNode* node in nodes){
        delete node;
    }
    for each(pair<string, Signal*> s in signals){
        delete s.second;
    }
}


void Model::MakeSignalList(){
    list<Signal*> oldSignals;
    for each(pair<string, Signal*> s in signals){
        oldSignals.push_back(s.second);
    }
    for each(BlifNode* node in nodes){
        for each(string s in node->inputs){
            if(signals.count(s) == 0){
                signals[s] = new Signal(s);
            }
            signals[s]->sinks.push_back(node);
        }
        for each(string s in node->outputs){
            if(signals.count(s) == 0){
                signals[s] = new Signal(s);
            }
            signals[s]->sources.push_back(node);
        }
    }
    
    //If a signal appears in the input list but is never used remove it.
    for each(Signal* s in inputs){
        if(signals.count(s->name) == 0)
            inputs.remove(s);
    }
    
    //If a signal appears in the output list but is never used remove it
    for each(Signal* s in outputs){
        if(signals.count(s->name) == 0)
            outputs.remove(s);
    }

    //Delete any newly unused signals. Because of our previous two loops, they won't be in inputs or outputs either.
    for each(Signal* s in oldSignals){
        if(signals.count(s->name) == 0)
            delete s;
    }
}


void Model::AddNode(BlifNode* node){
    nodes.push_back(node);
    for each(string s in node->inputs){
        if(signals.count(s) == 0){
            signals[s] = new Signal(s);
        }
        signals[s]->sinks.push_back(node);
    }
    for each(string s in node->outputs){
        if(signals.count(s) == 0){
            signals[s] = new Signal(s);
        }
        signals[s]->sources.push_back(node);
    }
}

void Model::MakeIOList(){
    inputs.clear();
    outputs.clear();
    for each(pair<string, Signal*> s in signals){
        if(s.second->sources.size() == 0)
            inputs.push_back(s.second);
        else
            outputs.push_back(s.second); //If it's not an input always export it as an output. We can trim unused signals later, and currently we can't detect if a signal is used in another partition or not.
    }                            //Excess outputs will get stripped anyway when the circuit is flattened.
}


unsigned Model::CalculateCriticalPath(BlifNode* node){

}

unsigned Model::CalculateCriticalPath(){
}
double Model::CalculateLatency(){
    return 0;
}

double Model::CalculateArea(){
    return nodes.size();
}