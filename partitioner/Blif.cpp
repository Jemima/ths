#include "Blif.h"
#include <sstream>
#include "BlifNode.h"


Blif::Blif(char * path)
{
    main = new Model();
    ifstream stream = ifstream(path, ios::in);
    if(stream.good() == false){
        cerr << "Error reading from " << path << endl;
        return;
    }
    string temp = getBlifLine(stream);
    char * temp_name = (char*)malloc(sizeof(char)*temp.length()-5);
    sscanf_s(temp.c_str(), ".model %s", temp_name, temp.length()-6);
    string name = temp_name;
    main->name = name;
    //Get the inputs
    temp = getBlifLine(stream);
    list<string> inputNames = getParams(temp, temp);
    for each(string s in inputNames){
        Signal* sig = new Signal(s);
        main->inputs.push_back(sig);
        main->signals[s] = sig;
    }
    //Get the outputs
    temp = getBlifLine(stream);
    list<string> outputNames = getParams(temp, temp);
    for each(string s in outputNames){
        Signal* sig = new Signal(s);
        main->outputs.push_back(sig);
        main->signals[s] = sig;
    }

    temp = getBlifLine(stream);
    while(stream.good()){
        string nodeName;
        list<string> params = getParams(temp, nodeName);
        BlifNode* node = BlifNode::MakeNode(nodeName, params);
        while(node->AddContents(temp)){
            temp = getBlifLine(stream);
        }
        main->nodes.push_back(node);
        if(temp == ".end")
            break;
    }
    main->MakeSignalList();
    models[name] = main;
}

///Given a string in format nodeName a b c d e... returns a list containing a b c d... and sets nodeName to nodeName
list<string> Blif::getParams(string line, string& nodeName){
    stringstream ss;
    list<string> l;
    ss << line;
    ss >> nodeName;
    while(ss.good()){
        string in;
        ss >> in;
        l.push_back(in);
    }
    return l;
}


Blif::~Blif(void)
{
    string mainName = main->name;
    delete main;
    for each(pair<string, Model*> m in models){
        if(m.first == mainName) // We already deleted main, so don't delete it again
            continue;
        delete m.second;
    }
}

//Reads in a line, continuing the line on any \ e.g.
// "blah\
// something else"
// turns into "blah something else"
// Ignores leading blank lines.
// Ignores comments
std::string Blif::getBlifLine(ifstream& stream)
{
    string line = "";
    bool cont = false;
    do{
        cont = false;
        string temp;
        getline(stream, temp);

        //If there's a comment on this line ignore everything afterwards
        unsigned pos = temp.find('#');
        if(pos != -1){
            temp = temp.substr(0, pos);
        }
        bool hasContent = false;
        for (unsigned n=0;n<temp.length();n++)
        {
            if(isspace(temp[n]) == false){
                hasContent = true;
                break;
            }
        }
        if(hasContent == true){
            line += temp;
            if(line[line.length()-1] == '\\'){
                line = line.substr(0, line.length()-1);
                cont = true;
            }
        } else {
            cont = true;
        }
    }while(cont && stream.good());
    return line;
}


void Blif::Write(string path, Model* model){
    ofstream ofile(path);
    ofile << ".model " << model->name << endl << ".inputs ";
    for each(Signal* s in model->inputs){
        ofile << s->name << " ";
    }
    ofile << endl << ".outputs ";
    for each(Signal* s in model->outputs){
        ofile << s->name << " ";
    }
    ofile << endl;

    for each(BlifNode* node in model->nodes){
        ofile << node->contents << endl;
    }
    ofile << ".end";
}