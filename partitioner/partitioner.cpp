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
const double RECOVERY_TIME = 100; //Max recovery time allowed (=2*steps*period+reconfigure)
const double VOTER_AREA = 1;
const double CLOCK_PERIOD = 1E-8;

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
    model->CutLoops();
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
        ("clock-period,p", po::value<double>(), "estimate for final clock period")
        ("max-area,a", po::value<double>(), "maximum area per partition")
        //("max-time,t", po::value<double>(), "maximum time per partition")
        ("recovery-time,r", po::value<double>(), "maximum recovery time per partition")
        ("quiet,q", "suppress all output besides output list")
        ("calculate,c", "doesn't partition, just calculates circuit stats e.g. circuit critical path and outputs to STDOUT")
        ("dot-file,d", po::value<string>(), "Output the circuit as a dot file")
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
    double clockPeriod = CLOCK_PERIOD;
    if(vm.count("clock-period")){
        clockPeriod = vm["clock-period"].as<double>();
        if(!quiet) cout << "Setting clock period"<< endl;
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
    double recoveryTime = RECOVERY_TIME;
    if(vm.count("recovery-time")){
        recoveryTime = vm["recovery-time"].as<double>();
        if(!quiet) cout << "Setting recovery time"<< endl;
    }
    string dotPath = "";
    if(vm.count("dot-file")){
        dotPath = vm["dot-file"].as<string>();
        if(!quiet) cout << "Outputting dot file to "<< dotPath << endl;
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
    model->_latency = clockPeriod;
    model->SetDotFile(dotPath);

    list<BlifNode*> queue;
    unsigned partitionCounter = 1;
    unordered_map<unsigned long, NodeState> nodes;
    BOOST_FOREACH(Signal* sig, model->outputs){ //Start with outputs and work back. Not all nodes may be reachable by an input, but to have an effect on the final circuit all nodes must be reachable from an output.
       if(sig->source != NULL){
         queue.push_front(sig->source);
       }
    }
    
    Model* current = new Model(clockPeriod);
    stringstream currName;
    currName << "partition" << model->name << partitionCounter;
    current->name = currName.str();
    unsigned counter = 0;
    if(justCalculate == false) {
            while(queue.size() > 0){
                counter++;
                BlifNode* temp = queue.front();
                int nodeId = temp->id;
                BlifNode* curr = temp->Clone(); //Make a new node and copy the front of the queue. Keep the original model intact.
                queue.pop_front();
                if(nodes[nodeId] != Unused){ //Already used. Not necessarily a cycle though.
                    if(nodes[nodeId] == Current){ // Repeat in current partition
                        continue;
                    } else if(nodes[nodeId] == Used){ //Repeat, but in another partition
                        continue;
                    } else {
                        throw "Shouldn't ever reach here, invalid NodeState";
                    }
                }
                nodes[nodeId] = Current;
                current->AddNode(curr);
                //double time = model->CalculateLatency();
                if(/*current->RecoveryTime(voterArea) > recoveryTime ||*/
                    current->CalculateArea()+voterArea > maxArea// || 
                    /*current->CalculateLatency()+voterLatency > maxTime*/){
                    TMR(current, outPath); // Do all the TMR'ing stuff.
                    partitionCounter++;

                    BOOST_FOREACH(string sig, curr->inputs){ 
                      if(model->GetBaseSignal(sig)->source != NULL){
                         queue.push_front(model->GetBaseSignal(sig)->source);
                      }
                    }
                    // delete curr;
                    delete current;
                    current = new Model(clockPeriod);
                    currName.str("");
                    currName.clear();
                    currName << "partition" << model->name << partitionCounter;
                    current->name = currName.str();
                } else {
                   BOOST_FOREACH(string sig, curr->inputs){          
                      if(model->signals[sig]->source != NULL){
                        queue.push_back(model->signals[sig]->source);
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
            //cout << "\n";
            //BOOST_FOREACH(string s, blif->masterClocks){
                //cout << s << " ";
            //}
            //cout << endl;
    }
    doCalculation(blif, quiet);
    delete blif;
    return 0;
}

