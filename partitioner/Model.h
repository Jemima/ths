#pragma once
#include <string>
#include "BlifNode.h"
#include "Signal.h"
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <set>


using namespace std;
class Model
{
   public:
      Model();
      Model(double latency);
      Model(Model* model);
      ~Model(void);
      list<Signal*> inputs;
      list<Signal*> outputs;
      set<BlifNode*> nodes;
      unordered_map<string, Signal*> signals;
      string name;
      int numCutLoops;
      int numLatches;
      int numLUTs;
      double latency;

      //Given a signal map for the main node, creates signals for the partition by promoting necessary signals which are driven or consumed outside the partition, to primary inputs or outputs
      void MakeIOList(Model* main);

      //Cuts the input from this node, indicating the input should be external to this submodel.
      void Cut(BlifNode* node);

      //Attempt to add node to this model. If it would make the maximum 
      //recovery time exceed maxRecoveryTime return false and leave model unchanged,
      //otherwise add node and make changes appropriately
      void AddNode(BlifNode* node, bool traverse);
      void RemoveNode(BlifNode* node);

      unsigned CalculateCriticalPath();

      double CalculateLatency();

      double CalculateArea();

      Signal* GetBaseSignal(string name);

      void CutSignal(Signal* signal);

      double RecoveryTime(unsigned numPartitions);

      void SetDotFile(string path);
   private:
      unsigned CalculateCriticalPath(BlifNode* node, unordered_map<int, unsigned> &visited);
      void updateCosts(BlifNode* root, BlifNode* parent, BlifNode* node, unsigned costToReach);
      void CutLoopsRecursive(BlifNode* parent, Signal* signal);
      int CalculateCriticalPathRecursive(BlifNode* node, int cost);
      unsigned maxCost;
      void init();
      unordered_map<string, string> cutLoops;
      unordered_map<unsigned long, unsigned> costs;
      unordered_map<unsigned long, short> explored;
      string dotPath;
      ofstream dotFile;
};

