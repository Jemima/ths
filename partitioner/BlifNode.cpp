#include "BlifNode.h"
#include <iostream>


BlifNode::BlifNode(void)
{
}


BlifNode::~BlifNode(void)
{
}

BlifNode* BlifNode::MakeNode(string type, list<string> params){
    static unsigned long _id = 0;
    _id++;
    BlifNode* node = new BlifNode();
    node->id = _id;
    node->type = type;
    if(type == ".names"){
        node->outputs.push_back(params.back()); //Last element is the single output
        params.pop_back();
        for each(string s in params){
            node->inputs.push_back(s);
        }
    } else if (type == ".latch"){
        node->inputs.push_front(params.front());
        params.pop_front();
        node->outputs.push_front(params.front());
        params.pop_front();
        if(params.size() >= 2){ //We have a clock specified
            params.pop_front();
            node->inputs.push_back(params.front());
        }
    } else if (type == ".subckt"){ //.subckt model-name formal1=actual1 formal2=actual2...
        cerr << ".subckt not implemented yet" << endl; // TODO: not implemented
    } else {
        cerr << "Error, unsupported node" << endl;
    }
    return node;
}

bool BlifNode::AddContents(string line){
    if(contents.length() == 0){
        contents = line;
    } else {
        if(line[0] == '.')
            return false;
        contents.append("\n");
        contents.append(line);
    }
    return true;
}