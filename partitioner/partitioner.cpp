#include <iostream>
#include "Blif.h"
#include <list>
#include <unordered_map>
#include <sstream>
#include <boost/program_options.hpp>
#include <boost/foreach.hpp>
#include <boost/range/adaptor/map.hpp>
namespace po = boost::program_options;

const double MAX_AREA = 800; //Max area allowed to be used per partition
const double MAX_TIME = 100; //Max time allowed for pipeline to finish ( = steps/clock+constant)
const double VOTER_AREA = 2;
const double CIRCUIT_LATENCY = 4;

using namespace std;
using namespace boost::adaptors;

enum NodeState{
    Unused = 0,
    Current,
    Used
};

void TMR(Model* model, string outPath){
    static int counter = 0;
    counter++;
    stringstream path;
    path << outPath << counter << ".blif";
    model->MakeSignalList(true);
    model->MakeIOList();
    Blif::Write(path.str(), model);
}

void doCalculation(Blif* blif, bool quiet){
    BOOST_FOREACH(Model* model, blif->models | map_values){
        if(!quiet){
            cout << "Model: " << model->name << endl;
            cout << "Critical path\t\tArea Estimate\t\tLatency Estimate\n";
        }
        cout << model->CalculateCriticalPath() << "\t" << model->CalculateArea() << "\t" << model->CalculateLatency() << endl;
    }
}

int main(int argc, char * argv[])
{
    po::options_description desc("Usage: [options] [-o outprefix] -f infile ");
    desc.add_options()
        ("help,h", "produce help message")
        ("infile,f", po::value<string>(), "input file")
        ("outfile,o", po::value<string>(), "output path prefix. Required unless -c is also passed")
        ("voter-area,v", po::value<double>(), "area of voter circuit")
        ("voter-latency,l", po::value<double>(), "constant delay in voter circuit elements")
        ("max-area,a", po::value<double>(), "maximum area per partition")
        ("max-time,t", po::value<double>(), "maximum time per partition")
        ("quiet,q", "suppress all output besides output list")
        ("calculate,c","doesn't partition, just calculates circuit stats e.g. circuit critical path and outputs to STDOUT")
    ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);    

    if (vm.count("help")) {
        cout << desc << endl;
        return 1;
    }
    bool error = false;
    if(vm.count("infile") == 0){
        cout << "Must specify an input file" << endl;
        error = true;
    }
    if(vm.count("outfile") == 0 && vm.count("calculate") == 0){
        cout << "Must specify an output path prefix" << endl;
        error = true;
    }
    bool quiet = false;
    if(vm.count("quiet") != 0)
        quiet = true;
    
    double voterArea = VOTER_AREA;
    if(vm.count("voter-area")){
        voterArea = vm["voter-area"].as<double>();
        if(!quiet) cout << "Setting voter area"<< endl;
    }
    //double area = voterArea;
    double voterLatency = CIRCUIT_LATENCY;
    if(vm.count("voter-latency")){
        voterLatency = vm["voter-latency"].as<double>();
        if(!quiet) cout << "Setting voter latency"<< endl;
    }
    double maxArea = MAX_AREA;
    if(vm.count("max-area")){
        maxArea = vm["max-area"].as<double>();
        if(!quiet) cout << "Setting max area"<< endl;
    }
    double maxTime = MAX_TIME;
    if(vm.count("max-time")){
        maxTime = vm["max-time"].as<double>();
        if(!quiet) cout << "Setting max time"<< endl;
    }
    bool justCalculate = false;
    if(vm.count("calculate")){
        justCalculate = true;
        if(!quiet) cout << "Calculating stats only"<< endl;
    }

    if(error){
        cout << desc << endl;
        return 1;
    }
    string outPath;
    if(justCalculate == false)
        outPath = vm["outfile"].as<string>();
    Blif* blif = new Blif(vm["infile"].as<string>().c_str());
    Model* model = blif->main;

    list<BlifNode*> queue;
    unsigned partitionCounter = 1;
    unordered_map<unsigned long, NodeState> nodes;
    BOOST_FOREACH(Signal* sig, model->outputs){ //Start with outputs and work back. Not all nodes may be reachable by an input, but to have an effect on the final circuit all nodes must be reachable from an output.
    #pragma warning(suppress : 6246) // Keep Visual Studio from complaining about duplicate declaration as part of the nested FOREACH macro
        BOOST_FOREACH(BlifNode* node, sig->sources){
            queue.push_back(node);
        }
    }
    
    Model* current = new Model();
    stringstream currName;
    currName << "partition" << model->name << partitionCounter;
    current->name = currName.str();
    unsigned counter = 0;
    if(justCalculate == false) {
            while(queue.size() > 0){
                counter++;
                BlifNode* curr = new BlifNode; //Make a new node and copy the front of the queue. Keep the original model intact.
                *curr = *queue.front();
                queue.pop_front();
                if(nodes[curr->id] != Unused){ //Already used, so we've detected a cycle
                    if(nodes[curr->id] == Current){ //Cycle within current subcircuit, so skip it. We need to cut cycles
                        //model->Cut(curr);
                        continue;
                    } else if(nodes[curr->id] == Used){ //Cycle, but back to a previous voter subcircuit
                        continue;
                    } else {
                        throw "Shouldn't ever reach here, invalid NodeState";
                    }
                }
                nodes[curr->id] = Current;
                current->AddNode(curr);
                //double time = model->CalculateLatency();
                if(current->CalculateArea()+voterArea > maxArea || 
                    current->CalculateLatency()+voterLatency > maxTime){
                    TMR(current, outPath); // Do all the TMR'ing stuff.
                    partitionCounter++;

                    BOOST_FOREACH(string sig, curr->inputs){ 
            #pragma warning(suppress : 6246) // Keep Visual Studio from complaining about duplicate declaration as part of the nested FOREACH macro
                        BOOST_FOREACH(BlifNode* node, model->GetBaseSignal(sig)->sources){
                            queue.push_back(node);
                        }
                    }
                   // delete curr;
                    delete current;
                    current = new Model;
                    currName.str("");
                    currName.clear();
                    currName << "partition" << model->name << partitionCounter;
                    current->name = currName.str();
                } else {

                   BOOST_FOREACH(string sig, curr->inputs){             
            #pragma warning(suppress : 6246) // Keep Visual Studio from complaining about duplicate declaration as part of the nested FOREACH macro
                       BOOST_FOREACH(BlifNode* node, model->signals[sig]->sources){
                           queue.push_back(node);
                       }
                   }
               }
            }
            //TMR the remaining node if it exists
            if(current->nodes.size() > 0){
                TMR(current, outPath);
            }
            delete current;
            BOOST_FOREACH(string s, blif->masterInputs){
                cout << s << " ";
            }
            cout << "\n";
            BOOST_FOREACH(string s, blif->masterOutputs){
                cout << s << " ";
            }
            cout << endl;
    }
    doCalculation(blif, quiet);
    //delete blif; We're exiting now, so memory gets cleared up anyway, and actually destroying everything properly takes a lot of time
    return 0;
}

