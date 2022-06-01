//
// Created by Anton on 01.06.2022.
//

#include "BasicBlock.h"
#include "CFG.h"

int BasicBlock::guid = 0;

BasicBlock::BasicBlock(CFG *cfg, std::vector<Instr*> instrs):
        instrs(std::move(instrs)) {
    id = guid;
    guid++;
    cfg->bbs.push_back(this);
}

void BasicBlock::createBr(BasicBlock *bb) {
    succs.push_back(bb);
    bb->preds.push_back(this);
}

void BasicBlock::createCondBr(InstrOperand *cond, BasicBlock *thenBB, BasicBlock *elseBB) {
    auto condInstr = new Instr(cond, InstrOp::COND);
    instrs.push_back(condInstr);
    createBr(thenBB);
    createBr(elseBB);
}

static std::string getOpStr(InstrOp op)
{
    switch (op) {
        case InstrOp::ADD: return "+";
        case InstrOp::SUB: return "-";
        case InstrOp::MUL: return "*";
        case InstrOp::DIV: return "/";
        case InstrOp::LT: return "\\<";
        case InstrOp::GT: return "\\>";
        case InstrOp::EQ: return "==";
        case InstrOp::NE: return "!=";
    }
    return "invalid_op";
}


std::ostream &operator<<(std::ostream &strm, Instr &ins) {
    switch (ins.op) {
        case InstrOp::ADD:
        case InstrOp::SUB:
        case InstrOp::MUL:
        case InstrOp::DIV:
        case InstrOp::LT:
        case InstrOp::GT:
        case InstrOp::EQ:
        case InstrOp::NE: {
            strm << *ins.lhs << " = " << *ins.rhs1 << " " << getOpStr(ins.op) <<
                " " << *ins.rhs2;
            break;
        }
        case InstrOp::ALLOCA: {
            strm << "alloca " << *ins.lhs;
            break;
        }
        case InstrOp::FOR: {
            strm << "for";
            break;
        }
        case InstrOp::BREAK: {
            strm << "break";
            break;
        }
        case InstrOp::COND: {
            strm << "cond_br " << *ins.lhs;
            break;
        }
        case InstrOp::RET: {
            strm << "ret " << *ins.lhs;
            break;
        }
        case InstrOp::ASG: {
            strm << *ins.lhs << " = " << *ins.rhs1;
            break;
        }
        case InstrOp::INVALID: {
            strm << "invalid instruction";
            break;
        }
        case InstrOp::PHI: {
            strm << "phi";
            break;
        }
    }
    return strm;
}

std::ostream &operator<<(std::ostream &strm, InstrOperand &op) {
    if (op.isIdent) {
        strm << op.name;
    } else {
        strm << op.num;
    }
    return strm;
}

