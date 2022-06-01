//
// Created by zoomo on 22.05.2022.
//

#include "Semantics.h"

bool Semantics::analyse(StmtsNode* prog) {
    return analyseBlock(nullptr, prog, {});
}

bool Semantics::analyseBlock(ForNode *forNode, StmtsNode *block, std::map<std::string, DefNode *> defs) {
    for (auto stmt: block->stmts) {
        if (auto def = dynamic_cast<DefNode*>(stmt)) {
            for (const auto& d: def->names) {
                defs[d] = def;
            }
        } else if (auto asgNode = dynamic_cast<AsgNode*>(stmt)) {
            auto it = defs.find(asgNode->name);
            if (it == defs.end()) {
                printf("unknown variable: %s\n", asgNode->name.data());
                return false;
            }
            auto ok = analyseExpr(asgNode->rhs.get(), defs);
            if (!ok) return false;
            asgNode->defNode = it->second;
        } else if (auto newForNode = dynamic_cast<ForNode*>(stmt)) {
            bool ok = analyseBlock(newForNode, newForNode->body.get(), defs);
            if (!ok) return false;
        } else if (auto ifNode = dynamic_cast<IfNode*>(stmt)) {
            bool ok = analyseBlock(forNode, ifNode->body.get(), defs);
            if (!ok) return false;
            ok = analyseExpr(ifNode->cond.get(), defs);
            if (!ok) return false;
        } else if (auto breakNode = dynamic_cast<BreakNode*>(stmt)) {
            if (!forNode) {
                printf("invalid break outside of loop\n");
                return false;
            }
            breakNode->forNode = forNode;
        } else if (auto returnNode = dynamic_cast<ReturnNode*>(stmt)) {
            bool ok = analyseExpr(returnNode->expr.get(), defs);
            if (!ok) return false;
        }
    }
    return true;
}

bool Semantics::analyseExpr(ASTNode *node, std::map<std::string, DefNode *> &defs) {
    if (auto varNode = dynamic_cast<VarNode*>(node)) {
        auto it = defs.find(varNode->name);
        if (it == defs.end()) {
            printf("unknown variable: %s\n", varNode->name.data());
            return false;
        }
        varNode->def = it->second;
    } else if (auto binOp = dynamic_cast<BinOpNode*>(node)) {
        auto ok = analyseExpr(binOp->lhs.get(), defs);
        if (!ok) return false;
        ok = analyseExpr(binOp->rhs.get(), defs);
        if (!ok) return false;
    } else if (auto condNode = dynamic_cast<CondNode*>(node)) {
        auto ok = analyseExpr(condNode->lhs.get(), defs);
        if (!ok) return false;
        ok = analyseExpr(condNode->rhs.get(), defs);
        if (!ok) return false;
    }
    return true;
}

//if (auto varNode = dynamic_cast<VarNode*>(stmt)) {
//auto it = defs.find(varNode->name);
//if (it == defs.end()) {
//printf("unknown variable: %s\n", varNode->name.data());
//return false;
//}
//varNode->def = it->second;
