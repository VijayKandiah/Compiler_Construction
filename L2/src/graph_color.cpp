#include "liveness.h"
#include "spiller.h"
#include "graph_color.h"
#include "utils.h"

using namespace std;

namespace L2{ 
    int debug_graphcolor = 0;
    // map each GP register string to respective color
    std::map<std::string, Color_Type> regmap = {
        {"r10", R10}, 
        {"r11", R11}, 
        {"r8", R8}, 
        {"r9", R9}, 
        {"rax", RAX}, 
        {"rcx", RCX}, 
        {"rdi", RDI}, 
        {"rdx", RDX}, 
        {"rsi", RSI}, 
        {"r12", R12},
        {"r13", R13}, 
        {"r14", R14},
        {"r15", R15}, 
        {"rbp", RBP}, 
        {"rbx", RBX} 
    };

    // map each color to respective GP register string
    std::map<Color_Type, std::string> colormap = {
        {R10, "r10"}, 
        {R11, "r11"}, 
        {R8, "r8"}, 
        {R9, "r9"}, 
        {RAX, "rax"}, 
        {RCX, "rcx"}, 
        {RDI, "rdi"}, 
        {RDX, "rdx"}, 
        {RSI, "rsi"}, 
        {R12, "r12"},
        {R13, "r13"}, 
        {R14, "r14"},
        {R15, "r15"}, 
        {RBP, "rbp"}, 
        {RBX, "rbx"}
    };

    std::string uniqueVar = "";
    int var_count = 0;

    // coloring algorithm
    void color_iGraph(Function* f, int Verbose){
        debug_graphcolor = Verbose;

        //Loop flags
        bool need_spill = 0; // bool to indicate if we need to spill at all
        bool spilled_already = 0;  // bool to indicate if we spilled already and are going through the loop with the spill_replace vars
        bool need_shuffle = 0; // see spilling algorithm below
        calcNumCalls(f);
        findUniqueVar(f);
        deadCodeElimination(f);
        if(Verbose){
            cout<< "Performing DCE and Register allocation for Function (" << f->name<<endl;
            cout<< "\t" << f->arguments << " " << f->locals <<endl;
            for (auto i : f->instructions)
                cout << "\t" << i->Inst <<endl;
            cout<< ")" <<endl;
        }
        auto Originalf = new Function();
        copy_func(f, Originalf); //stores a copy of the original function

        std::vector<Variable *> stackVars;
        std::vector<Variable *> spilledVars;
        std::vector<Variable *> PrevspilledVars;
        do{
            
             if(debug_graphcolor){
                interference_print(f);
            }
            else
                 interference(f); //Generate interference graph
            
            //Set initial color for all gpregs nad variables(set to SPILL initially)
            for (auto i : f->iGraph->variables){ 
                if(i->op_type == GPREG)
                   i->color = regmap[i->node];
                else
                    i->color = SPILL;
            }
            
            for(auto i : f->iGraph->variables) 
                i->original_edges = i->edges; //store initial topology

            /*
            repetedly select a node and remove it from the graph,
            /putting it on top of stack s: first nodes with degree<15, then nodes with degree>=15;
            first put nodes with larger degree, then put nodes with smaller degree;
            always using updated degree: recompute variable->edges (degree) each time remove node.
            */
            remove_node(f, stackVars, need_shuffle, PrevspilledVars, spilledVars);

            
            if(debug_graphcolor){
                cout<<endl<<endl;
                cout<<"variables STACK: "<<endl;
                for(auto var: stackVars){
                    cout<<var->node<<" ";
                }
                cout<<endl<<endl;
            }

            /*
            when the graph is empty, rebuild it
            Step 1)select a color on each node (except GP registers)
            caller-save regs prioritized
            */
            if(!stackVars.empty()){
                need_spill = rebuild_graph(f, stackVars, spilledVars);
            }

            if(debug_graphcolor){
                cout<<"SPILLED variables: "<<endl;
                for(auto var: spilledVars){
                    cout<<var->node<<" ";
                }
                cout<<endl<<endl;
                cout<<"ASSIGNED COLORS: "<<endl;
                for(auto var: f->iGraph->variables){
                    if(var->op_type == VAR)
                        cout<<var->node<<" : "<<colormap[var->color]<<endl;
                }
                cout<<endl<<endl;
            }

            if(!spilled_already)
                PrevspilledVars = spilledVars;
            spilledVars = {};

            
            /*
            Step 2) when not enough colors: select nodes to spill
            everytime you spill: Liveness analysis; interference graph; graph coloring

            Algorithm for Spilling to prevent getting stuck in graph coloring loop(works atleast for all the provided tests):

            IF Spilling all the variables in the first iteration works and all spill_replace vars have colors assigned to them --need_shuffle = 0
                exit color_iGraph 
            ELSE we need to spill even after spilling already (need to spill the spill_replace vars S0, S1, etc) --need_shuffle = 1
                Reset the function to initial state,
                    IF there are other nodes with greater than or equal edges as the spilled variable
                        Try assigning color to previously spilled variable first (move it to top of stackVars to prioritize assigning color to it) 
                        --This is done to basically see if we can solve this register assignment problem without spilling any more #variables
                    ELSE
                        add to set of spilled nodes: next variable with max edges less than that of previously spilled variable -- we spill one more variable even if it can get assigned a color 
            
            */

             if(spilled_already && need_spill){
                copy_func(Originalf, f); //restore original function to state before spilling
                need_shuffle = 1;
                spilled_already = 0;
            }
            else{
                if(need_spill)
                    spilled_already = spill_vars(f);
                else
                    replace_vars(f);
                clear_sets(f); 

                need_shuffle = 0;
            }
        }while(need_spill);
    }

