#pragma once

#include "LA.h"
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


namespace LA{
	void generateCode(Program p);
	void preProcess(Program* p);
	void generateBBS(Function * f);
	void condenseFunc(Function * f);
  	void formatVariables(Function * f);
  	void encodeConstants(Function * f);
	std::string translateInstruction(Instruction * i );
   	std::string handleCalls(Instruction* inst, bool isRetVar);
	std::string handleNewArray(Instruction* inst);
	std::string handleLengthInst(Instruction* inst);
	std::string handleArithOp(Instruction* inst);
	std::string handleBranch(Instruction* inst);
	std::string handleArrAcess(Instruction* inst, bool isRead);
	std::string handleTupleAcess(Instruction* inst, bool isRead);

}
