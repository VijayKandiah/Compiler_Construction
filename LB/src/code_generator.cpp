#include "code_generator.h"

using namespace std;
namespace LB{

	int var_count = 0; //used to generate new variables: %newVarLB_ + var_count++
	int lbl_count = 0; //used to generate new labels: :newLabelLB_ + lbl_count++
	std::string uniqueVar = "";
	std::string uniqueLabel = "";

 	void generateCode(Program p, int Verbose){
		std::ofstream outputFile;
		outputFile.open("prog.a");
 		for(auto f: p.functions){
 			var_count = 0;
 			lbl_count = 0;
 			preProcess(f);
 			nameBinding(f);
 			processLoops(f);
 			outputFile<< f->retType<< " "<< f->name<<"( ";
 			for(int i=0; i<f->arguments.size(); i++){
 				if(i == f->arguments.size()-1)
					outputFile<<f->arguments[i]->defType<<" "<<f->arguments[i]->str;
				else
					outputFile<<f->arguments[i]->defType<<" "<<f->arguments[i]->str<<", ";
			}
			outputFile<<" ){\n\t";
 		
 			for(auto i : f->instructions){
	 			outputFile << translateInstruction(i);	
 			}
 			outputFile<<"}"<<endl;
 		}
		outputFile.close();
		return;
 	}

 	void preProcess(Function* f){

		std::reverse(f->arguments.begin(), f->arguments.end());   
		f->instructions.erase (f->instructions.begin());
		f->instructions.pop_back();
		std::string lv = "";
		std::string ll = "";

		for(auto i: f->instructions){
			for(auto var : i->operands){
				if(var->op_type == VAR){
					if (var->str.size() >=lv.size()) 
						lv = var->str;
					}
				if(var->op_type == LABEL){
					if (var->str.size() >=ll.size()) 
						ll = var->str;
				}
			}
		}

	 	if (lv.empty())
 			lv +=  "newVarLB_";
 		else
 			lv +=  "_LBc_";
 		if (ll.empty())
 			ll +=  ":newLabelLB_";
 		else
 			ll +=  "_LBc_";

 		uniqueVar = lv;
 		uniqueLabel = ll;
		return;
	}




	void nameBinding(Function * f){
		auto scopeVars = new ScopeVars;

		for(auto i: f->instructions){
			if(i->op == SCOPE_OPEN){
				auto inScope = new Scope();
				scopeVars->scopes.push_back(inScope);
			}
			else if(i->op == DEFINE){
				if(!scopeVars->scopes.empty()){
					auto currentScope = scopeVars->scopes.back();
					currentScope->varMap.emplace_back(i->operands[0]->str, (uniqueVar + to_string(var_count)));
					i->operands[0]->str += uniqueVar + to_string(var_count++);
				}
			}
			else if(i->op == SCOPE_CLOSE){
				scopeVars->scopes.pop_back();
			}
			else{
				for(auto &var : i->operands){
					renameVariables(var, scopeVars);
				}
				for(auto &var : i->indices){
					renameVariables(var, scopeVars);
				}
			}
 		}
	}

	void renameVariables(Variable* var, ScopeVars* scopeVars){
		for(int j = scopeVars->scopes.size()-1; j>=0; j--){
			for(auto &item : scopeVars->scopes[j]->varMap){
				if(var->str == item.first){
					var->str = item.second;
					return;
				}
			}
		}
	}


