#include "Model.h"
#include <boost/foreach.hpp>
#include <boost/range/adaptor/map.hpp>
#include <algorithm>
#include <cmath>
Model::Model()
{
   init();
}
Model::Model(double latency)
{
   init();
   this->latency = latency;
}

Model::Model(Model* model)
{
   init();
   name = model->name;
   latency = model->latency;
}

void Model::init(){
   latency = 0;
   maxCost = 0;
   numLatches = 0;
   numLUTs = 0;
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

void Model::RemoveNode(BlifNode* node){
   nodes.erase(node);
   if(node->type == ".names")
      numLUTs--;
   else if (node->type == ".latch")
      numLatches--;
   Signal* s = signals[node->output];
   if(s != NULL && s->sinks.size() == 0){
      signals.erase(node->output);
      delete s;
   } else if (s != NULL){
      s->source = NULL;
   }
   list<string>::iterator it;
   for(it=node->inputs.begin();it!=node->inputs.end();it++){
      string str = *it;
      if(str.substr(0, 5) == "qqrin"){
         *it = str.substr(5, string::npos); // change the name back
      }
      s = signals[str];
      if(s == NULL)
         continue;
      s->sinks.remove(node);
      if(s->sinks.size() == 0 && s->source == NULL){
         signals.erase(str);
         delete s;
      }
   }
   //Need to retraverse entire graph to update critical path
   //We may have cut a loop we didn't need to, but ignore it. Just means one extra voter than the minimum
   //Assign the old critical path back to it
   maxCost = oldMaxCost;
}



void Model::AddNode(BlifNode* node, bool traverse){
   nodes.insert(node);
   oldMaxCost = maxCost;
   if(node->type == ".latch")
      numLatches++;
   else if(node->type == ".names")
      numLUTs++;
   else
      cerr << "Unknown node" << endl;
   unsigned maxInCost = 0;
   list<string>::iterator it;
   for(it=node->inputs.begin();it!=node->inputs.end();it++){
      string s = *it;
      if(cutLoops[s] != ""){
         *it = cutLoops[s];
         s   = cutLoops[s];
      }
      if(signals.count(s) == 0){
         signals[s] = new Signal(s);
      }
      signals[s]->sinks.push_back(node);
      BlifNode* source = signals[s]->source;
      if(source != NULL){
         if(costs[source->id] > maxInCost)
            maxInCost = costs[source->id];
      }
   }
   costs[node->id] = maxInCost+node->cost;
   string s = node->output;
   if(s != ""){
      if(signals.count(s) == 0){
         signals[s] = new Signal(s);
      }
      signals[s]->source = node;
   }
   if(traverse == false) //e.g. we're adding them for the initial model
      return;

   //We now need to update our longest path measurement, and cut any cycles if they've been created.
   //We need to cut cycles while calculating the longest path, as otherwise depending on where the cut is made, the longest path can change.
   //Traverse from node to output, updating costs. Cost at output is max.
   //The only path changes must be through our added node, therefore 
   //1. any cycles must pass through our node
   //2. the longest path must either stay the same, or pass through our node downstream to outputs.
   //Therefore, we start at node, descend into children. If we reach our start node, cut the loop.
   //If cost is same, we can't exist early as we need to detect loops, so keep going until we reach the end.
   //If cost is higher, update.

   explored.clear();
   updateCosts(node, NULL, node, maxInCost);
}

//costToReach does _NOT_ include the node cost, it's just the cost of the parent in this particular path
void Model::updateCosts(BlifNode* root, BlifNode* parent, BlifNode* node, unsigned costToReach){
   if(explored[node->id] == true && costToReach <= costs[node->id]) //TODO: Check that path can't get more expensive
      return;
   if(parent != NULL && node == root){ // cycle
      numCutLoops++;
      CutSignal(signals[parent->output]);
      //CutLoop(parent, node)
      return;
   }
   unsigned cost = costToReach+node->cost;
   if(cost > costs[node->id])
      costs[node->id] = cost;
   else
      cost = costs[node->id];
   if(cost > maxCost)
      maxCost = cost;
   BOOST_FOREACH(BlifNode* child, signals[node->output]->sinks){
      updateCosts(root, node, child, cost);
   }
   explored[node->id] = true;
   return;
}


bool IsNotSpecialSignal(const Signal* value){
   return !((value)->name.substr(0, 2) == "qq");
}

void Model::MakeIOList(Model* main){
   inputs.clear();
   outputs.clear();
   pair<string, Signal*> s;
   BOOST_FOREACH(s, this->signals){
      if(s.second->source == NULL)
         inputs.push_back(s.second);
      else if(main->signals[s.first] != NULL &&
         main->signals[s.first]->sinks.size() > s.second->sinks.size())
         outputs.push_back(s.second);
      else{
         BOOST_FOREACH(Signal* sig, main->outputs){
            if(s.first == sig->name){
               outputs.push_back(s.second);
               break;
            }
         }
      }
   }
   BOOST_FOREACH(BlifNode* n, this->nodes){
      if(n->type != ".latch")
         continue;
      if(signals[n->clock] == NULL){
         Signal* s = new Signal(n->clock);
         signals[s->name] = s;
         inputs.push_back(s);
      }
   }
}

unsigned Model::CalculateCriticalPath(){
   //static int asd = 0;
   //asd++;
   //if(asd%100 == 0)
      //cerr << asd << endl;
   //int cost = 0;
   //int max = 0;
   //explored.clear();
   //pair<string, Signal*> s;
   //BOOST_FOREACH(s, signals){
      //int t = CalculateCriticalPathRecursive(s.second->source, cost);
      //if(t > max)
         //max = t;
   //}
   //maxCost = max;
   return maxCost+1;
}

int Model::CalculateCriticalPathRecursive(BlifNode* node, int cost){
   if(node == NULL)
      return cost;
   if(explored[node->id] > 0)
      return cost;
   explored[node->id] = 1;
   cost += node->cost;
   int max = 0;
   BOOST_FOREACH(string s, node->inputs){
      int t = CalculateCriticalPathRecursive(signals[s]->source, cost);
      if(t > max)
         max = t;
   }
   explored[node->id] = 1;
   return max;
}

double Model::CalculateLatency(){
   //Code to calculate it here
   return latency;
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
   }
   return this->signals[name];
}


