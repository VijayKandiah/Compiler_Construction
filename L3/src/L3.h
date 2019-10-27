#pragma once

#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <set>
#include <iterator>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <stdint.h>
#include <assert.h>
#include <iostream>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <stdint.h>
#include <unistd.h>

using namespace std;
namespace L3 {

    /*
     * Instruction interface.
     */
    enum Operator { 
        MOV, // <-
        ADD, SUB, MUL, AND, // +=, -=, *=, &= aop
        LT, LE, EQ, GE, GT, // L2 comparison <, <=, = 
        SAL, SAR, //<<=, >>=
        LOAD,
        STORE,
        CALL,
        CALL_RETVAR,
        GOTO, //br label
        CJUMP, //br var label
        PRINT,
        ALLOCATE,
        ARRAYERROR,
        LBL,
        RET, // return
        RETVAR
    };

    enum Operand_Type {   
        NUM, 
        LABEL, 
        VAR
    };

    const std::string arg_registers[] = {"rdi","rsi","rdx","rcx","r8","r9"};
    
    struct Variable{
        Operand_Type op_type;
        std::string str;
    };

    struct Instruction{ 
        std::string Inst;
        Operator op; 
        std::vector<Variable *> operands;
        std::vector<std::string> GEN;
        std::vector<std::string> KILL;
        std::vector<std::string> IN;
        std::vector<std::string> OUT;
        std::vector<Instruction *> preds;
        std::vector<Instruction *> sucs;
        int pos;
        bool marked_for_del = false;
        bool context_below = false;
    };

    struct Node{
        virtual ~Node() = default;
        Node* parent; 
        std::vector<Node *> children;
    };

    struct Var_Node : Node{
        Variable* var;
    };

    struct Op_Node : Node{
        Operator op;
    };

    struct Tree{
        Node* root;
        std::vector<std::string> gen_insts;
        std::set<std::string> varNodes;
        bool marked_for_del = false;
        bool isCallInst = false;
        Instruction* callInst; //used by callInsts
    };

    struct Context{
        std::vector<Instruction *> instructions;
        std::vector<Tree *> trees;
    };

    /*
     * Function.
     */
    struct Function{
        std::string name;
        std::vector<Variable *> arguments;
        std::vector<Instruction *> instructions;  
        std::vector<Context *> contexts;
        bool starts_with_context = false;
    };

    /*
     * Program.
     */
    struct Program{
        std::vector<Function *> functions;
    };
}
