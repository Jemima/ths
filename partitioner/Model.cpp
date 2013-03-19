#include "Model.h"
#include <boost/foreach.hpp>
#include <algorithm>
#include <iostream>
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


void Model::MakeSignalList(bool cutLoops){
   pair<string, Signal*> signalPair;
   BOOST_FOREACH(signalPair, signals){
      delete signalPair.second;
   }
   signals.clear();
   /*if(cutLoops){
     BOOST_FOREACH(BlifNode* node, nodes){
#pragma warning(suppress : 6246) // Keep Visual Studio from complaining about duplicate declaration as part of the nested FOREACH macro
BOOST_FOREACH(string sig, cycles){
replace(node->inputs.begin(), node->inputs.end(), sig, "qqrinput"+sig);
replace(node->outputs.begin(), node->outputs.end(), sig, "qqroutput"+sig);
}
}
}*/

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
unordered_map<unsigned long, bool> visited;


void Model::AddNode(BlifNode* node){
   AddNode(node, true);
}


void Model::AddNode(BlifNode* node, bool shouldDetectLoops){
   nodes.insert(node);
   unsigned maxInCost = 0;
   BOOST_FOREACH(string s, node->inputs){ //Need to not add a signal if it causes a cycle
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
   visited.clear();
   updateCosts(node, maxInCost);
}
unsigned maxCost = 0;
void Model::updateCosts(BlifNode* node, unsigned costToReach){
   if(visited[node->id] == true){ //Already been here, don't get caught in a loop
      return;
   }
   visited[node->id] = true;
   costToReach += node->cost;
   maxLatencies[node->id] = costToReach;
   if(costToReach > maxCost)
      maxCost = costToReach;
   string s = node->output;
   if(s == "")
      return;
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
   return this->signals[name]; //We never reach here, but gcc complains about the function not returning anything from all possible paths
}


void Model::CutLoops(){
   //Traverse the model and cut any loops
   //Traverse signalwise, but store visited state of each node. Mark as exploring, then recurse into it.
   //Once finished recursing, mark finalised. If we're about to expand a node marked exploring, then we have a cycle, cut the current signal and return up the stack.
   //If it's marked finalised we have multiple paths, but not an actual cycle.

   BOOST_FOREACH(Signal* signal, outputs){
      this->CutLoopsRecurse(NULL, signal);
   }
}
unordered_map<int, bool> exploring; //Map from NodeId->visited state. Unset is not seen, true is exploring, false is finalised. Only care about the true case

void Model::CutLoopsRecurse(BlifNode* parent, Signal* signal){
   if(signal == NULL)
      return;
   BlifNode* node = signal->source;
   if(node == NULL)
      return;
   if(exploring[node->id] == true){ //cycle
      replace(parent->inputs.begin(), parent->inputs.end(), signal->name, "qqrin"+signal->name);
      //signal->source->output = "qqout"+signal->name;// Don't rename the output. Other signals may use it. 
      signals.erase(signal->name);
   } else {
      exploring[node->id] = true;
      BOOST_FOREACH(string s, node->inputs){
         if(signals.count(s) != 0){ //If we've already renamed one of the signals, we won't find it in our signal list
            CutLoopsRecurse(node, signals[s]);
         }
      }
   }
   exploring[node->id] = false;
}