void Model::SetDotFile(string path){
   dotPath = path;
}


BlifNode* DBG(set<BlifNode*> nodes, unsigned id){
   BOOST_FOREACH(BlifNode* node, nodes){
      if(node->id == id)
         return node;
   }
   return NULL;
}

//node is a sink of signal
void Model::CutSignal(Signal* signal){
   //node->output is now PO, called [name], and has no sinks
   string name = signal->name;
   Signal* sig = new Signal(name);
   sig->source = signal->source;
   signals[name] = sig;
   replace(outputs.begin(), outputs.end(), signal, sig); //In case this is an output, replace the old signal for the new one in the output list
   //signal->source->output = "qqout"+signal->name;// Don't rename the output. Other signals may use it. 

   //parent->input is now PI, called qqrin[name]
   signal->name = "qqrin"+name; // rename
   signal->source = NULL; // no source since PI
   signals[signal->name] = signal;
   BOOST_FOREACH(BlifNode* n, signal->sinks){
      replace(n->inputs.begin(), n->inputs.end(), name, "qqrin"+name);
   }
   cutLoops[name] = "qqrin"+name;
}


double Model::RecoveryTime(unsigned numPartitions){
   //resync+detect+reconfigure
   //=2*period*steps+f(area)
   double period = this->CalculateLatency();
   double steps = this->CalculateCriticalPath();
   double latency = period*steps;

   //Reconfiguration time
   double columns = max(numLUTs, numLatches);//max(numLUTs+voterLUTs*outputs.size(), numLatches);
   columns /= 160.0;
   columns = ceil(columns);
   double reconfigurationTime = 14.8e-6*columns;

   //Resynchronisation time = latency
   
   //Detection time = latency(+something? multiple errors?)
   
   //Communication time
   double communicationTime = 5*50*(numPartitions+1)*period;
   return reconfigurationTime + latency + latency + communicationTime;
}
