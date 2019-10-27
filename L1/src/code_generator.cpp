#include <string>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>


#include "code_generator.h"

using namespace std;

namespace L1{
void generate_code(Program p){

	/*
	* Open the output file.
	*/
	std::ofstream outputFile;
	outputFile.open("prog.S");


	//Clean operands
	p.entryPointLabel = "_" + p.entryPointLabel.substr(1, p.entryPointLabel.length() - 1); //clean entry labels
	for (auto f : p.functions){
		f->name = "_" + f->name.substr(1, f->name.length() - 1); //clean function name
		for (auto i : f->instructions){
			for (auto &&op : i->operands){
				switch (op->op_type){
					case GPREG:
						op->str.insert(0,"%"); // clean all register operands
						break;
					case LABEL:
						op->str[0] = '_'; //clean all labels
						break;
					case MEM:       
					{
					  	std::vector<std::string> result; //clean all mem x M
					  	std::string mem = op->str;
					  	std::istringstream iss(mem);
					  	for(std::string mem; iss >> mem; )
							result.push_back(mem);
					  	op->str = result[2] + "(%" + result[1] + ")"; // M(%x)
					  	break;
				 	}
				 	
					default:
						op->str.insert(0,"$");
				}
			}
		}
	}


	  /*
		* Generate target code
		tip: i->operands[n] = nth operand read, i->operands[0] = first operand read
		*/ 
	  //generate default code
	  outputFile << "\t.text\n\t.globl go\ngo:\n\t# save calle-saved registers\n\t"
			<<"pushq %rbx\n\tpushq %rbp\n\tpushq %r12\n\tpushq %r13\n\tpushq %r14\n\tpushq %r15\n\n\t"
			<<"call " + p.entryPointLabel<<"\n\n\t# restore callee-saved registers and return\n\t"
			<<"popq %r15\n\tpopq %r14\n\tpopq %r13\n\tpopq %r12\n\tpopq %rbp\n\tpopq %rbx\n\tretq\n";

	  	for (auto f : p.functions){
			outputFile << f->name + ":" <<endl;
			if (f->locals > 0) outputFile << "\tsubq $" + to_string(f->locals*8) + ", %rsp \t#Allocate locals\n"; //Allocate locals
			for (auto i : f->instructions){
				switch(i->op){
					case MOVQ:
						if(i->operands[1]->op_type == LABEL) 
							i->operands[1]->str.insert(0,"$"); //prefix $ for labels
						outputFile << "\tmovq " + i->operands[1]->str + ", " + i->operands[0]->str<<endl;
						break;
					case ADDQ: 
						outputFile << "\taddq " + i->operands[1]->str + ", " + i->operands[0]->str<<endl;
						break; 
					case SUBQ: 
						outputFile << "\tsubq " + i->operands[1]->str + ", " + i->operands[0]->str<<endl;
						break;
					case IMULQ:
						outputFile << "\timulq " + i->operands[1]->str + ", " + i->operands[0]->str<<endl;
						break;
					case ANDQ:
						outputFile << "\tandq " + i->operands[1]->str + ", " + i->operands[0]->str<<endl;
						break;
					case DEC:
						outputFile << "\tdec " + i->operands[0]->str<<endl;
						break;
					case INC:
						outputFile << "\tinc " + i->operands[0]->str<<endl;
						break;
					case SALQ:
						if (i->operands[1]->op_type == GPREG) 
							i->operands[1]->str = "%cl";
						outputFile << "\tsalq " + i->operands[1]->str + ", " + i->operands[0]->str<<endl;
						break;
					case SARQ:
						if (i->operands[1]->op_type == GPREG) 
							i->operands[1]->str = "%cl";
						outputFile << "\tsarq " + i->operands[1]->str + ", " + i->operands[0]->str<<endl;
						break;
					case LEA:
						outputFile << "\tlea (" + i->operands[1]->str + ", " + i->operands[2]->str + ", " + i->operands[3]->str.substr(1, i->operands[3]->str.length() - 1) + "), " + i->operands[0]->str<<endl;
						break;
					case CALL:
					{
						auto arg = std::stoi(i->operands[1]->str.substr(1, i->operands[1]->str.length() - 1));
						std::string str;
						if (arg > 6) str = to_string(((arg-6)*8) + 8);
						else str =to_string(8); 
						if (i->operands[0]->op_type == GPREG) 
							i->operands[0]->str.insert(0,"*");
						outputFile << "\tsubq $" + str + ", %rsp\n"; //Allocate locals
						outputFile << "\tjmp " + i->operands[0]->str<<endl;
						break;
					}   
					case GOTO:
						outputFile << "\tjmp " + i->operands[0]->str <<endl;
						break; 
					case PRINT:
						outputFile << "\tcall print"<<endl;
						break;
					case ALLOCATE:
						outputFile << "\tcall allocate"<<endl;
						break;
					case ARRAYERROR:
						outputFile << "\tcall array_error"<<endl;
						break;
					case LBL:                        
						outputFile << "_" + i->Inst.substr(1, i->Inst.length() - 1) + ":" <<endl;
						break; 
					case RETQ:
					{
						std::string str;
						if (f->arguments > 6) str = to_string(((f->arguments-6)*8) + f->locals*8);
						else str = to_string(f->locals*8);                            
						if(str != "0")outputFile << "\taddq $" + str + ", %rsp \n\tretq\n"; //Deallocate Locals and return
						else outputFile << "\tretq\n"; //return
						break; 
					}
					case LQ:
						if ((i->operands[2]->op_type == NUM) && (i->operands[1]->op_type == NUM)){
							if(std::stoi(i->operands[1]->str.substr(1, i->operands[1]->str.length() - 1)) < std::stoi(i->operands[2]->str.substr(1, i->operands[2]->str.length() - 1)))
								outputFile << "\tmovq $1, " + i->operands[0]->str<<endl;
							else
								outputFile << "\tmovq $0, " + i->operands[0]->str<<endl;
						}
						else if(i->operands[1]->op_type == NUM) {
							outputFile << "\tcmpq " + i->operands[1]->str + ", " + i->operands[2]->str<<endl;
							outputFile << "\tsetg " + byteregmap(i->operands[0]->str) <<endl;
							outputFile << "\tmovzbq " + byteregmap(i->operands[0]->str) + ", " + i->operands[0]->str<<endl;
						}
						else{
							outputFile << "\tcmpq " + i->operands[2]->str + ", " + i->operands[1]->str<<endl;
							outputFile << "\tsetl " + byteregmap(i->operands[0]->str) <<endl;
							outputFile << "\tmovzbq " + byteregmap(i->operands[0]->str) + ", " + i->operands[0]->str<<endl;
						}
						break;
					case LEQ:
						if ((i->operands[2]->op_type == NUM) && (i->operands[1]->op_type == NUM)){
							if(std::stoi(i->operands[1]->str.substr(1, i->operands[1]->str.length() - 1)) <= std::stoi(i->operands[2]->str.substr(1, i->operands[2]->str.length() - 1)))
								outputFile << "\tmovq $1, " + i->operands[0]->str<<endl;
							else
								outputFile << "\tmovq $0, " + i->operands[0]->str<<endl;
							}
							else if(i->operands[1]->op_type == NUM) {
								outputFile << "\tcmpq " + i->operands[1]->str + ", " + i->operands[2]->str<<endl;
								outputFile << "\tsetge " + byteregmap(i->operands[0]->str) <<endl;
								outputFile << "\tmovzbq " + byteregmap(i->operands[0]->str) + ", " + i->operands[0]->str<<endl;
							}
							else{
								outputFile << "\tcmpq " + i->operands[2]->str + ", " + i->operands[1]->str<<endl;
								outputFile << "\tsetle " + byteregmap(i->operands[0]->str) <<endl;
								outputFile << "\tmovzbq " + byteregmap(i->operands[0]->str) + ", " + i->operands[0]->str<<endl;
							}
							break;
				  	case EQ:
						if ((i->operands[2]->op_type == NUM) && (i->operands[1]->op_type == NUM)){
							if(std::stoi(i->operands[1]->str.substr(1, i->operands[1]->str.length() - 1)) == std::stoi(i->operands[2]->str.substr(1, i->operands[2]->str.length() - 1)))
								outputFile << "\tmovq $1, " + i->operands[0]->str<<endl;
							else
								outputFile << "\tmovq $0, " + i->operands[0]->str<<endl;
						}
						else if(i->operands[1]->op_type == NUM) {
							outputFile << "\tcmpq " + i->operands[1]->str + ", " + i->operands[2]->str<<endl;
							outputFile << "\tsete " + byteregmap(i->operands[0]->str) <<endl;
							outputFile << "\tmovzbq " + byteregmap(i->operands[0]->str) + ", " + i->operands[0]->str<<endl;
						}
						else{
							outputFile << "\tcmpq " + i->operands[2]->str + ", " + i->operands[1]->str<<endl;
							outputFile << "\tsete " + byteregmap(i->operands[0]->str) <<endl;
							outputFile << "\tmovzbq " + byteregmap(i->operands[0]->str) + ", " + i->operands[0]->str<<endl;
						}
						break;
				  	case JL:
						if ((i->operands[0]->op_type == NUM) && (i->operands[1]->op_type == NUM)){
							if(std::stoi(i->operands[0]->str.substr(1, i->operands[0]->str.length() - 1)) < std::stoi(i->operands[1]->str.substr(1, i->operands[1]->str.length() - 1)))
								outputFile << "\tjmp " + i->operands[2]->str<<endl;
							else
								if (i->operands.size() == 4)
									outputFile << "\tjmp " + i->operands[3]->str<<endl;
						}
						else if(i->operands[0]->op_type == NUM) {
							outputFile << "\tcmpq " + i->operands[0]->str + ", " + i->operands[1]->str<<endl;
							outputFile << "\tjg " + i->operands[2]->str<<endl;
							if (i->operands.size() == 4)
								outputFile << "\tjmp " + i->operands[3]->str<<endl;
						}
						else{
							outputFile << "\tcmpq " + i->operands[1]->str + ", " + i->operands[0]->str<<endl;
							outputFile << "\tjl " + i->operands[2]->str<<endl;
							if (i->operands.size() == 4)
								outputFile << "\tjmp " + i->operands[3]->str<<endl;
						}
						break;
				  	case JLE:
						if ((i->operands[0]->op_type == NUM) && (i->operands[1]->op_type == NUM)){
							if(std::stoi(i->operands[0]->str.substr(1, i->operands[0]->str.length() - 1)) <= std::stoi(i->operands[1]->str.substr(1, i->operands[1]->str.length() - 1)))
								outputFile << "\tjmp " + i->operands[2]->str<<endl;
							else
								if (i->operands.size() == 4)
									outputFile << "\tjmp " + i->operands[3]->str<<endl;
						}
						else if(i->operands[0]->op_type == NUM) {
							outputFile << "\tcmpq " + i->operands[0]->str + ", " + i->operands[1]->str<<endl;
							outputFile << "\tjge " + i->operands[2]->str<<endl;
							if (i->operands.size() == 4)
								outputFile << "\tjmp " + i->operands[3]->str<<endl;
						}
						else{
							outputFile << "\tcmpq " + i->operands[1]->str + ", " + i->operands[0]->str<<endl;
							outputFile << "\tjle " + i->operands[2]->str<<endl;
							if (i->operands.size() == 4)
								outputFile << "\tjmp " + i->operands[3]->str<<endl;
						}
						break;
				  case JE:
						if ((i->operands[0]->op_type == NUM) && (i->operands[1]->op_type == NUM)){
							if(std::stoi(i->operands[0]->str.substr(1, i->operands[0]->str.length() - 1)) == std::stoi(i->operands[1]->str.substr(1, i->operands[1]->str.length() - 1)))
								outputFile << "\tjmp " + i->operands[2]->str<<endl;
							else
								if (i->operands.size() == 4)
									outputFile << "\tjmp " + i->operands[3]->str<<endl;
						}
						else if(i->operands[0]->op_type == NUM) {
							outputFile << "\tcmpq " + i->operands[0]->str + ", " + i->operands[1]->str<<endl;
							outputFile << "\tje " + i->operands[2]->str<<endl;
							if (i->operands.size() == 4)
								outputFile << "\tjmp " + i->operands[3]->str<<endl;
						}
						else{
							outputFile << "\tcmpq " + i->operands[1]->str + ", " + i->operands[0]->str<<endl;
							outputFile << "\tje " + i->operands[2]->str<<endl;
							if (i->operands.size() == 4)
								outputFile << "\tjmp " + i->operands[3]->str<<endl;
						}
						break;                        
				}
			}
	  }
	  /*
		* Close the output file.
		*/
		outputFile.close();
		return ;
 	}

 	std::string byteregmap(std::string reg){
		if(reg == "%rdi") return "%dil";
		if(reg == "%r10") return "%r10b";
		if(reg == "%r13") return "%r13b";
		if(reg == "%r8") return "%r8b";
		if(reg == "%rbp") return "%bpl";
		if(reg == "%r11") return "%r11b";
		if(reg == "%r14") return "%r14b";
		if(reg == "%r9") return "%r9b";
		if(reg == "%rbx") return "%bl";
		if(reg == "%rdx") return "%dl";
		if(reg == "%r12") return "%r12b";
		if(reg == "%r15") return "%r15b";
		if(reg == "%rax") return "%al";
		if(reg == "%rcx") return "%cl";
		if(reg == "%rsi") return "%sil";
 	}
}
