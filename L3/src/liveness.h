#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include "L3.h"

namespace L3{ 
    void livenessPrint(Function* f);
    void liveness(Function* f);
    void genKill(Function* f);
    void printGenKill(Function *f); //for debug
    void inOut(Function* f);
    void computePredSuc(Function* f);
    void inOutPrint(Function * f);
}
