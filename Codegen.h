//
// Created by zoomo on 22.05.2022.
//

#ifndef LAB3_CODEGEN_H
#define LAB3_CODEGEN_H

#include <llvm/IR/Instructions.h>

#include "Parser.h"
class CodeGen {
public:
    static bool genCode(StmtsNode* prog);
    static llvm::AllocaInst *createAlloca(CodeGenContext* ctx, const std::string &var);
    static llvm::CmpInst::Predicate mapToLLVMCmp(ASTCmpOp op);
};
#endif //LAB3_CODEGEN_H
