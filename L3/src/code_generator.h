#pragma once

#include "L3.h"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <map>
#include <vector> 
#include <typeinfo>


namespace L3{
    void generateCode(Program p, int Verbose);
    void renameLabels(Program* p);
    std::string translateInstruction(Instruction * i );
    std::string generatePrelude(Function * f );
}
