//
// Created by Anton on 21.05.2022.
//

#include "Parser.h"
#include "Lexer.h"
#include <iostream>

std::unique_ptr<StmtsNode> Parser::parse() {
    return parseBlock();
}

//{ stmts }
std::unique_ptr<StmtsNode> Parser::parseBlock() {
    auto tok = currToken();
    if (tok.tag != L_CURLY_BR) {
        genError();
        return nullptr;
    }
    nextToken();
    bool ok;
    auto stmts = parseStmts(&ok);
    tok = currToken();
    nextToken();
    if (!ok || tok.tag != R_CURLY_BR) {
        genError();
        return nullptr;
    }
    return std::make_unique<StmtsNode>(stmts);
}

std::vector<ASTNode*> Parser::parseStmts(bool *ok) {
    std::vector<ASTNode*> stmts;
    *ok = true;

    while (true) {
        skipNewlines();
        auto tok = currToken();
        if (tok.tag == R_CURLY_BR) {
            return stmts;
        }
        auto stmt = parseStmt();
        if (stmt == nullptr) {
            *ok = false;
            return {};
        }
//        std::cout << stmt.get() << std::endl;
        tok = currToken();
        if (tok.tag != NL && tok.tag != R_CURLY_BR) {
            genError();
            *ok = false;
            return {};
        }
        stmts.push_back(stmt.release());
    }
    return stmts;
}

std::unique_ptr<ASTNode> Parser::parseStmt() {
    auto tok = currToken();
//    printf("scan sstmt %d\n", tok.tag);
    switch (tok.tag) {
        case KEYWORD_INT: {
            nextToken();
            return parseDef();
        }
        case IDENT: {
            return parseAsg();
        }
        case KEYWORD_FOR: {
            nextToken();
            auto body = parseBlock();
            if (!body) return nullptr;
            return std::make_unique<ForNode>(std::move(body));
        }
        case KEYWORD_IF: {
            nextToken();
            return parseIf();
        }
        case KEYWORD_BREAK: {
            nextToken();
            return std::make_unique<BreakNode>();
        }
        case KEYWORD_RETURN: {
            nextToken();
            auto expr = parseE();
            if (!expr) return nullptr;
            return std::make_unique<ReturnNode>(std::move(expr));
        }
    }
    return nullptr;
}

std::unique_ptr<ASTNode> Parser::parseDef() {
    auto tok = currToken();
    if (tok.tag != IDENT) {
        return genError();
    }
    std::vector<std::string> names;
    names.push_back(tok.ident);

    while (true) {
        tok = nextToken();
        if (tok.tag == NL) {
            return std::make_unique<DefNode>(names);
        } else if (tok.tag != COMMA) {
            return genError();
        }
        tok = nextToken();
        if (tok.tag != IDENT) {
            return genError();
        }
        names.push_back(tok.ident);
    }
    return std::make_unique<DefNode>(names);
}

std::unique_ptr<ASTNode> Parser::parseAsg() {
    auto tok = currToken();
    std::string name = tok.ident;
    if (nextToken().tag != ASG) {
        return nullptr;
    }
    nextToken();
    auto val = parseE();
    if (!val) return nullptr;
    return std::make_unique<AsgNode>(name, std::move(val));
}



std::unique_ptr<ASTNode> Parser::parseIf() {
    auto lhs = parseE();
    if (!lhs) return nullptr;
    auto tok = currToken();
    ASTCmpOp cmp = mapToAstCmp(tok.tag);
    if (cmp == AST_CMP_INVALID) return nullptr;
    nextToken();
    auto rhs = parseE();
    if (!rhs) return nullptr;
    auto body = parseBlock();
    if (!body) return nullptr;
    auto cond = std::make_unique<CondNode>(std::move(lhs), std::move(rhs), cmp);
    return std::make_unique<IfNode>(std::move(cond), std::move(body));

//    tok = currToken();
//    if (tok == KEYWORD_ELSE) {
//
//    }
}

