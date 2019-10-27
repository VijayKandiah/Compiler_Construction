#include "code_generator.h"

using namespace std;
namespace LA{

	int var_count = 0; //used to generate new variables: %newVar + var_count++
	int lbl_count = 0; //used to generate new labels: :newLabel + lbl_count++
	std::string uniqueVar = "";
	std::string uniqueLabel = "";

 	void generateCode(Program p){
		std::ofstream outputFile;
		outputFile.open("prog.IR");
		preProcess(&p);
 		for(auto f: p.functions){
 			
 			formatVariables(f);
 			condenseFunc(f);
 			encodeConstants(f);
 			generateBBS(f);
 			outputFile<< "define " << f->retType<< " :"<< f->name<<"( ";
 			for(int i=0; i<f->arguments.size(); i++){
 				if(i == f->arguments.size()-1)
					outputFile<<f->arguments[i]->defType<<" "<<f->arguments[i]->str;
				else
					outputFile<<f->arguments[i]->defType<<" "<<f->arguments[i]->str<<", ";
			}
			outputFile<<" ){"<<endl;
 		
 			for(auto i : f->instructions){
	 			outputFile << translateInstruction(i);	
 			}
 			outputFile<< "}" <<endl;
 			var_count = 0;
 			lbl_count = 0;
 		}
		outputFile.close();
		return;
 	}


 	void generateBBS(Function * f){
 		int startBB = true;
 		std::vector<Instruction *> organizedInsts;
 		auto entryInst = new Instruction();
        auto entryOP = new Variable();
        entryOP->op_type = LABEL;
        entryOP->str = uniqueLabel + to_string(lbl_count++);
        entryInst->operands.push_back(entryOP);
        entryInst->op = LBL;
        entryInst->Inst = entryOP->str;
        f->instructions.insert(f->instructions.begin(), entryInst);
 		for(int i = 0; i< f->instructions.size(); i++){ 
 			if(startBB){
 				if(f->instructions[i]->op != LBL){
					auto newInst = new Instruction();
					auto newOP = new Variable();
					newOP->op_type = LABEL;
					newOP->str = uniqueLabel + to_string(lbl_count++);
					newInst->operands.push_back(newOP);
					newInst->op = LBL;
					newInst->Inst = newOP->str;
					organizedInsts.push_back(newInst);
 				}
 				startBB = false;
 			}
 			else if(f->instructions[i]->op == LBL){
				auto newInst = new Instruction();
				auto newOP = new Variable();
				newOP->op_type = LABEL;
				newOP->str = f->instructions[i]->operands[0]->str;
				newInst->operands.push_back(newOP);
				newInst->op = JUMP;
				newInst->Inst = "br "+ newOP->str;
				organizedInsts.push_back(newInst);
 			}
 			organizedInsts.push_back(f->instructions[i]);
 			if((f->instructions[i]->op == RET) || 
 				(f->instructions[i]->op == RETVAR) || 
 				(f->instructions[i]->op == JUMP) || 
 				(f->instructions[i]->op == CJUMP)){
 				startBB = true;
 			}
 		}
 		f->instructions = organizedInsts;  
 		return;			
 	}

 	void preProcess(Program* p){
		for(auto f : p->functions){
				std::reverse(f->arguments.begin(), f->arguments.end());   
			for(auto &i : f->instructions){
				if(i->op == READ_ARRAY){
					for(auto tuple : f->tupleDefs){
						if(i->operands[1]->str == tuple)  i->op = READ_TUPLE;
					}   
				}
				else if(i->op == WRITE_ARRAY){
					for(auto tuple : f->tupleDefs){
						if(i->operands[0]->str == tuple)  i->op = WRITE_TUPLE;
					}   
				}
			}
		}

		std::string lv = "";
		std::string ll = "";
			for(auto f : p->functions){
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
	 		
	 		}
	 	if (lv.empty())
 			lv +=  "%newVarLA_";
 		else
 		lv = "%" + lv +  "_";
 		if (ll.empty())
 			ll +=  ":newLabelLA_";
 		else
 		ll +=  "_";
 		uniqueVar = lv;
 		uniqueLabel = ll;
		return;
	}

