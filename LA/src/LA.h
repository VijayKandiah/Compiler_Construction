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
namespace LA {

    /*
     * Instruction interface.
     */
    enum Operator { 
        MOV, // <-
        READ_ARRAY,
        WRITE_ARRAY,
        READ_TUPLE,
        WRITE_TUPLE,
        DEFINE, // define a var
        LENGTH, // var <- length var t
        ADD, SUB, MUL, AND, // +, -, *, & op
        LT, LE, EQ, GE, GT, // comparison <, <=, =, >=, >
        SAL, SAR, //<<, >>
        CALL,
        CALL_RETVAR,
        JUMP, //br label
        CJUMP, //br t label label
        PRINT, //print
        NEWARRAY, //var <- new Array(args)
        NEWTUPLE, //var <- new Tuple(args)
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
        bool dontEncode = false;
    };


    struct Instruction{ 
        std::string Inst;
        Operator op; 
        std::vector<Variable *> operands;
        std::vector<Variable *> indices; //only used by arrays
    };


    /*
     * Function.
     */
    struct Function{
        std::string name;
        std::string retType;
        std::vector<Variable *> arguments;
        std::vector<Instruction *> instructions;
        std::vector<std::string> tupleDefs;//vector of tuples defined in the function
        std::vector<std::string> codeDefs;//vector of codes defined in the function
    };

    /*
     * Program.
     */
    struct Program{
        std::vector<Function *> functions;
    };

}