#include <iostream>
#include "Blif.h"
#include <list>
#include <unordered_map>

const double MAX_AREA = 4; //Max area allowed to be used per partition
const double MAX_TIME = 10; //Max time allowed for pipeline to finish ( = steps/clock+constant)
const double VOTER_AREA = 2;
const double LATENCY_ESTIMATE = 1;

using namespace std;

enum NodeState{
    Unused = 0,
    Current,
    Used
};

void TMR(){
    cout << "TMR'd" << endl;
}

int main(int argc, char * argv[])
{
    if(argc != 2){
        cout << "Usage: partitioner infile.blif" << endl;
        return 1;
    }
    Blif* blif = new Blif(argv[1]);
    Model* model = blif->main;

    list<BlifNode*> queue;

    unordered_map<BlifNode*, NodeState> nodes;
    for each(Signal* sig in model->inputs){ //Start off with all nodes reading from an input in the queue
        for each(BlifNode* node in sig->sinks){
            queue.push_back(node);
        }
    }
    
    double area = VOTER_AREA; //Estimate of area. Starts at voter area
    double latency = LATENCY_ESTIMATE; // Estimate of latency, starts at voter latency.
    double criticalLength = 0; //Number of clock cycles needed for the pipeline i.e. number of latches (for .names and .latch only blif files)
    Model* current = new Model();


    while(queue.size() > 0){
        BlifNode* curr = queue.front();
        queue.pop_front();
        if(nodes[curr] != Unused){ //Already used, so we've detected a cycle
            if(nodes[curr] == Current) //Cycle within current subcircuit, so skip it, may do something special if needed
                continue;
            else if(nodes[curr] == Used){ //Cycle, but back to a previous voter subcircuit
                continue;
            } else {
                throw "Shouldn't ever reach here, invalid NodeState";
            }
        }
        nodes[curr] = Current;
        double nodearea = 1;
        double nodesteps = 0;
        if(curr->type == ".latch")
            nodesteps++;
        double time = 0;
        double nodelatency = 0;
        if(latency || nodelatency != 0){
            double frequency = 1.0/(latency+nodelatency);
            time = (criticalLength+nodesteps)/frequency;
        }
        if(area+nodearea > MAX_AREA || 
            time > MAX_TIME){
            TMR(); // Do all the TMR'ing stuff. Sets up for the current node to be added to a new voter subcircuit
            area = VOTER_AREA;
            latency = LATENCY_ESTIMATE;
            criticalLength = 0;
        }
        
        cout << curr->type << endl;
        area += nodearea;
        criticalLength += nodesteps;

        for each(string sig in curr->outputs){
            for each(BlifNode* node in model->signals[sig]->sinks){
                queue.push_back(node);
            }
        }
    }
    return 0;
}

