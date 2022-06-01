//
// Created by zoomo on 22.05.2022.
//
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Instructions.h>
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"

#include <algorithm>

#include "Parser.h"
#include "Codegen.h"


bool CodeGen::genCode(StmtsNode* prog) {
    CodeGenContext c_ctx;
    auto ctx = std::make_unique<llvm::LLVMContext>();
    auto builder = std::make_unique<llvm::IRBuilder<>>(*ctx);
    auto module = std::make_unique<llvm::Module>("Module", *ctx);

    llvm::FunctionType *main_type = llvm::FunctionType::
    get(llvm::Type::getInt32Ty(*ctx), false);
    llvm::Function *f_main = llvm::Function::Create(
            main_type, llvm::Function::ExternalLinkage,
            "main", module.get());
    llvm::BasicBlock *entryBB = llvm::BasicBlock::Create(
            *ctx, "entry", f_main);
    builder->SetInsertPoint(entryBB);

    c_ctx.ctx = std::move(ctx);
    c_ctx.builder = std::move(builder);
    c_ctx.module = std::move(module);

    prog->codegen(&c_ctx);

    c_ctx.module->print(llvm::errs(), nullptr);

    return true;
}

llvm::AllocaInst *CodeGen::createAlloca(CodeGenContext *ctx, const std::string &var) {
    llvm::Function *main = ctx->module->getFunction("main");

    llvm::IRBuilder<> tmp(&main->getEntryBlock(), main->getEntryBlock().begin());
    return tmp.CreateAlloca(llvm::Type::getInt32Ty(*ctx->ctx), nullptr, var);
}

llvm::CmpInst::Predicate CodeGen::mapToLLVMCmp(ASTCmpOp op) {
    switch (op) {
        case AST_LT: return llvm::CmpInst::ICMP_SLT;
        case AST_GT: return llvm::CmpInst::ICMP_SGT;
        case AST_EQ: return llvm::CmpInst::ICMP_EQ;
        case AST_NE: return llvm::CmpInst::ICMP_NE;
    }

    return llvm::CmpInst::BAD_ICMP_PREDICATE;
}

llvm::Value *NumberNode::codegen(CodeGenContext *ctx) {
//    llvm::ConstantFP
    return ctx->builder->getInt32(num);
}

llvm::Value *DefNode::codegen(CodeGenContext *ctx) {
    for (const auto& name: names) {
        auto alloca = CodeGen::createAlloca(ctx, name);
        ctx->builder->CreateStore(ctx->builder->getInt32(0), alloca);
        allocas.push_back(alloca);
    }
    return nullptr;
}

llvm::AllocaInst *DefNode::findAlloca(const std::string &name) {
    int i = 0;
    for (i = 0; i < names.size(); i++) {
        if (names[i] == name) {
            break;
        }
    }
    return allocas[i];
}

llvm::Value *VarNode::codegen(CodeGenContext *ctx) {
    auto alloca = def->findAlloca(name);
    return ctx->builder->CreateLoad(alloca->getAllocatedType(), alloca);
}

llvm::Value *BinOpNode::codegen(CodeGenContext *ctx) {
    auto lhsVal = lhs->codegen(ctx);
    auto rhsVal = rhs->codegen(ctx);
    switch (op) {
        case AST_PLUS: return ctx->builder->CreateAdd(lhsVal, rhsVal);
        case AST_MINUS: return ctx->builder->CreateSub(lhsVal, rhsVal);
        case AST_MUL: return ctx->builder->CreateMul(lhsVal, rhsVal);
        case AST_DIV: return ctx->builder->CreateSDiv(lhsVal, rhsVal);
    }
//    ctx->builder->CreateAdd()
    return nullptr;
}

llvm::Value *AsgNode::codegen(CodeGenContext *ctx) {
    auto val = rhs->codegen(ctx);
    auto alloca = defNode->findAlloca(name);
    ctx->builder->CreateStore(val, alloca);
    return nullptr;
}

llvm::Value *CondNode::codegen(CodeGenContext *ctx) {
    auto vall = lhs->codegen(ctx);
    auto valr = rhs->codegen(ctx);
    return ctx->builder->CreateCmp(CodeGen::mapToLLVMCmp(cmpOp), vall, valr);
}

llvm::Value *StmtsNode::codegen(CodeGenContext *ctx) {
    for (auto stmt: stmts) {
        auto ret = stmt->codegen(ctx);
        if (ret) return ret;
    }
    return nullptr;
}

llvm::Value *IfNode::codegen(CodeGenContext *ctx) {
    auto condV = cond->codegen(ctx);
    auto main = ctx->module->getFunction("main");
    auto thenBB = llvm::BasicBlock::Create(*ctx->ctx, "then", main);
    auto mergeBB = llvm::BasicBlock::Create(*ctx->ctx, "merge", main);

    ctx->builder->CreateCondBr(condV, thenBB, mergeBB);
    ctx->builder->SetInsertPoint(thenBB);
    auto ret = body->codegen(ctx);
    if (!ret) {
        ctx->builder->CreateBr(mergeBB);
    }

    ctx->builder->SetInsertPoint(mergeBB);
    return nullptr;
}

llvm::Value *ForNode::codegen(CodeGenContext *ctx) {
    auto main = ctx->module->getFunction("main");
    auto loopBB = llvm::BasicBlock::Create(*ctx->ctx, "loop", main);
    auto afterLoopBB = llvm::BasicBlock::Create(*ctx->ctx, "afterloop", main);
    afterBB = afterLoopBB;
    ctx->builder->CreateBr(loopBB);
    ctx->builder->SetInsertPoint(loopBB);
    auto ret = body->codegen(ctx);
    if (!ret) {
        ctx->builder->CreateBr(loopBB);
    }
//    ctx->builder->SetInsertPoint(loopBB);
    ctx->builder->SetInsertPoint(afterLoopBB);
    return nullptr;
}

llvm::Value *ReturnNode::codegen(CodeGenContext *ctx) {
    return ctx->builder->CreateRet(expr->codegen(ctx));
}

llvm::Value *BreakNode::codegen(CodeGenContext *ctx) {
    return ctx->builder->CreateBr(forNode->afterBB);
}