	 void condenseFunc(Function * f){ //Convert Shift Lefts to MULS and Combine consecutive MUL instructions -- Done before value encoding so that L3 can generate better code
 		for(auto &i : f->instructions){
 			if((i->op == SAL) && (i->operands[2]->op_type == NUM)){
 				i->op = MUL; //Convert shifts into MULS
 				i->operands[2]->str = to_string(1 << stoll(i->operands[2]->str));
 			}
 		}


 		bool flag = 0;
	 	do{
 			flag = 0;
	 		for(int i=0; i<f->instructions.size(); i++){ //Combine consecutive MULS
	 			if ((f->instructions[i]->op == MUL) && (f->instructions[i]->operands[2]->op_type == NUM) 
	 				&& (f->instructions[i+1]->op == MUL) && (f->instructions[i+1]->operands[2]->op_type == NUM)
	 				&& (f->instructions[i]->operands[0]->str == f->instructions[i+1]->operands[1]->str)) {

	 				f->instructions[i+1]->operands[2]->str = to_string(stoll(f->instructions[i]->operands[2]->str)*stoll(f->instructions[i+1]->operands[2]->str));
	 				f->instructions.erase(f->instructions.begin() + i);
	 				flag = 1;
	 				break;
	 			}
	 		}
	 	}while(flag);


 		return;
 	}

	void formatVariables(Function * f){
		bool code_flag = false;
 	 	for(auto &arg: f->arguments){
 			if (arg->op_type == VAR)
 				arg->str = "%" + arg->str;
		}
 		for(auto i: f->instructions){
 			if((i->op != CALL) && (i->op != CALL_RETVAR) && (i->op != PRINT)){
	 			for(auto &&var : i->operands){
	 				if (var->op_type == VAR)
	 					var->str = "%" + var->str;
	 			}
	 			for(auto &&idx : i->indices){
	 				if (idx->op_type == VAR)
	 					idx->str = "%" + idx->str;
	 			}
 			}
 			else{
 				if((i->op == PRINT) && (i->operands[0]->op_type == VAR))
 					i->operands[0]->str = "%" + i->operands[0]->str;
 						
				if(i->op == CALL){
					for(auto code : f->codeDefs){
						if(i->operands[0]->str == code){
							i->operands[0]->str = "%" + i->operands[0]->str;
							code_flag = true;
							break;
						}
					}
					if(!code_flag)  i->operands[0]->str = ":" + i->operands[0]->str;
					for(int j=1; j<i->operands.size(); j++){
						if (i->operands[j]->op_type == VAR)
							i->operands[j]->str = "%" + i->operands[j]->str;
					}
 				}
 				code_flag = false;
 				if(i->op == CALL_RETVAR){
 					if (i->operands[0]->op_type == VAR)
 						i->operands[0]->str = "%" + i->operands[0]->str;
					for(auto code : f->codeDefs){
						if(i->operands[1]->str == code){
							i->operands[1]->str = "%" + i->operands[1]->str;
							code_flag = true;
							break;
						}
					}
					if(!code_flag)  i->operands[1]->str = ":" + i->operands[1]->str;   
					for(int j=2; j<i->operands.size(); j++){
						if (i->operands[j]->op_type == VAR)
							i->operands[j]->str = "%" + i->operands[j]->str;
					}
 				}
 			}	
	 	}
 		return;		
 	}

 	void encodeConstants(Function * f){
 	 	for(auto &arg: f->arguments){
 			if (arg->op_type == NUM) 
 				arg->str = to_string((stol(arg->str) << 1)+1);
		}
 		for(auto i: f->instructions){
 			for(auto &&var : i->operands){
 				if ((var->op_type == NUM) && (!var->dontEncode)){
 					var->str = to_string((stol(var->str) << 1)+1);
 				}
 			}
 			for(auto &&idx : i->indices){
 				if (idx->op_type == NUM)
 					idx->str = to_string((stol(idx->str) << 1)+1);
 			}
 		}
 		return;			
 	}

