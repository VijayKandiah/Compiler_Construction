
#include "inst_selector.h"
#include "liveness.h"

 using namespace std;

 namespace L3{

 	bool debug_instSelector = 0;


	std::map<Operator, std::string> L2_operatormap = {
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


 	void generateContexts(Function *f){

 		auto newContext = new Context();
 		for(auto i : f->instructions){
 			if((i->op == LBL) || (i->op == GOTO) || (i->op == CJUMP) ||  (i->op == RET) || (i->op == RETVAR)){
 				if(!(newContext->instructions.empty())){
	 				auto pushbackContext = new Context();
	 				for(auto i : newContext->instructions){
	 					pushbackContext->instructions.push_back(i);
	 				}
					f->contexts.push_back(pushbackContext);
					newContext->instructions.clear();
					auto newContext = new Context();
				}
 			}
 			else{
 				newContext->instructions.push_back(i);	
 			}
 		}

 		for(int j=0; j<(f->contexts.size()); j++){
 			if(f->contexts[j]->instructions.empty())
 				f->contexts.erase(f->contexts.begin()+j);
 		}

 		if(!(f->contexts.empty())){
	 		for(auto i : f->instructions){
	 			if((i->op == LBL) || (i->op == GOTO) || (i->op == CJUMP) ||  (i->op == RET) || (i->op == RETVAR)){
	 				for(int j = 0; j<f->contexts.size(); j++){
	 					if(f->contexts[j]->instructions[0]->pos == i->pos+1){ 
	 						i->context_below = true; //Just a hack to mark position of contexts in source program.
	 					}
	 				}
	 			}
	 		}
	 	}

 		if(!(f->contexts.empty())){
	 		if(f->contexts[0]->instructions[0]->pos == 0){
	 			f->starts_with_context = true; 
	 		}
	 	}	
 	}

	void condenseContexts(Context *cont){ 
 	/*
 	If instructions in a context kill a variable twice and if there are no instructions with uses of that variable in between them, 
	first kill instruction can be removed
	*/

 		for (int i=0; i<cont->instructions.size()-1; i++){
 			for (int j=i+1; j<cont->instructions.size(); j++){
 				if((!cont->instructions[j]->KILL.empty()) && (!cont->instructions[i]->KILL.empty()) ){
	 				if(cont->instructions[j]->KILL[0] == cont->instructions[i]->KILL[0]){
	 					bool del = true;
	 					for(int k=i+1; k<=j; k++){
	 						if (std::count(cont->instructions[k]->GEN.begin(), cont->instructions[k]->GEN.end(), cont->instructions[i]->KILL[0]))
	 							del = false;
	 					}
	 					if(del == true){
	 						cont->instructions[i]->marked_for_del = true;
	 						break;
	 					}
	 				}
	 			}
 			}
 		}
 		cleanContext(cont); 

 		for(auto i : cont->instructions){ //change v1 <- v2 * (2^n) to v1 <- v2 << n;
 			if (i->op == MUL){
 				if(i->operands[2]->op_type == NUM){
 					if(stol(i->operands[2]->str) > 1){
	 					if(ceil(log2(stol(i->operands[2]->str)))== floor(log2(stol(i->operands[2]->str)))){
	 						i->op = SAL;
	 						i->operands[2]->str = to_string((int)log2(stol(i->operands[2]->str)));
						}
					}
 				}	
 			}
 		}
 		
 		return;
 	}


 	void generateTrees(Context *cont){ 
 	//Generates a tree for every instruction in the input context

 		for (auto i : cont->instructions){
 			auto tree = new Tree();
 			if((i->op == CALL) || (i->op == CALL_RETVAR) || (i->op == ALLOCATE) ||  (i->op == ARRAYERROR) || (i->op == PRINT)){
 				tree->callInst = i;
 				tree->isCallInst = true;
 			}
 			else{
	 			auto root_node = new Var_Node();
	 			auto op_node = new Op_Node();
	 			root_node->var = i->operands[0];
	 			tree->varNodes.insert(i->operands[0]->str);
	 			op_node->op = i->op;
	 			op_node->parent = root_node;
	 			root_node->children.push_back(op_node);
	 			for(int j = 1; j<i->operands.size(); j++){
	 				auto src_node = new Var_Node();
	 				src_node->var = i->operands[j];
	 				if(i->operands[j]->op_type == VAR)
	 					tree->varNodes.insert(i->operands[j]->str);
	 				src_node->parent = op_node;
	 				op_node->children.push_back(src_node);
	 			}
	 			tree->root = root_node;
	 		}
 			cont->trees.push_back(tree);
 		}
 	}



 	void FindMergeCandidates(Context *cont){
 		//Finds trees to merge 'safely' within a context and removes used trees afterwards

 		bool flag = false;
 		std::vector<std::string> leaf_vec_str;
 		std::vector<std::string> imNode_vec_str;
 		std::vector<Node *> leaf_vec;
 		do{
	 		flag = false;
	 		for (int i=0; i<cont->trees.size()-1; i++){
	 			if((cont->trees[i]->marked_for_del == false) && (cont->trees[i]->isCallInst == false)){
		 			for (int j=i+1; j<cont->trees.size(); j++){
		 				if((cont->trees[j]->marked_for_del == false)  && (cont->trees[j]->isCallInst == false)){
			 				if(Var_Node* inode = dynamic_cast<Var_Node*>(cont->trees[i]->root)){
			 					//cout<<"checking if i in j's leafs i:"<< to_string(i) << " j:" << to_string(j)<<endl;
			 					leaf_vec_str.clear();
			 					leaf_vec.clear();
			 					getLeafNodes(cont->trees[j]->root, leaf_vec, leaf_vec_str);
								if (std::count(leaf_vec_str.begin(), leaf_vec_str.end(), inode->var->str)){
			 						//cout<<"checking if i in j's OUT i:"<< to_string(i) << " j:" << to_string(j)<<endl;
			 						if (!(std::count(cont->instructions[j]->OUT.begin(), cont->instructions[j]->OUT.end(), inode->var->str))){
			 							//cout<<"checking every instruction between i and j i:"<< to_string(i) << " j:" << to_string(j)<<endl;
			 							bool merge = true;
				 						for(int k=i+1; k<j; k++){
							 				for(auto varNode : cont->trees[i]->varNodes){
						 						if (std::count(cont->instructions[k]->KILL.begin(), cont->instructions[k]->KILL.end(), varNode))
				 									merge = false;
						 					}
				 							
				 							if (std::count(cont->instructions[k]->GEN.begin(), cont->instructions[k]->GEN.end(), inode->var->str))
				 								merge = false;
				 					
				 							leaf_vec_str.clear();
				 							leaf_vec.clear();
			 								getLeafNodes(cont->trees[i]->root,leaf_vec, leaf_vec_str);
			 								for(auto leaf : leaf_vec_str){
			 									if (std::count(cont->instructions[k]->KILL.begin(), cont->instructions[k]->KILL.end(), leaf))
			 										merge = false;	 	
			 								}
				 						}

			 							if(merge == true){
		 									flag = true;
		 									if(debug_instSelector)
		 										cout<<"merging trees i:"<< to_string(i) << " j:" << to_string(j)<<endl;
		 									cont->trees[j]->varNodes.insert(cont->trees[i]->varNodes.begin(), cont->trees[i]->varNodes.end());
		 									mergeTree(cont->trees[i]->root, cont->trees[j]->root);
		 									cont->trees[i]->marked_for_del = true;
		 									break;
			 									
				 						}
				 					}
				 				}
			 				}
			 			}
		 			}
	 			}
	 			if (flag == true)
	 				break;
	 		}
	 	}while(flag);
	 	cleanContext(cont); //Removes trees within the context marked for deletion since they've already been merged to some other tree.
 	}


 	void mergeTree(Node* merge_this, Node* root){
 		//Merges the tree merge_this into tree root
 		if (!root) 
       		return;
       	if (!merge_this) 
       		return;

 		if(!(root->children.empty())){
			for(auto child : root->children){
				mergeTree(merge_this, child);
			}
		}
		else{
			if(Var_Node* node = dynamic_cast<Var_Node*>(root)){
				if(Var_Node* merge_node = dynamic_cast<Var_Node*>(merge_this)){
					if(merge_node->var->str == node->var->str){
						for(auto child : merge_this->children){
							appendtoTree(node, child);
						}
					}  
				}
			}
		}
		return;
 	}


	void appendtoTree(Node* treeNode,  Node* toAppend){
		if(Op_Node* opnode = dynamic_cast<Op_Node*>(toAppend)){
			auto copyNode = new Op_Node();
			copyNode->op = opnode->op;
			std::vector<Node *> tmpChildren;
			for(auto tmpChild : opnode->children){
				if(Var_Node* varnode = dynamic_cast<Var_Node*>(tmpChild)){
					auto copyVAR = new Var_Node();
					auto variable = new Variable();
					variable->str = varnode->var->str;
					variable->op_type = varnode->var->op_type;
					copyVAR->var = variable;
					copyVAR->parent = copyNode;
					copyVAR->children = varnode->children;
					tmpChildren.push_back(copyVAR);
				}
			}
			copyNode->children = tmpChildren;
			copyNode->parent = treeNode;
			treeNode->children.push_back(copyNode);
		}
		return;
	}



 	void getLeafNodes(Node* root, std::vector<Node *> &leaf_vec, std::vector<std::string> &leaf_vec_str){
 		//Gets a vector of leaf nodes of input tree
 		if (!root) 
       		return; 

		if(!(root->children.empty())){
			for(auto child : root->children){
				getLeafNodes(child, leaf_vec, leaf_vec_str);
			}
		}
		else{
			if(Var_Node* node = dynamic_cast<Var_Node*>(root)){
				leaf_vec_str.push_back(node->var->str);
				leaf_vec.push_back(node);
			}
			return;
		}	
 	}

 	void condenseTree(Node* root){ 
 		/*
 		removes consecutive move operations from tree  so the final code generated for this example:
 			v2<-v1
 			v3<-v2
 			v4<-v3 + 5
 		will be reduced to
 			v4<-v1
 			v4 += 5

 		This is always safe due to the restrictions with which we merge trees
		*/
 		std::vector<Node *> leaf_vec;
 		std::vector<std::string> leaf_vec_str;
 		getLeafNodes(root, leaf_vec, leaf_vec_str); //we traverse the tree from each leaf node to root
 		for(auto leaf : leaf_vec){
 			bool flag=  0;
 			do{
 				flag = removeConsecutiveMoves(leaf);
 			}while(flag);
 		}		
 	}


 	bool removeConsecutiveMoves(Node* leaf){
 		//Does the actual removing of consecutive move operations explained above.
 		bool flag = 0;
 		if (!leaf) 
       		return 0; 

 		if(Op_Node* node = dynamic_cast<Op_Node*>(leaf->parent)){
 			if(node->op == MOV){
 				if (!node->parent) {}
 				else if (!node->parent->parent) {}
				else{
 					leaf->parent = node->parent->parent;
 					for(auto &&child: node->parent->parent->children ){
 						if(child == node->parent){
 							child = leaf;
 						}
 					}
					node->parent = NULL;
					flag = 1;
				}
 			}
 		}
		if (!leaf->parent) 
			return 0;
		else{
			if (!leaf->parent->parent) 
				return 0 ;	
			else{
				removeConsecutiveMoves(leaf->parent->parent);
				return flag;
			}
 		}
 	}


 	void cleanContext(Context *cont){
 		//Removes trees within the context marked for deletion since they've already been merged to some other tree.
 		bool flag = 0;
 		do{
 			flag = 0;
	 		for(int i=0; i<cont->trees.size(); i++){
	 			if (cont->trees[i]->marked_for_del == true){
	 				cont->trees.erase(cont->trees.begin() + i);
	 				flag = 1;
	 				break;
	 			}
	 		}
	 	}while(flag);

	 	flag = 0;
	 	do{
 			flag = 0;
	 		for(int i=0; i<cont->instructions.size(); i++){
	 			if (cont->instructions[i]->marked_for_del == true){
	 				cont->instructions.erase(cont->instructions.begin() + i);
	 				flag = 1;
	 				break;
	 			}
	 		}
	 	}while(flag);

 		return;
 	}
	

	void printTree(Node* root){ //For Debugging

		if(Var_Node* node = dynamic_cast<Var_Node*>(root)){
			cout<<node->var->str<<" ";
		}
		if(Op_Node* node = dynamic_cast<Op_Node*>(root)){
			cout<<L2_operatormap[node->op]<<" ";
		}
		if(!(root->children.empty())){
			for(auto child : root->children){
				cout<<"(";
				printTree(child);
				cout<<")";
			}
		}
		return;
 	}


 	void maximalMunch(Node* root, std::vector<std::string> &insts){

 		bool flag = 0;
 		if(root->children.empty())
 			return;
 		
 		if(Op_Node* node = dynamic_cast<Op_Node*>(root->children[0])){
 			if(node->op == ADD){
	 			flag = matchLeaOP(root,insts); //checks for LEA w @ w w E pattern; 
	 				if (flag)
	 					return;
 			}
 			if((node->op == MOV) || (node->op == LOAD) || (node->op == STORE)){
 				flag = matchMovOP(root,insts); //handles all subtrees with single child per root:  Mov | Load | Store operations
 				if (flag)
 					return;
 			}
 			else{
 				if(!((node->op == LT) || (node->op == LE) || (node->op == EQ)|| (node->op == GT)|| (node->op == GE))){
	 				flag = matchLoadArithOP(root,insts); //checks for patterns like p1<- load v1;   v2 <- p1 + v3;  and reduces into into v2 <-mem v1 0; v2 += v3; 
	 				if (flag)
	 					return;
	 			}
 				matchArithOP(root,insts); //handles all arithOPs
 			}
 			return;	
 		}
 	}


	bool matchLoadArithOP(Node* root, std::vector<std::string> &insts){ 
		/*
		checks for all valid Load followed by ArithOp tiles in tree; 
		ex: 
			p1<- load v1;  
			v2 <- p1 + v3; 
		can be reduced to :
			v2 <-mem v1 0;  
			v2 += v3; 
		*/

		if(root->children.empty())
 			return 0;

 		if(Op_Node* node = dynamic_cast<Op_Node*>(root->children[0])){
			bool found_loadArith = false;
			if(!(node->children[0]->children.empty())){
	 			if(Op_Node* child = dynamic_cast<Op_Node*>(node->children[0]->children[0])){
	 				if(child->op == LOAD){
	 					if(Var_Node* dest = dynamic_cast<Var_Node*>(root)){
	 						if(Var_Node* src = dynamic_cast<Var_Node*>(node->children[1])){
	 							if(src->var->str != dest->var->str){
					 				std::vector<std::string> loadinst_vec = genLoadArithOP(root, child->children[0], node->children[1], node->op);
					 				insts.insert (insts.begin(), loadinst_vec.begin(), loadinst_vec.end());
									if(!(node->children[1]->children.empty()))
									 	maximalMunch(node->children[1], insts);
									if(!(child->children[0]->children.empty()))
									 	maximalMunch(child->children[0], insts);
									found_loadArith = true;
									return 1;
								}
							}
	 					}
	 				}
	 			}
			}	
			if(!(node->children[1]->children.empty())){
				if(!found_loadArith){
		 			if(Op_Node* child = dynamic_cast<Op_Node*>(node->children[1]->children[0])){
		 				if(child->op == LOAD){
		 					if(Var_Node* dest = dynamic_cast<Var_Node*>(root)){
		 						if(Var_Node* src = dynamic_cast<Var_Node*>(node->children[0])){
		 							if(src->var->str != dest->var->str){
						 				std::vector<std::string> loadinst_vec = genLoadArithOP(root, child->children[0], node->children[0], node->op);
						 				insts.insert (insts.begin(), loadinst_vec.begin(), loadinst_vec.end());
										if(!(node->children[0]->children.empty()))
										 	maximalMunch(node->children[0], insts);
										if(!(child->children[0]->children.empty()))
										 	maximalMunch(child->children[0], insts);
										return 1;
									}
								}
							}
		 				}
		 			}
		 		}
			}	
 		}
 		return 0;
	}

	bool matchMovOP(Node* root, std::vector<std::string> &insts){

		if(Op_Node* node = dynamic_cast<Op_Node*>(root->children[0])){
 			 //simple move | load | store operation tile
			std::vector<std::string> mvinst_vec = genMovOP(root, node->children[0], node->op);
			insts.insert (insts.begin(), mvinst_vec.begin(), mvinst_vec.end());
			if(!(node->children[0]->children.empty()))
				maximalMunch(node->children[0], insts);
			return 1;
 		}
 		return 0;
 	}


 	void matchArithOP(Node* root, std::vector<std::string> &insts){

 		if(Op_Node* node = dynamic_cast<Op_Node*>(root->children[0])){
			//tiles for arithmetic operation 
			std::vector<std::string> arithinst_vec = genArithOP(root, node->children[0], node->children[1], node->op); 
			insts.insert(insts.begin(), arithinst_vec.begin(), arithinst_vec.end());
			if(!(node->children[0]->children.empty()))
				maximalMunch(node->children[0], insts);
			if(!(node->children[1]->children.empty()))
				maximalMunch(node->children[1], insts);
			return;
 		}
 		return;
 	}


 	bool matchLeaOP(Node* root, std::vector<std::string> &insts){ //checks for all valid LEA tiles in tree

		if(root->children.empty())
 			return 0;

 		if(Op_Node* node = dynamic_cast<Op_Node*>(root->children[0])){

			//LEA tile with MUL on right branch of immediate VAR child of root
			bool left_lea = false;
			bool right_lea = false;
			if(!(node->children[0]->children.empty())){

				if(Op_Node* child = dynamic_cast<Op_Node*>(node->children[0]->children[0])){
					if(child->op == MUL){
						right_lea = checkLeaChildren(child->children[0], child->children[1]);
						left_lea = checkLeaChildren(child->children[1], child->children[0]);
						if(right_lea && !left_lea){
							std::vector<std::string> leainst_vec = genLeaOP(root, node->children[1],child->children[0],child->children[1] );
							insts.insert (insts.begin(), leainst_vec.begin(), leainst_vec.end());
						}
						else if(!right_lea && left_lea){
							std::vector<std::string> leainst_vec = genLeaOP(root, node->children[1],child->children[1], child->children[0]);
							insts.insert (insts.begin(), leainst_vec.begin(), leainst_vec.end());
						}
						if(right_lea ^ left_lea){
							if(!(node->children[1]->children.empty()))
						 	maximalMunch(node->children[1], insts);
							if(!(child->children[0]->children.empty()))
							 	maximalMunch(child->children[0], insts);
							if(!(child->children[1]->children.empty()))
							 	maximalMunch(child->children[1], insts);
							return 1;
						}
					}
				}

			}	
			else if(!(node->children[1]->children.empty())){ //LEA tile with MUL on left branch of immediate VAR child of root
				if(!left_lea && !right_lea){
 					if(Op_Node* child = dynamic_cast<Op_Node*>(node->children[1]->children[0])){
 						if(child->op == MUL){
 							right_lea = checkLeaChildren(child->children[0], child->children[1]);
 							left_lea = checkLeaChildren(child->children[1], child->children[0]);
 							if(right_lea && !left_lea){
	 							std::vector<std::string> leainst_vec = genLeaOP(root, node->children[0],child->children[0],child->children[1] );
	 							insts.insert (insts.begin(), leainst_vec.begin(), leainst_vec.end());
 							}
 							else if(!right_lea && left_lea){
 								std::vector<std::string> leainst_vec = genLeaOP(root, node->children[0],child->children[1], child->children[0]);
	 							insts.insert (insts.begin(), leainst_vec.begin(), leainst_vec.end());
 							}
 							if(right_lea ^ left_lea){
	 							if(!(node->children[0]->children.empty()))
								 	maximalMunch(node->children[0], insts);
								if(!(child->children[0]->children.empty()))
								 	maximalMunch(child->children[0], insts);
								if(!(child->children[1]->children.empty()))
								 	maximalMunch(child->children[1], insts);
								return 1;
							}
 						}
 					}
				}
			}	
 		}
 		return 0;
	}

	bool checkLeaChildren(Node* child1, Node* child2){

		bool flag = false;
	 	if(Var_Node* p1 = dynamic_cast<Var_Node*>(child1)){
			if (p1->var->op_type == VAR){
				if(Var_Node* number = dynamic_cast<Var_Node*>(child2)){
					if ((number->var->op_type == NUM) && (stoll(number->var->str) >= 0)) {
						flag = true;
					}
				}
			}
		}
		return flag;
	}

 	std::vector<std::string> genLoadArithOP(Node* dest, Node* source1, Node* source2, Operator op){

 		std::vector<std::string> return_vec;
 		if(Var_Node* dst = dynamic_cast<Var_Node*>(dest)){
 			if(Var_Node* src1 = dynamic_cast<Var_Node*>(source1)){
 				if(Var_Node* src2 = dynamic_cast<Var_Node*>(source2)){	
				return_vec.push_back(dst->var->str + L2_operatormap[LOAD] + src1->var->str + " 0");
				return_vec.push_back(dst->var->str + L2_operatormap[op] + src2->var->str);		
				}
			}
		}
		return return_vec;
 	}

 	std::vector<std::string> genMovOP(Node* dest, Node* source, Operator op){

 		std::vector<std::string> return_vec;
 		if(Var_Node* dst = dynamic_cast<Var_Node*>(dest)){
 			if(Var_Node* src = dynamic_cast<Var_Node*>(source)){
 				switch(op){
 					case MOV: 
 						if(dst->var->str  != src->var->str){
 							return_vec.push_back(dst->var->str + L2_operatormap[MOV] + src->var->str);
 						}
 						break;
 					case LOAD: 
						return_vec.push_back(dst->var->str + L2_operatormap[LOAD] + src->var->str + " 0");
						break;
					case STORE: 
						return_vec.push_back(L2_operatormap[STORE] + dst->var->str + " 0" + L2_operatormap[MOV] + src->var->str);
						break;
				}
			}	
		}
		return return_vec;
 	}
 	int var_count = 0;

 	std::vector<std::string> genLeaOP(Node* dest, Node* source1, Node* source2, Node* source3){

 		std::string src1Var;
 		std::vector<std::string> return_vec;
 		if(Var_Node* dst = dynamic_cast<Var_Node*>(dest)){
 			if(Var_Node* src1 = dynamic_cast<Var_Node*>(source1)){
 				if(src1->var->op_type == NUM){
 					src1Var = "%newVar_L3c_" + to_string(var_count++);
 					return_vec.push_back(src1Var + L2_operatormap[MOV] + src1->var->str);	
 				}
 				else
 					src1Var = src1->var->str;
 				if(Var_Node* src2 = dynamic_cast<Var_Node*>(source2)){
 					if(Var_Node* src3 = dynamic_cast<Var_Node*>(source3)){
 						if(src3->var->op_type == NUM){
 							int num = stoll(src3->var->str);
 							if((num == 1) || (num == 2) || (num == 4) || (num == 8)){
 								return_vec.push_back(dst->var->str + " @ " + src1Var + " " + src2->var->str + " " + src3->var->str);
 							}
 							else{ //for nE values other than 1|2|4|8: we generate v1 @ v2 v3 closest_floor_E ; v3 *= (nE - closest_floor_E); v1 += v3; 
 								if(num > 8){
 									return_vec.push_back(dst->var->str + " @ " + src1Var + " " + src2->var->str + " 8");
 									if((num - 8) > 1)
 										return_vec.push_back(src2->var->str + L2_operatormap[MUL] + to_string(num - 8));
 									return_vec.push_back(dst->var->str + L2_operatormap[ADD] + src2->var->str);
 								}
 								else if(num > 4){
 									return_vec.push_back(dst->var->str + " @ " + src1Var + " " + src2->var->str + " 4");
 									if((num - 4) > 1)
 										return_vec.push_back(src2->var->str + L2_operatormap[MUL] + to_string(num - 4)); 
 									return_vec.push_back(dst->var->str + L2_operatormap[ADD] + src2->var->str);
 								}
 								else if(num > 2){
 									return_vec.push_back(dst->var->str + " @ " + src1Var + " " + src2->var->str + " 2");
 									return_vec.push_back(dst->var->str + L2_operatormap[ADD] + src2->var->str); //p1 <- src2 * 3; dest<-p1 + src1; can be genrated as: dest lea src1 src2 2; dest += src2;
 								}
 								else if(num == 0){
 									return_vec.push_back(dst->var->str + L2_operatormap[MOV] + src1Var); //  p1 <- src2 * 0; dest<-p1 + src1; is basically: dest<- src1;
 								}
 							}
 						}
 					}
 				}
 			}
 		}
 		return return_vec;
 	}

 	std::vector<std::string> genArithOP(Node* dest, Node* source1, Node* source2, Operator op){

 		std::vector<std::string> return_vec;
 		if(Var_Node* dst = dynamic_cast<Var_Node*>(dest)){
 			if(Var_Node* src1 = dynamic_cast<Var_Node*>(source1)){
 				if(Var_Node* src2 = dynamic_cast<Var_Node*>(source2)){
 					switch(op){
					case LT: case LE: case EQ: 
						return_vec.push_back(dst->var->str + L2_operatormap[MOV] + src1->var->str + L2_operatormap[op] + src2->var->str); //cmps can be done with 1 instruction in L2 anyways
						break;
					case GT: case GE:
						return_vec.push_back(dst->var->str + L2_operatormap[MOV] + src2->var->str + L2_operatormap[op] + src1->var->str);
						break;
					default:
						if(dst->var->str == src2->var->str)
								return_vec.push_back(dst->var->str + L2_operatormap[op] + src1->var->str); //dest arithOp src1 //dest += src1
						
						else if((dst->var->str == src1->var->str))
								return_vec.push_back(dst->var->str + L2_operatormap[op] + src2->var->str); //dest arithOP src2
						else{
							if ((src1->var->op_type == VAR) && (src2->var->op_type == VAR) && (op == ADD)){
								return_vec.push_back(dst->var->str + " @ " + src1->var->str + " " + src2->var->str + " 1"); /*can generate lea for dest <- src1 + src2 as dest @ src1 src2 1 
																															-- This might not make the generated code run faster on OOO machines. */
							}
							if ((src1->var->op_type == NUM) && (src2->var->op_type == NUM)) { //if both sources are constants, perform the operation in-compiler and move result to dest.
								if(op == ADD)
									return_vec.push_back(dst->var->str + L2_operatormap[MOV] + to_string(stol(src1->var->str)+stol(src2->var->str)) ) ;
								if(op == SUB)
									return_vec.push_back(dst->var->str + L2_operatormap[MOV] + to_string(stol(src1->var->str)-stol(src2->var->str)) ) ;
								if(op == MUL)
									return_vec.push_back(dst->var->str + L2_operatormap[MOV] + to_string(stol(src1->var->str)*stol(src2->var->str)) ) ;
								if(op == AND)
									return_vec.push_back(dst->var->str + L2_operatormap[MOV] + to_string(stol(src1->var->str)&stol(src2->var->str)) ) ;
								if(op == SAL)
									return_vec.push_back(dst->var->str + L2_operatormap[MOV] + to_string(stol(src1->var->str)<<stol(src2->var->str)) ) ;
								if(op == SAR)
									return_vec.push_back(dst->var->str + L2_operatormap[MOV] + to_string(stol(src1->var->str)>>stol(src2->var->str)) ) ;
							}
							else{
								return_vec.push_back(dst->var->str + L2_operatormap[MOV] + src1->var->str); //use the atmoic case with 2 instructions if we fail to match with a pattern above
								return_vec.push_back(dst->var->str + L2_operatormap[op] + src2->var->str);  //dest <- src1; dest += src2
							}
						}	
						break;
					}	
				}
			}
		}
		return return_vec;
 	}


 	void handleCallTrees(Tree* tree){
 		std::string str;
 		switch(tree->callInst->op){
 			case CALL: //Call instructions
				str = handleCalls(tree->callInst, false);
				break;
			case CALL_RETVAR:
				str = handleCalls(tree->callInst, true);
				break;
			case PRINT:
				str = "rdi" + L2_operatormap[MOV] + tree->callInst->operands[0]->str + "\n\t\tcall print 1"; 
				break;
			case ALLOCATE:
				str = "rdi" + L2_operatormap[MOV] + tree->callInst->operands[1]->str + "\n\t\trsi" + L2_operatormap[MOV] + tree->callInst->operands[2]->str + "\n\t\tcall allocate 2\n\t\t" + tree->callInst->operands[0]->str + L2_operatormap[MOV] + "rax";  
				break;
			case ARRAYERROR:
				str = "rdi" + L2_operatormap[MOV] + tree->callInst->operands[0]->str + "\n\t\trsi" + L2_operatormap[MOV] + tree->callInst->operands[1]->str + "\n\t\tcall array-error 2";
				break;
		}
		tree->gen_insts.push_back(str);
		return;
 	}
 	int label_count = 0;
 	std::string handleCalls(Instruction* Inst, bool isRetVar){
 		
		std::string str;
		int stackarg;
		if(isRetVar)
			stackarg = (Inst->operands.size() - 7 )*(-8);
		else
			stackarg = (Inst->operands.size() - 6 )*(-8);
		std::string return_label =  ":call_label"+to_string(label_count);
		label_count++;
		str = "mem rsp -8" + L2_operatormap[MOV] +  return_label +  "\n\t\t";

		if(Inst->operands.size()<(8 +isRetVar)){
			for(int i = 1+isRetVar; i<Inst->operands.size(); i++){
				str+= arg_registers[i-1 - isRetVar] + L2_operatormap[MOV] + Inst->operands[i]->str + "\n\t\t";
			}
		}
		else{
			for(int i = (1+isRetVar); i<(7+isRetVar); i++){
				str+= arg_registers[i-1-isRetVar] + L2_operatormap[MOV] + Inst->operands[i]->str + "\n\t\t";
			}
			for(int i = (7+isRetVar); i<Inst->operands.size(); i++){
				str+= "mem rsp "+ to_string(stackarg) + L2_operatormap[MOV] + Inst->operands[i]->str + "\n\t\t";
				stackarg+= 8;
			}
		}
		str+= L2_operatormap[CALL] + Inst->operands[isRetVar]->str + " " + to_string(Inst->operands.size()-1-isRetVar) + "\n\t\t" + return_label;
		if(isRetVar)
			str+= Inst->operands[0]->str + L2_operatormap[MOV] + "rax";
		return str;
	}

 	void instructionSelection(Program* p, int Verbose){
 		debug_instSelector = Verbose;
  		for(auto f : p->functions){
			
			if(debug_instSelector)
				livenessPrint(f); 
			else
				liveness(f);

			generateContexts(f);


			if(!(f->contexts.empty())){
				for(auto con: f->contexts){

					if(debug_instSelector){
						cout<<"CONTEXT:"<<endl;
						for(auto inst : con->instructions){
							cout<<inst->Inst<<endl;
						}
						cout<<endl<<endl;
					}

					condenseContexts(con);

					if(debug_instSelector){
						cout<<"CONDENSED CONTEXT:"<<endl;
						for(auto inst : con->instructions){
							cout<<inst->Inst<<endl;
						}
						cout<<endl<<endl;
					}

					generateTrees(con); 

					if(debug_instSelector){
						cout<<"CALL TREES:"<<endl;
						for (auto tree : con->trees){
							if(tree->isCallInst == true)
								cout<<tree->callInst->Inst<<endl;
						}
						cout<<endl<<endl;
					}

					if(debug_instSelector){
						cout<<"TREES:"<<endl;
						for (auto tree : con->trees){
							if(tree->isCallInst == false){
								printTree(tree->root);
								cout<<endl;
							}
						}
						cout<<endl<<endl;
					}

					FindMergeCandidates(con);

					if(debug_instSelector){
						cout<<"MERGED TREES:"<<endl;
						for (auto tree : con->trees){
							if(tree->isCallInst == false){
								printTree(tree->root);
								cout<<endl;
							}
						}
						cout<<endl<<endl;
					}

					for (auto tree : con->trees){
						condenseTree(tree->root);
					}

					if(debug_instSelector){
						cout<<"CONDENSED TREES:"<<endl;
						for (auto tree : con->trees){
							if(tree->isCallInst == false){
								printTree(tree->root);
								cout<<endl;
							}
						}
						cout<<endl<<endl;
					}

					for (auto tree : con->trees){
						if(tree->isCallInst == false)
							maximalMunch(tree->root, tree->gen_insts);
						else
							handleCallTrees(tree);
					}

					if(debug_instSelector){
						cout<<"GENERATED INSTS:"<<endl;
						for (auto tree : con->trees){
							for (auto inst : tree->gen_insts){
							cout<<inst<<endl;
							}
							cout<<endl;
						}
						cout<<endl<<endl;
					}
				}
			}
		}
	}
}	