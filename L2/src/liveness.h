#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <vector>
#include <map>
#include <set>
#include <iterator>
#include <utility>
#include "L2.h"

namespace L2{ 
    void liveness_print(Function* f);
    void remUnusedVariables(Function* f);
    void gen_kill(Function* f);
    bool removeInsts(Function* f);
    void deadCodeElimination(Function* f);
    bool checkForDeadCode(Function* f, int instIndex);
    void print_gen_kill(Function *f); //for debug
    void in_out(Function* f);
    void interference(Function* f);
    void interference_print(Function* f);
    void interference_test(Function* f);
    void generate_igraph(Function* f);
    void insert_edges(InterferenceGraph* iG, std::vector<std::string> in_out);
    void connect_kill_out(InterferenceGraph* iG, std::vector<std::string> KILL, std::vector<std::string> OUT);
}
