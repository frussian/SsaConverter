//
// Created by Anton on 01.06.2022.
//

#ifndef LAB4_CFG_H
#define LAB4_CFG_H

#include "Parser.h"
#include "BasicBlock.h"

#include <map>
#include <set>

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

    void dfs(BasicBlock *bb, std::vector<BasicBlock*> *preorder, std::vector<BasicBlock*> *postorder);

    void unvisitBBs();
    std::map<BasicBlock*, std::set<BasicBlock*>> computeDF(std::vector<BasicBlock*> &postorder);
    void computeDominators(std::vector<BasicBlock*> &preorder);
};


#endif //LAB4_CFG_H
