//
// Created by Anton on 01.06.2022.
//

#ifndef LAB4_BASICBLOCK_H
#define LAB4_BASICBLOCK_H

#include <string>
#include <vector>

enum class InstrOp {
    INVALID = -1,
    ADD = 1,
    SUB,
    MUL,
    DIV,
    LT,
    GT,
    EQ,
    NE,
    ASG,
    ALLOCA,
    COND,  //if
    FOR,
    RET,
    BREAK,
    PHI
};

class InstrOperand {
public:
    InstrOperand(std::string nameArg):
        name(std::move(nameArg)), isIdent(true) {};
    InstrOperand(int num):
        num(num), isIdent(false) {};

    std::string name;
    int num;
    bool isIdent;
    friend std::ostream& operator<<(std::ostream &strm, InstrOperand&ins);
};

class Instr {
public:
    Instr(InstrOperand *lhs, InstrOp op, InstrOperand *rhs1 = nullptr,
                 InstrOperand *rhs2 = nullptr):
                 lhs(lhs), op(op), rhs1(rhs1), rhs2(rhs2) {};

    InstrOperand *lhs;
    InstrOp op;
    InstrOperand *rhs1;
    InstrOperand *rhs2;

    friend std::ostream& operator<<(std::ostream &strm, Instr&ins);
};

class CFG;

class BasicBlock {
public:
    explicit BasicBlock(CFG *cfg, std::vector<Instr*> instrs = std::vector<Instr*>());
    int id;
    void createBr(BasicBlock *bb);
    void createCondBr(InstrOperand* cond, BasicBlock *thenBB, BasicBlock *elseBB);
    std::vector<BasicBlock*> preds, succs;
    std::vector<Instr*> instrs;
    bool isVisited = false;
    BasicBlock *idom = nullptr;
    static int guid;
};


#endif //LAB4_BASICBLOCK_H
