
#include "code_generator.h"

using namespace std;

namespace L2{

	int localVars;

	// map each Operator to respective string
	std::map<Operator, std::string> operatormap = {
        {MOVQ, " <- "}, 
        {ADDQ, " += "}, 
        {SUBQ, " -= "}, 
        {IMULQ, " *= "}, 
        {ANDQ, " &= "}, 
        {DEC, "--"}, 
        {INC, "++"}, 
        {LQ, " < "}, 
        {LEQ, " <= "}, 
        {EQ, " = "},
        {JLE, " <= "}, 
        {JL, " < "},
        {JE, " = "}, 
        {SALQ, " <<= "}, 
        {SARQ, " >>= "},
        {LEA, " @ "},
        {CALL, "call "},
        {GOTO, "goto "}
	    };

	void generate_code(Program p, int Verbose){
	/*
	* Perform graph_coloring and compress instructions for each function.
	*/
	for(auto f: p.functions){
		color_iGraph(f, Verbose);
		condenseFunc(f);
	}
	/*
	* Open the output file.
	*/
	std::ofstream outputFile;
	outputFile.open("prog.L1");
	  	/*
		* Generate target code
		*/ 
	outputFile<< "(" << p.entryPointLabel <<endl;
    for (auto f : p.functions){
    	localVars = f->locals;
      	outputFile<< "\t (" << f->name<<endl;
      	outputFile<< "\n\t\t" << f->arguments << " " << f->locals << "\n\t\t";
		for (auto i : f->instructions)
			outputFile<< translate_instruction(i);
      	outputFile<< "\n\t )" <<endl;
    }
    outputFile<< ")" <<endl;
	  	/*
		* Close the output file.
		*/
	outputFile.close();
	return ;
 	}

 	void condenseFunc(Function * f){
 	/*
 		ex: v1 += 3
		v1 += 6
		will be condensed into v1 += 9
		also,
		instructions like v1<-v1 , v1+=0 and, v1*= 1 will be deleted.
	*/
 		bool flag = 0;
	 	do{
 			flag = 0;
	 		for(int i=0; i<f->instructions.size(); i++){
	 			if ((f->instructions[i]->op == ADDQ) || (f->instructions[i]->op == SUBQ) || (f->instructions[i]->op == SALQ) || (f->instructions[i]->op == SARQ)){
	 				if(f->instructions[i+1]->op == f->instructions[i]->op){
						if((f->instructions[i]->operands[1]->op_type == NUM) && (f->instructions[i+1]->operands[1]->op_type == NUM)
	 					&& (f->instructions[i]->operands[0]->str == f->instructions[i+1]->operands[0]->str)) {
			 				f->instructions[i+1]->operands[1]->str = to_string(stoll(f->instructions[i]->operands[1]->str)+stoll(f->instructions[i+1]->operands[1]->str));
			 				f->instructions.erase(f->instructions.begin() + i);
			 				flag = 1;
			 				break;
			 			}
			 		}
	 			}
	 			if(f->instructions[i]->op == MOVQ){
	 				if(f->instructions[i]->operands[0]->str ==  f->instructions[i]->operands[1]->str){
	 					f->instructions.erase(f->instructions.begin() + i);
			 			flag = 1;
			 			break;
	 				}
	 			}
	 			if(f->instructions[i]->op == IMULQ){
	 				if(f->instructions[i]->operands[1]->str ==  "1"){
	 					f->instructions.erase(f->instructions.begin() + i);
			 			flag = 1;
			 			break;
	 				}
	 			}
	 			if((f->instructions[i]->op == ADDQ) || (f->instructions[i]->op == SUBQ) || (f->instructions[i]->op == SARQ) || (f->instructions[i]->op == SALQ)){
	 				if(f->instructions[i]->operands[1]->str ==  "0"){
	 					f->instructions.erase(f->instructions.begin() + i);
			 			flag = 1;
			 			break;
	 				}
	 			}
	 		}
	 	}while(flag);
 		return;
 	}

	std::string translate_operand(Operand* oprnd ){ //Called for translating 'mem x M' and 'stack-args M' operands
		if(oprnd->op_type == SARG){
			return "mem rsp " + to_string(oprnd->Mvalue + localVars*8);
		}
		else if((oprnd->op_type == MEMVAR) || (oprnd->op_type == MEMREG)){
			return "mem " + oprnd->X + " " + to_string(oprnd->Mvalue);
		}
		else
			return oprnd->str;
	}

	std::string translate_instruction(Instruction * i ){

		for (auto &op : i->operands){
			op->str = translate_operand(op);  //translates special operands : mem X M and stack-args M
		}

		switch(i->op){
			case INC: case DEC: //Instructions with 1 operand
				return i->operands[0]->str + operatormap[i->op] + "\n\t\t";

			case LEA: //Instructions with 4 operands
				return i->operands[0]->str + operatormap[i->op] + i->operands[1]->str + " " +  i->operands[2]->str + " " + i->operands[3]->str + "\n\t\t";

			case CALL: //Call instructions
				return operatormap[i->op] + i->operands[0]->str + " " + i->operands[1]->str + "\n\t\t";

			case GOTO: //GOTO instructions
				return operatormap[i->op] + i->operands[0]->str + "\n\t\t";

			case LBL:  case ARRAYERROR: case ALLOCATE: case PRINT: case RETQ: // Instructions with no operands                  
				return i->Inst + "\n\t\t";

			case LQ: case LEQ: case EQ: // Compare Instructions  
				return i->operands[0]->str + operatormap[MOVQ] + i->operands[1]->str + operatormap[i->op] + i->operands[2]->str + "\n\t\t";

		  	case JL: case JLE: case JE: // Cjump Instructions  
		  		if (i->operands.size() == 4)
		  			return "cjump " + i->operands[0]->str + operatormap[i->op] + i->operands[1]->str + " " + i->operands[2]->str + " " + i->operands[3]->str + "\n\t\t";
				else
					return "cjump " + i->operands[0]->str + operatormap[i->op] + i->operands[1]->str + " " + i->operands[2]->str + "\n\t\t";       
			default: //Instructions with 2 operands
				return i->operands[0]->str + operatormap[i->op] + i->operands[1]->str + "\n\t\t";   
		}
	}
}
