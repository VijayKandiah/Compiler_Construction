#include "liveness.h"

using namespace std;

namespace L3{ 


	// Algorithm from Liveness slides to compute IN and OUT set
	void inOut(Function* f){

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
				for(auto gen : i->GEN){
					i->IN.push_back(gen);
				}
				i->IN.insert(i->IN.end(), difference.begin(), difference.end());

				// OUT = successor instruction's IN
				i->OUT = {};
				for(auto next: i->sucs){
					i->OUT.insert(i->OUT.end(), next->IN.begin(), next->IN.end());   
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
		return;
	}


	void computePredSuc(Function* f){

		for (int s = 0; s<f->instructions.size(); s++){
			auto i = f->instructions[s];
			if((i->op == RET) || (i->op == RETVAR) || (i->op == ARRAYERROR)){}
			else if(i->op == GOTO){
				for (int g = 0; g<f->instructions.size(); g++){
					if((f->instructions[g]->op == LBL) && (f->instructions[g]->operands[0]->str == i->operands[0]->str))
						i->sucs.push_back(f->instructions[g+1]);
				}
			}
			else if(i->op == CJUMP){

				for (int g = 0; g<f->instructions.size(); g++){
					if((f->instructions[g]->op == LBL) && (f->instructions[g]->operands[0]->str == i->operands[1]->str))
						i->sucs.push_back(f->instructions[g+1]);
				}
				i->sucs.push_back(f->instructions[s+1]);     
			} 
			else{
				if(s < f->instructions.size() - 1 )
					i->sucs.push_back(f->instructions[s+1]);        
			}
		}


		for(auto i: f->instructions){
			for(auto next: i->sucs){
				next->preds.push_back(i);
			}
		}
		return;
	}


	// Generate GEN and KILL set for each instruction
	void genKill(Function* f){

		for (auto i : f->instructions){
			switch(i->op){
				
				case STORE:
				
					
					if (i->operands[1]->op_type == VAR){ 
						i->GEN.push_back(i->operands[1]->str);
					}
				
					if (i->operands[0]->op_type == VAR){ 
						i->GEN.push_back(i->operands[0]->str);
					}

					break;
				

				// calls
				case CALL: case PRINT: case ARRAYERROR: 

					for(auto oprnd : i->operands){
						if(oprnd->op_type == VAR)
							i->GEN.push_back(oprnd->str);
					}
					
					break;


				case CALL_RETVAR: case ALLOCATE:

					if (i->operands[0]->op_type == VAR){ 
						i->KILL.push_back(i->operands[0]->str);
					}

					for(int j=1; j< i->operands.size(); j++){
						if (i->operands[j]->op_type == VAR){
							i->GEN.push_back(i->operands[j]->str);
						}
					}
					
					break;

				// return_var: 
				case RETVAR:						
					if (i->operands[0]->op_type == VAR){ // src is a register or variable
						i->GEN.push_back(i->operands[0]->str);
					}
					break;

				case CJUMP:
					if (i->operands[0]->op_type == VAR){ 
						i->GEN.push_back(i->operands[0]->str);
					}
					break;

				case MOV: case LOAD: case ADD: case SUB: case MUL: case AND: case SAL: case SAR: case LT: case LE: case EQ: case GE: case GT:
				
					if((i->op != MOV) && (i->op != LOAD)){
						if (i->operands[2]->op_type == VAR){ 
							i->GEN.push_back(i->operands[2]->str);
						}
					}

					if (i->operands[1]->op_type == VAR){ 
						i->GEN.push_back(i->operands[1]->str);
					}

					if (i->operands[0]->op_type == VAR){ 
						i->KILL.push_back(i->operands[0]->str);
					}

					break;
			}
		}
		return;
	}


	//prints gen and kill sets -  for debugging
	void printGenKill(Function *f){

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
		return;
	}


	void inOutPrint(Function * f){

		// Prints IN OUT set - for debugging
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
		return;	
	}


	void livenessPrint(Function* f){

		genKill(f); //generate GEN and KILL sets
		computePredSuc(f);
		printGenKill(f); 
		inOut(f);
		inOutPrint(f);
		return;
	}


	void liveness(Function* f){

		genKill(f); //generate GEN and KILL sets
		computePredSuc(f);		
		inOut(f);
		return;
	}
}
