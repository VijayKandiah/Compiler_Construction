#pragma once

#include "LB.h"
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


namespace LB{
	void generateCode(Program p, int Verbose);
	void preProcess(Function * f);
  	void nameBinding(Function * f);
  	void removeScopes(Function* f);
  	void renameVariables(Variable* var, ScopeVars* scopeVars);
	void processLoops(Function* f);
	std::string translateInstruction(Instruction * i );
   	std::string handleCalls(Instruction* inst, bool isRetVar);
	std::string handleNewArray(Instruction* inst);
	std::string handleArrAcess(Instruction* inst, bool isRead);
	std::string handleLoops(Instruction* inst);
	std::string handleContinue(Instruction* inst);
	std::string handleBreak(Instruction* inst);
}
