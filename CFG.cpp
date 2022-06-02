//
// Created by Anton on 01.06.2022.
//

#include <fstream>
#include <iostream>
#include "CFG.h"
#include "BasicBlock.h"
#include "Parser.h"

void CFG::genCFG()
{
    entryBB = new BasicBlock(this);
    insertPoint = entryBB;

    stmts->fold(this);
}

void CFG::genGraphiz(const std::string &filename)
{
    std::ofstream fstr;
    fstr.open(filename);
    fstr << "digraph {" << std::endl;

    for (auto bb: bbs) {
        fstr << "\tbb_" << bb->id << " [" << std::endl;
        fstr << "\t\tstyle=filled," << std::endl;
        fstr << "\t\tshape=record," << std::endl;
        fstr << "\t\tlabel=\"{bb" << bb->id << "\\n";
//        fstr << "\t\t\t\t" << bb->id << " |" << std::endl;
        for (auto ins: bb->instrs) {
            fstr << "\t\t\t"<< *ins << "\\n";
        }
        fstr << "\t\t}\";" << std::endl;
        fstr << "\t];" << std::endl;
        for (auto succ: bb->succs) {
            fstr << "\t" << "bb_" << bb->id << " -> bb_" << succ->id << ";" << std::endl;
        }
    }
    fstr << "}";
    fstr.close();
}

void CFG::dfs(BasicBlock *bb, std::vector<BasicBlock*> *preorder, std::vector<BasicBlock*> *postorder)
{
    if (bb->isVisited) return;
    bb->isVisited = true;
    if (preorder) preorder->push_back(bb);
    for (auto succ: bb->succs) {
        if (!succ->isVisited) dfs(succ, preorder, postorder);
    }
    if (postorder) postorder->push_back(bb);
}

void CFG::unvisitBBs()
{
    for (auto bb: bbs) {
        bb->isVisited = false;
    }
}

void CFG::computeDominators(std::vector<BasicBlock*> &preorder)
{
    entryBB->idom = entryBB;
    for (auto v: preorder) {
        unvisitBBs();
        v->isVisited = true;
        dfs(entryBB, nullptr, nullptr);
        for (auto bb: bbs) {
            if (!bb->isVisited) {
                bb->idom = v;
            }
        }
    }
    for (auto par: bbs) {
        for (auto child: bbs) {
            if (par != child && par == child->idom) {
                par->children.push_back(child);
            }
        }
    }

    for (auto bb: bbs) {
        std::cout << "bb " << bb->idom->id << " idom bb " << bb->id << std::endl;
    }
}

std::map<BasicBlock*, std::set<BasicBlock*>> CFG::computeDF(std::vector<BasicBlock*> &postorder)
{
    std::map<BasicBlock*, std::set<BasicBlock*>> df;
    for (auto bb: postorder) {
        for (auto succ: bb->succs) {
            if (bb != succ->idom) {
                df[bb].insert(succ);
            }
        }
        for (auto child: bb->children) {
            for (auto y: df[child]) {
                if (y->idom != bb) df[bb].insert(y);
            }
        }
    }
    return std::move(df);
}

std::map<std::string, std::set<BasicBlock*>> CFG::computeVarUsage()
{
    std::map<std::string, std::set<BasicBlock*>> usages;

    for (auto bb: bbs) {
        for (auto stmt: bb->instrs) {
            if (stmt->lhs && stmt->lhs->isIdent) {
                usages[stmt->lhs->name].insert(bb);
            }
        }
    }

    for (auto p: usages) {
        auto var = p.first;
        std::cout << var << " used in ";
        for (auto usage: p.second) {
            std::cout << usage->id << " ";
        }
        std::cout << std::endl;
    }

    return std::move(usages);
}

std::set<BasicBlock*> CFG::computeDFForSet(const std::set<BasicBlock*> &vs, std::map<BasicBlock*, std::set<BasicBlock*>> &df)
{
    std::set<BasicBlock*> res;
    for (auto v: vs) {
        auto s = df[v];
        res.insert(s.begin(), s.end());
    }
    return std::move(res);
}

std::set<BasicBlock*> CFG::computeDFIterable(const std::set<BasicBlock*> &vs, std::map<BasicBlock*, std::set<BasicBlock*>> &df)
{
    std::set<BasicBlock*> res;
    std::set<BasicBlock*> dfi = computeDFForSet(vs, df);
    bool changed = false;

    do {
        changed = false;
        dfi.insert(vs.begin(), vs.end());
        dfi = computeDFForSet(dfi, df);
        if (dfi != res) {
            res = dfi;
            changed = true;
        }
    } while (changed);

    return res;
}

void CFG::placePhis(std::map<BasicBlock*, std::set<BasicBlock*>> &df)
{
    auto usages = computeVarUsage();

    for (const auto& v: usages) {
        auto phiset = computeDFIterable(v.second, df);
        for (auto bb: phiset) {
            InstrOperand *lhs = new InstrOperand(v.first);
            Instr *phi = new Instr(lhs, InstrOp::PHI);
            phi->phiRhs.insert(phi->phiRhs.begin(), bb->preds.size(), v.first);
            bb->instrs.insert(bb->instrs.begin(), phi);
        }
    }
}

void CFG::toSsa()
{
    std::vector<BasicBlock*> preorder;
    std::vector<BasicBlock*> postorder;
    unvisitBBs();
    dfs(entryBB, &preorder, &postorder);
    for (int i = 0; i < preorder.size(); i++) {
        std::cout << preorder.at(i)->id << " ";
    }
    computeDominators(preorder);
    auto df = computeDF(postorder);
    placePhis(df);
}

