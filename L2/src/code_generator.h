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
#include "graph_color.h"
#include "liveness.h"

namespace L2{
    void generate_code(Program p, int Verbose);
    void condenseFunc(Function * f);
    std::string translate_operand(Operand* oprnd );
    std::string translate_instruction(Instruction * i );
}
