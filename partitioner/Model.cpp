#include "Model.h"


Model::Model(void)
{
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
}