static InstrOp mapToInstrOp(ASTArithOp op)
{
    switch (op) {
        case AST_PLUS: return InstrOp::ADD;
        case AST_MINUS: return InstrOp::SUB;
        case AST_MUL: return InstrOp::MUL;
        case AST_DIV: return InstrOp::DIV;
    }
    return InstrOp::INVALID;
}

static InstrOp mapToInstrOp(ASTCmpOp op)
{
    switch (op) {
        case AST_LT: return InstrOp::LT;
        case AST_GT: return InstrOp::GT;
        case AST_EQ: return InstrOp::EQ;
        case AST_NE: return InstrOp::NE;
    }
    return InstrOp::INVALID;
}

static int tmp_ver = 0;

static std::string mapToInstrOpString(InstrOp op) {
    tmp_ver++;
    std::string tmp;
    switch (op) {
        case InstrOp::ADD: tmp = "addtmp"; break;
        case InstrOp::SUB: tmp = "subtmp"; break;
        case InstrOp::MUL: tmp = "multmp"; break;
        case InstrOp::DIV: tmp = "divtmp"; break;
        case InstrOp::LT: tmp = "lttmp"; break;
        case InstrOp::GT: tmp = "gttmp"; break;
        case InstrOp::EQ: tmp = "egtmp"; break;
        case InstrOp::NE: tmp = "netmp"; break;
        default: tmp = "invalidtmp"; break;
    }
    return tmp.append(std::to_string(tmp_ver));
}

InstrOperand *NumberNode::fold(CFG *cfg)
{
    return new InstrOperand(num);
}

InstrOperand *VarNode::fold(CFG *cfg)
{
    return new InstrOperand(name);
}

InstrOperand *BinOpNode::fold(CFG *cfg)
{
    BasicBlock *bb = cfg->insertPoint;
    auto rhsFold1 = lhs->fold(cfg);
    auto rhsFold2 = rhs->fold(cfg);
    auto instrOp = mapToInstrOp(op);
    auto name = mapToInstrOpString(instrOp);
    auto instrLhs = new InstrOperand(name);
    auto instr = new Instr(instrLhs, instrOp, rhsFold1, rhsFold2);
    bb->instrs.push_back(instr);
    return instrLhs;
}

InstrOperand *DefNode::fold(CFG *cfg)
{
    BasicBlock *bb = cfg->insertPoint;

    for (const auto &name: names) {
        auto def = new InstrOperand(name);
        auto instr = new Instr(def, InstrOp::ALLOCA);
        bb->instrs.push_back(instr);
    }
    return nullptr;
}

InstrOperand *AsgNode::fold(CFG *cfg)
{
    BasicBlock *bb = cfg->insertPoint;
    auto rhsFold = rhs->fold(cfg);
    auto lhsInstr = new InstrOperand(name);
    bb->instrs.push_back(new Instr(lhsInstr, InstrOp::ASG, rhsFold));
    return nullptr;
}

InstrOperand *CondNode::fold(CFG *cfg)
{
    BasicBlock *bb = cfg->insertPoint;
    auto rhsFold1 = lhs->fold(cfg);
    auto rhsFold2 = rhs->fold(cfg);
    auto instrOp = mapToInstrOp(cmpOp);
    auto name = mapToInstrOpString(instrOp);
    auto instrLhs = new InstrOperand(name);
    auto instr = new Instr(instrLhs, instrOp, rhsFold1, rhsFold2);
    bb->instrs.push_back(instr);
    return instrLhs;
}

InstrOperand *StmtsNode::fold(CFG *cfg)
{
    for (auto stmt: stmts) {
        auto ret = stmt->fold(cfg);
        if (ret) return ret;
    }

    return nullptr;
}

InstrOperand *IfNode::fold(CFG *cfg)
{
    BasicBlock *bb = cfg->insertPoint;
    auto condFold = cond->fold(cfg);

    auto *trueBB = new BasicBlock(cfg);
    auto *mergeBB = new BasicBlock(cfg);
    bb->createCondBr(condFold, trueBB, mergeBB);

    cfg->insertPoint = trueBB;
    auto ret = body->fold(cfg);
    if (!ret) {
        cfg->insertPoint->createBr(mergeBB);
    }

    cfg->insertPoint = mergeBB;
    return nullptr;
}

InstrOperand *ForNode::fold(CFG *cfg)
{
    BasicBlock *bb = cfg->insertPoint;
    auto loopBB = new BasicBlock(cfg);
    auto afterLoopBB = new BasicBlock(cfg);

    bb->instrs.push_back(new Instr(nullptr, InstrOp::FOR));

    customAfterBB = afterLoopBB;
    bb->createBr(loopBB);
    cfg->insertPoint = loopBB;

    auto ret = body->fold(cfg);
    if (!ret) {
        cfg->insertPoint->createBr(loopBB);
    }

    cfg->insertPoint = afterLoopBB;
    return nullptr;
}

InstrOperand *ReturnNode::fold(CFG *cfg)
{
    BasicBlock *bb = cfg->insertPoint;
    auto retVal = expr->fold(cfg);
    auto ret = new Instr(retVal, InstrOp::RET);
    bb->instrs.push_back(ret);
    return retVal;
}

InstrOperand *BreakNode::fold(CFG *cfg)
{
    BasicBlock *bb = cfg->insertPoint;
    auto breakIns = new Instr(nullptr, InstrOp::BREAK);
    bb->instrs.push_back(breakIns);
    bb->createBr(forNode->customAfterBB);
    return reinterpret_cast<InstrOperand *>(0x1);
}