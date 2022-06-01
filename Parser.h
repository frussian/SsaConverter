//
// Created by Anton on 21.05.2022.
//

#ifndef LAB3_PARSER_H
#define LAB3_PARSER_H

#include <utility>
#include <vector>
#include <string>
#include <memory>
#include "llvm/IR/IRBuilder.h"
#include "Lexer.h"

class Token;

enum ASTCmpOp {
    AST_CMP_INVALID = -1,
    AST_LT = 0,
    AST_GT,
    AST_EQ,
    AST_NE
};

enum ASTArithOp {
    AST_PLUS = 0,
    AST_MINUS,
    AST_MUL,
    AST_DIV
};

class CodeGenContext {
public:
    std::unique_ptr<llvm::LLVMContext> ctx;
    std::unique_ptr<llvm::IRBuilder<>> builder;
    std::unique_ptr<llvm::Module> module;
    explicit CodeGenContext(){};
};

class Instr;
class InstrOperand;
class BasicBlock;
class CFG;

class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual llvm::Value *codegen(CodeGenContext *ctx) {return nullptr;};
    virtual InstrOperand *fold(CFG *cfg) {return nullptr;};
};

class NumberNode: public ASTNode {
public:
    explicit NumberNode(int num): num(num){};
    int num;
    llvm::Value *codegen(CodeGenContext *ctx) override;
    InstrOperand *fold(CFG *cfg) override;
};

class DefNode;

class VarNode: public ASTNode {
public:
    std::string name;
    DefNode *def = nullptr;
    explicit VarNode(std::string name):
        name(std::move(name)){};

    llvm::Value *codegen(CodeGenContext *ctx) override;
    InstrOperand *fold(CFG *cfg) override;
};

class BinOpNode: public ASTNode {
public:
    std::unique_ptr<ASTNode> lhs, rhs;
    ASTArithOp op;
    explicit BinOpNode(std::unique_ptr<ASTNode> lhs, std::unique_ptr<ASTNode> rhs,
                       ASTArithOp op):
    lhs(std::move(lhs)), rhs(std::move(rhs)), op(op){};

    llvm::Value *codegen(CodeGenContext *ctx) override;
    InstrOperand *fold(CFG *cfg) override;
};

class DefNode: public ASTNode {
public:
    std::vector<std::string> names;
    std::vector<llvm::AllocaInst*> allocas;
    explicit DefNode(std::vector<std::string> names): names(std::move(names)){};

    llvm::Value *codegen(CodeGenContext *ctx) override;
    InstrOperand *fold(CFG *cfg) override;

    llvm::AllocaInst *findAlloca(const std::string &name);
};

class AsgNode: public ASTNode {
public:
    std::string name;
    std::unique_ptr<ASTNode> rhs;
    DefNode *defNode = nullptr;
    explicit AsgNode(std::string name, std::unique_ptr<ASTNode> rhs):
        name(std::move(name)), rhs(std::move(rhs)){};

    llvm::Value *codegen(CodeGenContext *ctx) override;
    InstrOperand *fold(CFG *cfg) override;
};

class CondNode: public ASTNode {
public:
    std::unique_ptr<ASTNode> lhs, rhs;
    ASTCmpOp cmpOp;
    explicit CondNode(std::unique_ptr<ASTNode> lhs, std::unique_ptr<ASTNode> rhs,
                      ASTCmpOp cmpOp):
    lhs(std::move(lhs)), rhs(std::move(rhs)), cmpOp(cmpOp){};

    llvm::Value *codegen(CodeGenContext *ctx) override;
    InstrOperand *fold(CFG *cfg) override;
};

class StmtsNode: public ASTNode {
public:
    std::vector<ASTNode*> stmts;
    explicit StmtsNode(std::vector<ASTNode*> stmts):
            stmts(std::move(stmts)){};

    llvm::Value *codegen(CodeGenContext *ctx) override;
    InstrOperand *fold(CFG *cfg) override;
};

class IfNode: public ASTNode {
public:
    std::unique_ptr<CondNode> cond;
    std::unique_ptr<StmtsNode> body;
    explicit IfNode(std::unique_ptr<CondNode> cond, std::unique_ptr<StmtsNode> body):
        cond(std::move(cond)), body(std::move(body)){};

    llvm::Value *codegen(CodeGenContext *ctx) override;
    InstrOperand *fold(CFG *cfg) override;
};

class ForNode: public ASTNode {
public:
    std::unique_ptr<StmtsNode> body;
    llvm::BasicBlock *afterBB = nullptr;
    BasicBlock *customAfterBB = nullptr;
    explicit ForNode(std::unique_ptr<StmtsNode> body):
        body(std::move(body)){};

    llvm::Value *codegen(CodeGenContext *ctx) override;
    InstrOperand *fold(CFG *cfg) override;
};

class ReturnNode: public ASTNode {
public:
    std::unique_ptr<ASTNode> expr;
    explicit ReturnNode(std::unique_ptr<ASTNode> expr):
            expr(std::move(expr)){};

    llvm::Value *codegen(CodeGenContext *ctx) override;
    InstrOperand *fold(CFG *cfg) override;
};

class BreakNode: public ASTNode {
public:
    ForNode *forNode = nullptr;
    explicit BreakNode(){};

    llvm::Value *codegen(CodeGenContext *ctx) override;
    InstrOperand *fold(CFG *cfg) override;
};

class Parser {
public:
    explicit Parser(std::vector<Token> &tokens):
        tokens(tokens){};
    std::unique_ptr<StmtsNode> parse();
private:
    std::vector<Token> tokens;
    Token nextToken();
    Token currToken();
    Token peekToken();
    ASTCmpOp mapToAstCmp(DomainTag tag);
    void skipNewlines();
    std::unique_ptr<ASTNode> genError(const std::string& err = "err");
    std::unique_ptr<StmtsNode> parseBlock();
    std::vector<ASTNode*> parseStmts(bool *ok);
    std::unique_ptr<ASTNode> parseStmt();
    std::unique_ptr<ASTNode> parseDef();
    std::unique_ptr<ASTNode> parseAsg();
    std::unique_ptr<ASTNode> parseIf();
    std::unique_ptr<ASTNode> parseE();
    std::unique_ptr<BinOpNode> parseE2();
    std::unique_ptr<ASTNode> parseT();
    std::unique_ptr<ASTNode> parseF();
    std::unique_ptr<BinOpNode> parseT2();


    int i = 0;
};


#endif //LAB3_PARSER_H