    void calcNumCalls(Function* f){
        f->numCalls = 0;
        for(auto i: f->instructions){
            if((i->op == CALL) || (i->op == PRINT) || (i->op == ALLOCATE) || (i->op == ARRAYERROR)){
                f->numCalls++;
            }
        }
        return;
    }

    void findUniqueVar(Function *f){
        std::string lv = "";

        for(auto i: f->instructions){
            for(auto var : i->operands){
                if(var->op_type == VAR){
                    if (var->str.size() >=lv.size()) 
                        lv = var->str;
                }   
            }
        }
            
        if (lv.empty())
            lv +=  "%newVarL2_";
        else
        lv +=  "_L2c_";
        uniqueVar = lv;
        return;
    }

    void remove_node(Function* f, std::vector<Variable *> &stackVars, bool need_shuffle, std::vector<Variable *> &prevspilled, std::vector<Variable *> &spilled){
        bool flag = false;
        // find whether there is node with degree<15
        for(auto i : f->iGraph->variables){
            if ((i->edges.size()<15) && (i->op_type == VAR)){
                flag = true;
                break;
            }
        }

        while(flag){ // when there is node with degree<15
            int maxdegree = 0;
            Variable *maxnode; // points to the node with maximum edges <15
            for (auto i : f->iGraph->variables){
                if ((i->edges.size()<15) && (i->edges.size()>=maxdegree) && (i->op_type == VAR)){
                    maxdegree = i->edges.size();
                    maxnode =  i;
                }
            }
            f->iGraph->variables.erase(maxnode);
            maxnode->edges = {};
            stackVars.push_back(maxnode);
            for (auto i : f->iGraph->variables){
                for (auto e : i->edges){
                    if (e == maxnode->node){
                        i->edges.erase(e);
                        break;
                    }
                } 
            }
            flag = false;
            // find whether there is node with degree<15
            for(auto i : f->iGraph->variables){
                if ((i->edges.size()<15) && (i->op_type == VAR)){
                    flag = true;
                    break;
                }
            }
        }

        flag = false;
        for(auto i : f->iGraph->variables){
            if (i->op_type == VAR){
                flag = 1;
                break;
            }
        }

        while(flag){ // deal with all the remaining nodes with edges > 14
            int maxdegree = 0;
            Variable *maxnode; // points to the node with maximum edges
            for (auto i : f->iGraph->variables){
                if ((i->edges.size()>=maxdegree) && (i->op_type == VAR)){
                    maxdegree = i->edges.size();
                    maxnode = i;
                }
            }
            f->iGraph->variables.erase(maxnode);
            maxnode->edges = {};
            stackVars.push_back(maxnode);
            for (auto i : f->iGraph->variables){
                for (auto e : i->edges){
                    if (e == maxnode->node){
                        i->edges.erase(e);
                        break;
                    }
                }
            }
            flag = false;
            for(auto i : f->iGraph->variables){
                if (i->op_type == VAR){
                    flag = true;
                    break;
                }
            }
        }

        //handle need_shuffle to avoid getting stuck in graph coloring loop
        Variable *Varspilled;
        Variable *nextSpilledVar;
        int maxdegree = 0;
        if(need_shuffle){
            Variable * stackTop= stackVars.back();
            Varspilled = prevspilled.back();
            prevspilled = {};
            bool add_to_spill = 1;
            for(auto i : stackVars){
                for(auto e : i->original_edges){
                    if((Varspilled->node == e) && (i->original_edges.size()>= Varspilled->original_edges.size())){
                        add_to_spill = 0;
                    }      
                }         
            }
            for (auto i : stackVars){
                if ( (i->node != stackTop->node) && (i->original_edges.size()>maxdegree) && (i->op_type == VAR)){
                    maxdegree = i->edges.size();
                    nextSpilledVar = i;
                }
            }
            if(add_to_spill){   
                spilled.push_back(nextSpilledVar); //should push next var with max edges to spill
            }
            else{
                for(int j = 0; j<stackVars.size(); j++){
                    if(stackVars[j]->node == Varspilled->node){
                        std::rotate(stackVars.begin() + j, stackVars.begin() + j + 1, stackVars.end());
                        break;
                    }
                }
            } 
        }
    }