std::unique_ptr<ASTNode> Parser::parseE() {
    auto tok = currToken();
    auto lhs = parseT();
    if (!lhs) return nullptr;
    auto rhs = parseE2();
    if (rhs != nullptr) {
        rhs->lhs = std::move(lhs);
        return rhs;
    }
    return lhs;
}

std::unique_ptr<BinOpNode> Parser::parseE2() {
    auto tok = currToken();
    if (tok.tag == PLUS || tok.tag == MINUS) {
        nextToken();
        ASTArithOp op = tok.tag == PLUS ? AST_PLUS : AST_MINUS;
        auto rhs1 = parseT();
        if (!rhs1) return nullptr;
        auto rhs2 = parseE2();
        if (rhs2 != nullptr) {
             rhs2->lhs = std::move(rhs1);
             return std::make_unique<BinOpNode>(nullptr, std::move(rhs2), op);
        }
        return std::make_unique<BinOpNode>(nullptr, std::move(rhs1), op);
    }
    return nullptr;
}

std::unique_ptr<ASTNode> Parser::parseT() {
    auto lhs = parseF();
    if (!lhs) return nullptr;
    auto rhs = parseT2();
    if (rhs != nullptr) {
        rhs->lhs = std::move(lhs);
        return rhs;
    }
    return lhs;
}

std::unique_ptr<BinOpNode> Parser::parseT2() {
    auto tok = currToken();
    if (tok.tag == MULTIPLY || tok.tag == DIVIDE) {
        nextToken();
        ASTArithOp op = tok.tag == MULTIPLY ? AST_MUL : AST_DIV;
        auto rhs1 = parseF();
        if (!rhs1) return nullptr;
        auto rhs2 = parseT2();
        if (rhs2 != nullptr) {
            rhs2->lhs = std::move(rhs1);
            return std::make_unique<BinOpNode>(nullptr, std::move(rhs2), op);
        }
        return std::make_unique<BinOpNode>(nullptr, std::move(rhs1), op);
    }
    return nullptr;
}

std::unique_ptr<ASTNode> Parser::parseF() {
    auto tok = currToken();
    std::unique_ptr<ASTNode> factor;
    std::unique_ptr<ASTNode> f;
    if (tok.tag == MINUS) {
        tok = nextToken();
        factor = std::make_unique<NumberNode>(-1);
    }
    if (tok.tag == NUMBER) {
        nextToken();
        f = std::make_unique<NumberNode>(tok.num);
    }
    if (tok.tag == IDENT) {
        nextToken();
        f = std::make_unique<VarNode>(tok.ident);
    }
    if (tok.tag == L_ROUND_BR) {
        nextToken();
        auto val = parseE();
        tok = currToken();
        if (tok.tag != R_ROUND_BR) {
            return genError();
        }
        nextToken();
        f = std::move(val);
    }

    if (!f) return genError("err: failed to parse F");

    if (factor) {
        return std::make_unique<BinOpNode>(std::move(factor), std::move(f), AST_MUL);
    }

    return f;
}
Token Parser::nextToken() {
    i++;
    return currToken();
}

Token Parser::currToken() {
    if (i < tokens.size()) {
        return tokens[i];
    }
    return tokens[tokens.size()-1];
}

Token Parser::peekToken() {
    return tokens[i+1];
}

std::unique_ptr<ASTNode> Parser::genError(const std::string& err) {
    auto tok = tokens[i];
    printf("%s: unexpected token %d at (%d, %d)\n", err.data(), tok.tag, tok.frag.ys, tok.frag.xs);
    return nullptr;
}

void Parser::skipNewlines() {
    while (currToken().tag == NL) {
        nextToken();
    }
}

ASTCmpOp Parser::mapToAstCmp(DomainTag tag) {
    switch (tag) {
        case LT: return AST_LT;
        case GT: return AST_GT;
        case EQ: return AST_EQ;
        case NE: return AST_NE;
    }
    return AST_CMP_INVALID;
}
