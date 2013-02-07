#pragma once
#include <string>
#include "Model.h"
#include <unordered_map>
using namespace std;

class Blif
{
public:
    Blif(string path);
    ~Blif(void);
    unordered_map<string, Model*> models;
    Model *main;
    static void Write(string path, Model* model);
    list<string> masterInputs;
    list<string> masterOutputs;
private:
    list<string> getParams(string line, string& nodeName);
    string getBlifLine(ifstream& stream);
};

