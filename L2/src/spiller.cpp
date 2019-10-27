#include "spiller.h"
#include "liveness.h"
#include "utils.h"

using namespace std;

namespace L2{ 

	std::string spilledVar;
	std::string replacementVar;

	//sets spilltype for spilled variable {READ,WRITE,READWRITE,NO_OP} and update locals
	void set_spilltype(Function * f){

		//set globals
		spilledVar = f->spilled_variable;
		replacementVar = f->spill_replace;  

		for(auto i: f->instructions){
			i->spilltype = NO_OP; 

			if((!i->GEN.empty()) && (i->KILL.empty())){
				for(auto g : i->GEN){
					if(g == f->spilled_variable)
						i->spilltype = READ;
				}
			}

			else if((i->GEN.empty()) && (!i->KILL.empty())){
				for(auto k : i->KILL){
					if(k == f->spilled_variable)
						i->spilltype = WRITE;
				}	
			}

			else if((!i->GEN.empty()) && (!i->KILL.empty())){
				for(auto g : i->GEN){
					if(g == f->spilled_variable){
						for(auto k : i->KILL){
							if(k == f->spilled_variable){
								i->spilltype = READWRITE;
								break;
							}
							else
								i->spilltype = READ;
						}
					}
					else{
						for(auto k : i->KILL){
							if(k == f->spilled_variable)
								i->spilltype = WRITE;
						}
					}
				}
			}

			else
				i->spilltype = NO_OP;       
		}

		//update locals
		for(auto i: f->instructions){
			if((i->spilltype == READ) || (i->spilltype == WRITE) || (i->spilltype == READWRITE)){
				f->locals++;
				break;
			}
		}
	}

	//spills f->spilled_variable and replace with f->spill_replace
	void spill(Function *f){
		//set spilltype for spilled variable {READ,WRITE,READWRITE,NO_OP} and update locals
		set_spilltype(f); 
		
		int count = 0; 
		std::vector<Instruction *> newInsts; 
		for(auto i : f->instructions){
			switch(i->spilltype){
				case READ:
				{
					auto read = new Instruction(); 		//Insert a read from mem
					read_inst(read, f->locals, count);
					newInsts.push_back(read);

					replace_inst(i,count); 			//Replace instruction's operand
					newInsts.push_back(i);

					count++; 
					break;                  
				}

				case WRITE:
				{
					replace_inst(i,count);				//Replace instruction's operand 
					newInsts.push_back(i);

					auto write = new Instruction();		//Insert a write to mem
					write_inst(write, f->locals, count);
					newInsts.push_back(write);

					count++;
					break;                         
				}
					
				case READWRITE:
				{	
					auto read = new Instruction();		//Insert a read from mem
					read_inst(read, f->locals, count);
					newInsts.push_back(read);

					replace_inst(i,count); 				//Replace instruction's operand
					newInsts.push_back(i);

					auto write = new Instruction();		//Insert a write to mem
					write_inst(write, f->locals, count);
					newInsts.push_back(write);
					
					count++;
					break;   
				}					
				case NO_OP:
					newInsts.push_back(i);			//Do nothing
					break;
			}
		}      
		f->instructions = newInsts;   
	}

	//Replace instruction's operand
	void replace_inst(Instruction* i, int count){
		for(auto &oprnd : i->operands){
			if(oprnd->op_type == VAR){
				if(oprnd->str == spilledVar)
					oprnd->str = replacementVar + to_string(count);
			}
			if(oprnd->op_type == MEMVAR){
				if(oprnd->X == spilledVar){
					oprnd->X = replacementVar + to_string(count);
				}
			}							
		}
		replace_string(i->Inst,spilledVar, replacementVar + to_string(count));
	}

	//Insert a read from mem
	void read_inst(Instruction* i, int locals, int count){
		auto dest = new Operand();
		dest->str = replacementVar + to_string(count);
		dest->op_type = VAR;
		auto src = new Operand();
		src->str = "mem rsp " + to_string((locals-1)*8);
		src->op_type = MEMREG;
		src->X = "rsp";
		src->Mvalue = (locals-1)*8;
		i->operands.push_back(dest);
		i->operands.push_back(src); 
		i->Inst = dest->str + " <- " + src->str;
		i->op = MOVQ;
	}

	//Insert a write to mem
	void write_inst(Instruction* i, int locals, int count){
		auto src = new Operand();
		src->str = replacementVar + to_string(count); 
		src->op_type = VAR;
		auto dest = new Operand();
		dest->str = "mem rsp " + to_string((locals-1)*8);
		dest->op_type = MEMREG;
		dest->X = "rsp";
		dest->Mvalue = (locals-1)*8;
		i->operands.push_back(dest);
		i->operands.push_back(src); 
		i->Inst = dest->str + " <- " + src->str;
		i->op = MOVQ;
	}

	//Only called to pass 'make test_spill'
	void spill_print(Function * f){
		//print for spill
		gen_kill(f); //compute GEN KILL first for our algorithm
		spill(f);
		cout<< "(" << f->name<<endl;
		cout<< "\t" << f->arguments << " " << f->locals <<endl;
		for (auto i : f->instructions)
			cout << "\t" << i->Inst <<endl; 
		cout<< ")" <<endl;  
	}
}
