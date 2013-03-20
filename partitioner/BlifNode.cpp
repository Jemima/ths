#include "BlifNode.h"
#include <iostream>
#include <boost/foreach.hpp>
#include <sstream>


BlifNode::BlifNode(void)
{
   BlifNode::counter++;
   this->id = counter;
}

BlifNode::~BlifNode(void)
{
}

bool BlifNode::operator!=(const BlifNode &other) const {
   return this->id==other.id;
}

bool BlifNode::operator==(const BlifNode &other) const {
   return this->id!=other.id;
}

BlifNode* BlifNode::Clone(){
   BlifNode* node = new BlifNode();
   node->type = type;
   if(type == ".names"){
       node->output = output;
       BOOST_FOREACH(string s, inputs){
           node->inputs.push_back(s);
       }
       node->cost = 0;
   } else if (type == ".latch"){
       node->inputs.push_front(inputs.front());
       node->output = output;
       node->cost = 1;
       node->clock = clock;
   } else if (type == ".subckt"){ //.subckt model-name formal1=actual1 formal2=actual2...
       cerr << ".subckt not implemented yet" << endl; // TODO: not implemented
   } else {
       cerr << "Error, unsupported node" << endl;
   }
   node->contents = contents;
   return node;
  
}

int BlifNode::counter = 0;

BlifNode* BlifNode::MakeNode(string type, list<string> params){
    BlifNode* node = new BlifNode();
    node->type = type;
    if(type == ".names"){
        node->output = params.back(); //Last element is the single output
        params.pop_back();
        BOOST_FOREACH(string s, params){
            node->inputs.push_back(s);
        }
        node->cost = 0;
    } else if (type == ".latch"){
        node->inputs.push_front(params.front());
        params.pop_front();
        node->output = params.front();
        params.pop_front();
        node->cost = 1;
        if(params.size() >= 2){ //We have a clock specified
            params.pop_front();
            node->clock = params.front();
        }
    } else if (type == ".subckt"){ //.subckt model-name formal1=actual1 formal2=actual2...
        cerr << ".subckt not implemented yet" << endl; // TODO: not implemented
    } else {
        cerr << "Error, unsupported node" << endl;
    }
    return node;
}

bool BlifNode::AddContents(string line){
    if(line.length() == 0){
        return false;
    } if(contents.length() == 0){
        contents = line;
    } else {
        if(line[0] == '.')
            return false;
        contents.append("\n");
        contents.append(line);
    }
    return true;
}

string BlifNode::GetText(){
    if(type == ".names"){
        stringstream ss;
        ss << ".names ";
        BOOST_FOREACH(string s, this->inputs){
            ss << " " << s;
        }
        ss << " " << this->output;
        string::size_type pos = contents.find_first_of('\n');
        if(pos != string::npos)
            ss << contents.substr(pos, string::npos);
        return ss.str();
    } else if (type == ".latch"){
        stringstream temp(contents);
        string output;
        output = ".latch " + *inputs.begin() + " " + this->output;
        string nul;
        temp >> nul; //Discard .latch
        temp >> nul; //input
        temp >> nul; //output
        while(temp.good()){
            temp >> nul;
            output += " " + nul;
        }
        return output;
    } else {
        return contents;
    }
}
