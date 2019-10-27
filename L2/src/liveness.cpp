#include "liveness.h"
#include "utils.h"

using namespace std;

namespace L2{ 

	// Algorithm from Liveness slides to compute IN and OUT set
	void in_out(Function* f){
		

		bool flag = 0;
		do{ // while changes to any IN or OUT set
			flag = 0;
			for (int s = 0; s<f->instructions.size(); s++){
				auto i = f->instructions[s];
				std::vector<std::string> old_IN = i->IN; //store initial IN and OUT for current iteration of the loop
				std::vector<std::string> old_OUT = i->OUT;
				std::vector<std::string> difference; 

				//Compute OUT-KILL 
				difference = i->OUT;
				if (!(difference.empty())) {
					for(auto kill : i->KILL){
						for (int j=0; j<(difference.size()); j++){
							if(difference[j] == kill){
								difference.erase(difference.begin()+j);
								break;
							}
						}
					}
				}
				// IN = GEN U (OUT -KILL)
				i->IN = i->GEN;
				i->IN.insert(i->IN.end(), difference.begin(), difference.end());

				// OUT = successor instruction's IN
				// 1) goto label: successor is i right below label 
				// 2) cjump label: successor is next instruction U i right below label
				// 3) cjump label1 label2: successors is (i right below label1) U (i right below label2)
				// 4) return: out = {}
				if((i->op == RETQ) || (i->op == ARRAYERROR)) {}
				else if(i->op == GOTO){
					for (int g = 0; g<f->instructions.size(); g++){
						if((f->instructions[g]->op == LBL) && (f->instructions[g]->Inst == i->operands[0]->str))
							i->OUT = f->instructions[g]->IN;
					}
				}
				else if((i->op == JE) || (i->op == JL) || (i->op == JLE)){
					if (i->operands.size() == 4){
						for (int g = 0; g<f->instructions.size(); g++){
							if((f->instructions[g]->op == LBL) && (f->instructions[g]->Inst == i->operands[2]->str))
								i->OUT = f->instructions[g]->IN;
						}
						for (int g = 0; g<f->instructions.size(); g++){
							if((f->instructions[g]->op == LBL) && (f->instructions[g]->Inst == i->operands[3]->str))
								i->OUT.insert(i->OUT.end(), f->instructions[g]->IN.begin(), f->instructions[g]->IN.end());
						}
					}
					else{
						for (int g = 0; g<f->instructions.size(); g++){
							if((f->instructions[g]->op == LBL) && (f->instructions[g]->Inst == i->operands[2]->str))
								i->OUT = f->instructions[g]->IN;
						}
						i->OUT.insert(i->OUT.end(), f->instructions[s+1]->IN.begin(), f->instructions[s+1]->IN.end());
					}     
				} 
				else{
					if(s < f->instructions.size() - 1 )
						i->OUT = f->instructions[s+1]->IN;
				}   

				// remove repetitions IN -- Probably should have used sets instead...
                sort( i->IN.begin(), i->IN.end() );
                i->IN.erase( unique( i->IN.begin(), i->IN.end() ), i->IN.end() );

                // remove repetitions in OUT -- Probably should have used sets instead...
                sort( i->OUT.begin(), i->OUT.end() );
                i->OUT.erase( unique( i->OUT.begin(), i->OUT.end() ), i->OUT.end() );       

				if((old_IN != i->IN) || (old_OUT != i->OUT))
					flag = 1;
			} 
		} while(flag);
	}



