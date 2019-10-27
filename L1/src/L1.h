#pragma once

#include <vector>

namespace L1 {

    struct Item {
        std::string labelName;
    };

    /*
     * Instruction interface.
     */
    enum OperatorQ { // 
        MOVQ, // <-
        ADDQ, SUBQ, IMULQ, ANDQ, // +=, -=, *=, &= aop
        DEC, INC, // --, ++
        LQ, LEQ, EQ, // L1 comparison <, <=, = 
        JLE, JL, JE, // conditional jumps //skipped
        SALQ, SARQ, //<<=, >>=
        LEA, // w @ w w E 
        CALL,
        GOTO,
        PRINT,
        ALLOCATE,
        ARRAYERROR,
        LBL,
        RETQ // return
    };

    enum Operand_Type { 
        GPREG,   
        NUM, 
        LABEL, 
        MEM,
    };

    struct Operand{
        Operand_Type op_type;
        std::string str;
    };

    struct Instruction{
        std::string Inst;
        OperatorQ op; 
        std::vector<Operand *> operands; //gp_regs|labels|numbers|memxM
    };

    /*
     * Function.
     */
    struct Function{
        std::string name;
        int64_t arguments;
        int64_t locals;
        std::vector<Instruction *> instructions;
    };

    /*
     * Program.
     */
    struct Program{
        std::string entryPointLabel;
        std::vector<Function *> functions;
    };

}
