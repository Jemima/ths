#include "Blif.h"
#include <sstream>
#include <ctype.h>
#include <iostream>
#include <fstream>
#include "BlifNode.h"
#include <boost/foreach.hpp>


Blif::Blif(string path)
{
    main = new Model();
    ifstream stream(path.c_str(), ios::in);
    if(stream.good() == false){
        cerr << "Error reading from " << path << endl;
        return;
    }
    string temp = getBlifLine(stream);
    stringstream ss;
    string name;
    ss << temp; //Assume that the format is correct i.e. ".model [name]".
    ss << temp;
    ss >> name;
    main->name = name;
    //Get the inputs
    temp = getBlifLine(stream);
    list<string> inputNames = getParams(temp, temp);
    BOOST_FOREACH(string s, inputNames){
        Signal* sig = new Signal(s);
        main->inputs.push_back(sig);
        main->signals[s] = sig;
    }
    //Get the outputs
    temp = getBlifLine(stream);
    list<string> outputNames = getParams(temp, temp);
    BOOST_FOREACH(string s, outputNames){
        Signal* sig = new Signal(s);
        main->outputs.push_back(sig);
        main->signals[s] = sig;
    }

    temp = getBlifLine(stream);
    do{
        string nodeName;
        list<string> params = getParams(temp, nodeName);
        BlifNode* node = BlifNode::MakeNode(nodeName, params);
        while(node->AddContents(temp)){
            temp = getBlifLine(stream);
        }
        main->AddNode(node, false);
    } while(temp!= ".end");
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
    pair<string, Model*> m;
    BOOST_FOREACH(m, models){
        if(m.first == mainName) // We already deleted main, so don't delete it again
            continue;
        delete m.second;
    }
}

//Reads in a line, continuing the line on any \ e.g.
// "blah\[newline]something else"
// turns into "blah something else"
// Ignores leading blank lines.
// Ignores comments
std::string Blif::getBlifLine(ifstream& stream)
{
    string line = "";
    bool cont = false;
    if(stream.eof()){ //Once we hit eof .end is implied
        return ".end";
    }
    do{
        cont = false;
        string temp;
        getline(stream, temp);

        //If there's a comment on this line ignore everything afterwards
        int pos = temp.find('#');
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
        } else { //Empty line, skip to the next
            cont = true;
        }
    }while(cont && stream.good());
    if(line.length() == 0)
        return ".end"; //If we reach the end of the file before reading anything more, return .end
    return line;
}


void Blif::Write(string path, Model* model){
    ofstream ofile(path);
    ofile << ".model " << model->name << endl << ".inputs ";
    BOOST_FOREACH(Signal* s, model->inputs){
        ofile << s->name << " ";
    }
    ofile << endl << ".outputs ";
    BOOST_FOREACH(Signal* s, model->outputs){
        ofile << s->name << " ";
    }
    ofile << endl;

    BOOST_FOREACH(BlifNode* node, model->nodes){
        ofile << node->contents << endl;
    }
    ofile << ".end";
}
