#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <ctype.h>
#include "Model.h"
#include <unordered_map>
using namespace std;

class Blif
{
public:
    Blif(const char * path);
    ~Blif(void);
    unordered_map<string, Model*> models;
    Model *main;
    static void Write(string path, Model* model);
private:
    list<string> getParams(string line, string& nodeName);
    string getBlifLine(ifstream& stream);
};

