
#include "code_generator.h"

 using namespace std;

 namespace IR{


 	int var_count = 0; //used to generate new variables: newVar + var_count++
 	std::string uniqueVar = "";

 	void generateCode(Program p){

		std::ofstream outputFile;
		outputFile.open("prog.L3");
		preProcess(&p);
 		for(auto f: p.functions){
 			processBBs(f);
 			outputFile<< "define" << f->name<<"( ";
 			for(int i=0; i<f->arguments.size(); i++){
 				if(i == f->arguments.size()-1)
					outputFile<<f->arguments[i]->str;
				else
					outputFile<<f->arguments[i]->str<<", ";
			}
			outputFile<<" ){"<<endl;
 		
 			for(auto i : f->bblocks){
	 			outputFile<< "\t" << translateBBs(i)<<endl;	
 			}
 			outputFile<< "}" <<endl;
 			var_count = 0;
 		}
		outputFile.close();
		return;
 	}


 	void preProcess(Program* p){
		for(auto f : p->functions){
				std::reverse(f->arguments.begin(), f->arguments.end());   
			for(auto bb : f->bblocks){
				for(auto i : bb->instructions){
					if(i->op == READ_ARRAY){
						for(auto tuple : f->tupleDefs){
							if(i->operands[1]->str == tuple) i->op = READ_TUPLE;
						}   
					}
					else if(i->op == WRITE_ARRAY){
						for(auto tuple : f->tupleDefs){
							if(i->operands[0]->str == tuple) i->op = WRITE_TUPLE;
						}   
					}
				}
			}
		}

		std::string ll = "";
			for(auto f : p->functions){
				for(auto bb : f->bblocks){
					for(auto i: bb->instructions){
 						for(auto var : i->operands){
 							if(var->op_type == VAR){
	 							if (var->str.size() >=ll.size()) 
	 								ll = var->str;
	 						}
	 					}
	 				}
	 			}
	 		}
	 	if (ll.empty())
 			ll +=  "%newVar_";
 		else
 		ll +=  "_IRc_";
 		uniqueVar = ll;
		return;
	}


 	void processBBs(Function * f){
 		
 		for(int i = 0; i< f->bblocks.size()-1; i++){ //remove unconditional branches to successive block
 			if(f->bblocks[i]->exit->op == JUMP){
 				if(f->bblocks[i+1]->entry_label->operands[0]->str == f->bblocks[i]->exit->operands[0]->str)
 					f->bblocks[i]->removeExitInst = true;
 			}
 		}
 		return;
 	}

	std::map<Operator, std::string> L3_Operatormap = {
        {MOV, " <- "}, 
        {ADD, " + "}, 
        {SUB, " - "}, 
        {MUL, " * "}, 
        {AND, " & "}, 
        {LT, " < "}, 
        {LE, " <= "}, 
        {EQ, " = "},
        {GT, " < "}, 
        {GE, " <= "},
        {CJUMP, "br "}, 
        {SAL, " << "}, 
        {SAR, " >> "},
        {CALL, "call "},
        {JUMP, "br "},
        {RET, "return "}, 
        {RETVAR, "return "},
        {PRINT, "call print ("},
        {ARRAYERROR, "call array-error ("},
        {NEWARRAY, " <- call allocate ("},
        {WRITE_ARRAY, "store "},
        {READ_ARRAY, "<- load "}
	    };


	std::string translateBBs(BasicBlock *bb){
		std::string return_str;
		return_str += bb->entry_label->Inst + "\n\t";
		for(auto i : bb->instructions)
			return_str += translateInstruction(i);
		if(!bb->removeExitInst){
			switch(bb->exit->op){
				case RETVAR: case RET: case JUMP:
					return_str += bb->exit->Inst + "\n\t"; //L3 inst is same as IR inst for these cases
					break;
				case CJUMP:
					return_str += L3_Operatormap[CJUMP] + bb->exit->operands[0]->str + " " + bb->exit->operands[1]->str + "\n\t";
					return_str += L3_Operatormap[JUMP] + bb->exit->operands[2]->str + "\n\t";
					break;
			}	
		}
		return return_str;
	}

	
 	std::string translateInstruction(Instruction * i ){

		switch(i->op){
			case MOV: case ADD: case SUB: case MUL: case AND: case LT: case LE: case EQ: case GE: case GT: case SAR: case SAL:
				return i->Inst + "\n\t"; //L3 inst is same as IR inst for these cases

			case CALL:
				return handleCalls(i, false);
			case CALL_RETVAR:
				return handleCalls(i, true);

			case PRINT:
				return L3_Operatormap[PRINT] + i->operands[0]->str + ")\n\t"; 
		
			case ARRAYERROR:
				return L3_Operatormap[ARRAYERROR] + i->operands[0]->str + ", " + i->operands[1]->str + ")\n\t";  

			case NEWARRAY: 
				return handleNewArray(i);

			case NEWTUPLE:
				return i->operands[0]->str + L3_Operatormap[NEWARRAY] + i->indices[0]->str + ",1)\n\t"; 

			case LENGTH:
				return handleLengthInst(i);

			case READ_ARRAY:
				return handleArrAcess(i, true);

			case WRITE_ARRAY:
				return handleArrAcess(i, false);

			case READ_TUPLE:
				return handleTupleAcess(i, true);

			case WRITE_TUPLE:
				return handleTupleAcess(i, false);

			default:
				return "";
		}
	}


	std::string handleCalls(Instruction* inst, bool isRetVar){

		std::string str;
		if(!isRetVar)
			str += L3_Operatormap[CALL] + inst->operands[0]->str + " (";
		else
			str += inst->operands[0]->str + L3_Operatormap[MOV] + L3_Operatormap[CALL] + inst->operands[1]->str + " (";
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
		int newvar_count = var_count;
		std::vector<std::string> newvar_vec;
		std::string str;
		std::string mulvar; 
		if(inst->indices.size() >1){
			for(auto idx : inst->indices){
				newvar_vec.push_back(uniqueVar + to_string(newvar_count));
				str+= newvar_vec.back() + L3_Operatormap[MOV] + idx->str +  L3_Operatormap[SAR] + "1\n\t"; 
				newvar_count++;
			}
			mulvar = uniqueVar + to_string(newvar_count);
			str+= mulvar + L3_Operatormap[MOV] + newvar_vec[0] + L3_Operatormap[MUL] + newvar_vec[1] + "\n\t"; 
			if(inst->indices.size() >2){
				for(int i = 2; i<inst->indices.size();  i++){
					str+= mulvar + L3_Operatormap[MOV] + mulvar + L3_Operatormap[MUL] + newvar_vec[i] + "\n\t"; 
					}
				}
				newvar_count++;
				str+= mulvar + L3_Operatormap[MOV] + mulvar +  L3_Operatormap[SAL] + "1\n\t"; 
				str+= mulvar + L3_Operatormap[MOV] + mulvar +  L3_Operatormap[ADD] + "1\n\t"; 
				str+= mulvar + L3_Operatormap[MOV] + mulvar +  L3_Operatormap[ADD] + to_string((1 + inst->indices.size())<<1) + "\n\t"; 
				str+= inst->operands[0]->str + L3_Operatormap[NEWARRAY] + mulvar + ",1)\n\t"; 
		}
		else{ //single-dim array
			str+= uniqueVar + to_string(newvar_count) + L3_Operatormap[MOV] + inst->indices[0]->str +  L3_Operatormap[ADD] + to_string((1 + inst->indices.size())<<1) + "\n\t"; 
			str+= inst->operands[0]->str + L3_Operatormap[NEWARRAY] + uniqueVar + to_string(newvar_count++) + ",1)\n\t"; 
		}
	
		str+= uniqueVar + to_string(newvar_count) + L3_Operatormap[MOV] + inst->operands[0]->str +  L3_Operatormap[ADD] + "8\n\t"; 
		str+= L3_Operatormap[WRITE_ARRAY] + uniqueVar + to_string(newvar_count) +  L3_Operatormap[MOV] + to_string(((inst->indices.size())<<1)+1) + "\n\t";
		newvar_count++;

		for(int i = 0; i<inst->indices.size();  i++){
			str+= uniqueVar + to_string(newvar_count) + L3_Operatormap[MOV] + inst->operands[0]->str +  L3_Operatormap[ADD] + to_string((i+2)*8) +"\n\t"; 
			str+= L3_Operatormap[WRITE_ARRAY] + uniqueVar + to_string(newvar_count) +  L3_Operatormap[MOV] + inst->indices[i]->str + "\n\t";
			newvar_count++;
		}
		var_count = newvar_count;
		return str;
	}


	std::string handleLengthInst(Instruction* inst){
		int newvar_count = var_count;
		std::string str;
		if( inst->operands[2]->op_type == NUM){ //generate less instructions if v1<- length array NUM 
			str+= uniqueVar + to_string(newvar_count) + L3_Operatormap[MOV] +  inst->operands[1]->str +  L3_Operatormap[ADD] + to_string((stoll(inst->operands[2]->str)*8)+16)  + "\n\t"; 
			str+= inst->operands[0]->str +L3_Operatormap[READ_ARRAY] + uniqueVar + to_string(newvar_count) + "\n\t";
			var_count++;
		}
		else{
			str+= uniqueVar + to_string(newvar_count) + L3_Operatormap[MOV] + inst->operands[2]->str + L3_Operatormap[MUL] + "8\n\t"; 
			str+= uniqueVar + to_string(newvar_count +1) + L3_Operatormap[MOV] +  uniqueVar + to_string(newvar_count) + L3_Operatormap[ADD] + "16\n\t"; 
			str+= uniqueVar + to_string(newvar_count +2) + L3_Operatormap[MOV] +  inst->operands[1]->str +  L3_Operatormap[ADD] +uniqueVar + to_string(newvar_count+1) + "\n\t"; 
			str+= inst->operands[0]->str +L3_Operatormap[READ_ARRAY] + uniqueVar + to_string(newvar_count+2) + "\n\t";
			var_count += 3;
		}
		return str;
	}


	std::string handleArrAcess(Instruction* inst, bool isRead){
		int newvar_count = var_count;
		std::string str;
		std::string sumvar;
		std::vector<std::string> dimLength_vec; 
		if(inst->indices.size() > 2){ //Scalable algorithm for arrays with more than 2 Dimensions
			for(int i= 1; i< inst->indices.size(); i++){
				dimLength_vec.push_back(uniqueVar + to_string(newvar_count+2));
				str+= uniqueVar + to_string(newvar_count) + L3_Operatormap[MOV] +  inst->operands[isRead]->str + L3_Operatormap[ADD] + to_string(24+((i-1)*8)) + "\n\t"; 
				str+= uniqueVar + to_string(newvar_count+1) + L3_Operatormap[READ_ARRAY] +uniqueVar + to_string(newvar_count) + "\n\t"; 
				str+= dimLength_vec.back() + L3_Operatormap[MOV] + uniqueVar + to_string(newvar_count+1) +  L3_Operatormap[SAR] + "1\n\t"; 
				newvar_count+=3;
			}

			std::vector<std::string> sum_vec;
			std::vector<std::string> lastmul_vec;
			for(int i= inst->indices.size()-1; i>=0; i--){
				if(i != inst->indices.size()-1){
					if(!lastmul_vec.empty()){
						str+= uniqueVar + to_string(newvar_count) + L3_Operatormap[MOV] + lastmul_vec.back() + L3_Operatormap[MUL] + dimLength_vec[i] + "\n\t";
						lastmul_vec.push_back(uniqueVar + to_string(newvar_count));
					}
					else{
						lastmul_vec.push_back(uniqueVar + to_string(newvar_count));
						str+= uniqueVar + to_string(newvar_count) + L3_Operatormap[MOV] + dimLength_vec[i] + "\n\t";
					}

					newvar_count++;
					sum_vec.push_back(uniqueVar + to_string(newvar_count));
					str+= sum_vec.back() + L3_Operatormap[MOV] + inst->indices[i]->str + L3_Operatormap[MUL] + lastmul_vec.back() + "\n\t";
					newvar_count++;
				}
				else{
					sum_vec.push_back(uniqueVar + to_string(newvar_count));
					str+= uniqueVar + to_string(newvar_count) + L3_Operatormap[MOV] + inst->indices[i]->str + L3_Operatormap[MUL] + "1\n\t"; 
					newvar_count++;	
				}
			}

			 
			if(sum_vec.size() >1){
				sumvar = uniqueVar + to_string(newvar_count);
				str+= sumvar + L3_Operatormap[MOV] + sum_vec[0] + L3_Operatormap[ADD] + sum_vec[1] + "\n\t"; 
				if(sum_vec.size() >2){
					for(int i = 2; i<sum_vec.size();  i++){
						str+= sumvar + L3_Operatormap[MOV] + sumvar + L3_Operatormap[ADD] + sum_vec[i] + "\n\t"; 
					}
				}
				newvar_count++;
			}
			else{
				sumvar = sum_vec[0];
			}
		}
		else if(inst->indices.size() == 2){ //generate less instructions if 2D array
			sumvar = uniqueVar + to_string(newvar_count++);
			str+= sumvar + L3_Operatormap[MOV] +  inst->operands[isRead]->str + L3_Operatormap[ADD] + to_string(24) + "\n\t";
			str+= sumvar + L3_Operatormap[READ_ARRAY] + sumvar + "\n\t"; 
			str+= sumvar + L3_Operatormap[MOV] + sumvar +  L3_Operatormap[SAR] + "1\n\t";
			str+= sumvar + L3_Operatormap[MOV] + inst->indices[0]->str +  L3_Operatormap[MUL] + sumvar + "\n\t";
			str+= sumvar + L3_Operatormap[MOV] + inst->indices[1]->str +  L3_Operatormap[ADD] +  sumvar + "\n\t";
			
		}
		else{ //generate  less instructions if 1D array
			sumvar = uniqueVar + to_string(newvar_count++);
			str+=  sumvar + L3_Operatormap[MOV] +  inst->indices[0]->str + "\n\t"; 
		}

		str+= sumvar + L3_Operatormap[MOV] + sumvar + L3_Operatormap[MUL] + "8\n\t"; 
		str+= sumvar + L3_Operatormap[MOV] + sumvar + L3_Operatormap[ADD] + to_string(16+ (inst->indices.size()*8)) +"\n\t"; 
		str+= sumvar + L3_Operatormap[MOV] + inst->operands[isRead]->str + L3_Operatormap[ADD] + sumvar +"\n\t";
		if(!isRead) 
			str+= L3_Operatormap[WRITE_ARRAY] + sumvar +  L3_Operatormap[MOV] + inst->operands[1]->str +"\n\t"; 
		else
			str+= inst->operands[0]->str +L3_Operatormap[READ_ARRAY] + sumvar + "\n\t";
		var_count = newvar_count;
		return str;
	}

	std::string handleTupleAcess(Instruction* inst, bool isRead){
		std::string str;
		int newvar_count = var_count;
		str+= uniqueVar + to_string(newvar_count) + L3_Operatormap[MOV] + inst->indices[0]->str + L3_Operatormap[MUL] + "8\n\t";
		str+= uniqueVar + to_string(newvar_count +1) + L3_Operatormap[MOV] + uniqueVar + to_string(newvar_count) + L3_Operatormap[ADD] +  "8\n\t"; 
		str+= uniqueVar + to_string(newvar_count +2) + L3_Operatormap[MOV] + inst->operands[isRead]->str + L3_Operatormap[ADD]  + uniqueVar + to_string(newvar_count+1) + "\n\t"; 
		if(!isRead) 
			str+= L3_Operatormap[WRITE_ARRAY] + uniqueVar + to_string(newvar_count +2)  +  L3_Operatormap[MOV] + inst->operands[1]->str +"\n\t"; 
		else
			str+= inst->operands[0]->str + L3_Operatormap[READ_ARRAY] + uniqueVar + to_string(newvar_count +2) + "\n\t";

		var_count += 3;
		return str;
	}
}