	// Generate GEN and KILL set for each instruction
	void gen_kill(Function* f){
		for (auto i : f->instructions){
			switch(i->op){
						                
				case MOVQ: case ADDQ: case SUBQ: case IMULQ: case ANDQ: case SALQ: case SARQ:
				{
					if ((i->operands[1]->op_type == GPREG) || (i->operands[1]->op_type == VAR)){ // src is a register or variable
						if(i->operands[1]->str != "rsp") //not including rsp
							i->GEN.push_back(i->operands[1]->str);
					}
					else if ((i->operands[1]->op_type == MEMREG) || (i->operands[1]->op_type == MEMVAR)){ // src is memxM
						if(i->operands[1]->X != "rsp")
							i->GEN.push_back(i->operands[1]->X);  							
					}

					if ((i->operands[0]->op_type == GPREG) || (i->operands[0]->op_type == VAR)){ //dest is a register or variable
						if(i->op != MOVQ)
							i->GEN.push_back(i->operands[0]->str); 
						i->KILL.push_back(i->operands[0]->str);
					}
					else if ((i->operands[0]->op_type == MEMREG) || (i->operands[0]->op_type == MEMVAR)){ // dest is memxM
						if(i->operands[0]->X != "rsp")
							i->GEN.push_back(i->operands[0]->X); 
					}
					break;
				}

				case DEC: case INC:
					if ((i->operands[0]->op_type == GPREG) || (i->operands[0]->op_type == VAR)){ 
						i->KILL.push_back(i->operands[0]->str);
						i->GEN.push_back(i->operands[0]->str); 
					}
					break;

				case LEA: case LQ: case LEQ: case EQ:
					if(i->operands[2]->str != "rsp"){
						if ((i->operands[2]->op_type == GPREG) || (i->operands[2]->op_type == VAR)) 
							i->GEN.push_back(i->operands[2]->str); 
					}
					if(i->operands[1]->str != "rsp"){
						if ((i->operands[1]->op_type == GPREG) || (i->operands[1]->op_type == VAR)) 
							i->GEN.push_back(i->operands[1]->str); 
					}
					if ((i->operands[0]->op_type == GPREG) || (i->operands[0]->op_type == VAR)) 
						i->KILL.push_back(i->operands[0]->str); 
					break;

				case CALL:
				// call u N: GEN={u, args}; KILL={callerSav_registers, rax}
					if ((i->operands[0]->op_type == GPREG) || (i->operands[0]->op_type == VAR)) 
						i->GEN.push_back(i->operands[0]->str); 
					for (int s = 0; s<std::stoi(i->operands[1]->str); s++){
						if(s < 6)
							i->GEN.push_back(arg_registers[s]); 
					}
					for (int s = 0; s<sizeof(callerSav_registers)/sizeof(callerSav_registers[0]); s++)
						i->KILL.push_back(callerSav_registers[s]); 
					i->KILL.push_back("rax");
					break;

				// call RUNTIME N: GEN={args}; KILL={callerSav_registers, rax}
				case PRINT: case ALLOCATE: case ARRAYERROR:
					i->GEN.push_back(arg_registers[0]);
					if(i->op != PRINT)
						i->GEN.push_back(arg_registers[1]);  
					for (int s = 0; s<sizeof(callerSav_registers)/sizeof(callerSav_registers[0]); s++)
						i->KILL.push_back(callerSav_registers[s]); 
					i->KILL.push_back("rax");
					break;

				// return: GEN={calleeSav_register, rax}; KILL={}
				case RETQ:						
					for (int s = 0; s<sizeof(calleeSav_registers)/sizeof(calleeSav_registers[0]); s++)
						i->GEN.push_back(calleeSav_registers[s]); 
					i->GEN.push_back("rax");
					break;

				case JL: case JLE: case JE:
					if(i->operands[1]->str != "rsp"){
						if ((i->operands[1]->op_type == GPREG) || (i->operands[1]->op_type == VAR))
							i->GEN.push_back(i->operands[1]->str);  
					}
					if(i->operands[0]->str != "rsp"){
						if ((i->operands[0]->op_type == GPREG) || (i->operands[0]->op_type == VAR))
							i->GEN.push_back(i->operands[0]->str); 
					}
					break;
			}
		}
	}

	//prints gen and kill sets -  for debugging
	void print_gen_kill(Function *f){
		cout <<"(\n (gen"<<endl;
		for (auto i : f->instructions){
			cout<<"  (";
			for (auto g : i->GEN){
				if (g[0] == '%')
					cout << g.substr(1, g.length() - 1) << " ";
				else
					cout << g << " ";
			}
			cout<<" )"<<endl;
		}
		cout <<" )"<<endl;
		cout <<"\n (kill"<<endl;
		for (auto i : f->instructions){
			cout<<"  (";
			for (auto g : i->KILL){
				if (g[0] == '%')
					cout << g.substr(1, g.length() - 1) << " ";
				else
					cout << g << " ";
			}
			cout<<" )"<<endl;
		}
		cout <<" )\n\n)"<<endl;
	}