    bool rebuild_graph(Function* f, std::vector<Variable *> &s, std::vector<Variable *> &spilled){

        std::vector<Color_Type> color_vec;
        if(f->numCalls <= 4)
            color_vec = {RDI, RSI, RDX, R8, R9, R10, RCX, R11, RAX, R12, R13, R14, R15, RBP, RBX}; //caller-save regs first
        else
            color_vec = {R12, R13, R14, R15, RBP, RBX, RDI, RSI, RDX, R8, R9, R10, RCX, R11, RAX}; //callee-save regs first

        bool failed = 0; //whether need to spill
        std::vector<Color_Type> not_usable; //set of registers not usable to color current_var
        Variable *current_var;
        int assigned_color;
        while(!s.empty()){
            current_var = s.back();
            s.pop_back();
            not_usable = {};
            for(auto i : f->iGraph->variables){
                //if current_var->node in i->origin_edges
                if(i->original_edges.count(current_var->node)){
                    for(auto reg : color_vec){
                        if(i->color == reg)
                            not_usable.push_back(reg);
                    }
                    current_var->edges.insert(i->node);
                    i->edges.insert(current_var->node);
                }               
            }

            //find usable color indices
            assigned_color = 15;
            for(int j = 0; j<15; j++){
                if(!(std::find(not_usable.begin(), not_usable.end(), color_vec[j]) != not_usable.end())){//if j not in indices
                    assigned_color = j;
                    break;
                } 
            }

            if(assigned_color > 14){ //we have only 15 gpregs
                failed = 1; //need to spill
                spilled.push_back(current_var);
            }
            else
                current_var->color = color_vec[assigned_color];
              
            
            f->iGraph->variables.insert(current_var);
        
        }

        for(auto i : f->iGraph->variables){
            for(auto var : spilled){
                if (i->node == var->node)
                    i->color = SPILL;
            }
        }
        return failed;
    }


    void replace_vars(Function* f){ // replace all colored variables

        for(auto i : f->iGraph->variables){
            if((i->op_type == VAR) && (i->color != SPILL)){
                for (auto j : f->instructions){
                    for (auto &&op : j->operands){
                        if(op->str == i->node){
                            op->str = colormap[i->color];
                            op->op_type = GPREG;
                        }
                        if((op->op_type == MEMVAR) && (op->X == i->node)){
                            op->X = colormap[i->color];
                            op->op_type = MEMREG;
                        }
                    }
                }   
            }
        }
    }


    bool spill_vars(Function* f){ //call the spiller
        var_count = 0;
        // replace all colored variables first before spilling
        //replace_vars(f);
        for(auto i : f->iGraph->variables){
            if((i->op_type == VAR) && (i->color == SPILL)){
                f->spill_replace = uniqueVar + to_string(var_count++);
                f->spilled_variable = i->node;
                spill(f);
            }
        }
        return 1;
    }


    void copy_func(Function *f, Function *Originalf){ //deep_copy essential parts of function to restore it to original state later

        Originalf->name = f->name;
        Originalf->arguments = f->arguments;
        Originalf->locals= f->locals;
        Originalf->instructions = {};
        Originalf->iGraph = {};

        for (auto i : f->instructions){
            auto newInst = new Instruction();
            newInst->Inst= i->Inst;
            newInst->op= i->op;
            for(auto o : i->operands){
                auto ops = new Operand();
                ops->op_type = o->op_type;
                ops->str = o->str;
                ops->Mvalue = o->Mvalue;
                ops->X = o->X;
                newInst->operands.push_back(ops);
            }          
            Originalf->instructions.push_back(newInst);       
        }
        return;
    }


    void clear_sets(Function *f){ //clear gen,kill,in,out sets of function

        for(auto i : f->instructions ){
            i->IN = {};
            i->OUT = {};
            i->GEN = {};
            i->KILL = {};
        }
    }
}