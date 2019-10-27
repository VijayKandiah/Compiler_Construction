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
namespace IR {

    /*
     * Instruction interface.
     */
    enum Operator { 
        MOV, // <-
        READ_ARRAY,
        WRITE_ARRAY,
        READ_TUPLE,
        WRITE_TUPLE,
        DEFINE, // define a var - we ignore these
        LENGTH, // var <- length var t
        ADD, SUB, MUL, AND, // +, -, *, & op
        LT, LE, EQ, GE, GT, // comparison <, <=, =, >=, >
        SAL, SAR, //<<, >>
        CALL,
        CALL_RETVAR,
        JUMP, //br label
        CJUMP, //br t label label
        PRINT, //call print
        NEWARRAY, //var <- new array(args)
        NEWTUPLE, //var <- new tuple(args)
        ARRAYERROR, //call array-error
        LBL,
        RET, // return
        RETVAR
    };

    enum Operand_Type {   
        NUM, 
        LABEL, 
        VAR
    };


    struct Variable{
        Operand_Type op_type;
        std::string str;
        std::string defType;
    };


    struct Instruction{ 
        std::string Inst;
        Operator op; 
        std::vector<Variable *> operands;
        std::vector<Variable *> indices; //only used by arrays
    };

    struct BasicBlock{
        Instruction* entry_label;
        std::vector<Instruction *> instructions;
        Instruction* exit;
        bool removeExitInst = false; //to remove unconditional branches if its safe to do so
    };

    /*
     * Function.
     */
    struct Function{
        std::string name;
        std::string retType;
        std::vector<Variable *> arguments;
        std::vector<BasicBlock *> bblocks;
        std::vector<std::string> tupleDefs;
        std::vector<std::string> codeDefs;
    };

    /*
     * Program.
     */
    struct Program{
        std::vector<Function *> functions;
    };

}