	// called to pass 'make test_liveness'
	void liveness_print(Function* f){
		gen_kill(f); //generate GEN and KILL sets
		in_out(f);
		// Print IN OUT set
		cout <<"(\n (in"<<endl;
		for (auto i : f->instructions){
			cout<<"  (";
			for (auto g : i->IN){
				if (g[0] == '%')
					cout << g.substr(1, g.length() - 1) << " ";
				else
					cout << g << " ";
			}
			cout<<" )"<<endl;
		}
		cout <<" )"<<endl;
		cout <<"\n (out"<<endl;
		for (auto i : f->instructions){
			cout<<"  (";
			for (auto g : i->OUT){
				if (g[0] == '%')
					cout << g.substr(1, g.length() - 1) << " ";
				else
					cout << g << " ";
			}
			cout<<" )"<<endl;
		}
		cout <<" )\n\n)"<<endl;
	}

	
	void interference_print(Function* f){ 
		gen_kill(f); //generate GEN and KILL sets
		print_gen_kill(f); //for debugging	
		liveness_print(f); //generates IN OUT sets before interference graph
		remUnusedVariables(f); //removes variables not present in any IN set of the function
		generate_igraph(f);

		//prints Interference Graph
		for (auto i : f->iGraph->variables){
			if (i->op_type == VAR)
				cout << i->node.substr(1, i->node.length() - 1) << " ";
			else
				cout << i->node << " ";
			if (!i->edges.empty()){
				for(auto e: i->edges){
					if (e[0] == '%')
						cout << e.substr(1, e.length() - 1) << " ";
					else
						cout << e << " ";
				}
				cout<<endl;
			}
			else
				cout<<endl;
		}
	}

	//Only called to pass 'make test_interference'
	void interference_test(Function* f){ 
		gen_kill(f); //generate GEN and KILL sets
		in_out(f);
		generate_igraph(f);

		//prints Interference Graph
		for (auto i : f->iGraph->variables){
			if (i->op_type == VAR)
				cout << i->node.substr(1, i->node.length() - 1) << " ";
			else
				cout << i->node << " ";
			if (!i->edges.empty()){
				for(auto e: i->edges){
					if (e[0] == '%')
						cout << e.substr(1, e.length() - 1) << " ";
					else
						cout << e << " ";
				}
				cout<<endl;
			}
			else
				cout<<endl;
		}
	}

	//Computes interference graph
	void interference(Function* f){ 
		gen_kill(f); //generate GEN and KILL sets	
		in_out(f); //generates IN OUT sets before interference graph
		remUnusedVariables(f);
		generate_igraph(f); //Computes interference graph
		
	}

	void deadCodeElimination(Function* f){  //Performs a 2-level DCE: instructions killing variables (not gpregs) not read by any variable other than itself in the whole function are removed
		gen_kill(f);
		bool deleteInst;
		bool flag;
		do{
			flag = false;
			for (int i = 0; i<f->instructions.size(); i++){
				if(f->instructions[i]->KILL.size() == 1){
					if(f->instructions[i]->operands[0]->op_type == VAR){
						deleteInst = checkForDeadCode(f, i);
						if(deleteInst){
							f->instructions[i]->marked_for_del = true;	
							break;
						}
					}
				}
			}

			flag = removeInsts(f);

		}while(flag);

		for(auto i : f->instructions ){
            i->GEN = {};
            i->KILL = {};
        }
		return;
	}

	bool checkForDeadCode(Function* f, int instIndex){
		for (int j = 0; j<f->instructions.size(); j++){
			if (std::count(f->instructions[j]->GEN.begin(), f->instructions[j]->GEN.end(), f->instructions[instIndex]->KILL[0])){
				if (!std::count(f->instructions[j]->KILL.begin(), f->instructions[j]->KILL.end(), f->instructions[instIndex]->KILL[0])){
					if(f->instructions[j]->KILL.empty())
						return 0;
					else{
						for(auto kill: f->instructions[j]->KILL){
							if(kill[0] != '%')
								return 0;
						}
					}
					for (int k = 0; k<f->instructions.size(); k++){
						if (std::count(f->instructions[k]->GEN.begin(), f->instructions[k]->GEN.end(), f->instructions[j]->KILL[0])){
							if (!std::count(f->instructions[k]->KILL.begin(), f->instructions[k]->KILL.end(), f->instructions[instIndex]->KILL[0])){
								if (!std::count(f->instructions[k]->KILL.begin(), f->instructions[k]->KILL.end(), f->instructions[j]->KILL[0]))
									return 0;
							}
						}
					}
				} 
			}
		}
		return 1;
	}

