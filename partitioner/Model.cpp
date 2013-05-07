#include "Model.h"
#include <boost/foreach.hpp>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <fstream>
Model::Model()
{
   _latency = 0;
   maxCost = 0;
   numLatches = 3;
   numLUTs = 2;
   numCutLoops = 0;
}
Model::Model(double latency)
{
   _latency = latency;
   maxCost = 0;
   numLatches = 3;
   numLUTs = 2;
   numCutLoops = 0;
}

Model::Model(Model* model)
{
   name = model->name;
   _latency = model->_latency;
   maxCost = 0;
   numLatches = 3;
   numLUTs = 2;
   numCutLoops = 0;
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
   pair<string, Signal*> signalPair;
   BOOST_FOREACH(signalPair, signals){
      delete signalPair.second;
   }
   signals.clear();

   BOOST_FOREACH(BlifNode* node, nodes){
#pragma warning(suppress : 6246) // Keep Visual Studio from complaining about duplicate declaration as part of the nested FOREACH macro
      BOOST_FOREACH(string s, node->inputs){
         string sigName = s;
         if(signals.count(s) == 0){
            signals[sigName] = new Signal(sigName);
         }
         signals[sigName]->sinks.push_back(node);
      }
      string s = node->output;
      string sigName = s;
      if(signals.count(s) == 0){
         signals[sigName] = new Signal(sigName);
      }
      signals[sigName]->source = node;
   }

MakeIOList();
}


unordered_map<unsigned long, unsigned> maxLatencies;
unordered_map<unsigned long, short> explored;

void Model::AddNode(BlifNode* node){
   nodes.insert(node);
   unsigned maxInCost = 0;
   BOOST_FOREACH(string s, node->inputs){
      if(signals.count(s) == 0){
         signals[s] = new Signal(s);
      }
      signals[s]->sinks.push_back(node);
      BlifNode* source = signals[s]->source;
      if(source != NULL){
         if(maxLatencies[source->id] > maxInCost)
            maxInCost = maxLatencies[source->id];
      }
   }
   maxLatencies[node->id] = maxInCost+node->cost;
   string s = node->output;
   if(s != ""){
      if(signals.count(s) == 0){
         signals[s] = new Signal(s);
      }
      signals[s]->source = node;
   }
   if(maxInCost == 0) //Adding a no-cost node can't increase the max path, so skip calculating the changes.
      return;
   explored.clear();
   updateCosts(node, maxInCost);
}
void Model::updateCosts(BlifNode* node, unsigned costToReach){
   if(explored[node->id] == 1){ //Already been here, don't get caught in a loop
      return;
   }

   explored[node->id] = 1;
   costToReach += node->cost;
   maxLatencies[node->id] = costToReach;
   if(costToReach > maxCost)
      maxCost = costToReach;
   string s = node->output;
   if(s == "")
      return;
   BOOST_FOREACH(BlifNode* newNode, this->signals[s]->sinks){
      if(maxLatencies[newNode->id] >= costToReach+newNode->cost)
         continue; //If it won't increase the cost skip it
      updateCosts(newNode, costToReach);
   }
   explored[node->id] = 2;
}

void Model::MakeIOList(){
   inputs.clear();
   outputs.clear();
   pair<string, Signal*> s;
   BOOST_FOREACH(s, signals){
      if(s.second->source == NULL)
         inputs.push_back(s.second);
      else
         outputs.push_back(s.second); //If it's not an input always export it as an output. We can trim unused signals later, and currently we can't detect if a signal is used in another partition or not.
   }                            //Excess outputs will get stripped anyway when the circuit is flattened.
}

unsigned Model::CalculateCriticalPath(){
   return maxCost;
}
double Model::CalculateLatency(){
   //Code to calculate it here
   return 5;
}

double Model::CalculateArea(){
   return nodes.size();
}



Signal* Model::GetBaseSignal(string name){
   if(name[0] == 'q' && name[1] == 'q'){ //Special meaning signal. Translate into what it is in the untouched original
      if(name[3] == 'i'){ //qqrin
         string realName = name.substr(5, string::npos);
         return this->signals[realName];
      }
   } else {
      return this->signals[name];
   }
   return this->signals[name]; //We never reach here, but gcc complains about the function not returning anything from all possible paths
}


string dotPath;
void Model::SetDotFile(string path){
   dotPath = path;
}
ofstream dotFile;

void Model::CutLoops(){
   //Traverse the model and cut any loops
   //Traverse signalwise, but store visited state of each node. Mark as exploring, then recurse into it.
   //Once finished recursing, mark finalised. If we're about to expand a node marked exploring, then we have a cycle, cut the current signal and return up the stack.
   //If it's marked finalised we have multiple paths, but not an actual cycle.
    explored.clear();
    if(dotPath != ""){
      dotFile.open(dotPath);
      dotFile << "digraph main{" << endl;
   }
   numCutLoops = 0;
   BOOST_FOREACH(Signal* signal, outputs){
      this->CutLoopsRecurse(NULL, signal);
   }
   if(dotPath != ""){
      dotFile << "}" << endl;
      dotFile.close();
   }
}

BlifNode* DBG(set<BlifNode*> nodes, unsigned id){
   BOOST_FOREACH(BlifNode* node, nodes){
      if(node->id == id)
         return node;
   }
   return NULL;
}

void Model::CutLoopsRecurse(BlifNode* parent, Signal* signal){
   if(signal == NULL)
      return;
   BlifNode* node = signal->source;
   if(node == NULL)
      return;
   //cerr << "Exploring " << node->id << " - " << node->output << endl;
   if(parent != NULL && dotPath != "")
      dotFile << parent->output << " -> " << node->output <<";" << endl;
   if(explored[node->id] == 1){ //cycle
      numCutLoops++;
      replace(parent->inputs.begin(), parent->inputs.end(), signal->name, "qqrin"+signal->name);
      //signal->source->output = "qqout"+signal->name;// Don't rename the output. Other signals may use it. 
      signals.erase(signal->name);
   } else if(explored[node->id] == 2){ //Already explored, and dealt with any loops
      return;
   } else {
      explored[node->id] = 1;
      BOOST_FOREACH(string s, node->inputs){
         if(signals.count(s) != 0){ //If we've already renamed one of the signals, we won't find it in our signal list
            CutLoopsRecurse(node, signals[s]);
         }
      }
   }
   explored[node->id] = 2;
}

double Model::RecoveryTime(double voterArea){
    //resync+detect+reconfigure
    //=2*period*steps+f(area)
    double period = this->CalculateLatency();
    double steps = this->CalculateCriticalPath();
    double reconfigure = this->CalculateReconfigurationTime(voterArea);

    return 2.0*period*steps+reconfigure;
}


double Model::CalculateReconfigurationTime(double voterArea){
    double area = this->CalculateArea()+voterArea;
    return floor(area/20)*1E-8;
}
