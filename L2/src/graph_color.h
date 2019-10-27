#pragma once

#include "L2.h"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <map>
#include <vector>

namespace L2{
    void color_iGraph(Function* f, int Verbose);
    void calcNumCalls(Function* f);
    void findUniqueVar(Function *f);
    void remove_node(Function* f, std::vector<Variable *> &stackVars, bool need_shuffle, std::vector<Variable *> &prevspilled, std::vector<Variable *> &spilled);
    bool rebuild_graph(Function* f, std::vector<Variable *> &s, std::vector<Variable *> &spilled);
    bool spill_vars(Function* f);
    void replace_vars(Function* f);
    void clear_sets(Function *f);
	void copy_func(Function *f, Function *Originalf);
}