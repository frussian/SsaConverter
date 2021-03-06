//
// Created by Anton on 01.06.2022.
//

#ifndef LAB4_CFG_H
#define LAB4_CFG_H

#include "Parser.h"
#include "BasicBlock.h"

#include <map>
#include <set>
#include <deque>

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
    static std::map<BasicBlock*, std::set<BasicBlock*>> computeDF(std::vector<BasicBlock*> &postorder);
    void computeDominators(std::vector<BasicBlock*> &preorder);

    void computeVarUsage();

    static std::set<BasicBlock *> computeDFForSet(const std::set<BasicBlock *> &vs, std::map<BasicBlock *, std::set<BasicBlock *>> &df);

    static std::set<BasicBlock *>
    computeDFIterable(const std::set<BasicBlock *> &vs, std::map<BasicBlock *, std::set<BasicBlock *>> &df);

    void placePhis(std::map<BasicBlock *, std::set<BasicBlock *>> &df);

    std::deque<int> stack;
    int counter = 0;

    void traverse(BasicBlock *bb, std::string &var);

    void renameVar(std::string &var);

    std::map<std::string, std::set<BasicBlock*>> usages;
};


#endif //LAB4_CFG_H