	std::map<Operator, std::string> IR_Operatormap = {
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
        {CJUMP, "br "}, 
        {SAL, " << "}, 
        {SAR, " >> "},
        {JUMP, "br "},
        {RET, "return "}, 
        {RETVAR, "return "},
        {PRINT, "call print ("},
        {LENGTH, " <- length "},
        {CALL, "call "},
        {CALL_RETVAR, " <- call "},
        {NEWARRAY, " <- new Array("},
        {NEWTUPLE, " <- new Tuple("}
	    };

	
 	std::string translateInstruction(Instruction * i ){

		switch(i->op){
			case DEFINE:
				return i->operands[0]->defType + " " + i->operands[0]->str + "\n\t";

			case MOV: 
				return i->operands[0]->str + IR_Operatormap[MOV] + i->operands[1]->str + "\n\t"; 

			case ADD: case SUB: case MUL: case AND: case LT: case LE: case EQ: case GE: case GT: case SAR: case SAL:
				return handleArithOp(i);

			case CALL:
				return handleCalls(i, false);
			case CALL_RETVAR:
				return handleCalls(i, true);

			case PRINT:
				return IR_Operatormap[PRINT] + i->operands[0]->str + ")\n\t"; 

			case NEWARRAY:
				return handleNewArray(i);

			case NEWTUPLE:
				return i->operands[0]->str + IR_Operatormap[NEWTUPLE] + i->indices[0]->str + ")\n\t"; 

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

			case RET:
				return IR_Operatormap[RET] + "\n\t";

			case RETVAR: case JUMP:
				return IR_Operatormap[i->op] + i->operands[0]->str + "\n\t";

			case CJUMP:
				return handleBranch(i);

			case LBL:
				return i->operands[0]->str + "\n\t";

			default:
				return "";
		}
	}


	std::string handleCalls(Instruction* inst, bool isRetVar){

		std::string str;
		if(!isRetVar)
			str += IR_Operatormap[CALL] + inst->operands[0]->str + "(";
		else
			str += inst->operands[0]->str + IR_Operatormap[MOV] + IR_Operatormap[CALL] + inst->operands[1]->str + "(";
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
		str += inst->operands[0]->str + IR_Operatormap[NEWARRAY];
		for(int j=0; j<inst->indices.size(); j++){
			if(j != inst->indices.size()-1)
				str+= inst->indices[j]->str + ", ";
			else
				str+= inst->indices[j]->str + ")\n\t";	
		}
		return str;
	}


	std::string handleLengthInst(Instruction* inst){
		std::string str;
		if(inst->operands[2]->op_type == NUM) 
			str+= inst->operands[0]->str + IR_Operatormap[LENGTH] + inst->operands[1]->str + " " + to_string(stol(inst->operands[2]->str)>>1) + "\n\t";	
		else{
			str+= "int64 " + uniqueVar + to_string(var_count)+ "\n\t";
			str+= uniqueVar + to_string(var_count) + IR_Operatormap[MOV] + inst->operands[2]->str + IR_Operatormap[SAR] +  "1\n\t";
			str+= inst->operands[0]->str + IR_Operatormap[LENGTH] + inst->operands[1]->str + " " + uniqueVar + to_string(var_count) + "\n\t";
		}
		var_count++;
		return str;
	}

	std::string handleBranch(Instruction* inst){
		std::string str;
		if(inst->operands[0]->op_type == NUM)
			str+= IR_Operatormap[CJUMP]+ to_string(stol(inst->operands[0]->str)>>1) + " " + inst->operands[1]->str + " " +  inst->operands[2]->str + "\n\t";	
		else{
			str+= "int64 " + uniqueVar + to_string(var_count)+ "\n\t";
			str+= uniqueVar + to_string(var_count) + IR_Operatormap[MOV] + inst->operands[0]->str + IR_Operatormap[SAR] +  "1\n\t";
			str+= IR_Operatormap[CJUMP]+ uniqueVar + to_string(var_count) + " " + inst->operands[1]->str + " " +  inst->operands[2]->str + "\n\t";
		}
		var_count++;
		return str;
	}

