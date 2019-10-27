#pragma once

#include "IR.h"
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


namespace IR{
    void generateCode(Program p);
    void preProcess(Program* p);
  	void processBBs(Function * f );
  	std::string translateBBs(BasicBlock *bb);
    std::string translateInstruction(Instruction * i );
    std::string handleCalls(Instruction* inst, bool isRetVar);
    std::string handleNewArray(Instruction* inst);
    std::string handleLengthInst(Instruction* inst);
    std::string handleArrAcess(Instruction* inst, bool isRead);
    std::string handleWriteArrInst(Instruction* inst);
    std::string handleTupleAcess(Instruction* inst, bool isRead);
}
