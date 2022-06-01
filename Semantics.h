//
// Created by zoomo on 22.05.2022.
//

#ifndef LAB3_SEMANTICS_H
#define LAB3_SEMANTICS_H

#include "Parser.h"
#include <map>

class Semantics {
public:
    static bool analyse(StmtsNode*prog);
private:
    static bool analyseBlock(ForNode *forNode, StmtsNode *block, std::map<std::string, DefNode *> defs);
    static bool analyseExpr(ASTNode *node, std::map<std::string, DefNode *> &defs);
};


#endif //LAB3_SEMANTICS_H