	std::string handleArithOp(Instruction* inst){
		std::string str;
		if(inst->operands[1]->op_type == NUM){
			if(inst->operands[2]->op_type == NUM){
				str+= inst->operands[0]->str + IR_Operatormap[MOV] + to_string(stol(inst->operands[1]->str)>>1) +  IR_Operatormap[inst->op] +  to_string(stol(inst->operands[2]->str)>>1)+ "\n\t";
			}
			else{
				if((inst->op == ADD) || (inst->op == SUB) || (inst->op == AND)) {
					str+= inst->operands[0]->str + IR_Operatormap[MOV] + to_string(stol(inst->operands[1]->str)-1) +  IR_Operatormap[inst->op] +  inst->operands[2]->str + "\n\t";
					return str;
				}
				else{
					str+= "int64 " + uniqueVar + to_string(var_count)+ "\n\t";
					str+= uniqueVar + to_string(var_count) + IR_Operatormap[MOV] + inst->operands[2]->str + IR_Operatormap[SAR] +  "1\n\t";
					str+= inst->operands[0]->str + IR_Operatormap[MOV] + to_string(stol(inst->operands[1]->str)>>1) +  IR_Operatormap[inst->op] +  uniqueVar + to_string(var_count++) + "\n\t";
				}
			}
		}
		else if(inst->operands[2]->op_type == NUM){
			if((inst->op == ADD) || (inst->op == SUB) || (inst->op == AND)) {
				str+= inst->operands[0]->str + IR_Operatormap[MOV] + inst->operands[1]->str +  IR_Operatormap[inst->op] +  to_string(stol(inst->operands[2]->str)-1) + "\n\t";
				return str;
			}
			else{
				str+= "int64 " + uniqueVar + to_string(var_count)+ "\n\t";
				str+= uniqueVar + to_string(var_count) + IR_Operatormap[MOV] + inst->operands[1]->str + IR_Operatormap[SAR] +  "1\n\t";
				str+= inst->operands[0]->str + IR_Operatormap[MOV] + uniqueVar + to_string(var_count++) +  IR_Operatormap[inst->op] +  to_string(stol(inst->operands[2]->str)>>1) + "\n\t";
			}
		}
		else{
			str+= "int64 " + uniqueVar + to_string(var_count)+ "\n\t";
			str+= "int64 " + uniqueVar + to_string(var_count+1)+ "\n\t";
			str+= uniqueVar + to_string(var_count) + IR_Operatormap[MOV] + inst->operands[1]->str + IR_Operatormap[SAR] +  "1\n\t";
			str+= uniqueVar + to_string(var_count+1) + IR_Operatormap[MOV] + inst->operands[2]->str + IR_Operatormap[SAR] +  "1\n\t";
			str+= inst->operands[0]->str + IR_Operatormap[MOV] + uniqueVar + to_string(var_count) +  IR_Operatormap[inst->op] +  uniqueVar + to_string(var_count+1) + "\n\t";
			var_count+=2;
		}
		str+= inst->operands[0]->str + IR_Operatormap[MOV] + inst->operands[0]->str + IR_Operatormap[SAL] +  "1\n\t";
		str+= inst->operands[0]->str + IR_Operatormap[MOV] + inst->operands[0]->str + IR_Operatormap[ADD] +  "1\n\t";
		return str;
	}