	void processLoops(Function* f){
		bool flag = 0;
	 	do{
			flag = 0;
	 		for(int i=0; i<f->instructions.size(); i++){
	 			if ((f->instructions[i]->op == WHILE) && (f->instructions[i]->condLabel == NULL)){
	 				auto newInst = new Instruction();
	 				newInst->op = LBL;
	 				auto newOp = new Variable();
            		newOp->op_type = LABEL;
            		newOp->str = uniqueLabel + to_string(lbl_count++);		
            		newInst->operands.push_back(newOp);
            		newInst->Inst = newOp->str;
            		f->instructions[i]->condLabel = newInst;
	 				f->instructions.insert(f->instructions.begin() + i, newInst);
	 				flag = 1;
	 				break;
	 			}
	 		}
	 	}while(flag);
	   

	 	//Algorithm from slides
	 	std::set<Instruction *> whileSeen;
	 	std::vector<Instruction *>loopStack;

	 	for(auto i : f->instructions){
	 		if(!loopStack.empty()){
	 			i->loop = loopStack.back();
	 		}

	 		if((i->op == WHILE) && (!whileSeen.count(i))){
	 			loopStack.push_back(i);
	 			whileSeen.insert(i);
	 			continue;
	 		}

	 		if(i->op == LBL){
	 			for(auto j : f->instructions){
	 				if((j->op == WHILE) && (j->operands[2]->str == i->operands[0]->str)){
	 					if(!whileSeen.count(j)){
	 						loopStack.push_back(j);
		 					break;
		 				}
	 				}
	 				else if((j->op == WHILE) && (j->operands[3]->str == i->operands[0]->str)){
						loopStack.pop_back();
	 					break;
	 				}
	 			}
	 		}
	 	}

		return;	 		
	}


std::map<Operator, std::string> LA_Operatormap = {
        {MOV, " <- "}, 
        {ADD, " + "}, 
        {SUB, " - "}, 
        {MUL, " * "}, 
        {AND, " & "}, 
        {LT, " < "}, 
        {LE, " <= "}, 
        {EQ, " = "},
        {GT, " > "}, 
        {GE, " >= "},
        {SAL, " << "}, 
        {SAR, " >> "},
        {JUMP, "br "},
        {RET, "return "}, 
        {RETVAR, "return "},
        {PRINT, "print ("},
        {LENGTH, " <- length "},
        {NEWARRAY, " <- new Array("},
        {NEWTUPLE, " <- new Tuple("}
	    };

	
 	std::string translateInstruction(Instruction * i ){

		switch(i->op){
			case DEFINE:
				return i->operands[0]->defType + " " + i->operands[0]->str + "\n\t";

			case MOV: 
				return i->operands[0]->str + LA_Operatormap[MOV] + i->operands[1]->str + "\n\t"; 

			case ADD: case SUB: case MUL: case AND: case LT: case LE: case EQ: case GE: case GT: case SAR: case SAL:
				return i->operands[0]->str + LA_Operatormap[MOV] + i->operands[1]->str + LA_Operatormap[i->op] + i->operands[2]->str + "\n\t";

			case CALL:
				return handleCalls(i, false);

			case CALL_RETVAR:
				return handleCalls(i, true);

			case PRINT:
				return LA_Operatormap[PRINT] + i->operands[0]->str + ")\n\t"; 

			case NEWARRAY:
				return handleNewArray(i);

			case READ_ARRAY_TUPLE:
				return handleArrAcess(i, true);

			case WRITE_ARRAY_TUPLE:
				return handleArrAcess(i, false);

			case IF: case WHILE:
				return handleLoops(i);

			case CONTINUE: 
				return handleContinue(i);

			case BREAK: 
				return handleBreak(i);

			case LBL:
				return i->operands[0]->str + "\n\t";

			case RET:
				return LA_Operatormap[RET] + "\n\t";

			case RETVAR: case JUMP:
				return LA_Operatormap[i->op] + i->operands[0]->str + "\n\t";

			case NEWTUPLE:
				return i->operands[0]->str + LA_Operatormap[NEWTUPLE] + i->indices[0]->str + ")\n\t"; 

			case LENGTH:
				return i->operands[0]->str + LA_Operatormap[LENGTH] + i->operands[1]->str + " " + i->operands[2]->str + "\n\t"; 

			default:
				return "";
		}
	}


	std::string handleCalls(Instruction* inst, bool isRetVar){

		std::string str;
		if(!isRetVar)
			str += inst->operands[0]->str + "(";
		else
			str += inst->operands[0]->str + LA_Operatormap[MOV] + inst->operands[1]->str + "(";
		if(inst->operands.size() > 1+isRetVar){
			for(int i = 1+isRetVar; i<inst->operands.size(); i++){
				if(i != inst->operands.size()-1)
					str+= inst->operands[i]->str + ", ";
				else
					str+= inst->operands[i]->str + ")\n\t";
			}
		}
		else
			str+= ")\n\t";
		return str;
	}


	std::string handleNewArray(Instruction* inst){
		std::string str;
		str += inst->operands[0]->str + LA_Operatormap[NEWARRAY];
		for(int j=0; j<inst->indices.size(); j++){
			if(j != inst->indices.size()-1)
				str+= inst->indices[j]->str + ", ";
			else
				str+= inst->indices[j]->str + ")\n\t";	
		}
		return str;
	}


	std::string handleArrAcess(Instruction* inst, bool isRead){
		std::string str;
		if(!isRead)
			str += inst->operands[0]->str + "[";
		else
			str += inst->operands[0]->str + LA_Operatormap[MOV] + inst->operands[1]->str + "[";
	
		for(int j=0; j<inst->indices.size(); j++){
			if(j != inst->indices.size()-1)
				str+= inst->indices[j]->str + "][";
			else
				str+= inst->indices[j]->str + "]";	
		}
		if(!isRead)
			str+= LA_Operatormap[MOV] + inst->operands[1]->str + "\n\t";
		else
			str+= "\n\t";
		return str;
	}

	std::string handleLoops(Instruction* inst){
		std::string str;
		str+= "int64 " + uniqueVar + to_string(var_count)+ "\n\t";
		str+= uniqueVar + to_string(var_count) + LA_Operatormap[MOV] +  inst->operands[0]->str + LA_Operatormap[inst->cond]  + inst->operands[1]->str + "\n\t";
		str+= LA_Operatormap[JUMP] + uniqueVar + to_string(var_count) + inst->operands[2]->str + " " + inst->operands[3]->str + "\n\t";
		var_count++;
		return str;
	}

	std::string handleContinue(Instruction* inst){
		std::string jmpLabel = inst->loop->condLabel->Inst;
		return LA_Operatormap[JUMP] + jmpLabel + "\n\t";
	}

	std::string handleBreak(Instruction* inst){
		std::string jmpLabel = inst->loop->operands[3]->str;
		return LA_Operatormap[JUMP] + jmpLabel + "\n\t";
	}

 }