	void remUnusedVariables(Function* f){ //Removes instructions killing variables not present in the IN sets of any instruction within a function
		bool flag;
		bool deleteInst;
		do{
			flag = false;
			for (int i = 0; i<f->instructions.size(); i++){
				deleteInst = true;
				for(auto op : f->instructions[i]->operands){
					if((op->op_type == VAR) || (op->op_type == GPREG)) {
						for (int j = 0; j<f->instructions.size(); j++){
							 for (auto in : f->instructions[j]->IN){
							 	if(in == op->str)
									deleteInst = false;
							}
						}
						if(deleteInst){
							f->instructions[i]->marked_for_del = true;	
							break;
						}
						if(!deleteInst){
							f->instructions[i]->marked_for_del = false;	
							break;
						}
					}
				}
			}


			flag = removeInsts(f);

		}while(flag);
		
		return;
	}

	bool removeInsts(Function* f){
		bool retFlag = false;
		bool deleteInst = false;
 		do{
 			deleteInst = false;
			for(int i=0; i<f->instructions.size(); i++){
	 			if (f->instructions[i]->marked_for_del == true){
	 				f->instructions.erase(f->instructions.begin() + i);
	 				deleteInst = true;
	 				retFlag = true;
	 				break;
	 			}
		 	}
		}while(deleteInst);
		return retFlag;
	}


	void generate_igraph(Function* f){
		auto iG = new InterferenceGraph();

		//adding gp_regs to iG & connectiong gp_regs to each other
		for (auto reg : gp_registers){
			auto newVar = new Variable();
			newVar->node = reg;
			newVar->op_type = GPREG;
			for(auto tempreg :gp_registers ){
				if(tempreg != reg)
					newVar->edges.insert(tempreg);
			}
			iG->variables.insert(newVar);
		}

		//Creating a set of all other variables in the function
		std::set<std::string> vars;
		for (auto i : f->instructions){
			for(auto in: i->IN){
				if(in[0] == '%')
				vars.insert(in);
			}
		}
		

		//adding all variables to the iG as nodes
		for (auto i : vars){
			auto newVar = new Variable();
			newVar->node = i;
			newVar->op_type = VAR;
			iG->variables.insert(newVar);
		}


		for(auto i : f->instructions){
			//connecting variables in IN[i] or OUT[i] set with each other
			insert_edges(iG, i->IN);
			insert_edges(iG, i->OUT);

			//connecting variables in KILL[i] with those in OUT[i]
			if(!((i->op == MOVQ) && ((i->operands[1]->op_type == GPREG) || (i->operands[1]->op_type == VAR)) && ((i->operands[0]->op_type == GPREG) || (i->operands[0]->op_type == VAR))))
				connect_kill_out(iG, i->KILL, i->OUT);

			//adding edges between every gp_reg except rcx and src variable for SOP -- architectural constraint
			if(((i->op == SALQ) || (i->op == SARQ)) && (i->operands[1]->op_type == VAR)) {
				for(auto var : iG->variables){
					for(auto tempreg : gp_registers ){
						if(tempreg != "rcx"){
							if (var->node == i->operands[1]->str)
								var->edges.insert(tempreg);
							if (var->node == tempreg)
								var->edges.insert(i->operands[1]->str);
						}
					}
				}
			}
		}

		f->iGraph = iG;
	}

	//connecting variables in IN[i] or OUT[i] set with each other
	void insert_edges(InterferenceGraph* iG, std::vector<std::string> in_out ){
		for (auto in : in_out){
			for(auto var : iG->variables){
				if(var->node == in){
					for(auto temp_in : in_out){
						if (in != temp_in)
							var->edges.insert(temp_in);   
					}
				}
			}
		}
	}

	//connects variables in KILL[i] with those in OUT[i]
	void connect_kill_out(InterferenceGraph* iG, std::vector<std::string> KILL, std::vector<std::string> OUT){
		for(auto var : iG->variables){
			for (auto kill : KILL){
				for(auto out : OUT){
					if (kill != out){
						if(var->node == kill)
							var->edges.insert(out);   
						if(var->node == out)
							var->edges.insert(kill);  
					} 
				}
			}
		}
	}
}