	std::string handleArrAcess(Instruction* inst, bool isRead){
		std::string str;
		int newvar_count = var_count;
		str+= "int64 " + uniqueVar + to_string(newvar_count)+ "\n\t";
		str+= uniqueVar + to_string(newvar_count) + IR_Operatormap[MOV] + inst->operands[isRead]->str + IR_Operatormap[EQ] +  "0\n\t";
		str+= IR_Operatormap[CJUMP]+ uniqueVar + to_string(newvar_count) + " " + uniqueLabel + to_string(lbl_count) + " " + uniqueLabel + to_string(lbl_count+1) + "\n\t";
		str+= uniqueLabel + to_string(lbl_count)+ "\n\t";
		str+= "call array-error(0,0)\n\t";
		str+= IR_Operatormap[JUMP] + uniqueLabel + to_string(lbl_count+1) + "\n\t";
		str+= uniqueLabel + to_string(lbl_count+1)+ "\n\t";
		newvar_count++;
		lbl_count+= 2;
		std::vector<std::string> dimLength_vec;
		
		for(int i =0; i< inst->indices.size(); i++){
			str+= "int64 " + uniqueVar + to_string(newvar_count)+ "\n\t";
			dimLength_vec.push_back(uniqueVar + to_string(newvar_count));
			str+= dimLength_vec.back() + IR_Operatormap[MOV] + inst->indices[i]->str + IR_Operatormap[SAR] +  "1\n\t";
			newvar_count++;

			str+= "int64 " + uniqueVar + to_string(newvar_count)+ "\n\t";
			str+= "int64 " + uniqueVar + to_string(newvar_count+1)+ "\n\t";
			str+= uniqueVar+ to_string(newvar_count) + IR_Operatormap[LENGTH] + inst->operands[isRead]->str + " " + to_string(i) + "\n\t";
			str+= uniqueVar+ to_string(newvar_count) + IR_Operatormap[MOV] + uniqueVar+ to_string(newvar_count) + IR_Operatormap[SAR] +  "1\n\t";

			str+= uniqueVar + to_string(newvar_count+1) + IR_Operatormap[MOV] + dimLength_vec.back() + IR_Operatormap[GE] + uniqueVar+ to_string(newvar_count) + "\n\t";
			str+= IR_Operatormap[CJUMP]+ uniqueVar + to_string(newvar_count+1) + " " + uniqueLabel + to_string(lbl_count) + " " + uniqueLabel + to_string(lbl_count+1) + "\n\t";

			str+= uniqueLabel + to_string(lbl_count)+ "\n\t";
			str+= "call array-error(" +  inst->operands[isRead]->str + ", " + inst->indices[i]->str +")\n\t";
			str+= IR_Operatormap[JUMP] + uniqueLabel + to_string(lbl_count+1) + "\n\t";
			str+= uniqueLabel + to_string(lbl_count+1)+ "\n\t";
			lbl_count+=2;
			newvar_count+=2;
		}

		if(isRead){
			str+= inst->operands[0]->str + IR_Operatormap[MOV] + inst->operands[1]->str;
			for(int i =0; i< dimLength_vec.size(); i++){
				str+= "[" + dimLength_vec[i] + "]";
			}
			str+= "\n\t";
		}
		else{
			str+= inst->operands[0]->str; 
			for(int i =0; i< dimLength_vec.size(); i++){
				str+= "[" + dimLength_vec[i] + "]";
			}
			str+= IR_Operatormap[MOV] + inst->operands[1]->str + "\n\t";
		}
		var_count = newvar_count;
		return str;
	}

	std::string handleTupleAcess(Instruction* inst, bool isRead){
		std::string str;
		int newvar_count = var_count;
		str+= "int64 " + uniqueVar + to_string(newvar_count)+ "\n\t";
		str+= uniqueVar + to_string(newvar_count) + IR_Operatormap[MOV] + inst->operands[isRead]->str + IR_Operatormap[EQ] +  "0\n\t";
		str+= IR_Operatormap[CJUMP]+ uniqueVar + to_string(newvar_count) + " " + uniqueLabel + to_string(lbl_count) + " " + uniqueLabel + to_string(lbl_count+1) + "\n\t";
		str+= uniqueLabel + to_string(lbl_count)+ "\n\t";
		str+= "call array-error(0,0)\n\t";
		str+= IR_Operatormap[JUMP] + uniqueLabel + to_string(lbl_count+1) + "\n\t";
		str+= uniqueLabel + to_string(lbl_count+1) + "\n\t";
		newvar_count++;
		lbl_count+= 2;
		str+= "int64 " + uniqueVar + to_string(newvar_count) + "\n\t";
		str+= uniqueVar + to_string(newvar_count) + IR_Operatormap[MOV] + inst->indices[0]->str + IR_Operatormap[SAR] +  "1\n\t";
		if(isRead){
			str+= inst->operands[0]->str + IR_Operatormap[MOV] + inst->operands[1]->str + "[" + uniqueVar + to_string(newvar_count) + "]\n\t";
		}
		else{
			str+= inst->operands[0]->str + "[" + uniqueVar + to_string(newvar_count) + "]" + IR_Operatormap[MOV] + inst->operands[1]->str + "\n\t";
		}
		var_count = ++newvar_count;
		return str;
	}
 }
