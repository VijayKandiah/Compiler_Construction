#pragma once

#include <vector>
#include <set>
#include <string>
namespace L2 {

    /*
     * Instruction interface.
     */
    enum Operator { 
        MOVQ, // <-
        ADDQ, SUBQ, IMULQ, ANDQ, // +=, -=, *=, &= aop
        DEC, INC, // --, ++
        LQ, LEQ, EQ, // L2 comparison <, <=, = 
        JLE, JL, JE, // conditional jumps //skipped
        SALQ, SARQ, //<<=, >>=
        LEA, // w @ w w E 
        CALL,
        GOTO,
        PRINT,
        ALLOCATE,
        ARRAYERROR,
        LBL,
        RETQ, // return
    };

    enum Var_Type { 
        READ,   // spilled variable is in GEN set only
        WRITE, // spilled variable is in both GEN and KILL sets
        READWRITE, // spilled variable is in KILL set only
        NO_OP // spilled variable is not in both GEN AND KILL sets
    };

    enum Operand_Type { 
        GPREG,   
        NUM, 
        LABEL, 
        MEMREG,
        MEMVAR,
        VAR,
        SARG
    };

    // colors are registers
    enum Color_Type{
        R10, R11, R8, R9, RAX, RCX, RDI, RDX, RSI,
        R12, R13, R14, R15, RBP, RBX,
        SPILL
    };

    struct Operand{
        Operand_Type op_type;
        std::string str;
        int Mvalue;
        std::string X;
    };

    struct Instruction{ 
        std::string Inst;
        Operator op; 
        std::vector<Operand *> operands;
        Var_Type spilltype;
        bool marked_for_del = false;
        std::vector<std::string> GEN;
        std::vector<std::string> KILL;
        std::vector<std::string> IN;
        std::vector<std::string> OUT;
        std::vector<Instruction *> preds;
        std::vector<Instruction *> sucs;

    };

    struct Variable{
        std::string node;
        Operand_Type op_type;
        std::set<std::string> edges;
        std::set<std::string> original_edges; //store the topology
        Color_Type color;
    };
    
    struct InterferenceGraph{
        std::set<Variable *> variables;
    };


    /*
     * Function.
     */
    struct Function{
        std::string name;
        int64_t arguments;
        int64_t locals;
        std::vector<Instruction *> instructions;
        InterferenceGraph* iGraph;
        std::string spill_replace; //variable which replaces the spilled variable
        std::string spilled_variable; //variable in function to be spilled
        int64_t numCalls;
    };


    /*
     * Program.
     */
    struct Program{
        std::string entryPointLabel;
        std::vector<Function *> functions;
    };

}