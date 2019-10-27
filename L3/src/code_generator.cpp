
#include "code_generator.h"
#include "inst_selector.h"

 using namespace std;

 namespace L3{


 	void generateCode(Program p, int Verbose){

 		renameLabels(&p);
  		instructionSelection(&p, Verbose);
		std::ofstream outputFile;
		outputFile.open("prog.L2");
 		int localVars = 0;
		outputFile<< "(" << ":main" <<endl;
 		for(auto f: p.functions){
 			outputFile<< "\t(" << f->name<<endl;
 			outputFile<< "\t\t" << to_string(f->arguments.size()) << " " << to_string(localVars) <<endl;
 			outputFile<< "\t\t" << generatePrelude(f)<<endl;
 			int context_index = 0;
 			if(f->starts_with_context == true){
 				for (auto tree : f->contexts[context_index]->trees){
 					for (auto inst : tree->gen_insts){
 						outputFile<< "\t\t" <<inst<<endl;
 					}
 				}
 				context_index++;
 			}

 			for(auto i : f->instructions){
 				if((i->op == LBL) || (i->op == GOTO) || (i->op == CJUMP) ||  (i->op == RET) || (i->op == RETVAR)){
	 				outputFile<< "\t\t" << translateInstruction(i)<<endl;
	 				if(i->context_below == true){
	 					for (auto tree : f->contexts[context_index]->trees){
	 						for (auto inst : tree->gen_insts){
	 							outputFile<< "\t\t" <<inst<<endl;
	 						}
	 					}
	 					context_index++;
	 				}
	 			}
 			}
 			outputFile<< "\t )" <<endl;
 		}
		outputFile<< ")" <<endl;
		outputFile.close();
		return;
 	}


 	void renameLabels(Program* p){

 		std::string ll = "";

 		for(auto f : p->functions){
 			for(auto i: f->instructions){
 				for(auto var : i->operands){
	 				if((var->op_type == LABEL) && (i->op != CALL) && (i->op != CALL_RETVAR)){
	 					if (var->str.size() >=ll.size())
	 						ll = var->str;
	 				}
	 			}
 			}
 		}

 		if (ll.empty())
 			ll +=  ":newLabelL3_";
 		else
 		ll +=  "_";

 		for(auto f : p->functions){
 			for(auto i: f->instructions){
 				for(auto &&var : i->operands){
	 				if((var->op_type == LABEL) && (i->op != MOV) && (i->op != STORE) && (i->op != CALL) && (i->op != CALL_RETVAR)){
 						var->str = ll + f->name.substr(1, f->name.length() -1 ) + var->str.substr(1, var->str.length() -1 );
	 				}
 				}
 			}
 		}
 	}


	std::map<Operator, std::string> L2_Operatormap = {
        {MOV, " <- "}, 
        {ADD, " += "}, 
        {SUB, " -= "}, 
        {MUL, " *= "}, 
        {AND, " &= "}, 
        {LT, " < "}, 
        {LE, " <= "}, 
        {EQ, " = "},
        {GT, " < "}, 
        {GE, " <= "},
        {LOAD, " <- mem "}, 
        {STORE, "mem "},
        {CJUMP, "cjump "}, 
        {SAL, " <<= "}, 
        {SAR, " >>= "},
        {CALL, "call "},
        {GOTO, "goto "},
        {RET, "return "}, 
        {RETVAR, "return "}
	    };


 	std::string translateInstruction(Instruction * i ){

		switch(i->op){
			
			case GOTO: //GOTO instructions
				return L2_Operatormap[i->op] + i->operands[0]->str;

			case CJUMP: //GOTO instructions
				return L2_Operatormap[i->op] + i->operands[0]->str + L2_Operatormap[EQ] + " 1 " + i->operands[1]->str;

			case LBL:        
				return i->operands[0]->str;

			case RET:
				return "rax" + L2_Operatormap[MOV] + "0" + "\n\t\t" + L2_Operatormap[i->op]; //Killing rax before returning frees up rax to be used for register allocation in L2.

			case RETVAR:
			 	return "rax" + L2_Operatormap[MOV] + i->operands[0]->str + "\n\t\t" + L2_Operatormap[i->op];   

			default:
				return "";
		}
	}


	std::string generatePrelude(Function * f ){

		std::string str;
		int stackarg = 0;
		if(f->arguments.size()<7){
			for(int i = 0; i<f->arguments.size(); i++){
				str+= f->arguments[i]->str  + L2_Operatormap[MOV] + arg_registers[i] + "\n\t\t";
			}
		}
		else{
			for(int i = 0; i<6; i++){
				str+= f->arguments[i]->str  + L2_Operatormap[MOV] + arg_registers[i] + "\n\t\t";
			}
			for(int i = 6; i<f->arguments.size(); i++){
				str+= f->arguments[i]->str  + L2_Operatormap[MOV] + "stack-arg "+ to_string(stackarg) + "\n\t\t";
				stackarg+= 8;
			}
		}
		return str;
	}
 }
