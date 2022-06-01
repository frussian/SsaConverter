//
// Created by Anton on 01.06.2022.
//

#ifndef LAB4_CFG_H
#define LAB4_CFG_H

#include "Parser.h"
#include "BasicBlock.h"

class CFG {
public:
    CFG(StmtsNode *stmts): stmts(stmts){};
    void genCFG();
    void genGraphiz(const std::string &filename);
    void toSsa();
    BasicBlock *insertPoint;
    std::vector<BasicBlock*> bbs;
private:
    StmtsNode *stmts;
    BasicBlock *entryBB;

    std::vector<BasicBlock *> computePreOrder();

    void dfs(BasicBlock *bb, std::vector<BasicBlock*> *preorder);

    void unvisitBBs();

    void computeDominators();
};


#endif //LAB4_CFG_H
