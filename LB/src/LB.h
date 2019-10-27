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
namespace LB {

    /*
     * Instruction interface.
     */
    enum Operator { 
        MOV, // <-
        READ_ARRAY_TUPLE,
        WRITE_ARRAY_TUPLE,
        IF,
        WHILE,
        CONTINUE,
        BREAK,
        SCOPE_OPEN,
        SCOPE_CLOSE,
        DEFINE, // define a var
        LENGTH, // var <- length var t
        ADD, SUB, MUL, AND, // +, -, *, & op
        LT, LE, EQ, GE, GT, // comparison <, <=, =, >=, >
        SAL, SAR, //<<, >>
        CALL,
        CALL_RETVAR,
        JUMP, //br label
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
    };

    struct Scope{
        std::vector <std::pair<std::string,std::string>> varMap;
    };

    struct ScopeVars{
        std::vector<Scope *> scopes;
    };


    struct Instruction{ 
        std::string Inst;
        Operator op; 
        Operator cond; 
        std::vector<Variable *> operands;
        std::vector<Variable *> indices; //only used by arrays
        Instruction * condLabel;
        Instruction * loop;
    };


    /*
     * Function.
     */
    struct Function{
        std::string name;
        std::string retType;
        std::vector<Variable *> arguments;
        std::vector<Instruction *> instructions;
    };

    /*
     * Program.
     */
    struct Program{
        std::vector<Function *> functions;
    };

}